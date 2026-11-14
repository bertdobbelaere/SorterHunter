#pragma once
#include <vector>
#include <unordered_set>
#include "sn_to_latex.h"

namespace sh::tools {

    namespace {
        [[nodiscard]] inline bool is_irrelevant(
            const Ece& swap,
            const std::unordered_set<int>& unrelated
        ) {
            return unrelated.contains(swap.first) && unrelated.contains(swap.second);
        }
    }

    [[nodiscard]] inline std::vector<std::vector<Ece>> annotate_unnecessary(
        const std::vector<std::unordered_set<int>>& unrelated_groups,
        const std::vector<std::vector<Ece>>& sn,
        const std::string& colour = "red"
    ) {
        std::vector<std::vector<Ece>> result = sn;
        std::vector<std::unordered_set<int>> u_groups = unrelated_groups;

        const int n_layers = static_cast<int>(sn.size());
        for (int i = n_layers - 1; i >= 0; --i) {
            std::vector<Ece>& layer = result[i];
 
            for (std::unordered_set<int>& unrelated : u_groups) {
                std::vector<int> unnecessary;
                for (Ece& swap : layer) {
                    if (is_irrelevant(swap, unrelated)) {
                        swap.colour = colour;
                    }
                    else {
                        unnecessary.push_back(swap.first);
                        unnecessary.push_back(swap.second);
                    }
                }
                for (int u : unnecessary) {
                    unrelated.erase(u);
                }
            }
        }
        return result;
    }

    [[nodiscard]] inline std::vector<std::unordered_set<int>> get_unrelated_groups(int k, int channels) {
        std::unordered_set<int> group1;
        std::unordered_set<int> group2;
        for (int i = 0; i < channels; ++i) {
            if (i < k) {
                group1.insert(i);
            }
            else {
                group2.insert(i);
            }
        }
        std::vector<std::unordered_set<int>> unrelated_groups;
        unrelated_groups.push_back(std::move(group1));
        unrelated_groups.push_back(std::move(group2));
        return unrelated_groups;
    }
}