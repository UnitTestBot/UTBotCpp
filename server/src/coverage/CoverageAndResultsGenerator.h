/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COVERAGEGENERATOR_H
#define UNITTESTBOT_COVERAGEGENERATOR_H

#include "Coverage.h"
#include "CoverageTool.h"
#include "SettingsContext.h"
#include "TestRunner.h"
#include "streams/WriterUtils.h"
#include "streams/coverage/CoverageAndResultsWriter.h"

#include <protobuf/testgen.pb.h>

#include <utility>

class CoverageAndResultsGenerator : public TestRunner {
public:
    CoverageAndResultsGenerator(
        testsgen::CoverageAndResultsRequest const *coverageAndResultsRequest,
        CoverageAndResultsWriter *coverageAndResultsWriter);

    grpc::Status generate(bool withCoverage, testsgen::SettingsContext &settingsContext);
    grpc::Status generate(bool withCoverage, utbot::SettingsContext &settingsContext);

    const Coverage::CoverageMap &getCoverageMap();
    const nlohmann::json &getTotals();
private:
    CoverageAndResultsWriter* coverageAndResultsWriter = nullptr;

    Coverage::CoverageMap coverageMap{};
    nlohmann::json totals{};

    void collectCoverage();
    void showErrors() const;
};


#endif // UNITTESTBOT_COVERAGEGENERATOR_H