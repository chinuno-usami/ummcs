#include "storage.h"
#include <string>
#include <vector>
#include <iostream>
#include "log.h"

// 将数据封装成Slice对象
#define TO_SLICE(type, value) \
    Slice(reinterpret_cast<const char*>(&value), sizeof(type))

// 从取出的数据中还原为原来的类型
#define FROM_STRING(type ,value) \
    *reinterpret_cast<const type*>(value.data())

// 取值操作，和本文关系不大
#define GET_VALUE(handle, key, output) \
    db_->Get(rocksdb::ReadOptions(), handle, Slice(key), output);

using namespace std;
using namespace ummcs;
using namespace rocksdb;

Storage::Storage(const char *db_path)
{
    rocksdb::Options options;
    options.create_if_missing = true;

    // 获取cf列表
    rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db_);
    if (!status.ok())
    {
        LOG_ERROR("open rocksdb failed:%s", status.getState());
    }
    std::vector<std::string> cf_names;
    DB::ListColumnFamilies(options, db_path, &cf_names);
    delete db_;


//    DBOptions loaded_db_opt;
//    std::vector<ColumnFamilyDescriptor> cf_descs;
//    LoadLatestOptions(db_path, Env::Default(), &loaded_db_opt,
//                    &cf_descs);


    // 加载所有cf
    std::vector<ColumnFamilyDescriptor> cf_descs;
    for(const auto& name:cf_names){
        LOG_DEBUG("load cf:%s", name.c_str());
        cf_descs.push_back(ColumnFamilyDescriptor(name, ColumnFamilyOptions()));
    }
    std::vector<ColumnFamilyHandle*> cf_handles;
    status = DB::Open(options, db_path, cf_descs, &cf_handles, &db_);
    if (!status.ok()){
        LOG_ERROR("open rocksdb %s failed:%s", db_path, status.getState());
    }
    // 填充handle表
    for(auto handle: cf_handles){
        // 不加载默认cf
        //if(handle->GetName() == "default") { continue; }
        cf_handles_[handle->GetName()] = handle;
    }
}

Storage::~Storage(){
    for (auto handle:cf_handles_){
        delete handle.second;
    }
    delete db_;
}

bool Storage::create_cf(const std::string& name){
    if(cf_handles_.find(name) != cf_handles_.end()){ return true; }
    ColumnFamilyHandle* handle;
    Status s = db_->CreateColumnFamily(ColumnFamilyOptions(), name, &handle);
    if(!s.ok()){
        LOG_ERROR("create cf %s failed:%s", name.c_str(), s.getState());
        return false;
    }
    cf_handles_[name] = handle;
    return true;
}

template<class T>
bool Storage::put_value(const std::string& cf, const std::string& key, T value){
    auto status = db_->Put(rocksdb::WriteOptions(), cf_handles_[cf], Slice(key), TO_SLICE(T, value));
    if (status.ok()) {
        return true;
    } else {
        LOG_ERROR("%s", status.getState());
        return false;
    }

}

template<>
bool Storage::put_value<std::string>(const std::string& cf, const std::string& key, string value){
    auto status = db_->Put(rocksdb::WriteOptions(), cf_handles_[cf], Slice(key), Slice(value));
    if (status.ok()) {
        return true;
    } else {
        LOG_ERROR("%s", status.getState());
        return false;
    }

}

template<class T> T Storage::get_value(const std::string& cf, const std::string& key){
    string db_value;
    auto status = GET_VALUE(cf_handles_[cf], key, &db_value);
    if (status.ok()){
        T ret = FROM_STRING(T, db_value);
        return ret;
    } else {
        LOG_ERROR("%s", status.getState());
        throw std::out_of_range(status.getState());
        // return T();
    }
}

template<>
std::string Storage::get_value<std::string>(const std::string& cf, const std::string& key){
    string db_value;
    auto status = db_->Get(rocksdb::ReadOptions(),
            cf_handles_[cf], Slice(key), &db_value);
    if (status.ok()){
        return db_value;
    } else {
        LOG_ERROR("%s", status.getState());
        throw std::out_of_range(status.getState());
        // return std::string();
    }
}

bool Storage::drop_cf(const std::string& cf){
    auto it = cf_handles_.find(cf);
    if(it == cf_handles_.end()){
        return true;
    }
    Status s = db_->DropColumnFamily(it->second);
    if(s.ok()){
        return true;
    } else {
        LOG_ERROR("%s", s.getState());
    }
    return false;
}

rocksdb::Iterator* Storage::new_iter(const std::string& cf){
    return db_->NewIterator(rocksdb::ReadOptions(), cf_handles_[cf]);
}

template bool Storage::put_value<time_t>(const std::string& cf, const std::string& key, time_t value);
template time_t Storage::get_value<time_t>(const std::string& cf, const std::string& key);
