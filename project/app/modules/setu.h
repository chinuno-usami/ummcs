#pragma once
#include <string>
#include <string_view>
#include "module.h"

namespace ummcs{
    class Setu : public ModuleInterface
    {
    public:
        Setu() = delete;
        Setu(Storage& storage):ModuleInterface(storage) {};
        ~Setu() override {}
        void run(mirai::Session&) override;
        void stop() override {}
        void process(const mirai::Event &e, mirai::Session &sess) override;

    private:
        mirai::Message get_result(int r18, const std::string_view keyword);
        mirai::Message process_message_friend(int r18, const mirai::Message& msg);
        mirai::Message process_message_group(int r18, const mirai::Message& msg);
        // id, r18
        std::map<int64_t, int> groups_info_;
        std::map<int64_t, int> users_info_;
        std::string apikey_;
    };
}
