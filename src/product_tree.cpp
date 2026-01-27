#include "product_tree.h"
#include "radix.h"

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <execution>
#include <cassert>

using namespace std::views;
using std::vector;
namespace fs = std::filesystem;

#pragma pack(push, 1)
struct Cache_Node {
    // compact node for storing to file
    uint64_t  effects           : data::size_effects;
    uint32_t  prev_node         : 28;
    uint8_t   last_ingredient   : 5;
    uint8_t   ing_count         : 6;
    uint16_t  cost              : 10;

    Cache_Node() {};
    Cache_Node(const Product_Node& node) {
        this->effects           = node.product.effect_mask;
        this->prev_node         = node.previous_node;
        this->last_ingredient   = node.last_ingredient;
        this->ing_count         = node.ingredient_count;
        this->cost              = node.product.cost;
    };

    Product_Node to_product_node() const{
        Product_Node node{};
        node.previous_node          = prev_node;
        node.last_ingredient        = last_ingredient;
        node.ingredient_count       = ing_count;

        node.product.effect_mask    = effects;
        node.product.effect_count   = count_effects(effects);
        node.product.cost           = cost;
        // node.product.sell_price     = sell_price;
        return node;
    }
};
#pragma pack(pop)

const auto a = sizeof(Product_Node);
const auto b = sizeof(Cache_Node);

auto get_cache_name(Drug base_drug, int max_rank) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%s-%s.blob", base_drug.name, data::rank_names[max_rank]);
    return std::string(buf);
}

Product_Tree::Product_Tree(Drug base_drug, int max_ingredients, int max_rank) {
    this->base_drug         = base_drug;
    this->max_rank          = max_rank;
    this->ingredient_depth  = 0;
    this->num_threads_      = std::thread::hardware_concurrency();
    this->maxed_out        = false;

    this->ingredients       = ing_vec();
    for (auto& ing : data::ingredients) {
        if (ing.rank <= max_rank) {
            this->ingredients.push_back(&ing);
        }
    }

    fs::path cache_dir("./tree_cache/");
    if (!fs::is_directory(cache_dir)) {
        fs::create_directories(cache_dir);
    }
    
    int cache_ing_count = 0;
    
    fs::path cache_path = cache_dir;
    cache_path += get_cache_name(base_drug, max_rank);
    auto found = fs::exists(cache_path);

    auto start = std::chrono::high_resolution_clock::now(); // start clock
    // either initialise new tree or load from existing
    if (!found) {
        initialise();
        cache_ing_count = 0;
        grow_to_ingredient_count(max_ingredients);
    }
    else {
        read_from_cache(cache_path, max_ingredients);
        cache_ing_count = this->ingredient_depth;
        if (cache_ing_count < max_ingredients && !this->maxed_out) {
            // only rebuild the lookup if more products need to be generated
            rebuild_lookup();
            grow_to_ingredient_count(max_ingredients);
        }
    }
   
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Execution Time: " << duration.count() << " seconds." << std::endl;

    // if the cache was smaller than current tree (or didn't exist) write new cache
    if (this->ingredient_depth > cache_ing_count) {
        // to-do: run this on separate thread
        write_to_cache(cache_path);
    }
}

void Product_Tree::initialise() {
    Product base_product      = {};
    base_product.effect_mask  = base_drug.base_effects;
    base_product.effect_count = count_effects(base_product.effect_mask);
    base_product.sell_price   = base_drug.price;

    Product_Node base_node    = {};
    base_node.product         = base_product;

    this->lookup        = {{base_product.effect_mask, {0, 0}}};
    this->store         = {{base_node}};
    this->n_ing_counts  = {1};
}

struct Cache_Header { // might want to expand this in the future
    int max_ingredients;
    bool maxed_out;
    int products_per_ingredient[0];
};

template<typename T>
void write_vector_to_file(std::vector<T> v, std::ofstream& outfile) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    outfile.write(reinterpret_cast<const char*>(v.data()), v.size() * sizeof(T));
}

