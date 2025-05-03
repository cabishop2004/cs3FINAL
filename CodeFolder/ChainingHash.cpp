#include "ChainingHash.h"
#include <cstring>

// Default hash: sum of chars modulo table size
static size_t default_string_hash(const string& s, size_t mod) {
    size_t h = 0;
    for (char c : s) {
        h = (h * 31 + static_cast<unsigned char>(c)) % mod;
    }
    return h;
}

template<typename Key, typename Value>
ChainingHash<Key,Value>::ChainingHash(size_t table_size, HashFunc hf)
    : hsize(table_size), table(table_size), count(0)
{
    hash_func = hf ? hf : default_string_hash;
}

template<typename Key, typename Value>
void ChainingHash<Key,Value>::insert(const Key& key, const Value& value) {
    size_t idx = hash_func(key, hsize);
    table[idx].push_back(make_pair(key, value));
    count++;
}

template<typename Key, typename Value>
bool ChainingHash<Key,Value>::find(const Key& key, Value& value_out) const {
    size_t idx = hash_func(key, hsize);
    for (auto& kv : table[idx]) {
        if (kv.first == key) {
            value_out = kv.second;
            return true;
        }
    }
    return false;
}

template<typename Key, typename Value>
size_t ChainingHash<Key,Value>::size() const { return count; }

template<typename Key, typename Value>
double ChainingHash<Key,Value>::load_factor() const {
    return static_cast<double>(count) / hsize;
}

template<typename Key, typename Value>
void ChainingHash<Key,Value>::set_hash_function(HashFunc hf) {
    assert(hf != nullptr);
    hash_func = hf;
}
