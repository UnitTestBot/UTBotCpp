#include <map>
#include <string>
#include <vector>
#include <istream>
#include <utils/StringUtils.h>
#include "loguru.h"

namespace StatsUtils {
    using CSVTable = std::map<std::string, std::vector<std::string>>;
     CSVTable readCSV(std::istream &istream, char sep, bool removeSpaces = true) {
        std::map<std::string, std::vector<std::string>> parsedCSV;
        std::string header;
        std::getline(istream, header);
        std::vector<std::string> keys = StringUtils::split(header, sep);
        if (removeSpaces) {
            keys = CollectionUtils::transform(keys, [](std::string s){ StringUtils::trim(s); return s; });
        }
        for (const auto &key: keys) {
            parsedCSV[key] = {};
        }
        while (true) {
            std::string row;
            std::getline(istream, row);
            if (row.empty()) {
                break;
            }
            std::vector<std::string> values = StringUtils::split(row, sep);
            if (values.size() != keys.size()) {
                LOG_S(WARNING) << "Cannot parse CSV. Invalid format";
                return {};
            }
            for (std::size_t i = 0; i < values.size(); i++) {
                if (removeSpaces) {
                    StringUtils::trim(values[i]);
                }
                parsedCSV[keys[i]].push_back(values[i]);
            }
        }
        return parsedCSV;
    }
}
