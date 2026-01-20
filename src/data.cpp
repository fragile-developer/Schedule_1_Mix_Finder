#include "data.h"

namespace data {
    template<std::size_t N>
    constexpr std::array<Effect, N> get_effects() {
        std::array<Effect, N> result = {};
        for (size_t i = 0; i < N; i++) {
            auto raw = raw::effects[i];
            Effect e;

            e.mask          = (mask)1 << i;
            e.name          = raw.name;
            e.abbreviation  = raw.abbreviation;
            e.mult          = (float)raw.mult;
            e.addiction     = (float)raw.addiction;
            e.color         = raw.color;

            result[i] = e;
        }
        return result;
    }

    template<std::size_t N>
    constexpr std::array<Transform, N> get_transforms() {
        std::array<Transform, N> result = {};
        for (size_t i = 0; i < N; i++) {
            Transform tx = {1, 1};
            tx.from <<= raw::rules[i][0];
            tx.to   <<= raw::rules[i][1];
            result[i] = tx;
        }
        return result;
    }
    
    template<std::size_t N>
    constexpr std::array<Ingredient, N> get_ingredients() {
        std::array<Ingredient, N> result = {};
        for (size_t i = 0; i < N; i++) {
            Ingredient ing = {};
            ing.name        = raw::ingredients[i].name;
            ing.price       = raw::ingredients[i].price;
            ing.base_effect = effects[raw::ingredients[i].effect].mask;
            ing.index       = (int)i;
            ing.rank        = raw::ingredients[i].rank - min_rank;

            int tx_start    = raw::ingredients[i].span[0];
            int tx_len      = raw::ingredients[i].span[1];
            ing.txs.ptr     = &data::transforms[tx_start];
            ing.txs.len     = tx_len;

            result[i] = ing;
        }
        return result;
    }

    template<std::size_t N>
    constexpr std::array<Drug, N> get_drugs() {
        std::array<Drug, N> result = {};
        for (size_t i = 0; i < N; i++) {
            Drug drug = {};
            drug.name = raw::base_products[i].name;
            drug.price = raw::base_products[i].price;
            int raw_effect = raw::base_products[i].base_effect;
            drug.base_effects = raw_effect >= 0 ? (mask)1 << raw_effect : 0;

            result[i] = drug;
        }
        return result;
    }

    constexpr std::array<Effect, size_effects>  effects     = get_effects<size_effects>();
    constexpr std::array<Transform, size_txs>   transforms  = get_transforms<size_txs>();
    constexpr std::array<Ingredient, size_ings> ingredients = get_ingredients<size_ings>();
    constexpr std::array<Drug, size_drugs>      drugs       = get_drugs<size_drugs>();
}

constexpr uint32_t binom(int n, int k) {
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    uint64_t res = 1;
    for (int i = 1; i <= k; ++i)
        res = res * (n - (k - i)) / i;
    return static_cast<uint32_t>(res);
}

uint32_t rank_mask(mask mask) {
    uint32_t rank = 0;
    int set_bits_seen = 0;

    for (int i = 0; i < data::size_effects; ++i) {
        if (mask & (1ULL << i)) {
            set_bits_seen++;

            // Add combinations for all smaller sets
            for (int j = 0; j < set_bits_seen; ++j)
                rank += binom(i, j + 1);  // combinations where next bit is here
        }
    }

    // Also include total count of combinations for fewer than `popcount(mask)` bits
    int pop = std::popcount(mask & ((1ULL << data::size_effects) - 1));
    for (int i = 0; i < pop; ++i)
        rank += binom(data::size_effects, i);

    return rank;
}

mask unrank_mask(uint32_t rank) {
    uint64_t mask = 0;
    int total = 0;

    // Find number of bits set (k) such that sum_{i=0}^{k-1} C(34, i) <= rank
    int k = 0;
    uint32_t sum = 0;
    for (; k <= 8; ++k) {
        uint32_t next = binom(data::size_effects, k);
        if (rank < sum + next) break;
        sum += next;
    }
    rank -= sum;  // Now rank is within C(34, k)

    // Reconstruct combination of size `k`
    int x = data::size_effects;
    for (int i = k; i > 0; --i) {
        x--;
        while (binom(x, i) > rank)
            x--;
        mask |= (1ULL << x);
        rank -= binom(x, i);
    }

    return mask;
}

// precompute binom_table?
//uint32_t binom_table[35][9];  // C(n, k), n ∈ [0, 34], k ∈ [0, 8]