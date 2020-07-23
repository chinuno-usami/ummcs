# UMMCS
Ultimate Mecha Musume Core System  

## 总览

基于[Mirai](https://github.com/mamoe/mirai)平台的[HTTP](https://github.com/mamoe/mirai-api-http)模块的[C++封装库](https://github.com/Chlorie/miraipp-library)的[模板](https://github.com/Chlorie/miraipp-library)的模块化中二机娘QQ机器人系统

## 依赖
- [miraipp-template](https://github.com/Chlorie/miraipp-library)依赖的所有需求
- [feedparser](https://github.com/chinuno-usami/feedparser):RSS模块需要，已作为子模块引入
- [RocksDB](https://github.com/facebook/rocksdb):数据存储
- [gumbo](https://github.com/google/gumbo-parser):HTTP解析，RSS模块使用
- [toml11](https://github.com/ToruNiina/toml11):toml配置，配置文件解析

## 使用
修改`config.toml`配置，填写
- `bot`:机器人QQ号
- `auth_key`:mirai-http的认证码
- `admins`:管理员QQ号
- 各个子模块的配置选项

配置`mirai`平台及`mirai-http`相关配置  
配置完成后启动`mirai`平台，直接运行可执行文件`ummcs`即可
