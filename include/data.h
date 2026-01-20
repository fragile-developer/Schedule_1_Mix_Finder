#pragma once

#include <data_structures.h>
#include <ranges>

namespace data {
    constexpr size_t size_effects = std::size(raw::effects);
    constexpr size_t size_ings    = std::size(raw::ingredients);
    constexpr size_t size_txs     = std::size(raw::rules);
    constexpr size_t size_drugs   = std::size(raw::base_products);

    extern const std::array<Effect, size_effects>   effects;
    extern const std::array<Ingredient, size_ings>  ingredients;
    extern const std::array<Transform, size_txs>    transforms;
    extern const std::array<Drug, size_drugs>       drugs;

    constexpr size_t get_max_tx_range() {
        size_t max = 0;
        for (size_t i = 0; i < size_txs; i++) {
            auto range = raw::rules[i][1];
            max = max > range ? max : range;
        }
        return max;
    }
    constexpr size_t max_tx_range = get_max_tx_range();

    constexpr auto rank_bounds() {
        auto min = raw::ingredients[0].rank;
        auto max = min;

        for (auto ing : raw::ingredients) {
            min = min > ing.rank ? ing.rank : min;
            max = max < ing.rank ? ing.rank : max;
        }
        return std::pair(min, max);
    }
    constexpr size_t min_rank     = rank_bounds().first;
    constexpr size_t max_rank     = rank_bounds().second;
    constexpr size_t size_ranks   = max_rank - min_rank + 1;

    constexpr auto rank_names = Constexpr_Span(&raw::rank_names[min_rank], size_ranks);
}

template<typename T>
constexpr mask mask_from_effect_names(const T& names) {
    mask m = 0;
    for (auto& elem : names) {
        auto str = std::string(elem);
        int i = 0;
        for (auto& eff : raw::effects) {
            if (str == eff.name) {
                m |= ((mask)1 << i);
            }
            i++;
        }
    }
    return m;
};

// in case bitmask compression is needed - combinatorics functions
uint32_t  rank_mask(mask mask);
mask      unrank_mask(uint32_t rank);