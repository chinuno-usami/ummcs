#pragma once
#include <map>
#include <memory>
#include <functional>
#include "mirai/mirai.h"
#include "../configure.h"
#include "../storage.h"
namespace ummcs{
    class ModuleManager;
    class ModuleInterface
    {
    public:
        friend ModuleManager;
        ModuleInterface(Storage& storage):storage_(storage){}
        virtual ~ModuleInterface() {}
        virtual void run(mirai::Session &sess) = 0;
        virtual void stop() = 0;
        virtual void process(const mirai::Event &e, mirai::Session &sess) = 0;
    protected:
        bool check_privilege(int64_t uid);
        bool check_user_enable(int64_t uid);
        bool check_group_enable(int64_t gid);
        std::shared_ptr<ummcs::ModuleConfig> config_;
        Storage& storage_;
    private:
        void set_config(std::shared_ptr<ummcs::ModuleConfig> config) { config_ = config; }
    };

    class ModuleManager
    {
    public:
        ModuleManager() = delete;
        ModuleManager(Storage& storage):storage_(storage){};
        ~ModuleManager() { stop_modules(); };
        void regist_modules();
        void load_modules();
        void run_modules(mirai::Session &sess);
        void stop_modules();
        void process_entry(const mirai::Event &e, mirai::Session &sess);
    private:
        Storage& storage_;
        std::map<std::string, std::function<std::shared_ptr<ModuleInterface>(void)>> module_factories_;
        std::map<std::string, std::shared_ptr<ModuleInterface>> module_handles_;
    };
}
