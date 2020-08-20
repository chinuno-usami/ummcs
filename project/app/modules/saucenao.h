#pragma once
#include <string>
#include "module.h"

namespace ummcs{
    class Saucenao : public ModuleInterface
    {
    public:
        Saucenao() = delete;
        Saucenao(Storage& storage):ModuleInterface(storage) {};
        ~Saucenao() override {}
        void run(mirai::Session&) override {}
        void stop() override {}
        void process(const mirai::Event &e, mirai::Session &sess) override;

    private:
        mirai::Message get_result(const std::string& text);
        mirai::Message process_message(const mirai::Message& msg);
    };
}
