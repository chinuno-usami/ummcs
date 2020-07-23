#pragma once

#include <time.h>
#include <map>
#include <list>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
#include <memory>
#include <functional>
#include <condition_variable>

#include "feedparser.h"

namespace ummcs {
    using namespace std::chrono_literals;
    struct RssItem {
        void check_update();
        std::string id;
        std::string url;
        std::function<void(FeedInfo*)> callback;
    };

    // (next_check_time, ptr_to_item)
    using RssQItem = std::pair<std::chrono::system_clock::time_point, std::shared_ptr<RssItem>>;

    constexpr bool rss_greater(const RssQItem& lhs, const RssQItem& rhs) {
        return lhs.first > rhs.first;
    }

    class Rss
    {
        public:
            Rss ();
            ~Rss();
            void run();
            void stop();
            bool add_item(std::shared_ptr<RssItem> item);
            std::list<std::shared_ptr<RssItem>> get_items();
            bool remove_item(const std::string& id);

            inline void set_check_interval(std::chrono::seconds interval) {
                interval_ = interval;
            }

        private:
            void main_loop();

            bool running_ = true;
            std::thread th_;
            std::condition_variable cv_;
            std::chrono::seconds interval_ = 60s;
            // (id, ptr_to_item)
            std::map<std::string, std::shared_ptr<RssItem>> items_;
            std::mutex lock_;
    };

}
