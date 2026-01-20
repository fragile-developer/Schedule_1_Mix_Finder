#pragma once

namespace raw {
    struct Effect {
        const char* name;
        const char* abbreviation;
        double      mult;
        double      addiction;
        const char* color;
    };

    struct Ingredient {
        const char* name;
        int         index;
        const char* abbreviation;
        int         price;
        int         effect;
        int         rank;
        int         span[2];
    };

    struct Base_Product {
        const char* name;
        const char* abbreviation;
        int         price;
        int         base_effect;
        int         rank;
    };

    inline constexpr Effect effects[] = {
        {"Calming"           , "Ca", 0.10, 0.00, "#fdba74"},
        {"Euphoric"          , "Eu", 0.18, 0.23, "#fde68a"},
        {"Focused"           , "Fc", 0.16, 0.10, "#75F1FD"},
        {"Munchies"          , "Mu", 0.12, 0.10, "#C96E57"},
        {"Paranoia"          , "Pa", 0.00, 0.00, "#f87171"},
        {"Refreshing"        , "Re", 0.14, 0.10, "#bef264"},
        {"Smelly"            , "Sm", 0.00, 0.00, "#84cc16"},
        {"Calorie-Dense"     , "Cd", 0.28, 0.10, "#e879f9"},
        {"Disorienting"      , "Di", 0.00, 0.00, "#FE7551"},
        {"Energizing"        , "En", 0.22, 0.34, "#a3e635"},
        {"Gingeritis"        , "Gi", 0.20, 0.00, "#fb923c"},
        {"Sedating"          , "Se", 0.26, 0.00, "#818cf8"},
        {"Sneaky"            , "Sn", 0.24, 0.33, "#a8a29e"},
        {"Toxic"             , "To", 0.00, 0.00, "#a3e635"},
        {"Athletic"          , "At", 0.32, 0.61, "#7dd3fc"},
        {"Balding"           , "Ba", 0.30, 0.00, "#c79232"},
        {"Foggy"             , "Fo", 0.36, 0.10, "#94a3b8"},
        {"Laxative"          , "La", 0.00, 0.10, "#a16207"},
        {"Seizure-Inducing"  , "Si", 0.00, 0.00, "#FEE900"},
        {"Slippery"          , "Sl", 0.34, 0.31, "#7dd3fc"},
        {"Spicy"             , "Sp", 0.38, 0.67, "#f87171"},
        {"Bright-Eyed"       , "Be", 0.40, 0.20, "#67e8f9"},
        {"Glowing"           , "Gl", 0.48, 0.47, "#85E459"},
        {"Jennerising"       , "Je", 0.42, 0.34, "#e879f9"},
        {"Schizophrenia"     , "Sc", 0.00, 0.00, "#645AFD"},
        {"Thought-Provoking" , "Tp", 0.44, 0.37, "#f9a8d4"},
        {"Tropic Thunder"    , "Tt", 0.46, 0.80, "#fdba74"},
        {"Anti-Gravity"      , "Ag", 0.54, 0.61, "#3b82f6"},
        {"Cyclopean"         , "Cy", 0.56, 0.10, "#FEC174"},
        {"Electrifying"      , "El", 0.50, 0.23, "#22d3ee"},
        {"Explosive"         , "Ex", 0.00, 0.00, "#ef4444"},
        {"Long Faced"        , "Lf", 0.52, 0.61, "#fde047"},
        {"Shrinking"         , "Sh", 0.60, 0.34, "#B6FEDA"},
        {"Zombifying"        , "Zo", 0.58, 0.60, "#71AB5D"}
    };

    inline constexpr Ingredient ingredients[] = {
        {"Cuke"              ,  0, "A",  2,  9,  5, { 0,  7}},
        {"Flu Medicine"      ,  1, "B",  5, 11,  8, { 7, 10}},
        {"Gasoline"          ,  2, "C",  5, 13,  9, {17, 11}},
        {"Donut"             ,  3, "D",  3,  7,  5, {28,  7}},
        {"Energy Drink"      ,  4, "E",  6, 14, 10, {35,  9}},
        {"Mouth Wash"        ,  5, "F",  4, 15,  7, {44,  4}},
        {"Motor Oil"         ,  6, "G",  6, 19, 11, {48,  5}},
        {"Banana"            ,  7, "H",  2, 10,  5, {53,  9}},
        {"Chili"             ,  8, "I",  7, 20, 13, {62,  6}},
        {"Iodine"            ,  9, "J",  8, 23, 15, {68,  6}},
        {"Paracetamol"       , 10, "K",  3, 12,  5, {74, 10}},
        {"Viagor"            , 11, "L",  4, 26,  6, {84,  5}},
        {"Horse Semen"       , 12, "M",  9, 31, 17, {89,  4}},
        {"Mega Bean"         , 13, "N",  7, 16, 12, {93, 10}},
        {"Addy"              , 14, "O",  9, 25, 16, {103,  5}},
        {"Battery"           , 15, "P",  8, 21, 14, {108,  6}}
    };

