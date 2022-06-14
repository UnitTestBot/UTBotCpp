#ifndef UNITTESTBOT_COVERAGE_H
#define UNITTESTBOT_COVERAGE_H

#include "utils/CollectionUtils.h"

#include <protobuf/testgen.grpc.pb.h>

#include <unordered_map>
#include <vector>

namespace Coverage {
    struct FileCoverage {
        struct SourcePosition {
            uint32_t line;
            uint32_t character;
        };
        struct SourceRange {
            SourcePosition start;
            SourcePosition end;
        };
        struct SourceLine {
            uint32_t line;
            bool operator< (const SourceLine& r) const {
                return (line < r.line);
            }
        };
        std::vector<SourceRange> coveredRanges;
        std::vector<SourceRange> uncoveredRanges;
        std::set<SourceLine> fullCoverageLines;
        std::set<SourceLine> partialCoverageLines;
        std::set<SourceLine> noCoverageLines;
        std::set<SourceLine> noCoverageLinesBorders;
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
