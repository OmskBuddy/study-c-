#ifndef ACP_CACHE_HPP
#define ACP_CACHE_HPP

#include <algorithm>
#include <cstddef>
#include <list>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache {
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&...alloc_args)
        : m_max_top_size(cache_size), m_max_low_size(cache_size), m_alloc(std::forward<AllocArgs>(alloc_args)...) {}

    [[nodiscard]] std::size_t size() const { return top.size() + low.size(); }

    [[nodiscard]] bool empty() const { return size() == 0; }

    template <class T>
    T &get(const Key &key);

    std::ostream &print(std::ostream &strm) const;

    friend std::ostream &operator<<(std::ostream &strm, const Cache &cache) { return cache.print(strm); }

private:
    void balance_top() {
        if (top.size() > m_max_top_size) {
            low.push_front(top.back());
            top.pop_back();
            balance_low();
        }
    }

    void balance_low() {
        if (low.size() > m_max_low_size) {
            m_alloc.template destroy<KeyProvider>(low.back());
            low.pop_back();
        }
    }

    const std::size_t m_max_top_size;
    const std::size_t m_max_low_size;
    std::list<KeyProvider *> top;
    std::list<KeyProvider *> low;
    Allocator m_alloc;
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T &Cache<Key, KeyProvider, Allocator>::get(const Key &key) {
    auto top_it = std::find_if(top.begin(), top.end(), [&key](const KeyProvider *ptr) { return *ptr == key; });

    if (top_it != top.end()) {
        top.splice(top.begin(), top, top_it);
    } else {
        auto low_it = std::find_if(low.begin(), low.end(), [&key](const KeyProvider *ptr) { return *ptr == key; });

        if (low_it != low.end()) {
            top.splice(top.begin(), low, low_it);
            balance_top();
        } else {
            low.push_front(m_alloc.template create<T>(key));
            balance_low();
            return *static_cast<T *>(low.front());
        }
    }
    return *static_cast<T *>(top.front());
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream &Cache<Key, KeyProvider, Allocator>::print(std::ostream &strm) const {
    return strm << "Priority: <empty>"
                << "\nRegular: <empty>"
                << "\n";
}

#endif  // ACP_CACHE_HPP
