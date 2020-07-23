#include "rss.h"
#include "log.h"

using namespace ummcs;

void RssItem::check_update(){
    auto info = feedparser_parse_url(url.c_str());
    if(info){
        callback(info);
        feedparser_release_feedinfo(&info);
    } else {
        LOG_ERROR("get feed failed.");
    }
}

Rss::Rss(){
}

void Rss::run(){
    th_ = std::thread(&ummcs::Rss::main_loop, this);
}

Rss::~Rss(){
    stop();
}

void Rss::stop(){
    running_ = false;
    cv_.notify_all();
    if(th_.joinable()){
        th_.join();
    }
}

bool Rss::add_item(std::shared_ptr<RssItem> item){
    std::lock_guard<std::mutex> lock(lock_);
    if(items_.find(item->id) != items_.end()){
        std::string strErr(u8"id已在订阅列表:");
        strErr.append(item->id);
        throw strErr;
        return false;
    }

    items_.insert({item->id, item});
    return true;
}

bool Rss::remove_item(const std::string& id){
    std::lock_guard<std::mutex> lock(lock_);
    auto it = items_.find(id);
    if(it == items_.end()){
        std::string strErr(u8"id不在订阅列表:");
        strErr.append(id);
        throw strErr;
        return false;
    }

    items_.erase(it);
    return true;
}

void Rss::main_loop(){
    while(running_){
        {
            std::lock_guard<std::mutex> lock(lock_);
            for (auto &item : items_)
            {
                item.second->check_update();
            }
        }
        std::unique_lock<std::mutex> lock(lock_);
        cv_.wait_for(lock, interval_, [this]{
            return !running_;
        });
    }
}

std::list<std::shared_ptr<RssItem>> Rss::get_items(){
    std::list<std::shared_ptr<RssItem>> lst;
    for(const auto& item: items_){
        lst.push_back(item.second);
    }
    return lst;
}
