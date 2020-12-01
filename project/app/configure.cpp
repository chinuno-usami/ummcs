#include <filesystem>
#include "configure.h"

using namespace ummcs;

Config& Config::get_instance()
{
    static Config config;
    return config;
}

void Config::parse(const char* path){
    data = toml::parse(std::filesystem::path(path));
    bot = toml::find<int64_t>(data, "bot");
    auth_key = toml::find<std::string>(data, "auth_key");
    toml::array& admins_arr = toml::find<toml::array>(data, "admins");
    for(const auto& val:admins_arr){
        admins.emplace(val.as_integer());
    }

    const auto& modules = toml::find<toml::table>(data, "modules");
    for(const auto& val: modules){
        auto conf = std::make_shared<ModuleConfig>();
        conf->id = val.first;
        conf->data = val.second;
        conf->enable = toml::find<bool>(val.second, "enable");
        conf->name = toml::find<std::string>(val.second, "name");
        std::vector<int64_t> vec_admins = toml::find<std::vector<int64_t>>(val.second, "admins");
        std::vector<int64_t> vec_users_enable = toml::find<std::vector<int64_t>>(val.second, "users_enable");
        std::vector<int64_t> vec_groups_enable = toml::find<std::vector<int64_t>>(val.second, "groups_enable");
        for (auto item : vec_admins) {
            conf->admins.emplace(item);
        }
        for (auto item : vec_users_enable) {
            conf->users_enable.emplace(item);
        }
        for (auto item : vec_groups_enable) {
            conf->groups_enable.emplace(item);
        }

        modules_config.insert({val.first, conf});
    }
}
