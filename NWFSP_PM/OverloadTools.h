#pragma once
#include<numeric>
#include"solution.h"

// OVERLOAD Hashmap for Solution_type
uint32_t murmurhash(const uint8_t* data, std::size_t len, uint32_t seed = 0) {
	uint32_t h = seed;
	if (len > 3) {
		const uint32_t* key_x4 = (const uint32_t*)data;
		std::size_t i = len >> 2;

		do {
			uint32_t k = *key_x4++;
			k *= 0xcc9e2d51;
			k = (k << 15) | (k >> 17);
			k *= 0x1b873593;

			h ^= k;
			h = (h << 13) | (h >> 19);
			h = (h * 5) + 0xe6546b64;
		} while (--i);

		data = (const uint8_t*)key_x4;
	}

	if (len & 3) {
		std::size_t i = len & 3;
		uint32_t k = 0;
		data = &data[i - 1];

		do {
			k <<= 8;
			k |= *data--;
		} while (--i);

		k *= 0xcc9e2d51;
		k = (k << 15) | (k >> 17);
		k *= 0x1b873593;
		h ^= k;
	}

	h ^= len;

	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

namespace std {
	template<>
	struct hash<NWFSP_Solution> {
		std::size_t operator()(const NWFSP_Solution& solu) const {
			return murmurhash(reinterpret_cast<const uint8_t*>(solu.sequence.data()), solu.sequence.size() * sizeof(int));
		}
	};
}





namespace std {
	template <>
	struct hash<std::vector<int>> {
		std::size_t operator()(const std::vector<int>& vec) const {
			std::size_t hash = 0;
			std::size_t prime = 31;
			for (std::size_t i = 0; i < vec.size(); ++i) {
				hash = hash * prime + std::hash<int>()(vec[i]);
			}
			return hash;
		}
	};
}




