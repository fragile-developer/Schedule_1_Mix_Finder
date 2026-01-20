#include "custom_components.h"
#include <ftxui/dom/table.hpp>

using namespace ftxui;
using namespace std::views;
using std::vector, std::array, std::string;

constexpr auto table_row_count = data::size_effects; // could make this dynamic later
using Node_Accessor = auto(*)(const Product_Node*) -> int;
struct Table_Field {
    Sort_Field id;
    std::string name;
    Node_Accessor accessor;
    bool ascending_default;
};

const Table_Field table_fields[] = {
    {Ing_Count,     "Ing Count",    [](const Product_Node* n){return (int)n->ingredient_count;},     true},
    {Profit,        "Profit",       [](const Product_Node* n){return (int)n->product.get_profit();}, false},
    {Sell_Price,    "Sell Price",   [](const Product_Node* n){return (int)n->product.sell_price;},   false},
    {Cost,          "Cost",         [](const Product_Node* n){return (int)n->product.cost;},         true}
};

auto node_comparitor(const Sort_Field field, bool ascending = false) {
    auto acc = table_fields[field].accessor;
    return [=](const Product_Node* a, const Product_Node* b) {
        return ascending ? acc(a) < acc(b) : acc(a) > acc(b);
    };
}

void sort_data(vector<Product_Node*>& data, const Sort_Field field, bool ascending = false) {
    if (data.empty()) return;
    auto partition_size = std::min(data.size(), table_row_count);
    std::partial_sort(data.begin(), data.begin() + partition_size, data.end(), node_comparitor(field, ascending));
}

Product_Tree tree_from_query(Query_State& query_state) {
    auto& ta = query_state.tree_args;
    return Product_Tree(data::drugs[ta.drug_n], ta.ing_count, ta.rank);
}

ProductTableBase::ProductTableBase(Query_State& query_state)
    : query_state_(query_state), tree_(tree_from_query(query_state)) {
        old_state_ = query_state_;

        // sort buttons
        sort_buttons_ = {};
        for (const auto& tf :table_fields) {
            auto& cs = query_state.sorting;
            auto bt = SortButton(tf.name, (int)tf.id, tf.ascending_default, (int&)cs.sort_field, cs.ascending);
            sort_buttons_.push_back(bt);
        }
        button_container_ = Container::Horizontal(sort_buttons_);
        Add(button_container_);

        // initialise data
        update_filter();
        update_sort();
};

void ProductTableBase::update() {
    auto& ta = query_state_.tree_args;
    if (ta != old_state_.tree_args) {
        if ((ta.drug_n != old_state_.tree_args.drug_n) ||
             (ta.rank   != old_state_.tree_args.rank)  ||
              (ta.ing_count > tree_.ingredient_depth && !tree_.maxed_out)) {
                update_query();
        }
        else    update_filter();
    }
    else if (query_state_.filters != old_state_.filters) {
        update_filter();
    }
    else if (query_state_.sorting != old_state_.sorting) {
        update_sort();
    }
    old_state_ = query_state_;
}

void ProductTableBase::update_query() {
    tree_ = tree_from_query(query_state_);
    update_filter();
}

void ProductTableBase::update_filter() {
    data_ = tree_.get_filtered(
        query_state_.filters.whitelist,
          query_state_.filters.blacklist,
            query_state_.tree_args.ing_count);
    update_sort();
}

void ProductTableBase::update_sort() {
    sort_data(data_, query_state_.sorting.sort_field, query_state_.sorting.ascending);
    update_rows();
    selected_row_ = 0;
}

void ProductTableBase::update_rows() {
    rows_.clear();
    rows_.push_back(headings_);

    for (auto node : data_ | take(table_row_count)) {
        vector<Element> row = {};
        for (auto& tf : table_fields) {
            auto value    = tf.accessor(node);
            auto elem     = text(std::to_string(value))| align_right;
            row.push_back(elem);
        }
        auto pretty_effects = vector<Element>();
        auto eff_inds = node->product.get_effects_indices();
        for (auto i : eff_inds) {
            pretty_effects.push_back(gui_effects[i].get_element(true));
        }
        row.push_back(hbox(pretty_effects));
        rows_.push_back(row);
    }
}

Element ProductTableBase::OnRender() {
    Element view;
    update();
    selected_node_ = data_.empty() ? nullptr : data_[selected_row_];
    result_count_  = (int)data_.size();

    if (true) {
        update_headings();
        Table table = Table(rows_);
        table.SelectColumns(0, -1).SeparatorVertical(LIGHT);
        table.SelectColumns(1, -1).Border(LIGHT);
        if (result_count_) {
            if (hovered_) {
                table.SelectRow(hovered_row_ + 1).Decorate(bgcolor(Color::DarkBlue));
            }
            table.SelectRow(1 + selected_row_).Decorate(bold);
            table.SelectRow(1 + selected_row_).Decorate(bgcolor(Color::RGB(50,50,50)));
        }
        table.SelectAll().Border(LIGHT);
        table.SelectRectangle(0, -1, 0, 0).Border(LIGHT);

        view = table.Render();
    }

    return view | reflect(box_);
}

bool ProductTableBase::OnEvent(Event event) {
    if (event.is_mouse()) return OnMouseEvent(event);

    return button_container_->OnEvent(event);
}

bool ProductTableBase::OnMouseEvent(Event event) {
    auto x = event.mouse().x;
    auto y = event.mouse().y;
    
    auto rows_box = Box(box_);
    rows_box.y_min += 3;
    rows_box.y_max -= 1;

    if (rows_box.Contain(x, y)) {
        hovered_     = true;
        hovered_row_ = std::clamp(y - rows_box.y_min, 0, (int)data_.size()-1);
        if (event.mouse().button == Mouse::Left && event.mouse().motion == Mouse::Pressed) {
            selected_row_ = hovered_row_;
        }
        return true;
    }
    hovered_ = false;
    
    return button_container_->OnEvent(event);
}

void ProductTableBase::update_headings() {
    headings_.clear();
    for (const auto& bt : sort_buttons_) {
        headings_.push_back(bt->Render());
    }
    headings_.push_back(text("Effects") | bold | size(WIDTH, GREATER_THAN, 16));
    if (!rows_.empty()) rows_[0] = headings_;
}

Component ProductTableBase::get_product_detail() {
    return ProductDetail(selected_node_, tree_, result_count_);
}

Component ProductTable(Query_State& query_state) {
    return Make<ProductTableBase>(query_state);
}