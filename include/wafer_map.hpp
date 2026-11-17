#pragma once

#include <cstddef>
#include <iosfwd>
#include <vector>

namespace wafer {

enum class Site { outside, pass, defect };

class WaferMap {
public:
    static WaferMap parse(std::istream& input);
    static constexpr std::size_t max_dimension = 1024;

    std::size_t rows() const { return cells_.size() / columns_; }
    std::size_t columns() const { return columns_; }
    Site at(std::size_t row, std::size_t column) const;

private:
    WaferMap(std::size_t columns, std::vector<Site> cells);
    std::size_t columns_;
    std::vector<Site> cells_;
};

struct Cluster {
    std::size_t size;
    std::size_t top;
    std::size_t left;
    std::size_t bottom;
    std::size_t right;

    bool operator==(const Cluster&) const = default;
};

struct Summary {
    std::size_t passed = 0;
    std::size_t defective = 0;
    std::size_t outside = 0;
    std::vector<Cluster> clusters;

    double yield_percent() const;
};

Summary analyze(const WaferMap& map);

}  // namespace wafer
