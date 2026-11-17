#include "wafer_map.hpp"

#include <algorithm>
#include <array>
#include <istream>
#include <stdexcept>
#include <string>
#include <utility>

namespace wafer {
namespace {

bool read_row(std::istream& input, std::string& line)
{
    line.clear();
    constexpr auto max_line = WaferMap::max_dimension * 2; // Includes optional CR.
    for (char character; input.get(character); ) {
        if (character == '\n')
            return true;
        if (line.size() == max_line)
            throw std::invalid_argument("Row is too wide.");
        line.push_back(character);
    }
    if (input.bad() || !input.eof())
        throw std::runtime_error("Unable to read wafer map.");
    return !line.empty();
}

Site parse_site(char value)
{
    switch (value) {
        case '.': return Site::outside;
        case 'P': return Site::pass;
        case 'D': return Site::defect;
        default: throw std::invalid_argument("Expected '.', 'P', or 'D'.");
    }
}

}  // namespace

WaferMap::WaferMap(std::size_t columns, std::vector<Site> cells)
    : columns_(columns), cells_(std::move(cells)) {}

Site WaferMap::at(std::size_t row, std::size_t column) const
{
    if (row >= rows() || column >= columns_)
        throw std::out_of_range("Site is outside the grid.");
    return cells_[row * columns_ + column];
}

WaferMap WaferMap::parse(std::istream& input)
{
    std::vector<Site> cells;
    std::size_t columns = 0;
    std::size_t row_number = 0;
    for (std::string line; read_row(input, line); ) {
        if (++row_number > max_dimension)
            throw std::invalid_argument("Too many rows.");
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty() || line.size() % 2 == 0)
            throw std::invalid_argument("Expected one site per comma-separated field on row " +
                                        std::to_string(row_number) + ".");
        const auto width = line.size() / 2 + 1;
        if (columns == 0)
            columns = width;
        if (width != columns)
            throw std::invalid_argument("Unequal column count on row " + std::to_string(row_number) + ".");

        for (std::size_t index = 0; index < line.size(); index += 2) {
            if (index != 0 && line[index - 1] != ',')
                throw std::invalid_argument("Expected a comma on row " + std::to_string(row_number) + ".");
            cells.push_back(parse_site(line[index]));
        }
    }
    if (row_number == 0)
        throw std::invalid_argument("Map is empty.");
    return WaferMap(columns, std::move(cells));
}

double Summary::yield_percent() const
{
    const auto inspected = passed + defective;
    return inspected == 0 ? 0.0 : 100.0 * static_cast<double>(passed) / static_cast<double>(inspected);
}

Summary analyze(const WaferMap& map)
{
    const auto width = map.columns();
    const auto height = map.rows();
    Summary summary;
    std::vector<bool> visited(width * height);
    std::vector<std::size_t> frontier;
    constexpr std::array<std::pair<int, int>, 4> neighbors{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

    for (std::size_t row = 0; row < height; ++row) {
        for (std::size_t column = 0; column < width; ++column) {
            const Site site = map.at(row, column);
            if (site == Site::outside) {
                ++summary.outside;
                continue;
            }
            if (site == Site::pass) {
                ++summary.passed;
                continue;
            }

            const auto start = row * width + column;
            if (visited[start])
                continue;
            visited[start] = true;
            frontier.clear();
            frontier.push_back(start);
            Cluster cluster{0, row, column, row, column};
            for (std::size_t next = 0; next < frontier.size(); ++next) {
                const auto current_row = frontier[next] / width;
                const auto current_column = frontier[next] % width;
                ++cluster.size;
                cluster.top = std::min(cluster.top, current_row);
                cluster.left = std::min(cluster.left, current_column);
                cluster.bottom = std::max(cluster.bottom, current_row);
                cluster.right = std::max(cluster.right, current_column);

                for (const auto& [dy, dx] : neighbors) {
                    const auto y = static_cast<int>(current_row) + dy;
                    const auto x = static_cast<int>(current_column) + dx;
                    if (y < 0 || y >= static_cast<int>(height) || x < 0 || x >= static_cast<int>(width))
                        continue;
                    const auto neighbor_row = static_cast<std::size_t>(y);
                    const auto neighbor_column = static_cast<std::size_t>(x);
                    const auto index = neighbor_row * width + neighbor_column;
                    if (!visited[index] && map.at(neighbor_row, neighbor_column) == Site::defect) {
                        visited[index] = true;
                        frontier.push_back(index);
                    }
                }
            }
            summary.defective += cluster.size;
            summary.clusters.push_back(cluster);
        }
    }
    return summary;
}

}  // namespace wafer
