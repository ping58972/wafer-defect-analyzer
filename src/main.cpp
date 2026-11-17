#include "wafer_map.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: wafer-inspect <wafer-map.csv>\n";
        return 2;
    }
    try {
        std::ifstream input(argv[1]);
        if (!input)
            throw std::runtime_error("Cannot open input file.");

        const auto map = wafer::WaferMap::parse(input);
        const auto summary = wafer::analyze(map);
        std::cout << "rows=" << map.rows() << " cols=" << map.columns()
                  << " passed=" << summary.passed << " defective=" << summary.defective
                  << " outside=" << summary.outside << " clusters=" << summary.clusters.size() << '\n';
        if (summary.passed + summary.defective == 0)
            std::cout << "yield=N/A\n";
        else
            std::cout << "yield=" << std::fixed << std::setprecision(2)
                      << summary.yield_percent() << "%\n";

        std::cout << "cluster,size,top,left,bottom,right\n";
        std::size_t id = 0;
        for (const auto& cluster : summary.clusters)
            std::cout << ++id << ',' << cluster.size << ',' << cluster.top << ',' << cluster.left
                      << ',' << cluster.bottom << ',' << cluster.right << '\n';
    } catch (const std::exception& error) {
        std::cerr << "wafer-inspect: " << error.what() << '\n';
        return 1;
    }
}
