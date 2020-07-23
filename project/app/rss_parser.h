#pragma once
#include <string>
#include <gumbo.h>
#include <mirai/mirai.h>

namespace ummcs{
    class RssParser
    {
    private:
        void parse(GumboNode* node, mirai::Message& msg);
        /* data */
    public:
        RssParser() {}
        ~RssParser() {}

        mirai::Message parse_all(const std::string &html);
    };

} // namespace ummcs