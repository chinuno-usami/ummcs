#include <sstream>
#include <charconv>
#include <nlohmann/json.hpp>

#include "autoban.h"
#include "../log.h"
using namespace ummcs;
using namespace mirai;

void Autoban::process(const mirai::Event& e, mirai::Session& sess) {
    static constexpr std::string_view deny = u8"你不配改规则哼哒";
    namespace ut = mirai::utils;
    Config& config = config.get_instance();

    if (e.type() == mirai::EventType::group_message) {
        const auto &ev = e.get<mirai::GroupMessage>();
        // 检查是否启用
        if(!check_group_enable(ev.sender.group.id)){
            return;
        }
        const auto& msg = ev.message.content;
        // 更改规则
        if (const auto opt = msg.match_types<msg::At, msg::Plain>())
        {
            do{
                const auto [at, plain] = *opt;
                if (at.target != config.bot){
                    break;
                }
                auto plain_view = mirai::utils::trim_whitespace(plain.view());
                static constexpr std::string_view prefix = u8"banrule ";
                static constexpr std::string_view prefix_add = u8"add ";
                static constexpr std::string_view prefix_remove = u8"del ";
                static constexpr std::string_view prefix_list = u8"list";
                if (!ut::starts_with(plain_view, prefix)){
                    break;
                }
                // 鉴权
                if(!(check_privilege(ev.sender.id) || ev.sender.permission!=mirai::Permission::member)){
                    sess.send_quote_message(ev, deny);
                    // 可能有伪装成命令的情况，还是要做检查
                    break;
                }
                // 子命令
                auto subcmd = ut::remove_prefix(plain_view, prefix.size());
                if (ut::starts_with(subcmd, prefix_add)){
                    auto ret = add_rule(std::to_string(ev.sender.group.id), ut::remove_prefix(subcmd, prefix_add.size()));
                    sess.send_quote_message(ev, ret);
                    return;
                } else if (ut::starts_with(subcmd, prefix_remove)) {
                    auto ret = remove_rule(std::to_string(ev.sender.group.id), ut::remove_prefix(subcmd, prefix_remove.size()));
                    sess.send_quote_message(ev, ret);
                    return;
                } else if (ut::starts_with(subcmd, prefix_list)) {
                    auto ret = list_rule(std::to_string(ev.sender.group.id));
                    sess.send_quote_message(ev, ret);
                    return;
                }
            } while(0);

        }
        // 规则匹配
        const std::string str = ev.message.content.stringify();
        auto it = rules_.find(std::to_string(ev.sender.group.id));
        if(it == rules_.end()){ return; }
        for(size_t i = 0; i < it->second.size(); ++i){
            std::smatch match;
            if (std::regex_search(str, match, it->second[i].re)) {
                // 匹配
                // 检查bot权限
                if(ev.sender.group.permission == mirai::Permission::member){
                    // bot是普通群员，向群主要权限
                    // 获取群成员列表，找群主
                    auto members = sess.member_list(ev.sender.group.id);
                    for(const auto& member:members){
                        if(member.permission == mirai::Permission::owner){
                            // 发送要权限消息
                            mirai::Message req;
                            req += mirai::msg::At(member.id);
                            req += u8"快给我管理！我要ban人！";
                            sess.send_message(ev.sender.group.id, req);
                            break;
                        }
                    }
                } else if (ev.sender.bot_has_higher_permission()){
                    // 一切正常
                    sess.mute(ev.sender.group.id, ev.sender.id, std::chrono::seconds(300));
                }
                // 发送触发通知
                std::stringstream ss;
                ss << u8"触发了禁言规则 " << i << ":" << it->second[i].str;
                sess.send_quote_message(ev, ss.str());
                return;
            }
        }
    }
}


void Autoban::run(mirai::Session&){
    storage_.create_cf(config_->id);
    // 加载每个群的规则
    auto it = storage_.new_iter(config_->id);
    using json = nlohmann::json;
    LOG_DEBUG("load ban rules:");
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::vector<Autoban::ReType> rules;
        auto jdata = json::parse(it->value().ToString());
        for(auto &rule:jdata){
            Autoban::ReType re;
            std::string text = rule.get<std::string>();
            re.re = text;
            re.str = text;
            rules.emplace_back(re);
        }
        rules_.insert({it->key().ToString(), rules});
        LOG_DEBUG("%s:%s", it->key().ToString().c_str(), it->value().ToString().c_str());
    }
    delete it;
}
std::string Autoban::add_rule(const std::string& gid, const std::string_view textv){
    std::string text(textv);
    auto it = rules_.find(gid);
    Autoban::ReType re;
    re.str = text;
    re.re = text;
    if(it == rules_.end()) {
        // 一条规则都没有。插入节点
        rules_.insert({gid, {{re}}});
    } else {
        rules_[gid].emplace_back(re);
    }
    using json = nlohmann::json;
    json new_rule;
    for(const auto& rule: rules_[gid]){
        new_rule.push_back(rule.str);
    }
    storage_.put_value<std::string>(config_->id, gid, new_rule.dump());
    return u8"规则已添加";
}
std::string Autoban::remove_rule(const std::string& gid, const std::string_view rid){
    auto it = rules_.find(gid);
    if(it == rules_.end()) { return u8"本群没有禁言规则"; }
    size_t id;
    auto result = std::from_chars(rid.data(), rid.data() + rid.size(), id);
    if (result.ec == std::errc::invalid_argument) {
        return u8"删除规则要给规则ID";
    }
    if(id >= it->second.size()){
        return u8"给的规则不存在";
    }
    std::stringstream ss;
    ss << "规则:" << id << " 内容:" << it->second[id].str << "已移除";
    it->second.erase(it->second.begin()+id);
    // 写回存储
    using json = nlohmann::json;
    json new_rule;
    for(const auto& rule: it->second){
        new_rule.push_back(rule.str);
    }
    storage_.put_value<std::string>(config_->id, gid, new_rule.dump());
    return ss.str();
}
std::string Autoban::list_rule(const std::string& gid){
    auto it = rules_.find(gid);
    if(it == rules_.end()){ return u8"本群没有禁言规则"; }
    std::stringstream ss;
    for(size_t i=0; i < it->second.size(); ++i){
        ss << "ID:" <<i << u8" 规则:" << it->second[i].str << std::endl;
    }
    return ss.str();
}
