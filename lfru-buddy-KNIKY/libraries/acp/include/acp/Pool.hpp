#ifndef ACP_POOL_HPP
#define ACP_POOL_HPP

#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <new>
#include <vector>

class PoolAllocator {
public:
    PoolAllocator(std::size_t const minP, std::size_t const maxP);

    void* allocate(std::size_t const num);

    void deallocate(void const* ptr);

private:
    void split_block(std::size_t position, std::size_t k_pow);
    void union_blocks(std::size_t position);

    [[nodiscard]] std::size_t get_num_of_blocks(std::size_t position) const;
    [[nodiscard]] std::size_t get_pos_of_neighbor(std::size_t position) const;

    [[nodiscard]] std::size_t find_free_block(std::size_t cnt);

    const std::size_t min_pow;
    const std::size_t max_pow;
    const std::size_t m_cnt;

    std::vector<std::size_t> size_of_block;
    std::vector<std::byte> storage;
    std::vector<bool> used;
};

#endif  // ACP_POOL_HPP
