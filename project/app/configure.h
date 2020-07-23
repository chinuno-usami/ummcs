#pragma once
#include <set>
#include <map>
#include <list>
#include <string>
#include <memory>
#include <toml.hpp>
namespace ummcs
{
    struct ModuleConfig{
        bool enable;
        std::string id;
        std::string name;
        std::set<int64_t> groups_enable;
        std::set<int64_t> users_enable;
        std::set<int64_t> admins;  // 模块管理员
        toml::value data; // 原始配置，模块独有配置从这里取
    };
    struct Config
    {
        static Config& get_instance();
        void parse(const char* path);

        int64_t bot;
        std::string auth_key;
        std::set<int64_t> admins;  // 全局管理员，最高权限
        std::map<std::string, std::shared_ptr<ModuleConfig>> modules_config; // 每个模块的配置
        toml::value data;
    };

} // namespace ummcs
