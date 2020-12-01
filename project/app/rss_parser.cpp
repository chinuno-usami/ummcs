#include "rss_parser.h"
#include <iostream>
#include <string_view>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <cpr/cpr.h>
#include "configure.h"
#include "log.h"

using namespace ummcs;
using namespace std::literals;
namespace fs = std::filesystem;

mirai::Message RssParser::parse_all(const std::string& html){
    mirai::Message msg;
    GumboOutput* output = gumbo_parse_with_options(
          &kGumboDefaultOptions, html.c_str(), html.size());

    parse(output->root, msg);

    gumbo_destroy_output(&kGumboDefaultOptions, output);
    return msg;
}

void RssParser::parse(GumboNode* node, mirai::Message& msg){
    if(!node){ return; }

    if (node->type == GUMBO_NODE_TEXT)
    {
        // 文本行
        //LOG_DEBUG("%s", node->v.text.text);
        msg += node->v.text.text;
        return;
    }
    if (node->type == GUMBO_NODE_WHITESPACE)
    {
        //LOG_DEBUG(" ");
        msg += " ";
    }
    if (node->type != GUMBO_NODE_ELEMENT)
    {
        return;
    }
    // 换行
    if (node->v.element.tag == GUMBO_TAG_BR)
    {
        //LOG_DEBUG("\n");
        msg += "\n";
        goto NEXT;
    }
NEXT:
    // 递归处理子节点
    GumboVector *children = &node->v.element.children;
    for (size_t i = 0; i < children->length; ++i)
    {
        parse(static_cast<GumboNode *>(children->data[i]), msg);
    }

    // 链接
    if (node->v.element.tag == GUMBO_TAG_A)
    {
        GumboAttribute *attr = gumbo_get_attribute(&(node->v.element.attributes), "href");
        if (attr)
        {
            std::string ret("( ");
            ret.append(attr->value);
            ret.append(" )");
            msg += std::string_view(ret);
            //LOG_DEBUG("(%s)", attr->value);
        }
    }
    // 图片
    if (node->v.element.tag == GUMBO_TAG_IMG)
    {
        GumboAttribute *attr = gumbo_get_attribute(&(node->v.element.attributes), "src");
        if (attr)
        {
            // 先保存到本地再上传防止mirai服务卡死
            std::string image_url(attr->value);
            cpr::Response r;
            if(use_proxy_){
                r = cpr::Get(cpr::Url{image_url},cpr::Proxies{{"https", proxy_}, {"http", proxy_} });
            } else {
                r = cpr::Get(cpr::Url{image_url});
            }
            auto pos_beg = image_url.find("//")+2;
            auto pos_end = image_url.rfind("/")+1;
            std::string rel_dir = image_url.substr(pos_beg, pos_end-pos_beg);
            std::string filename = image_url.substr(pos_end);
            if(filename.size() > 255){
                filename = filename.substr(filename.size()-200);
            }
            std::string image_dir(plugin_image_path_);
            image_dir.append("/");
            image_dir.append(rel_dir);
            //LOG_DEBUG("rel_dir:%s,filename:%s,image_dir:%s", rel_dir.c_str(), filename.c_str(), image_dir.c_str());
            fs::create_directories(image_dir);
            std::ofstream ofs(image_dir+"/"+filename);
            ofs.write(r.text.data(), r.text.size());
            mirai::msg::Image img;
            img.path = rel_dir+"/"+filename;
            //img.url = attr->value;
            msg += img;
            //LOG_DEBUG("(img:%s)", attr->value);
        }
    }
}

RssParser::RssParser(){
    auto& config = Config::get_instance();
    auto& feed_config = config.modules_config["feed"]->data;
    plugin_image_path_ = toml::find<std::string>(feed_config, "plugin_image_path");
    proxy_ = toml::find<std::string>(feed_config, "proxy");
    use_proxy_ = toml::find<bool>(feed_config, "use_proxy");
}
