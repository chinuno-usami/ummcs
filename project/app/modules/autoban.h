#pragma once
#include <map>
#include <regex>
#include <vector>
#include <string>
#include <string_view>
#include "module.h"

namespace ummcs{
    class Autoban : public ModuleInterface
    {
        struct ReType {
            std::regex re;
            std::string str;
        };
    public:
        Autoban() = delete;
        Autoban(Storage& storage):ModuleInterface(storage) {}
        ~Autoban() override {}
        void run(mirai::Session&) override;
        void stop() override {}
        void process(const mirai::Event &e, mirai::Session &sess) override;

    private:
        std::string add_rule(const std::string& gid, const std::string_view rule);
        std::string remove_rule(const std::string& gid, const std::string_view rule);
        std::string list_rule(const std::string& gid);
        std::map<std::string, std::vector<ReType>> rules_;
    };
}
