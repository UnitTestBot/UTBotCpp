#ifndef UTBOTCPP_CSVPRINTER_H
#define UTBOTCPP_CSVPRINTER_H

#include "utils/StringUtils.h"

namespace printer {
    class CSVPrinter {
    public:
        explicit CSVPrinter(const std::vector<std::string> &header, std::string sep);
        bool printRow(const std::vector<std::string> &row);
        bool printTotal(const std::vector<std::string> &total);
        std::stringstream getStream() {
            std::stringstream out(ss.str());
            return out;
        };
    private:
        std::stringstream ss;
        std::size_t numColumns;
        std::string sep;
    };
}


#endif //UTBOTCPP_CSVPRINTER_H
