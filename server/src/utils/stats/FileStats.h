#ifndef UTBOTCPP_FILESTATS_H
#define UTBOTCPP_FILESTATS_H

#include "utils/CollectionUtils.h"
#include "utils/stats/CSVPrinter.h"
#include "ProjectContext.h"

/*
 * FileStats implements different statistics grouped by file
 * T class implements statistics for one file
 * T class should have operator += and method with the following signature:
 * std::vector<std::string> toStrings(bool isTotal) const
 */
template <typename T>
class FileStats {
protected:
    utbot::ProjectContext projectContext;
    CollectionUtils::MapFileTo<T> statsMap;
public:
    T getTotal() {
        T total;
        for (const auto &[filePath, fileStats]: statsMap) {
            total += fileStats;
        }
        return total;
    }
    virtual std::vector<std::string> getHeader() = 0;
    virtual std::vector<std::string> getTotalStrings() {
        return getTotal().toStrings();
    };
    explicit FileStats(utbot::ProjectContext projectContext) : projectContext(std::move(projectContext)) {}
    printer::CSVPrinter toCSV() {
        std::vector<std::string> header = { "File" };
        CollectionUtils::extend(header, getHeader());
        printer::CSVPrinter printer(header, ",");
        for (const auto &[filePath, fileStats] : statsMap) {
            std::vector<std::string> row = { fs::relative(filePath, projectContext.projectPath).string() };
            CollectionUtils::extend(row, fileStats.toStrings() );
            if (!printer.printRow(row)) {
                return printer;
            }
        }
        std::vector<std::string> totalRow = { "Total:" };
        CollectionUtils::extend(totalRow, getTotalStrings() );
        printer.printTotal(totalRow);
        return printer;
    }
};

#endif //UTBOTCPP_FILESTATS_H
