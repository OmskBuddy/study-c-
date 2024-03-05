#include "ga/Genome.hpp"

std::unordered_map<std::string_view, std::vector<std::string_view>> make_graph(size_t k,
                                                                               const std::vector<std::string>& reads) {
    std::unordered_map<std::string_view, std::vector<std::string_view>> graph;
    const size_t read_size = reads.front().size();

    for (const auto& read : reads) {
        for (size_t i = 0; i + k < read_size; i++) {
            std::string_view from = std::string_view{read}.substr(i, k);
            std::string_view to   = std::string_view{read}.substr(i + 1, k);
            graph[from].push_back(to);
        }
    }

    return graph;
}

std::string_view start_node(std::unordered_map<std::string_view, std::vector<std::string_view>>& graph) {
    std::string_view start = graph.begin()->first;
    std::unordered_map<std::string_view, int> in_degree;
    std::unordered_map<std::string_view, int> out_degree;

    for (const auto& [from, to] : graph) {
        for (const auto& node : to) {
            in_degree[node]++;
            out_degree[from]++;
        }
    }

    for (const auto& el : graph) {
        int power = out_degree[el.first] - in_degree[el.first];

        if (power > 0 && power % 2 == 1) {
            start = el.first;
            break;
        }
    }

    return start;
}

std::vector<std::string_view> euler(std::unordered_map<std::string_view, std::vector<std::string_view>>& graph) {
    std::string_view start = start_node(graph);
    std::vector<std::string_view> euler_path;

    std::vector<std::string_view> stack = {start};
    while (!stack.empty()) {
        const std::string_view node = stack.back();
        stack.pop_back();

        auto& edges = graph[node];
        if (!edges.empty()) {
            stack.push_back(node);
            const std::string_view next = edges.front();

            edges.erase(edges.begin());
            stack.push_back(next);
        } else {
            euler_path.push_back(node);
        }
    }

    return euler_path;
}

std::string merge_genome(std::vector<std::string_view>& euler_path) {
    std::reverse(euler_path.begin(), euler_path.end());
    std::string result = static_cast<std::string>(euler_path[0]);

    for (size_t i = 1; i < euler_path.size(); i++) {
        result += euler_path[i].back();
    }

    return result;
}

namespace genome {

std::string assembly(size_t k, const std::vector<std::string>& reads) {
    if (k == 0 || reads.empty()) {
        return "";
    }

    std::unordered_map<std::string_view, std::vector<std::string_view>> graph = make_graph(k, reads);
    std::vector<std::string_view> genome                                      = euler(graph);

    return merge_genome(genome);
}

}  // namespace genome
