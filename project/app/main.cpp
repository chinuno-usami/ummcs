#include <string.h>
#include <time.h>

#include <signal.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>
#include <mirai/mirai.h> // 包含此头文件来使用所有 Mirai++ 的 API

#include "rss.h"
#include "configure.h"
#include "storage.h"
#include "modules/nbnhhsh.h"
#include "rss_parser.h"

bool running = true;

void sigint_handler(int sig) {
    running = false;
    std::cout << "got sig:" << sig << std::endl;
    std::cout << "exiting..." << std::endl;
}

using namespace std;
using namespace ummcs;
int main()
{
    using namespace std::chrono_literals;
    signal(SIGINT, &sigint_handler);

    Config& config = Config::get_instance();
    config.parse("config.toml");

   // 持久化，用于存储模块状态
    ummcs::Storage storage("ummcs_storage");

    ModuleManager manager(storage);

    manager.regist_modules();
    manager.load_modules();


    using namespace mirai::literals; // 拉入 _uid _gid 等字面量运算符
    mirai::Session sess(config.auth_key, mirai::uid_t(config.bot)); // 创建并授权 bot session
    sess.config({}, true); // 开启 WebSocket
    manager.run_modules(sess);
    sess.subscribe_messages([&](const mirai::Event& event) // 监听所有消息接受事件
    {
        manager.process_entry(event, sess);
    }, mirai::error_logger, mirai::ExecutionPolicy::thread_pool); // 设定异常处理函数为日志输出，执行策略为使用线程池
    while(running){
        std::this_thread::sleep_for(1s);
    }
    manager.stop_modules();
    std::cout << "bye." << std::endl;
    return 0;
}
