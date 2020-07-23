#include <sstream>
#include "core.h"
#include "../configure.h"
#include "log.h"

using namespace std;
using namespace ummcs;
using namespace mirai;

std::string CoreModule::list_modules(){
    std::stringstream ss;
    ss << u8"模块信息:" <<endl;
    auto& config = Config::get_instance();
    for(const auto& module:config.modules_config){
        ss << u8" ID:" << module.first <<endl;
        ss << u8" 模块名:" << module.second->name <<endl;
        ss << u8" 已启用:" << (module.second->enable?u8"是":u8"否") <<endl <<endl;
    }
    return ss.str();
}
std::string CoreModule::about(){
    auto& core_config = config_->data;
    return toml::find<std::string>(core_config, "about");
}
std::string CoreModule::clean_module_data(int64_t uid, std::string_view name){
    std::string module_name(name.begin(), name.end());
    if(!check_privilege(uid)){
        return u8"只允许管理员清除模块资料";
    }
    if(storage_.drop_cf(module_name)){
        return module_name+u8"模块的资料已经清掉啦";
    }else {
        return module_name+u8"模块的资料清除有点问题";
    }
}
void CoreModule::process(const mirai::Event& e, mirai::Session& sess) {
    namespace ut = mirai::utils;
    Config& config = config.get_instance();
    if (e.type() == mirai::EventType::group_message) {
        const auto &ev = e.get<mirai::GroupMessage>();
        const auto& msg = ev.message.content;
        if (const auto opt = msg.match_types<msg::At, msg::Plain>())
        {
            const auto [at, plain] = *opt;
            if (at.target != config.bot){ return; }
            auto plain_view = mirai::utils::trim_whitespace(plain.view());
            auto msg = process_cmd(ev.sender.id, plain_view);
            if(msg.empty()){ return; }
            sess.send_quote_message(ev, msg);
            return;
        }
    }
    if (e.type() == mirai::EventType::friend_message) {
        const auto &ev = e.get<mirai::FriendMessage>();
        const auto& msg = ev.message.content;
        if (const auto opt = msg.match_types<msg::Plain>())
        {
            const auto [plain] = *opt;
            auto plain_view = mirai::utils::trim_whitespace(plain.view());
            auto msg = process_cmd(ev.sender.id, plain_view);
            if(msg.empty()){ return; }
            sess.send_message(ev.sender.id, msg);
            return;
        }
    }
    return;
}

mirai::Message CoreModule::process_cmd(int64_t uid, std::string_view text){
    namespace ut = mirai::utils;
    static constexpr std::string_view list_modules_view = u8"list_modules";
    static constexpr std::string_view about_view = u8"about";
    static constexpr std::string_view clean_data_view = u8"clean_data ";
    if (ut::starts_with(text, list_modules_view)){
        LOG_DEBUG("core list_modules");
        return list_modules();
    }
    else if (ut::starts_with(text, about_view)){
        LOG_DEBUG("core about");
        return about();
    }
    else if (ut::starts_with(text, clean_data_view)){
        LOG_DEBUG("core clean_module_data:%s", text.data());
        return clean_module_data(uid, ut::remove_prefix(text, clean_data_view.size()));
    }
    return mirai::Message();
}

