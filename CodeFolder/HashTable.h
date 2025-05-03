#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string>
#include <cassert>

using namespace std;

template<typename Key, typename Value>
class HashTable {
public:
    virtual ~HashTable() {}
    virtual void insert(const Key& key, const Value& value) = 0;
    virtual bool find(const Key& key, Value& value_out) const = 0;
    virtual size_t size() const = 0;
    virtual double load_factor() const = 0;
};

#endif // HASHTABLE_H
