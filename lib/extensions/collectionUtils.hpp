#ifndef COLLECTION_UTILS_HPP
#define COLLECTION_UTILS_HPP

#include <vector>
#include <unordered_map>
#include <exception>
#include "math.hpp"

template <class K, class V>
std::vector<K> GetKeys(const std::unordered_map<K,V>& map) {
    std::vector<K> result;
    for (auto it = map.begin(); it != map.end(); ++it) {
        result.push_back(it->first);
    }
    return result;
}
template <class K, class V>
std::vector<V> GetValues(const std::unordered_map<K,V>& map) {
    std::vector<V> result;
    for (auto it = map.begin(); it != map.end(); ++it) {
        result.push_back(it->second);
    }
    return result;
}

// Appends a range of elements from a given vector onto another buffer vector.
template <class T>
void AppendRange(std::vector<T>& buffer, const std::vector<T>& range) {
    for (T value : range) {
        buffer.push_back(value);
    }
}

// Removes a specified element at most once from a provided vector
// Returns `true` if the element was found and removed. `false` otherwise.
template <class T>
bool Remove(std::vector<T>& vector, const T& value) {
    for (auto it = vector.begin(); it != vector.end(); ++it) {
        if (*it == value) {
            vector.erase(it);
            return true;
        }
    }
    return false;
}

template <class T>
T Sample(std::vector<T>& vector) {
    if (vector.size() == 0) {
        throw std::out_of_range("Tried to sample a random value from an empty vector!");
    }
    size_t idx = Random(0, vector.size());
    return vector.at(idx);
}

#endif