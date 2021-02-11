#include <sstream>
#include <random>
#include <regex>

#include "aorb.h"
using namespace ummcs;
using namespace mirai;

void AOrB::process(const mirai::Event& e, mirai::Session& sess) {
    static constexpr std::string_view deny = u8"dbq。你只能继续选择困难";
    if (e.type() == mirai::EventType::group_message) {
        const auto &ev = e.get<mirai::GroupMessage>();

        auto msg = process_message_group(ev.message.content);
        if(msg.empty()){ return; }
        bool enable = check_user_enable(ev.sender.id) || check_group_enable(ev.sender.group.id);
        if (!enable){
            sess.send_quote_message(ev, deny);
            return;
        }
        sess.send_quote_message(ev, msg);
        return;
    }
    if (e.type() == mirai::EventType::friend_message) {
        const auto &ev = e.get<mirai::FriendMessage>();

        auto msg = process_message_friend(ev.message.content);
        if(msg.empty()){ return; }
        bool enable = check_user_enable(ev.sender.id);
        if (!enable){
            sess.send_message(ev.sender.id, deny);
            return;
        }
        sess.send_message(ev.sender.id, msg);
        return;
    }
}

mirai::Message AOrB::process_message_group(const mirai::Message& msg){
    namespace ut = mirai::utils;
    Config& config = config.get_instance();
    if (const auto opt = msg.match_types<msg::At, msg::Plain>())
    {
        const auto [at, plain] = *opt;
        if (at.target != config.bot){
            return mirai::Message();
        }
        const std::string str = plain.stringify();
        return get_result(str);
    }
    return mirai::Message();
}

mirai::Message AOrB::process_message_friend(const mirai::Message& msg){
    namespace ut = mirai::utils;
    if (const auto opt = msg.match_types<msg::Plain>())
    {
        const auto [plain] = *opt;
        const std::string str = plain.stringify();
        return get_result(str);
    }
    return mirai::Message();
}

uint8_t AOrB::aorb(){
    std::random_device rd;
    std::mt19937::result_type seed = rd() ^ (
            (std::mt19937::result_type)
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
                ).count() +
            (std::mt19937::result_type)
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
                ).count() );

    static std::mt19937 gen(seed);
    return gen()%2;
}

mirai::Message AOrB::get_result(const std::string& input){
    static const std::regex re(u8R"RE(选(.+)还是(.+))RE");
    std::smatch match;
    if (std::regex_search(input, match, re)){
        const auto [a,b] = mirai::utils::parse_captures<void, std::string, std::string>(match);
        switch(aorb()){
            case 0:
                return a;
            case 1:
                return b;
        }
    }
    return mirai::Message();
}
