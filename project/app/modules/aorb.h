#pragma once
#include <string>
#include <string_view>
#include "module.h"

namespace ummcs{
    class AOrB : public ModuleInterface
    {
    public:
        AOrB() = delete;
        AOrB(Storage& storage):ModuleInterface(storage) {};
        ~AOrB() override {}
        void run(mirai::Session&) override {}
        void stop() override {}
        void process(const mirai::Event &e, mirai::Session &sess) override;

    private:
        uint8_t aorb();
        mirai::Message get_result(const std::string& input);
        mirai::Message process_message_friend(const mirai::Message& msg);
        mirai::Message process_message_group(const mirai::Message& msg);
    };
}
