#ifndef CHAINING_HASH_H
#define CHAINING_HASH_H

#include "HashTable.h"
#include <list>
#include <vector>
#include <cstdlib>  // for size_t
#include <cstring>

using namespace std;

template<typename Key, typename Value>
class ChainingHash : public HashTable<Key,Value> {
public:
    typedef size_t (*HashFunc)(const Key&, size_t);
    explicit ChainingHash(size_t table_size, HashFunc hf = nullptr)
      : hsize(table_size), table(table_size), count(0) {
        hash_func = hf ? hf : default_string_hash;
    }
    ~ChainingHash() override = default;

    void insert(const Key& key, const Value& value) override {
        size_t idx = hash_func(key, hsize);
        // 1) look for an existing key in the chain
        for (auto& kv : table[idx]) {
            if (kv.first == key) {
                kv.second = value;    // overwrite existing count
                return;
            }
        }
        // 2) not found → insert new
        table[idx].push_back(make_pair(key, value));
        count++;
    }
    

    bool find(const Key& key, Value& value_out) const override {
        size_t idx = hash_func(key, hsize);
        for (auto& kv : table[idx]) {
            if (kv.first == key) {
                value_out = kv.second;
                return true;
            }
        }
        return false;
    }

    size_t size() const override { return count; }

    double load_factor() const override {
        return static_cast<double>(count) / hsize;
    }

    void set_hash_function(HashFunc hf) {
        assert(hf != nullptr);
        hash_func = hf;
    }

private:
    size_t hsize;
    vector<list<pair<Key,Value>>> table;
    size_t count;
    HashFunc hash_func;

    // Default hash: Horner's rule
    static size_t default_string_hash(const string& s, size_t mod) {
        size_t h = 0;
        for (char c : s) {
            h = (h * 31 + static_cast<unsigned char>(c)) % mod;
        }
        return h;
    }
};

#endif // CHAINING_HASH_H
