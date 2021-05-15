#pragma once
#include <map>
#include <string>
#include <string_view>
#include "module.h"

namespace ummcs{
    class Bilibili : public ModuleInterface
    {
        struct VideoInfo {
            std::string id;
            std::string pic;
            std::string title;
            std::string desc;
            std::string link;
            std::string up;
        };
        enum class IdType {
            AID,
            BVID,
            SSID,
            EPID
        };
    public:
        Bilibili() = delete;
        Bilibili(Storage& storage):ModuleInterface(storage) {}
        ~Bilibili() override {}
        void run(mirai::Session&) override {}
        void stop() override {}
        void process(const mirai::Event &e, mirai::Session &sess) override;

        VideoInfo get_info_app(const std::string& url);
        VideoInfo get_info(IdType type, const std::string& id);
        VideoInfo get_bgm_info(IdType type, const std::string& id);
        VideoInfo get_video_info(IdType type, const std::string& id);
    };
}
