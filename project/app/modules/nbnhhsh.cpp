#include <sstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "nbnhhsh.h"
using namespace ummcs;
using namespace mirai;

void Nbnhhsh::process(const mirai::Event& e, mirai::Session& sess) {
    static constexpr std::string_view deny = u8"dbq。你没有权限可以hhsh";
    if (e.type() == mirai::EventType::group_message) {
        const auto &ev = e.get<mirai::GroupMessage>();

        bool enable = check_user_enable(ev.sender.id) || check_group_enable(ev.sender.group.id);
        if (!enable){
            sess.send_quote_message(ev, deny);
            return;
        }
        auto msg = process_message_group(ev.message.content);
        if(msg.empty()){ return; }
        sess.send_quote_message(ev, msg);
        return;
    }
    if (e.type() == mirai::EventType::friend_message) {
        const auto &ev = e.get<mirai::FriendMessage>();

        bool enable = check_user_enable(ev.sender.id);
        if (!enable){
            sess.send_message(ev.sender.id, deny);
            return;
        }
        auto msg = process_message_friend(ev.message.content);
        if(msg.empty()){ return; }
        sess.send_message(ev.sender.id, msg);
        return;
    }
}

mirai::Message Nbnhhsh::process_message_group(const mirai::Message& msg){
    namespace ut = mirai::utils;
    Config& config = config.get_instance();
    if (const auto opt = msg.match_types<msg::At, msg::Plain>())
    {
        const auto [at, plain] = *opt;
        if (at.target != config.bot){
            return mirai::Message();
        }
        auto plain_view = mirai::utils::trim_whitespace(plain.view());
        static constexpr std::string_view prefix = u8"nbnhhsh ";
        if (!ut::starts_with(plain_view, prefix)){
            return mirai::Message();
        }
        std::string ret = get_result(ut::remove_prefix(plain_view, prefix.size()));
        return ret;
    }
    return mirai::Message();
}

mirai::Message Nbnhhsh::process_message_friend(const mirai::Message& msg){
    namespace ut = mirai::utils;
    Config& config = config.get_instance();
    if (const auto opt = msg.match_types<msg::Plain>())
    {
        const auto [plain] = *opt;
        auto plain_view = mirai::utils::trim_whitespace(plain.view());
        static constexpr std::string_view prefix = u8"nbnhhsh ";
        if (!ut::starts_with(plain_view, prefix)){
            return mirai::Message();
        }
        std::string ret = get_result(ut::remove_prefix(plain_view, prefix.size()));
        return ret;
    }
    return mirai::Message();
}

std::string Nbnhhsh::get_result(const std::string_view text){
    using json = nlohmann::json;
    std::stringstream req_stream;
    req_stream << R"({"text":")" << text << R"("})";
    cpr::Response r = cpr::Post(cpr::Url{"https://lab.magiconch.com/api/nbnhhsh/guess"},
                                cpr::Body{req_stream.str()},
                                cpr::Header{
                                    {"Content-Type", "application/json"},
                                    {"authority", "lab.magiconch.com"},
                                    {"user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"},
                                    {"origin", "https://lab.magiconch.com"},
                                    {"referer", "https://lab.magiconch.com/nbnhhsh/"},
                                });
    if (r.status_code != 200)
    {
        std::string ret(u8"无法与卜卜口侧达成共识：");
        ret.append(std::to_string(r.status_code));
        return ret;
    }
    auto results = json::parse(r.text);
    std::stringstream ss;
    for (auto &element : results)
    {
        ss << element["name"].get<std::string>() << ": ";
        for (auto &trans : element["trans"])
        {
            ss << trans.get<std::string>() << " ";
        }
        ss << std::endl;
    }
    return ss.str();
}
