#ifndef UNITTESTBOT_CSVREADER_H
#define UNITTESTBOT_CSVREADER_H

#include <map>
#include <string>
#include <vector>
#include <istream>
#include <utils/StringUtils.h>
#include "loguru.h"

namespace StatsUtils {
    using CSVTable = std::map<std::string, std::vector<std::string>>;
    CSVTable readCSV(std::istream &istream, char sep, bool removeSpaces = true);
}

#endif //UNITTESTBOT_CSVREADER_H
