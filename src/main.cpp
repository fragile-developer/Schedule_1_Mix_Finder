#include "gui.h"
#include <iostream>

constexpr int DEFAULT_RANK  = data::size_ranks - 1;  // Highest Rank - Hustler III
constexpr int DEFAULT_DRUG  = 5;                    // Cocaine

#ifdef NDEBUG
    constexpr int DEFAULT_COUNT = 8; // 8 is quick to calculate while still useful
#else                      
    constexpr int DEFAULT_COUNT = 4; // debug builds are much slower
#endif

int main(int argc, char* argv[]) {

    // the worst cmd arg parsing ever
    int drug_n = argc > 1 ? std::stoi(argv[1]) : DEFAULT_DRUG;
    int ing_c  = argc > 2 ? std::stoi(argv[2]) : DEFAULT_COUNT;
    int rank_n = argc > 3 ? std::stoi(argv[3]) : DEFAULT_RANK;

    run_gui(drug_n, ing_c, rank_n);

    return 0;
}