    inline constexpr Base_Product base_products[] = {
        {"OG Kush"           , "OH", 35,  0,  5},
        {"Sour Diesel"       , "SL", 35,  5,  8},
        {"Green Crack"       , "GK", 35,  9, 11},
        {"Grandaddy Purple"  , "GE", 35, 11, 13},
        {"Meth"              , "MH", 70, -1, 10},
        {"Cocaine"           , "CE", 150, -1, 30}
    };

    inline constexpr int rules[][2] = {
        /* Cuke         */ { 1, 17}, {16, 28}, {10, 25}, { 3, 14}, {19,  3}, {12,  4}, {13,  1},
        /* Flu Medicine */ {14,  3}, { 0, 21}, {28, 16}, {29,  5}, { 1, 13}, { 2,  0}, {17,  1}, { 3, 19}, {32,  4}, {25, 10},
        /* Gasoline     */ { 8, 22}, {29,  8}, { 9,  1}, { 1, 20}, {10,  6}, {23, 12}, {17, 16}, { 3, 11}, { 4,  0}, {32,  2}, {12, 26},
        /* Donut        */ {27, 19}, {15, 12}, { 7, 30}, { 2,  1}, {23, 10}, { 3,  0}, {32,  9},
        /* Energy Drink */ { 8, 29}, { 1,  9}, { 2, 32}, {16, 17}, {22,  8}, {24, 15}, {11,  3}, {20,  1}, {26, 12},
        /* Mouth Wash   */ { 0, 27}, { 7, 12}, {30, 11}, { 2, 23},
        /* Motor Oil    */ { 9,  3}, { 1, 11}, {16, 13}, { 3, 24}, { 4, 27},
        /* Banana       */ { 0, 12}, {28,  9}, { 8,  2}, { 9, 25}, { 2, 18}, {31,  5}, { 4, 23}, { 6, 27}, {13,  6},
        /* Chili        */ {27, 26}, {14,  1}, {17, 31}, { 3, 13}, {32,  5}, {12, 21},
        /* Iodine       */ { 0, 15}, { 7, 10}, { 1, 18}, {16,  4}, { 5, 25}, {13, 12},
        /* Paracetamol  */ { 0, 19}, {29, 14}, { 9,  4}, { 2, 10}, {16,  0}, {22, 13}, { 3, 27}, { 4, 15}, {20, 21}, {13, 26},
        /* Viagor       */ {14, 12}, { 8, 13}, { 1, 21}, {17,  0}, {32, 10},
        /* Horse Semen  */ {27,  0}, {10,  5}, {18,  9}, {25, 29},
        /* Mega Bean    */ {14, 17}, { 0, 22}, { 9, 28}, { 2,  8}, {23,  4}, {18,  2}, {32, 29}, {19, 13}, {12,  0}, {25,  9},
        /* Addy         */ {30,  1}, {16,  9}, {22,  5}, {31, 29}, {11, 10},
        /* Battery      */ {28, 22}, {29,  1}, { 1, 33}, {17,  7}, { 3, 26}, {32,  3}
    };

    inline constexpr const char* rank_names[] = {
        "Street Rat I",     "Street Rat II",    "Street Rat III",   "Street Rat IV",    "Street Rat V",
        "Hoodlum I",        "Hoodlum II",       "Hoodlum III",      "Hoodlum IV",       "Hoodlum V",
        "Peddler I",        "Peddler II",       "Peddler III",      "Peddler IV",       "Peddler V",
        "Hustler I",        "Hustler II",       "Hustler III",      "Hustler IV",       "Hustler V",
        "Bagman I",         "Bagman II",        "Bagman III",       "Bagman IV",        "Bagman V",
        "Enforcer I",       "Enforcer II",      "Enforcer III",     "Enforcer IV",      "Enforcer V",
        "Shot Caller I",    "Shot Caller II",   "Shot Caller III",  "Shot Caller IV",   "Shot Caller V",
        "Block Boss I",     "Block Boss II",    "Block Boss III",   "Block Boss IV",    "Block Boss V",
        "Underlord I",      "Underlord II",     "Underlord III",    "Underlord IV",     "Underlord V",
        "Baron I",          "Baron II",         "Baron III",        "Baron IV",         "Baron V",
        "Kingpin"
    };
}