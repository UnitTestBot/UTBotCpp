/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COVERAGE_H
#define UNITTESTBOT_COVERAGE_H

#include "utils/CollectionUtils.h"

#include <protobuf/testgen.grpc.pb.h>

#include <unordered_map>
#include <vector>

namespace Coverage {
    struct FileCoverage {
        struct SourcePosition {
            int line;
            int character;
        };
        struct SourceRange {
            SourcePosition start;
            SourcePosition end;
        };
        std::vector<SourceRange> coveredRanges;
        std::vector<SourceRange> uncoveredRanges;
    };
    using CoverageMap = CollectionUtils::MapFileTo<FileCoverage>;

    class FileTestsStatus : public std::unordered_map<std::string, testsgen::TestStatus> {
    };

    class TestStatusMap : public CollectionUtils::MapFileTo<FileTestsStatus> {
    public:
        int getNumberOfTests();
    };

}

#endif //UNITTESTBOT_COVERAGE_H
