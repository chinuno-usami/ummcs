#pragma once
#include <string>
#include <string_view>
#include "module.h"

namespace ummcs {
    class CoreModule: public ModuleInterface {
        public:
            CoreModule() = delete;
            CoreModule(Storage& storage):ModuleInterface(storage) {};
            ~CoreModule() override {}
            void run(mirai::Session&) override {}
            void stop() override {}
            void process(const mirai::Event &e, mirai::Session &sess) override;
        private:
            std::string list_modules();
            std::string about();
            std::string clean_module_data(int64_t uid, std::string_view name);
            mirai::Message process_cmd(int64_t uid, std::string_view text);
    };
}
