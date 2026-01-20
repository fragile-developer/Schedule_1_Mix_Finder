#include "data_structures.h"
#include "data.h"
#include <cmath>

int count_effects(mask m){
    int count = 0;
    while (m != 0) {
        if (m & 1) {
            count++;
        }
        m >>= 1;
    }
    return count;
}

uint16_t Product::calc_sell_price(int base_price) {
    double mult = this->get_mult();
    this->sell_price = (uint16_t)std::round(mult*base_price);
    return this->sell_price;
}

std::vector<int> Product::get_effects_indices() const {
    std::vector<int> indices = {};
    int i = 0;
    for (auto effect : data::effects) {
        if (effect_mask & effect.mask) indices.push_back(i);
        i++;
    }
    return indices;
}

std::vector<const char*> Product::get_effect_names() const{
    std::vector<const char*> names = {};
    for (auto i : this->get_effects_indices()) {
        names.push_back(raw::effects[i].name);
    }
    return names;
}

float Product::get_mult() const{
    float mult = 1.0;
    for (auto effect : data::effects) {
        if (effect_mask & effect.mask) mult += effect.mult;
    }
    return mult;
}

float Product::get_addiction() const{
    float addiction = 0.0;
    for (auto effect : data::effects) {
        if (effect_mask & effect.mask) addiction += effect.addiction;
    }
    return addiction;
}

int Product::get_profit() const{
    return this->sell_price - this->cost;
}

Product Product::update_product(const Ingredient& ing) const {
    auto original = effect_mask;
    bool blocked[data::max_tx_range] = {0};
    auto working = original;

    // Phase 1
    for (std::size_t i = 0; i < ing.txs.len; i++) {
        auto tx = ing.txs[i];
        blocked[i] = false; // fix junk data
        if (!(original & tx.from)) continue;
        if (original & tx.to) {
            blocked[i] = true;
            continue;
        } 
        working ^= (tx.from | tx.to);
    }

    // Phase 2
    for (std::size_t i = 0; i < ing.txs.len; i++) {
        if (!blocked[i]) continue;
        auto tx = ing.txs[i];
        if (!(working & tx.from)) continue;
        if (working & tx.to)      continue;
        working ^= (tx.from | tx.to);
    }

    auto new_product = *this;
    // Add base
    if (this->effect_count < 8 && !(working & ing.base_effect)){
        working |= ing.base_effect;
        new_product.effect_count++;
    }

    new_product.cost += ing.price;
    new_product.effect_mask = working;

    return new_product;
}

RGBA::RGBA(const char* hex_string) {
    auto n = std::sscanf(hex_string, "#%2x%2x%2x%2x", &r, &g, &b, &a);
    if (n < 4) {
        a = 255;
        if (n != 3) throw std::invalid_argument("Color must be in format #RRGGBB");
    }
};