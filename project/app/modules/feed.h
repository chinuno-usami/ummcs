#pragma once
#include <thread>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#include "../rss.h"
#include "module.h"

namespace ummcs{
    class FeedModule : public ModuleInterface
    {
    public:
        FeedModule() = delete;
        FeedModule(Storage& storage):ModuleInterface(storage) {};
        ~FeedModule() override { stop(); };
        void run(mirai::Session& sess) override;
        void stop() override;
        void process(const mirai::Event&, mirai::Session&) override {}

    private:
        std::map<std::string ,std::vector<int64_t>> item_enable_groups;
        std::map<std::string ,std::vector<int64_t>> item_enable_users;
        std::thread th_;
        Rss rss_;
    };
}