template<typename T>
std::vector<T> read_vector_from_file(size_t size, std::ifstream& infile) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    
    std::vector<T> v(size);
    infile.read(reinterpret_cast<char*>(v.data()), size * sizeof(T));

    return v;
}

void Product_Tree::write_to_cache(const fs::path& path) const {
    std::ofstream outfile(path, std::ios::binary);
    if (!outfile) throw std::ios_base::failure("Failed to open file for writing");
    
    // write header (trivial for now)
    Cache_Header header;
    header.max_ingredients = this->ingredient_depth;
    header.maxed_out = this->maxed_out;
    outfile.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // write ingredient counts
    write_vector_to_file(this->n_ing_counts, outfile);

    // write product nodes (lookup needs to be reconstructed on read)
    auto shrunk = std::vector<Cache_Node>(store.size());
    for (const auto& [i, node] : enumerate(store)) {
        shrunk[i] = Cache_Node(node);
    }
    write_vector_to_file(shrunk, outfile);

    outfile.close();
}

void Product_Tree::read_from_cache(const fs::path& path, int max_ingredients) {
    std::ifstream infile(path, std::ios::binary);
    if (!infile) throw std::ios_base::failure("Failed to open file for reading");

    // read header
    Cache_Header header;
    infile.read(reinterpret_cast<char*>(&header), sizeof(header));
    // read in ing counts
    auto all_counts = read_vector_from_file<size_t>((size_t)(header.max_ingredients + 1), infile);
    
    this->n_ing_counts = {};
    size_t total_nodes = 0;
    for (auto n : all_counts | take(max_ingredients + 1)) {
        total_nodes += n;
        n_ing_counts.push_back(n);
    }
    this->ingredient_depth = (int)n_ing_counts.size() - 1;
    this->maxed_out = header.maxed_out && max_ingredients >= header.max_ingredients;

    // read in nodes
    auto shrunk = read_vector_from_file<Cache_Node>(total_nodes, infile);
    this->store = std::vector<Product_Node>(total_nodes);

    auto unpack_chunk = [](std::span<Cache_Node> cached, Node_Span full, int base_price) {
        for (const auto& [cache_node, full_node] : zip(cached, full)){
            full_node = cache_node.to_product_node();
            full_node.product.calc_sell_price(base_price);
        }
    };

    vector<std::thread> threads;

    for (size_t i = 0; i < num_threads_; i++) {
        auto chunk_size = shrunk.size()/num_threads_;
        auto count = i < num_threads_ - 1 ? chunk_size : shrunk.size() - i*(chunk_size);

        auto in_span  = std::span<Cache_Node>(&shrunk[i*chunk_size], count);
        auto out_span = Node_Span(&store[i*chunk_size], count);
        auto base_price = this->base_drug.price;
        threads.emplace_back([=]() {unpack_chunk(in_span, out_span, base_price);});
    }

    for (auto& t : threads) {
        t.join();
    }
    threads.clear();
}

void Product_Tree::rebuild_lookup() {
    // rebuild lookup
    this->lookup = {};
    lookup.reserve(store.size());
    for (const auto& [i, node] : enumerate(this->store)) {
        auto key = node.product.effect_mask;
        auto ind = (uint32_t)i;
        if (!lookup.contains(key)) {
            lookup[key] = {ind, ind, node.product.cost, node.ingredient_count};
            continue;
        }
        auto& en = lookup.at(key);
        assert(node.product.cost < en.cheapest_cost); // only reason to find a second node
        en.cheapest_ind  = ind;
        en.cheapest_cost = node.product.cost;
    }
}

