# Overview
This is a tool for finding optimal mixes in the game [Schedule 1](https://www.scheduleonegame.com/).

It is written in C++ and uses a terminal GUI for its interface. It is designed to be used in tandum with the https://schedule1.tools/ website.

The tool uses a breadth-first-search (BFS) algorithm to search all the possible mixes that meet the specified criteria. This can produce tens of millions of potential results and the search tree is cached to disc under ```./tree_cache/```. Be aware this can store up to 600MB on disc per distinct query. 

For a desired effect combination, the tool finds both the cheapest recipe and the recipe with the fewest steps. The UI allows you to require/exclude specific effects as well as set the player rank (and thus available ingredients).

This is a personal code project. It wasn't intended to be a public facing repo and I used it as an excuse to learn C++ and FTXUI. It grew organically as I learned new features of the language and libraries. As a result, it is very rough and not well architected. Hopefully I can come back and refactor it in the future. Use at your own risk.

## Build Steps
This project uses CMake to fetch dependencies and build.

Run these to set up the initial CMake folders
```sh
cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake -S . -B build/debug   -G Ninja -DCMAKE_BUILD_TYPE=Debug
```
Build with the following:
```sh
cmake --preset release                # config preset -- resets the config
cmake --build --preset build-release  # build preset  -- builds with that preset
```
Run with:
```sh
.\build\release\bin\MixFinder.exe
```
###  Helpful commands
Use this to clear the cache when adding new files (or just delete the entire folder)
```sh
rm build/Debug/CMakeCache.txt
rm -rf build/Debug/CMakeFiles
cmake --preset debug
```
VSCode can set a default build preset, swap between them with 
```
Press Ctrl+Shift+P
Run CMake: Select Configure Preset
```
## Dependencies
This project uses FTXUI for its GUI
https://github.com/ArthurSonzogni/ftxui

It uses abseil for the faster flat_hash_map
https://github.com/abseil/abseil-cpp.git

# To-do
There are a lot of ways to improve the tool and code base. Here's what I will prioritise if I have time:

## Features

1. Add mushrooms. <br> I haven't played since the mushrooms update but it should be trivial too add them as a drug type.
2. Include quality and other sales multiplier to the profit calculation. <br> Right now the profit assumes baseline sell value which is nearly never the actual sell value. Being able to toggle multipliers will give the user a better idea of their profit margins and the optimal mixes.
3. Select/Clear All button for the filters. 
4. Clear cache option. 
5. When a drug is selected show the fastest and the cheapest recipe simultaneously. 

## Codebase
1. Separate the query from the UI. <br> 
The product table currently owns the query handling logic. This should really be a separate API. <br> 
The API can even be split into a separate modular build so that another UI can run over the top of it.
2. The GUI should really be a class. <br> 
It grew from a basic FTXUI "hello world" example. Now that I understand the library better I can encapsulate a lot more of the render logic.
3. The checklists should also be a class.
4. The query currently prints its calculation status to the terminal, blocking and obscuring the GUI. It should instead log to a message queue and provide a read-only view to its progress.
5. The UI should not freeze when calculating a new query, it should instead display a loading screen and allow the user to change their mind.
6. A lot of the C++ is quite poor quality and needlessly complex. <br> The include heirarchy can be tidied up. <br> I probably don't need the ```constexpr``` everywhere. Loading the data at runtime is trivial and makes the code much more readable.
7. The multi-threading optimisations still have room for improvement. The biggest bottleneck is the de-duplication of new mixes.
8. There are GPU techniques that can signigicantly speed up calculations. A CUDA build would be a really interesting stretch goal.
