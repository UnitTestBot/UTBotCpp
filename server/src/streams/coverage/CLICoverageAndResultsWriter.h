#ifndef UNITTESTBOT_CLICOVERAGEANDRESULTSWRITER_H
#define UNITTESTBOT_CLICOVERAGEANDRESULTSWRITER_H

#include "CoverageAndResultsWriter.h"

class CLICoverageAndResultsWriter : public CoverageAndResultsWriter {
public:
    explicit CLICoverageAndResultsWriter(const fs::path &resultsDirectory);

    virtual void writeResponse(const utbot::ProjectContext &projectContext,
                               const Coverage::TestResultMap &testsResultMap,
                               const Coverage::CoverageMap &coverageMap,
                               const nlohmann::json &totals,
                               std::optional<std::string> errorMessage) override;


private:
    fs::path resultsDirectory;
};


#endif // UNITTESTBOT_CLICOVERAGEANDRESULTSWRITER_H