void Product_Tree::grow_to_ingredient_count(int ing_count) {
    size_t last_size  = this->n_ing_counts.back();
    size_t last_start = this->store.size() - last_size;

    while (this->ingredient_depth < ing_count) {

        store.reserve(store.size() + last_size*this->ingredients.size()); // Excessive memory reservation

        auto this_size = grow_next_level(Node_Span(&store[last_start], last_size), last_start);
        if (!this_size) {
            this->maxed_out = true; // mark for no further growth attempts
            break;
        }

        this->n_ing_counts.push_back(this_size);
        this->ingredient_depth = (int)n_ing_counts.size() - 1;

        std::cout << std::format("Found {} new products with {} ingredients\n",  this_size, this->ingredient_depth);
        
        last_start += last_size;
        last_size = this_size;

        // pre-calculate sell prices of new products
        for (size_t i = last_start; i < store.size(); i++) {
            auto& prod = this->store[i].product;
            prod.sell_price = prod.calc_sell_price(this->base_drug.price);
        }
    }
}

bool Product_Tree::attempt_add_node(const Product_Node& new_node) {
    // attempts to add new node to tree. returns true if node count increases
    const auto& new_product = new_node.product;
    const auto& key         = new_node.product.effect_mask;
    const uint32_t new_ind  = (int)store.size();

    if (!lookup.contains(key)) {
        // if product doesn't already exist, add new node and set as cheapest/fastest
        store.push_back(new_node);
        lookup[key] = {new_ind, new_ind, new_node.product.cost, new_node.ingredient_count};
        return true;
    }
    else {
        auto& existing = lookup.at(key);

        if (new_product.cost < existing.cheapest_cost) {
            auto& cheapest = store[existing.cheapest_ind];
            existing.cheapest_cost = new_product.cost;
            if (new_node.ingredient_count <= cheapest.ingredient_count) {
                cheapest = new_node; // replace existing node in-place
                return false;        // even though node was updated, count did not increase
            }
            else {
                store.push_back(new_node);                  // create new node. this can lead to more than 2 nodes for a given key
                existing.cheapest_ind  = new_ind;           // update cheapest ind, leave fastest as-is
                return true;
            }
        }
        // no point checking if new node is faster that existing as nodes are generated ascending ingredient count
        return false;
    }
    // unreachable
    return false;
}

void Product_Tree::process_chunk(const Node_Span input, Node_Span output) const {
    auto offset    = &input[0] - &store[0];
    auto ing_count = ingredients.size();

    for (size_t i = 0; i < input.size(); i++) {
        const auto& node = input[i];
        for (size_t ing_ind = 0; ing_ind < ing_count; ing_ind++) {
            const Ingredient& ing_ref = *(ingredients[ing_ind]);
            auto new_node = node.update(ing_ref, i + offset);
            output[ing_count*i + ing_ind] = std::move(new_node);
        }
    }
}

vector<Sortable> radix_chunk(const Node_Span& span) {
    auto sortables = vector<Sortable>(span.size());
    for (size_t i = 0; i < span.size(); i++) {
        auto& n = span[i];
        auto key = n.product.effect_mask << 16 | (mask)n.product.cost;
        sortables[i] = {key, i};
    }
    
    // radix_sort(sortables); // is slower than std::sort most of the time
    std::sort(sortables.begin(), sortables.end(), [&](const Sortable& a, const Sortable& b){return a.first < b.first;});
    return sortables;
}

void Product_Tree::reduce_chunk(const Node_Span input, vector<Product_Node>& output) const {
    // sort_chunk(input);
    auto sortables = radix_chunk(input);

    // reduce vector to only nodes that can be added to final tree
    output.reserve(input.size()/4); // rough estimate
    mask last_key = 0;
    for (const auto& sb : sortables){
        auto key  = sb.first >> 16;
        auto cost = sb.first & ((1<<16)-1);
        if (key != last_key) {
            last_key = key;
            
            if (this->lookup.contains(key)){
                if (this->lookup.at(key).cheapest_cost <= cost) continue;
            }
            auto& node = input[sb.second];
            output.push_back(std::move(node));
        }
    }
}

