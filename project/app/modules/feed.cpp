#include "feed.h"
#include "../rss_parser.h"
#include "log.h"

using namespace std::literals;

namespace ummcs{
std::string feedstring2string(FeedString& str){
    return std::string(str.data, str.size);
}
time_t get_ts_from_string(const std::string& in){
    std::istringstream ss(in);
    struct tm t;
    ss >> std::get_time(&t, "%a %b %d %H:%M:%S %Y");
    if (ss.fail())
    {
        LOG_ERROR("parse failed");
    }
    else
    {
        return mktime(&t);
    }
    return 0;
}

}

using namespace std;
using namespace ummcs;

void FeedModule::run(mirai::Session& sess){
    storage_.create_cf(config_->id);
    const auto& module_config = config_->data;
    auto sources = toml::find<std::vector<toml::value>>(module_config, "source");
    for(const auto& item: sources){
        auto ritem = std::make_shared<ummcs::RssItem>();
        auto url = toml::find<std::string>(item, "url");
        auto id = toml::find<std::string>(item, "name");
        std::vector<int64_t> groups = toml::find<std::vector<int64_t>>(item, "groups_enable");
        std::vector<int64_t> users = toml::find<std::vector<int64_t>>(item, "users_enable");
        item_enable_groups[id] = groups;
        item_enable_users[id] = users;
        ritem->id = id;
        ritem->url = url;
        ritem->callback = [&sess, id, this](FeedInfo* feedp){
            RssParser parser;
            FeedInfo& feed = *feedp;
            static time_t latest_time = storage_.get_value<time_t>(config_->id, "last_ts"s);
            time_t new_time = latest_time;
            bool first_time = latest_time == 0? true:false;
            size_t size = feed.size;
            std::string feed_title = feedstring2string(feed.title);
            mirai::Message msg;
            //LOG_DEBUG(u8"个数:%lu", size);
            for(size_t i = 0; i < size; ++i){
                FeedEntry& entry = feed.entries[i];
                time_t item_time = entry.published;
                //LOG_DEBUG("ts:%lu", item_time);
                if(item_time <= latest_time) {
                    //LOG_DEBUG("continue");
                    continue;
                } else {
                    //LOG_DEBUG("process new item");
                }
                if(!msg.empty()) {
                    msg += "\n\n";
                }
                msg += u8"来源:";
                //LOG_DEBUG(u8"来源:%s", feed_title.c_str());
                msg += feed_title+"\n";
                msg += u8"标题:";
                std::string title = feedstring2string(entry.title);
                msg += title;
                msg += "\n";
                //LOG_DEBUG(u8"标题:%s", title.c_str());
                auto desc = feedstring2string(entry.summary);
                msg += u8"内容:";
                msg += parser.parse_all(desc);
                msg += "\n";
                msg += "链接:";
                std::string link = feedstring2string(entry.link);
                //LOG_DEBUG(u8"链接:%s", link.c_str());
                msg += link;
                if (item_time > new_time) {
                    //LOG_DEBUG("update to %lu", item_time);
                    new_time = item_time;
                } else {
                    //LOG_DEBUG("new:%lu,item:%lu", new_time, item_time);
                }
            }
            if(latest_time < new_time){
                //LOG_DEBUG("update latest to %lu", new_time);
                latest_time = new_time;
                storage_.put_value<time_t>(config_->id, "last_ts"s, latest_time);
            } else {
                //LOG_DEBUG("new:%lu,latest:%lu", new_time, latest_time);
            }
            if(first_time) { return; }
            if(msg.empty()){ return; }
            for(auto gid:item_enable_groups[id]){
                sess.send_message(mirai::gid_t(gid), msg);
            }
            for(auto uid:item_enable_users[id]){
                sess.send_message(mirai::uid_t(uid), msg);
            }
        };
        rss_.add_item(ritem);
    }
    rss_.set_check_interval(60s);
    rss_.run();
}

void FeedModule::stop(){
    rss_.stop();
}
