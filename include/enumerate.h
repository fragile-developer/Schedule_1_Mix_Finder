#include <ranges>
#include <utility>
#include <iterator>

#if defined(__cpp_lib_ranges_enumerate) && __cpp_lib_ranges_enumerate >= 202202L
    using std::views::enumerate;
#else
// enumerate adaptor for C++20 ranges
namespace my_views {
    template <std::ranges::input_range R>
    requires std::ranges::view<R>
    class enumerate_view : public std::ranges::view_interface<enumerate_view<R>> {
    private:
        R base_;

        struct iterator {
            std::ranges::iterator_t<R> current_;
            std::size_t index_ = 0;

            using iterator_category = std::input_iterator_tag;
            using value_type = std::pair<std::size_t, std::ranges::range_reference_t<R>>;

            iterator() = default;
            explicit iterator(std::ranges::iterator_t<R> it, std::size_t idx = 0) : current_(it), index_(idx) {}

            value_type operator*() const {
                return {index_, *current_};
            }

            iterator& operator++() {
                ++current_;
                ++index_;
                return *this;
            }

            void operator++(int) { ++*this; }

            bool operator==(const iterator& other) const {
                return current_ == other.current_;
            }
            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }
        };

    public:
        enumerate_view() = default;
        explicit enumerate_view(R base) : base_(std::move(base)) {}

        iterator begin() {
            return iterator{std::ranges::begin(base_), 0};
        }

        iterator end() {
            return iterator{std::ranges::end(base_)};
        }
    };

    // Helper function to create enumerate_view
    template <std::ranges::input_range R>
    enumerate_view<std::ranges::views::all_t<R>> enumerate(R&& r) {
        return enumerate_view<std::ranges::views::all_t<R>>{std::ranges::views::all(std::forward<R>(r))};
    }
}

using namespace my_views;
#endif