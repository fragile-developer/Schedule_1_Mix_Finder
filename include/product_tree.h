#pragma once

#include "data.h"
#include "enumerate.h"
#include <filesystem>

#include <ankerl/unordered_dense.h>

struct Product_Node;
using Product_List = Constexpr_Span<Product_Node>;

using entry     = struct Entry{uint32_t fastest_ind; uint32_t cheapest_ind; uint16_t cheapest_cost; uint16_t fewest_ing;};
using ing_vec   = std::vector<const Ingredient*>;
using Node_Span = std::span<Product_Node>;
using Lookup    = ankerl::unordered_dense::map<mask, entry>;

class Product_Tree {
    friend struct Product_Node;
    
public:
    ing_vec ingredients;
    int max_rank;
    int ingredient_depth;
    Drug base_drug;
    
    bool maxed_out; // flag for when search is exhausted

    Product_Tree(Drug base_drug, int max_ingredients = 8, int max_rank = data::size_ranks - 1);
    std::vector<Product_Node*> get_filtered(mask whitelist = 0, mask blacklist = 0, int ingredient_count = -1);
    std::vector<Product_Node> store;

private:
    Lookup lookup;
    std::vector<size_t> n_ing_counts;
    unsigned int num_threads_;

    void initialise();
    void grow_to_ingredient_count(int ing_count);
    size_t grow_next_level(const Node_Span prev, const size_t offset);
    bool attempt_add_node(const Product_Node& new_product);
    void process_chunk(const Node_Span input, Node_Span output) const;
    void reduce_chunk(const Node_Span input, std::vector<Product_Node>& output) const;
    void rebuild_lookup();

    void write_to_cache (const std::filesystem::path& path) const;
    void read_from_cache(const std::filesystem::path& path, int max_ingredients);
};

struct Product_Node {
    Product  product;
    uint32_t previous_node;
    uint16_t last_ingredient;
    uint16_t ingredient_count;

    Product_Node update(const Ingredient& ing, size_t index) const;
    ing_vec get_ingredients(const Product_Tree& tree) const;
    std::string get_product_hash(const Product_Tree& tree) const;
};