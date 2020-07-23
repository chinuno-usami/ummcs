#include "rss_parser.h"
#include <iostream>
#include <string_view>
#include <sstream>
#include "log.h"

using namespace ummcs;
using namespace std::literals;

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
            mirai::msg::Image img;
            img.url = attr->value;
            msg += img;
            //LOG_DEBUG("(img:%s)", attr->value);
        }
    }
}
