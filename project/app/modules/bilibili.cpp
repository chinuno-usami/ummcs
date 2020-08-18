#include <regex>
#include <sstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "bilibili.h"
#include "../log.h"
using namespace ummcs;
using namespace mirai;

void Bilibili::process(const mirai::Event& e, mirai::Session& sess) {
    namespace ut = mirai::utils;
    Config& config = config.get_instance();

    if (e.type() == mirai::EventType::group_message) {
        const auto &ev = e.get<mirai::GroupMessage>();
        // 检查是否启用
        if(!check_group_enable(ev.sender.group.id)){
            return;
        }
        const std::string str = ev.message.content.stringify();
        Bilibili::VideoInfo info;
        static const std::regex re_app(u8R"RE(https:\\?\\?/\\?\\?/b23.tv\\?\\?/([0-9a-zA-Z]+))RE");
        static const std::regex re_aid(u8R"RE(https://www.bilibili.com/video/av(\d+))RE");
        static const std::regex re_bid(u8R"RE(https://www.bilibili.com/video/(BV[0-9a-zA-Z]+))RE");
        std::smatch match;
        if (std::regex_search(str, match, re_app)){
            // 检查是否小程序
            mirai::msg::Image meme;
            meme.url = "https://i.loli.net/2020/04/27/HegAkGhcr6lbPXv.png";
            sess.send_message(ev.sender.group.id, mirai::Message(meme));
            //sess.send_quote_message(ev, mirai::Message(meme));
            const auto [path] = mirai::utils::parse_captures<void, std::string>(match);
            info = get_info_app(path);
        } else if (std::regex_search(str, match, re_aid)) {
            const auto [id] = mirai::utils::parse_captures<void, std::string>(match);
            info = get_info(Bilibili::IdType::AID, id);
        } else if (std::regex_search(str, match, re_bid)) {
            const auto [id] = mirai::utils::parse_captures<void, std::string>(match);
            info = get_info(Bilibili::IdType::BVID, id);
        } else {
            return;
        }

        mirai::Message ret;
        mirai::msg::Image cover;
        cover.url = info.pic;
        ret += cover;
        std::stringstream ss;
        ss << u8"\nav" << info.aid << std::endl;
        ss << u8"标题:" << info.title << std::endl;
        ss << u8"UP:" << info.up << std::endl;
        ss << u8"简介:" << info.desc << std::endl;
        ss << u8"链接:" << info.link;
        ret += ss.str();
        sess.send_message(ev.sender.group.id, ret);
    }
}

Bilibili::VideoInfo Bilibili::get_info_app(const std::string& path){
    static const std::string prefix("https://b23.tv/");
    std::string short_url = prefix+path;
    cpr::Url url = cpr::Url{ short_url };
    LOG_DEBUG("url:%s", short_url.c_str());
    cpr::Session session;
    session.SetOption(url);
    session.SetRedirect(false);
    // 奇奇怪怪的总是有几率获取不到真实地址
    // 加了个重试，再不行我也没办法了
    std::string redirect;
    for(size_t retry=0; retry<3; ++retry){
        cpr::Response r = session.Get();
        redirect = r.header["Location"];
        if(!redirect.empty()){ break; }
    }
    std::string_view redirectv(redirect);
    auto paramloc = redirectv.find('?');
    std::string_view redirect_noparamv = redirectv.substr(0, paramloc);
    auto bvloc = redirect_noparamv.rfind('/');
    std::string_view bv = redirect_noparamv.substr(bvloc+1, paramloc-bvloc);

    return get_info(Bilibili::IdType::BVID, std::string(bv));
}

Bilibili::VideoInfo Bilibili::get_info(Bilibili::IdType type, const std::string& id){
    using json = nlohmann::json;
    LOG_DEBUG("id:%s", id.c_str());
    std::string key = type==Bilibili::IdType::AID?"aid":"bvid";
    cpr::Response r = cpr::Get(cpr::Url{"https://api.bilibili.com/x/web-interface/view"}, cpr::Parameters{{key, id}});
    auto j = json::parse(r.text);
    auto jdata = j["data"];
    LOG_DEBUG("info:%s",r.text.c_str());
    Bilibili::VideoInfo info;
    info.pic = jdata["pic"].get<std::string>();
    info.title = jdata["title"].get<std::string>();
    info.desc = jdata["desc"].get<std::string>();
    info.aid = jdata["aid"].get<unsigned int>();
    info.up = jdata["owner"]["name"].get<std::string>();
    info.link = "https://www.bilibili.com/video/";
    switch(type){
        case Bilibili::IdType::AID:
            info.link.append("av");
            info.link.append(id);
            break;
        case Bilibili::IdType::BVID:
            info.link.append(id);
            break;
        default:
            info.link = u8"获取链接失败";
            break;
    }
    return info;
}