size_t Product_Tree::grow_next_level(const Node_Span prev, const size_t offset) {
    const auto ing_count = this->ingredients.size();

    auto new_nodes = vector<Product_Node>(prev.size() * ing_count);
    if (prev.size() < 1000000) { // single thread for small lists
        process_chunk(Node_Span(prev.begin(), prev.size()), Node_Span(new_nodes.begin(), new_nodes.size()));
        size_t new_products = 0;
        for (const auto& new_node : new_nodes){
            new_products += attempt_add_node(new_node);
        }
        return new_products;
    }

    std::cout << "Generating ...\n";
    vector<std::thread> threads;
    vector<Node_Span> updated_chunks;
    
    for (size_t i = 0; i < num_threads_; i++) {
        auto chunk_size = prev.size()/num_threads_;
        auto count = i < num_threads_ - 1 ? chunk_size : prev.size() - i*(chunk_size);

        auto in_span  = Node_Span(&prev[i*chunk_size], count);
        auto out_span = Node_Span(&new_nodes[i*(chunk_size)*ing_count], count*ing_count);
        updated_chunks.push_back(out_span);

        threads.emplace_back([this, in_span, out_span]() {(this->process_chunk)(in_span, out_span);});
    }

    for (auto& t : threads) {
        t.join();
    }
    threads.clear();

    std::cout << "Sorting and Reducing ...\n";
    vector<vector<Product_Node>> reduced_chunks(num_threads_);

    for (size_t i = 0; i < num_threads_; i++) {
        Node_Span in_span = updated_chunks[i];

        vector<Product_Node>& output = reduced_chunks[i];
        threads.emplace_back([this, in_span, &output]() {(this->reduce_chunk)(in_span, output);});
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Adding ...\n";
    size_t new_products = 0;
    for (const auto& chunk : reduced_chunks) {
        for (const auto& new_node : chunk){
            new_products += attempt_add_node(new_node);
        }
    }
    return new_products;
}

std::vector<Product_Node*> Product_Tree::get_filtered(const mask whitelist, const mask blacklist, const int ingredient_count) {

    auto white_filter = [&](Product_Node& nd){return (nd.product.effect_mask & whitelist) == whitelist;};
    auto black_filter = [&](Product_Node& nd){return !(nd.product.effect_mask & blacklist);};
    auto ing_limit    = [&](Product_Node& nd){return ingredient_count < 0 || nd.ingredient_count <= ingredient_count;};
    auto to_pointer   = [] (Product_Node& nd){return (Product_Node*)&nd;};

    auto iter = store | take_while(ing_limit) | filter(white_filter) | filter(black_filter) | transform(to_pointer);

    return std::ranges::to<std::vector<Product_Node*>>(iter);
}

Product_Node Product_Node::update(const Ingredient& ing, size_t index) const {
    auto new_product = product.update_product(ing);

    auto new_node    = Product_Node();
    new_node.product          = new_product;
    new_node.ingredient_count = ingredient_count + 1;
    new_node.last_ingredient  = ing.index;
    new_node.previous_node    = (int)index;

    return new_node;
}

ing_vec Product_Node::get_ingredients(const Product_Tree& tree) const { 
    Product_Node node = *this;

    ing_vec ingredients = {};
    while (node.ingredient_count > 0) {
        const Ingredient* ing = &data::ingredients[node.last_ingredient];
        ingredients.push_back(ing);
        node = tree.store[node.previous_node];
    }

    std::reverse(ingredients.begin(), ingredients.end());

    return ingredients;
}

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64_encode(const std::string &in) {
    std::string out;
    int val=0, valb=-6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::string Product_Node::get_product_hash(const Product_Tree& tree) const {
    auto ingredients = this->get_ingredients(tree);
    std::string hash = std::string(tree.base_drug.name) + ':';
    for (auto& ing : ingredients) {
        hash += 'A' + ing->index;
    }
    return base64_encode(hash);
}