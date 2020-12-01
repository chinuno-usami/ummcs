#pragma once
#include <string>
#include <gumbo.h>
#include <mirai/mirai.h>

namespace ummcs{
    class RssParser
    {
    private:
        void parse(GumboNode* node, mirai::Message& msg);
        bool use_proxy_;
        std::string plugin_image_path_;
        std::string proxy_;
    public:
        RssParser();
        ~RssParser() {}

        mirai::Message parse_all(const std::string &html);
    };

} // namespace ummcs
