#pragma once
#include <string>
#include <string_view>
#include "module.h"

namespace ummcs{
    class Nbnhhsh : public ModuleInterface
    {
    public:
        Nbnhhsh() = delete;
        Nbnhhsh(Storage& storage):ModuleInterface(storage) {};
        ~Nbnhhsh() override {}
        void run(mirai::Session&) override {}
        void stop() override {}
        void process(const mirai::Event &e, mirai::Session &sess) override;

    private:
        std::string get_result(const std::string_view text);
        mirai::Message process_message_friend(const mirai::Message& msg);
        mirai::Message process_message_group(const mirai::Message& msg);
    };
}