#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include "hutils.h"

namespace sh::tools {

	// extended compare/exchange element
	struct Ece {
		int first;
		int second;
		std::string colour;
	};

	namespace {

		constexpr float dot_size = 4.f;
		constexpr float part_sep_space = 0.4f;
		constexpr float layer_sep_space = 0.8f;

		[[nodiscard]] inline bool has_overlap(const Ece& e, const std::vector<Ece>& d) {
			for (const Ece& swap : d) {
				if (swap.second <= e.first) {
					// ok
				}
				else if (swap.first >= e.second) {
					// ok
				}
				else {
					return true;
				}
			}
			return false;
		}

		[[nodiscard]] inline std::vector<std::vector<Ece>> partition(const std::vector<Ece>& layer) {
			std::vector<std::vector<Ece>> result;
			for (const Ece& swap : layer) {
				bool stored = false;
				for (std::vector<Ece>& part : result) {
					if (!has_overlap(swap, part)) {
						part.push_back(swap);
						stored = true;
						break;
					}
				}
				if (!stored) {
					std::vector<Ece> new_part;
					new_part.push_back(swap);
					result.push_back(std::move(new_part));
				}
			}
			return result;
		}

		[[nodiscard]] inline float draw_layer(
			const std::vector<Ece>& layer,
			int layer_id,
			int channels,
			float x,
			bool draw_box,
			std::stringstream& ss)
		{
			const float x_start = x;
			const auto partitions = partition(layer);
			const float width = layer_sep_space + ((partitions.size() - 1) * part_sep_space) + layer_sep_space;

			if (draw_box) {
				const std::string x1 = std::to_string(x);
				const std::string x2 = std::to_string(x + width);
				const std::string y1 = "-0.5";
				const std::string y2 = std::to_string(channels - 1) + ".5";

				ss << "\\fill[gray!15](" << x1 << "," << y1 << ")--(" << x2 << "," << y1 << ")--(" << x2 << "," << y2 << ")--(" << x1 << "," << y2 << ")--cycle;" << std::endl;
			}
			x += layer_sep_space;

			for (const auto& part : partition(layer)) {
				for (const auto& swap : part) {
					const std::string colour = (swap.colour.length() == 0) ? "black" : swap.colour;
					ss << "\\filldraw[" << colour << "](" << x << "," << swap.first << ") circle(" << dot_size << "pt); " << std::endl;
					ss << "\\filldraw[" << colour << "](" << x << "," << swap.second << ") circle(" << dot_size << "pt); " << std::endl;
					ss << "\\draw[thick," << colour << "](" << x << "," << swap.first << ")--(" << x << "," << swap.second << ");" << std::endl;
				}
				x += part_sep_space;
			}
			ss << "\\node at(" << x_start + width/2 << ",-0.3){" << layer_id << "};" << std::endl;
			return x + layer_sep_space - part_sep_space;
		}

		[[nodiscard]] inline std::string to_latex(
			const std::vector<std::vector<Ece>>& layers,
			int channels
		) {
			std::stringstream ss;

			ss << "\\documentclass[tikz, border = 5mm]{standalone}" << std::endl;
			ss << "\\begin{document}" << std::endl;
			ss << "\\begin{tikzpicture}" << std::endl;

			bool draw_box = true;
			float x = 0.f;
			int layer_id = 0;
			for (const std::vector<Ece>& layer : layers) {
				ss << std::endl << "% layer " << layer_id << std::endl;
				x = draw_layer(layer, layer_id, channels, x, draw_box, ss);
				draw_box = !draw_box;
				layer_id++;
			}

			ss << std::endl;

			for (int i = 0; i < channels; ++i) {
				ss << "\\node at(-0.3," << i << "){" << i << "};" << std::endl;
				ss << "\\draw[thick](0," << i << ")--(" << x << "," << i << ");" << std::endl;
			}
			ss << std::endl;

			ss << "\\end{tikzpicture}" << std::endl;
			ss << "\\end{document}" << std::endl;
			return ss.str();
		}
	}

	inline std::vector<std::vector<Ece>> convert_to_ece(
		const std::vector<std::vector<Pair_t>>& d,
		const std::string& colour = "black"
	) {
		std::vector<std::vector<Ece>> result;
		for (const std::vector<Pair_t>& layer : d) {
			std::vector<Ece> new_layer;
			for (const Pair_t& swap : layer) {
				Ece ece;
				ece.first = swap.lo;
				ece.second = swap.hi;
				ece.colour = colour;
				new_layer.push_back(std::move(ece));
			}
			result.push_back(std::move(new_layer));
		}
		return result;
	}
	
	inline std::vector<std::vector<Ece>> convert_to_ece(
		const std::vector<std::vector<std::pair<int, int>>>& d,
		const std::string& colour = "black"
	) {
		std::vector<std::vector<Ece>> result;
		for (const std::vector<std::pair<int, int>>& layer : d) {
			std::vector<Ece> new_layer;
			for (const std::pair<int, int>& swap : layer) {
				Ece ece;
				ece.first = swap.first;
				ece.second = swap.second;
				ece.colour = colour;
				new_layer.push_back(std::move(ece));
			}
			result.push_back(std::move(new_layer));
		}
		return result;
	}

	inline void write_latex(
		const std::vector<std::vector<Ece>>& layers,
		int channels,
		const std::string& filename
	) {
		std::ofstream outputFile(filename);
		if (!outputFile.is_open()) {
			std::cerr << "Failed to open the file " << filename << " for writing." << std::endl;
			return;
		}
		outputFile << to_latex(layers, channels);
		outputFile.close();
	}

	inline void write_latex(
		const std::vector<std::vector<std::pair<int, int>>>& layers,
		int channels,
		const std::string& filename
	) {
		write_latex(convert_to_ece(layers), channels, filename);
	}
}

