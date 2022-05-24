/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */


#ifndef UNITTESTBOT_CLICOVERAGEANDRESULTSWRITER_H
#define UNITTESTBOT_CLICOVERAGEANDRESULTSWRITER_H

#include "CoverageAndResultsWriter.h"

class CLICoverageAndResultsWriter : public CoverageAndResultsWriter {
public:
    explicit CLICoverageAndResultsWriter(const fs::path &resultsDirectory);

    virtual void writeResponse(const Coverage::TestStatusMap &testsStatusMap,
                               const Coverage::CoverageMap &coverageMap,
                               const nlohmann::json &totals,
                               std::optional<std::string> errorMessage) override;


private:
    fs::path resultsDirectory;
};


#endif // UNITTESTBOT_CLICOVERAGEANDRESULTSWRITER_H
