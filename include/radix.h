#include <iostream>
#include <vector>
#include <algorithm>

// Define Sortable type
using Sortable = std::pair<uint64_t, size_t>;

// Counting sort for a specific byte (8-bit chunk)
void counting_sort(std::vector<Sortable>& input, std::vector<Sortable>& output, size_t byte_pos) {
    std::vector<int> count(256, 0);  // 256 buckets for base-256 sorting

    // Count the occurrences of each 8-bit chunk (0 to 255)
    for (const auto& pair : input) {
        uint8_t byte = (pair.first >> (byte_pos * 8)) & 0xFF; // Extract 8 bits
        count[byte]++;
    }

    // Compute the starting index for each bucket
    for (size_t i = 1; i < 256; ++i) {
        count[i] += count[i - 1];
    }

    // Place the elements in the correct bucket based on the byte
    for (int i = (int)input.size() - 1; i >= 0; --i) {
        uint8_t byte = (input[i].first >> (byte_pos * 8)) & 0xFF;
        output[--count[byte]] = input[i];
    }
}

// Radix sort function (8-bit chunk version)
void radix_sort(std::vector<Sortable>& arr) {
    std::vector<Sortable> input = arr;
    std::vector<Sortable> output(arr.size());
    
    // Perform counting sort for every byte position (64 bits = 8 bytes)
    for (size_t byte_pos = 0; byte_pos < 8; ++byte_pos) {
        counting_sort(input, output, byte_pos);
        std::swap(input, output);  // Swap input and output buffers
    }

    // If the output buffer is not the original input, move the result into the original buffer
    if (&input != &arr) {
        std::swap(input, arr);  // Final move
    }
}