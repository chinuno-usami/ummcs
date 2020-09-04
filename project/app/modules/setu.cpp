#include <sstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "setu.h"
#include "log.h"
using namespace ummcs;
using namespace mirai;
using namespace std::literals;

// 初始化加载配置信息
void Setu::run(mirai::Session&){
    auto& config = config_->data;
    const auto& groups = toml::get<std::vector<toml::value>>(config.at("groups"));
    const auto& users = toml::get<std::vector<toml::value>>(config.at("users"));
    for(const auto& group:groups){
        uint64_t number = toml::find<uint64_t>(group, "number");
        int r18 = toml::find_or<int64_t>(group, "r18", 0);
        if(r18<0||r18>3){ r18 = 0; }
        groups_info_.insert({number, r18});
        LOG_DEBUG("加载色图配置 群%lu:%d", number, r18);
    }
    for(const auto& user:users){
        uint64_t number = toml::find<uint64_t>(user, "number");
        int r18 = toml::find_or<int64_t>(user, "r18", 0);
        if(r18<0||r18>3){ r18 = 0; }
        users_info_.insert({number, r18});
        LOG_DEBUG("加载色图配置 用户%lu:%d", number, r18);
    }
    apikey_ = toml::find_or<std::string>(config, "apikey", "");
}

void Setu::process(const mirai::Event& e, mirai::Session& sess) {
    static constexpr std::string_view deny = u8"好好学习少看色图";
    if (e.type() == mirai::EventType::group_message) {
        const auto &ev = e.get<mirai::GroupMessage>();

        if(!check_group_enable(ev.sender.group.id)){
            return;
        }
        bool enable = false;
        auto it = groups_info_.find(ev.sender.group.id);
        if(it != groups_info_.end()){
            enable = true;
        } else {
            enable = false;
        }
        if (!enable){
            sess.send_quote_message(ev, deny);
            return;
        }
        auto msg = process_message_group(it->second, ev.message.content);
        if(msg.empty()){ return; }
        sess.send_quote_message(ev, msg);
        return;
    }
    if (e.type() == mirai::EventType::friend_message) {
        const auto &ev = e.get<mirai::FriendMessage>();

        if(!check_group_enable(ev.sender.id)){
            return;
        }
        auto it = users_info_.find(ev.sender.id);
        bool enable = false;
        if(it != users_info_.end()){
            enable = true;
        } else {
            enable = false;
        }
        if (!enable){
            sess.send_message(ev.sender.id, deny);
            return;
        }
        auto msg = process_message_friend(it->second, ev.message.content);
        if(msg.empty()){ return; }
        sess.send_message(ev.sender.id, msg);
        return;
    }
}

mirai::Message Setu::process_message_group(int r18, const mirai::Message& msg){
    namespace ut = mirai::utils;
    Config& config = config.get_instance();
    if (const auto opt = msg.match_types<msg::At, msg::Plain>())
    {
        const auto [at, plain] = *opt;
        if (at.target != config.bot){
            return mirai::Message();
        }
        auto plain_view = mirai::utils::trim_whitespace(plain.view());
        if(plain_view == u8"图"){
            return get_result_nose();
        }
        if(plain_view == u8"色图"){
            return get_result(r18, ""sv);
        }
        static constexpr std::string_view suffix = u8"色图";
        if (!ut::ends_with(plain_view, suffix)){
            return mirai::Message();
        }
        auto ret = get_result(r18, ut::remove_suffix(plain_view, suffix.size()));
        return ret;
    }
    return mirai::Message();
}

mirai::Message Setu::process_message_friend(int r18, const mirai::Message& msg){
    namespace ut = mirai::utils;
    Config& config = config.get_instance();
    if (const auto opt = msg.match_types<msg::Plain>())
    {
        const auto [plain] = *opt;
        auto plain_view = mirai::utils::trim_whitespace(plain.view());
        if(plain_view == u8"图"){
            return get_result_nose();
        }
        if(plain_view == u8"色图"){
            return get_result(r18, ""sv);
        }
        static constexpr std::string_view suffix = u8"色图";
        if (!ut::ends_with(plain_view, suffix)){
            return mirai::Message();
        }
        auto ret = get_result(r18, ut::remove_suffix(plain_view, suffix.size()));
        return ret;
    }
    return mirai::Message();
}

mirai::Message Setu::get_result(int r18, const std::string_view keyword){
    using json = nlohmann::json;
    cpr::Session sess;
    sess.SetUrl(cpr::Url{"https://api.lolicon.app/setu/"});
    cpr::Parameters params{{"r18",std::to_string(r18)}};

    if(!keyword.empty()){
        params.AddParameter({"keyword", keyword});
        std::string keyw(keyword.data(), keyword.size());
        LOG_DEBUG("色图关键字:%s", keyw.c_str());
    }
    if(!apikey_.empty()){
        params.AddParameter({"apikey", apikey_});
    }
    sess.SetParameters(params);
    cpr::Response r = sess.Get();
    if (r.status_code != 200)
    {
        std::string ret(u8"色图api好像挂了:");
        ret.append(std::to_string(r.status_code));
        return ret;
    }
    auto results = json::parse(r.text);
    LOG_DEBUG("%s",r.text.c_str());
    auto code = results["code"].get<int>();
    if(code != 0){
        return results["msg"].get<std::string>();
    }
    mirai::Message msg;
    auto setu  = results["data"][0];
    for(auto& tag:setu["tags"].get<std::vector<std::string>>()){
        LOG_DEBUG("tag:%s", tag.c_str());
    }
    mirai::msg::Image pic;
    pic.url = setu["url"].get<std::string>();
    msg += pic;
    msg += u8"\n标题:";
    msg += setu["title"].get<std::string>();
    msg += u8"\nPixiv ID:";
    msg += std::to_string(setu["pid"].get<int>());
    msg += u8"\n作者:";
    msg += setu["author"].get<std::string>();
    msg += u8"\n作者ID:";
    msg += std::to_string(setu["uid"].get<int>());
    return msg;
}

mirai::Message Setu::get_result_nose(){
    using json = nlohmann::json;
    cpr::Response r = cpr::Get(cpr::Url{"http://www.dmoe.cc/random.php?return=json"});
    if (r.status_code != 200)
    {
        std::string ret(u8"随机图api好像挂了:");
        ret.append(std::to_string(r.status_code));
        return ret;
    }
    auto results = json::parse(r.text);
    LOG_DEBUG("%s",r.text.c_str());
    // 傻逼PHP！好好给数字不行吗？会死吗？为什么所有PHP后端给的类型都要是字符串！
    auto code = std::stoi(results["code"].get<std::string>());
    if(code != 200){
        std::string ret(u8"随机图api好像挂了:");
        ret.append(std::to_string(r.status_code));
        return ret;
    }
    mirai::msg::Image pic;
    pic.url = results["imgurl"].get<std::string>();
    return mirai::Message(pic);
}
