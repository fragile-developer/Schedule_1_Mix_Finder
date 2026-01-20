#include "gui.h"
#include <iostream>


int main(int argc, char* argv[]) {

    // the worst cmd arg parsing ever
    int drug_n = argc > 1 ? std::stoi(argv[1]) : 5;                         // Cocaine
    int ing_c  = argc > 2 ? std::stoi(argv[2]) : 8;                        // 12 ingredient mix
    int rank_n = argc > 3 ? std::stoi(argv[3]) : data::size_ranks - 1;    // Highest Rank - Hustler III?

    run_gui(drug_n, ing_c, rank_n);

    return 0;
}