#pragma once

#include <iostream>
#include <array>
#include <vector>

#include <raw_data.h>

// forward declarations
struct Product;
struct Ingredient;
struct Effect;
struct Transform;

template <typename T>
struct Constexpr_Span { // mimics std::span but available as constexpr (should be uneeded C++23)
    const T* ptr;
    std::size_t len;

    constexpr const T* begin() const { return ptr; }
    constexpr const T* end() const { return ptr + len; }
    constexpr std::size_t size() const { return len; }
    constexpr const T& operator[](std::size_t i) const { return ptr[i]; }
};

using Tx_Span = Constexpr_Span<Transform>;
using mask    =  unsigned long long;

struct RGBA {
    int r, g, b, a;
    RGBA(const char* hex_string);
};
struct Product {
    mask        effect_mask;
    uint8_t     effect_count;
    uint16_t    cost;
    uint16_t    sell_price;

    uint16_t    calc_sell_price(int base_price);
    float       get_mult() const;
    float       get_addiction() const;
    int         get_profit() const;
    Product     update_product(const Ingredient& ing) const;

    std::vector<int> get_effects_indices() const;
    std::vector<const char*> get_effect_names() const;
};

struct Ingredient {
    const char* name;
    int         price;
    mask        base_effect;
    int         index;
    int         rank;
    Tx_Span     txs;
};

struct Effect {
    mask   mask;
    const char* name;
    const char* abbreviation;
    const char* color;
    float       mult;
    float       addiction;
};

struct Transform {
    mask  from;
    mask  to;
};

struct Drug {
    const char* name;
    int         price;
    mask        base_effects;
};

int count_effects(mask m);