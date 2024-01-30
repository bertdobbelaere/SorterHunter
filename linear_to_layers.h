/**
 * @file linear_to_layers.h
 * @brief Various utility functions for SorterHunter program
 * @author Bert Dobbelaere bert.o.dobbelaere[at]telenet[dot]be
 *
 * Copyright (c) 2024 Bert Dobbelaere
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>

#include "htypes.h"

namespace sh::tools {

    [[nodiscard]] inline std::vector<Pair_t> remove_prefix(const std::vector<Pair_t>& net, const std::vector<Pair_t>& prefix) {
        const int net_size = static_cast<int>(net.size());
        const int prefix_size = static_cast<int>(prefix.size());
        for (int i = 0; i < prefix_size; ++i) {
            const Pair_t& p1 = net.at(i);
            const Pair_t& p2 = prefix.at(i);
            if ((p1.hi != p2.hi) || (p1.lo != p2.lo)) {
                std::cout << "ERROR remove_prefix: investigate!" << std::endl;
            }
        }
        std::vector<Pair_t> result;
        for (int i = prefix_size; i < net_size; ++i) {
            result.push_back(net.at(i));
        }
        return result;
    }

    [[nodiscard]] inline std::vector<Pair_t> remove_postfix(const std::vector<Pair_t>& net, const std::vector<Pair_t>& postfix) {
        const int net_size = static_cast<int>(net.size());
        const int postfix_size = static_cast<int>(postfix.size());
        for (int i = 0; i < postfix_size; ++i) {
            const Pair_t& p1 = net.at((net_size - 1) - i);
            const Pair_t& p2 = postfix.at((postfix_size - 1) - i);
            if ((p1.hi != p2.hi) || (p1.lo != p2.lo)) {
                std::cout << "ERROR remove_postfix: investigate!" << std::endl;
            }
        }
        std::vector<Pair_t> result;
        for (int i = 0; i < (net_size - postfix_size); ++i) {
            result.push_back(net.at(i));
        }
        return result;
    }

    // Converts a linear network representation as pairs into a layer representation.
    [[nodiscard]] inline std::vector<std::vector<std::pair<int, int>>> linear_to_layers(const std::vector<std::pair<int, int>>& input_lst)
    {
        std::vector<std::vector<std::pair<int, int>>> result;
        std::unordered_map<int, int> lbl_state;

        for (const auto& [i, j] : input_lst)
        {
            if (!lbl_state.contains(i)) {
                lbl_state[i] = 0;
            }

            if (!lbl_state.contains(j)) {
                lbl_state[j] = 0;
            }

            const int k = std::max(lbl_state[i], lbl_state[j]);

            if (k >= result.size()) {
                result.push_back(std::vector<std::pair<int, int>>());
            }
            result[k].push_back(std::make_pair(i, j));
            lbl_state[i] = lbl_state[j] = k + 1;
        }

        for (auto& layer : result) {
            sort(layer.begin(), layer.end());
        }

        return result;
    }

    // Converts a linear network representation as pairs into a layer representation.
    [[nodiscard]] inline std::vector<std::vector<std::pair<int, int>>> linear_to_layers(const std::vector<Pair_t>& input_lst) {
        std::vector<std::pair<int, int>> tmp;
        for (const Pair_t& p : input_lst) {
            tmp.push_back(std::make_pair(p.lo, p.hi));
        }
        return linear_to_layers(tmp);
    }

    [[nodiscard]] inline std::string layers_to_string(const std::vector<std::vector<std::pair<int, int>>>& layers)
    {
        std::stringstream ss;
        for (const auto& layer : layers) 
        {
            const int s = static_cast<int>(layer.size());
            ss << "[";
            for (int i = 0; i < s; ++i) 
            {
                ss << "(" << layer[i].first << "," << layer[i].second << ")";
                if (i < (s - 1)) {
                    ss << ",";
                }
            }
            ss << "]" << std::endl;
        }
        return ss.str();
    }

    [[nodiscard]] inline std::string layers_to_string_mojo(const std::vector<std::vector<std::pair<int, int>>>& layers)
    {
        std::stringstream ss;
        int layer_id = 0;
        for (const auto& layer : layers)
        {
            const int s = static_cast<int>(layer.size());
            ss << "result.add_layer(" << layer_id << ", VariadicList(";
            layer_id++;
            for (int i = 0; i < s; ++i)
            {
                ss << "(" << layer[i].first << "," << layer[i].second << ")";
                if (i < (s - 1)) {
                    ss << ",";
                }
            }
            ss << "))" << std::endl;
        }
        return ss.str();
    }

    inline void test_linear_to_layers()
    {
        const std::vector<std::pair<int, int>> input = { {2, 4}, {7, 9}, {0, 8}, {3, 11}, {0, 7}, {4, 11}, {1, 6}, {5, 10}, {2, 5}, {6, 9}, {1, 3}, {8, 10}, {0, 2}, {9, 11}, {4, 6}, {5, 7}, {2, 5}, {6, 9}, {0, 1}, {10, 11}, {3, 7}, {4, 8}, {2, 3}, {8, 9}, {1, 4}, {7, 10}, {4, 5}, {6, 7}, {1, 2}, {9, 10}, {3, 5}, {6, 8}, {2, 4}, {7, 9}, {3, 6}, {5, 8}, {3, 4}, {7, 8}, {5, 6} };
        const std::vector<std::vector<std::pair<int, int>>> layers = linear_to_layers(input);
        std::cout << layers_to_string(layers);

        // Expected the result
        //[(0, 8), (1, 6), (2, 4), (3, 11), (5, 10), (7, 9)]
        //[(0, 7), (1, 3), (2, 5), (4, 11), (6, 9), (8, 10)]
        //[(0, 2), (4, 6), (5, 7), (9, 11)]
        //[(0, 1), (2, 5), (3, 7), (4, 8), (6, 9), (10, 11)]
        //[(1, 4), (2, 3), (7, 10), (8, 9)]
        //[(1, 2), (4, 5), (6, 7), (9, 10)]
        //[(2, 4), (3, 5), (6, 8), (7, 9)]
        //[(3, 6), (5, 8)]
        //[(3, 4), (5, 6), (7, 8)]
    }
}