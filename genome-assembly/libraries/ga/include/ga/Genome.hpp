#ifndef GA_GENOME_HPP
#define GA_GENOME_HPP

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace genome {

std::string assembly(size_t, const std::vector<std::string>&);

}  // namespace genome

#endif  // GA_GENOME_HPP
