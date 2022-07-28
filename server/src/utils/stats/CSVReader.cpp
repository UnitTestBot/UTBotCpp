#include "CSVReader.h"

namespace StatsUtils {
    CSVTable readCSV(std::istream &istream, char sep, bool removeSpaces) {
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
        std::string row;
        while (std::getline(istream, row)) {
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
