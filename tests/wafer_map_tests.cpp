#include "wafer_map.hpp"

#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

void check(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

wafer::Summary analyze(std::string_view source)
{
    std::istringstream input{std::string(source)};
    return wafer::analyze(wafer::WaferMap::parse(input));
}

void expect_invalid(std::string_view source)
{
    try {
        analyze(source);
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("Malformed map was accepted.");
}

void run(std::string_view test)
{
    if (test == "counts") {
        const auto summary = analyze("P,D,.\r\nP,P,.\r\n");
        check(summary.passed == 3 && summary.defective == 1 && summary.outside == 2, "Incorrect site counts.");
        check(std::abs(summary.yield_percent() - 75.0) < 1e-9, "Incorrect yield.");
    } else if (test == "connectivity") {
        const auto summary = analyze("D,D,.\n.,D,.\n.,.,D");
        check(summary.clusters.size() == 2, "Diagonal contact must not merge components.");
        check(summary.clusters[0] == wafer::Cluster{3, 0, 0, 1, 1}, "Incorrect component bounds.");
        check(summary.clusters[1] == wafer::Cluster{1, 2, 2, 2, 2}, "Incorrect isolated component.");
    } else if (test == "boundaries") {
        const auto summary = analyze(".,D\nD,.");
        check(summary.clusters.size() == 2, "Opposite row boundaries must not connect.");
        check(analyze("D\nD").clusters.front().size == 2, "Vertical neighbors must connect.");
    } else if (test == "empty") {
        const auto summary = analyze(".,.\n.,.");
        check(summary.clusters.empty() && summary.passed + summary.defective == 0, "Expected no inspected sites.");
        check(analyze("P").defective == 0, "A passing map has no defects.");
    } else if (test == "malformed") {
        for (const auto* input : {"", "\n", "P,D\nP", "P,,D", "P,D,", "P X", "P,X", "P\n\n"})
            expect_invalid(input);
    } else if (test == "maximum") {
        std::string row = "D";
        for (std::size_t column = 1; column < wafer::WaferMap::max_dimension; ++column)
            row += ",D";
        check(analyze(row).clusters.front().size == wafer::WaferMap::max_dimension, "Maximum width rejected.");
        expect_invalid(row + ",D");
        std::string tall;
        for (std::size_t line = 0; line < wafer::WaferMap::max_dimension; ++line)
            tall += "D\n";
        check(analyze(tall).clusters.front().size == wafer::WaferMap::max_dimension, "Maximum height rejected.");
        expect_invalid(tall + "D\n");
    } else {
        throw std::runtime_error("Unknown test case.");
    }
}

}  // namespace

int main(int argc, char* argv[])
{
    try {
        if (argc != 2)
            throw std::runtime_error("Supply one test case name.");
        run(argv[1]);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
