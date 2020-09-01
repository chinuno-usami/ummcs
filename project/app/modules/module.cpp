#include "module.h"
#include "../configure.h"
#include "core.h"
#include "nbnhhsh.h"
#include "feed.h"
#include "autoban.h"
#include "bilibili.h"
#include "saucenao.h"
#include "setu.h"
#include "log.h"
using namespace ummcs;

void ModuleManager::regist_modules(){
    module_factories_.insert({"nbnhhsh", [this] { return std::make_shared<Nbnhhsh>(storage_); }});
    module_factories_.insert({"feed", [this] { return std::make_shared<FeedModule>(storage_); }});
    module_factories_.insert({"core", [this] { return std::make_shared<CoreModule>(storage_); }});
    module_factories_.insert({"autoban", [this] { return std::make_shared<Autoban>(storage_); }});
    module_factories_.insert({"bilibili", [this] { return std::make_shared<Bilibili>(storage_); }});
    module_factories_.insert({"saucenao", [this] { return std::make_shared<Saucenao>(storage_); }});
    module_factories_.insert({"setu", [this] { return std::make_shared<Setu>(storage_); }});
}

void ModuleManager::load_modules(){
    auto& config = Config::get_instance();
    for (auto &conf : config.modules_config) {
        if (conf.second->enable){
            auto it = module_factories_.find(conf.first);
            if(it == module_factories_.end()){
                LOG_WARN("module %s not exist", conf.first.c_str());
                continue;
            }
            auto handle = it->second();
            handle->set_config(conf.second);
            module_handles_.insert({conf.first, handle});
        }
    }
}

void ModuleManager::run_modules(mirai::Session &sess){
    for (auto handle : module_handles_) {
        handle.second->run(sess);
    }
}

void ModuleManager::stop_modules(){
    for (auto handle : module_handles_) {
        handle.second->stop();
    }
}
void ModuleManager::process_entry(const mirai::Event &e, mirai::Session &sess){
    for (auto handle : module_handles_) {
        handle.second->process(e, sess);
    }
}

bool ModuleInterface::check_privilege(int64_t uid){
    Config& config = Config::get_instance();
    // 检查是否全局管理员
    if (config.admins.count(uid) != 0){
        return true;
    }
    // 检查是否插件管理员
    if (!config_) { return false; }
    if (config_->admins.count(uid) != 0){
        return true;
    }
    return false;
}

bool ModuleInterface::check_user_enable(int64_t uid){
    if (!config_) { return false; }
    if (config_->users_enable.count(uid) != 0){
        return true;
    }
    return false;
}

bool ModuleInterface::check_group_enable(int64_t gid){
    if (!config_) { return false; }
    if (config_->groups_enable.count(gid) != 0){
        return true;
    }
    return false;
}
