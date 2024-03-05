#include "acp/Pool.hpp"

std::size_t get_pow(std::size_t pow) {
    return (1 << pow);
}

std::size_t log_up(std::size_t num) {
    auto dnum = static_cast<double>(2 * num - 1);
    return static_cast<std::size_t>(log2(dnum));
}

PoolAllocator::PoolAllocator(const std::size_t minP, const std::size_t maxP)
    : min_pow(minP)
    , max_pow(maxP)
    , m_cnt(1 << (maxP - minP))
    , size_of_block(1 << (maxP - minP))
    , storage(1 << maxP)
    , used(1 << (maxP - minP), false) {
    size_of_block[0] = maxP;
}

void* PoolAllocator::allocate(std::size_t const num) {
    const std::size_t pos = find_free_block(log_up(num));
    if (pos != m_cnt) {
        used[pos] = true;
        return &storage[pos * get_pow(min_pow)];
    }
    throw std::bad_alloc{};
}

void PoolAllocator::deallocate(void const* ptr) {
    auto b_ptr               = static_cast<const std::byte*>(ptr);
    const auto begin         = &storage[0];
    const std::size_t offset = (b_ptr - begin) / get_pow(min_pow);
    if (offset < m_cnt) {
        used[offset] = false;
        union_blocks(offset);
    }
}

// private methods

void PoolAllocator::split_block(std::size_t position, std::size_t k_pow) {
    std::size_t pow = size_of_block[position];
    if (pow > k_pow && pow > min_pow) {
        size_of_block[position]                               = pow - 1;
        size_of_block[position + get_num_of_blocks(position)] = pow - 1;
        split_block(position, k_pow);
    }
}

void PoolAllocator::union_blocks(std::size_t position) {
    if (size_of_block[position] == max_pow) {
        return;
    }
    std::size_t nposition = get_pos_of_neighbor(position);
    std::size_t left      = std::min(position, nposition);
    std::size_t right     = std::max(position, nposition);
    if (!used[nposition] && size_of_block[position] == size_of_block[nposition]) {
        size_of_block[right] = 0;
        size_of_block[left]++;
        union_blocks(left);
    }
}

std::size_t PoolAllocator::get_num_of_blocks(std::size_t position) const {
    return get_pow(size_of_block[position] - min_pow);
}

std::size_t PoolAllocator::get_pos_of_neighbor(std::size_t position) const {
    std::size_t num_blocks   = get_num_of_blocks(position);
    std::size_t pos_of_block = position / num_blocks;
    return ((pos_of_block % 2 == 0) ? (position + num_blocks) : (position - num_blocks));
}

std::size_t PoolAllocator::find_free_block(std::size_t k_pow) {
    std::size_t ind      = 0;
    std::size_t best_pos = m_cnt;

    while (ind < size_of_block.size()) {
        if (!used[ind] && size_of_block[ind] >= k_pow &&
            (best_pos == m_cnt || size_of_block[best_pos] > size_of_block[ind])) {
            best_pos = ind;
        }
        ind += get_num_of_blocks(ind);
    }

    if (best_pos != m_cnt) {
        split_block(best_pos, k_pow);
        return best_pos;
    }
    return m_cnt;
}
