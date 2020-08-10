#pragma once
#include <map>
#include <rocksdb/db.h>

namespace ummcs
{
    class Storage
    {
    private:
        rocksdb::DB* db_ = nullptr;
        std::map<std::string, rocksdb::ColumnFamilyHandle*> cf_handles_;
    public:
        Storage(const char* db_path);
        ~Storage();

        template<class T>
        bool put_value(const std::string& cf, const std::string& key, T value);

        template<class T>
        T get_value(const std::string& cf, const std::string& key);

        bool create_cf(const std::string& name);
        bool drop_cf(const std::string& cf);
        // 需要手动delete
        rocksdb::Iterator* new_iter(const std::string& name);
    };
} // namespace ummcs
