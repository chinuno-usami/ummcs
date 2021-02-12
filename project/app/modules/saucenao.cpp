#include <ctime>
#include <sstream>
#include <string_view>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "saucenao.h"
using namespace ummcs;
using namespace mirai;

void Saucenao::process(const mirai::Event& e, mirai::Session& sess) {
    if (e.type() == mirai::EventType::group_message) {
        const auto &ev = e.get<mirai::GroupMessage>();

        bool enable = check_user_enable(ev.sender.id) || check_group_enable(ev.sender.group.id);
        if (!enable){
            return;
        }
        auto msg = process_message(ev.message.content);
        if(msg.empty()){ return; }
        sess.send_quote_message(ev, msg);
        return;
    }
    if (e.type() == mirai::EventType::friend_message) {
        const auto &ev = e.get<mirai::FriendMessage>();

        bool enable = check_user_enable(ev.sender.id);
        if (!enable){
            return;
        }
        auto msg = process_message(ev.message.content);
        if(msg.empty()){ return; }
        sess.send_message(ev.sender.id, msg);
        return;
    }
}

mirai::Message Saucenao::process_message(const mirai::Message& msg){
    namespace ut = mirai::utils;
    Config& config = config.get_instance();
    if (const auto opt = msg.match_types<msg::Plain, msg::Image>())
    {
        const auto [plain, img] = *opt;
        auto plain_view = mirai::utils::trim_whitespace(plain.view());
        if(plain_view == u8"搜图"){
            if(img.url){
                return get_result(*img.url);
            }
        } else {
            return mirai::Message();
        }
    }
    return mirai::Message();
}


mirai::Message Saucenao::get_result(const std::string& text){
    using json = nlohmann::json;
    const auto& module_config = config_->data;
    auto api_key = toml::find<std::string>(module_config, "api_key");
    cpr::Response r = cpr::Get(cpr::Url{"https://saucenao.com/search.php"},
                                cpr::Parameters{{"db", "999"}, {"output_type", "2"},
                                                {"testmode", "1"}, {"url", text},
                                                {"api_key", api_key}});
    if (r.status_code != 200)
    {
        std::string ret(u8"好像有点问题。。等下再试试看:");
        ret.append(std::to_string(r.status_code));
        return ret;
    }
    auto results = json::parse(r.text);
    if(int status = results["header"]["status"].get<int>() != 0){
        if(status == -2){
            return u8"请求太过频繁了，喝杯红茶休息一下8";
        } else {
            std::string ret(u8"好像有点问题。。等下再试试看:");
            ret.append(std::to_string(status));
            return ret;
        }
    }
    mirai::Message msg;
    for (auto &element : results["results"])
    {
        auto header = element["header"];
        std::string sim = header["similarity"].get<std::string>();
        if(std::stof(sim) < 70.0f) { continue;; }
        std::string thumb = header["thumbnail"].get<std::string>();
        mirai::msg::Image img;
        img.url = thumb;
        msg += img;
        msg += u8"\n相似度:";
        msg += sim;
        msg += u8"%\n链接:\n";
        auto data = element["data"];
        for (auto &url : data["ext_urls"])
        {
            msg += url.get<std::string>();
            msg += "\n";
        }
    }
    if(msg.empty()){
        return u8"没找到相似的图片";
    }
    return msg;
}
