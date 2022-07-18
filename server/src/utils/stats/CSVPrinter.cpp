#include "CSVPrinter.h"

#include <utility>

#include "loguru.h"

namespace printer {
    CSVPrinter::CSVPrinter(const std::vector <std::string> &header, std::string sep) :
            sep(std::move(sep)), numColumns(header.size()) {
        printRow(header);
    }

    bool CSVPrinter::printRow(const std::vector<std::string> &row) {
        if (row.size() != numColumns) {
            LOG_S(WARNING) << "Failed to create csv: row size and header size differs";
            return false;
        }
        ss << StringUtils::joinWith(row, sep) << '\n';
        return true;
    }

    bool CSVPrinter::printTotal(const std::vector<std::string> &total) {
        return printRow(total);
    }
}
