#ifndef PROBING_HASH_H
#define PROBING_HASH_H

#include "HashTable.h"
#include <vector>
#include <cstdlib>  // for size_t
#include <cstring>
#include <cassert>
#include <stdexcept>

using namespace std;
enum SlotState { EMPTY, OCCUPIED, DELETED };

template<typename Key, typename Value>
class ProbingHash : public HashTable<Key,Value> {
public:
    typedef size_t (*HashFunc)(const Key&, size_t);
    ProbingHash(size_t table_size, double max_load, HashFunc hf = nullptr)
      : hsize(table_size), table(table_size), count(0), max_load(max_load), hash_func(hf ? hf : default_string_hash) {
        for (auto& e : table) e.state = EMPTY;
    }
    ~ProbingHash() override = default;

    void insert(const Key& key, const Value& value) override {
        size_t idx = hash_func(key, hsize);
        size_t start = idx;

        // 1) Upser, if key exists, overwrite value
        do {
            if (table[idx].state == OCCUPIED && table[idx].key == key) {
                table[idx].value = value;
                return;
            }
            if (table[idx].state == EMPTY) break;
            idx = (idx + 1) % hsize;
        } while (idx != start);

        // 2)enforce load factor
        if (load_factor() >= max_load) {
            throw runtime_error("Load factor exceeded");
        }
        // find next available slot
        while (table[idx].state == OCCUPIED) {
            idx = (idx + 1) % hsize;
        }
        table[idx] = {key, value, OCCUPIED};
        count++;
    }

    bool find(const Key& key, Value& value_out) const override {
        size_t idx = hash_func(key, hsize);
        size_t start = idx;
        do {
            if (table[idx].state == EMPTY) return false;
            if (table[idx].state == OCCUPIED && table[idx].key == key) {
                value_out = table[idx].value;
                return true;
            }
            idx = (idx + 1) % hsize;
        } while (idx != start);
        return false;
    }

    size_t size() const override { return count; }
    double load_factor() const override { return static_cast<double>(count) / hsize; }
    void set_hash_function(HashFunc hf) { assert(hf); hash_func = hf; }

private:
    struct Entry { Key key; Value value; SlotState state; };
    size_t hsize;
    vector<Entry> table;
    size_t count;
    double max_load;
    HashFunc hash_func;

    // Horners rule
    static size_t default_string_hash(const string& s, size_t mod) {
        size_t h = 0;
        for (char c : s) h = (h * 31 + static_cast<unsigned char>(c)) % mod;
        return h;
    }
};

#endif // PROBING_HASH_H
