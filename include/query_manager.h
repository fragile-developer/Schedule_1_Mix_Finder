#pragma once

#include "data.h"
#include <compare>

enum Sort_Field : int {
    Ing_Count,
    Profit,
    Sell_Price,
    Cost,
};

enum Run_State : int {
    Ready    = 0,
    Running  = 1,
    Finished = 2
};

struct Query_State {
    struct QS_Query {
        int drug_n      = 5;
        int ing_count   = 8;
        int rank        = data::size_ranks - 1;

        auto operator<=>(const QS_Query&) const = default;
    } tree_args;
    struct QS_Filter {
        mask whitelist  = 0;
        mask blacklist  = 0;

        auto operator<=>(const QS_Filter&) const = default;
    } filters;
    struct QS_Sort {
        Sort_Field sort_field = Profit;
        bool       ascending  = false;
        float      bonus_mult = 1.0f;

        auto operator<=>(const QS_Sort&) const = default;
    } sorting;

    auto operator<=>(const Query_State&) const = default;
};

class Query_Manager {
public:
    Query_State query_state{};

    Query_Manager(Query_State state);
    
    void check_for_update();
private:
    Query_State last_processed_state_{};


};