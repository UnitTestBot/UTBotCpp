#ifndef UNITTESTBOT_SETTINGSCONTEXT_H
#define UNITTESTBOT_SETTINGSCONTEXT_H

#include <chrono>
#include <optional>
#include <protobuf/testgen.grpc.pb.h>
using namespace ::testsgen;

namespace testsgen {
    class SettingsContext;
}

namespace utbot {
    class SettingsContext {
    public:
        explicit SettingsContext(const testsgen::SettingsContext &settingsContext);

        SettingsContext(bool generateForStaticFunctions,
                        bool verbose,
                        int32_t timeoutPerFunction,
                        int32_t timeoutPerTest,
                        bool useDeterministicSearcher,
                        bool useStubs,
                        ErrorMode errorMode);

        const bool generateForStaticFunctions;
        const bool verbose;
        const std::optional<std::chrono::seconds> timeoutPerFunction, timeoutPerTest;
        const bool useDeterministicSearcher;
        const bool useStubs;
        ErrorMode errorMode;
    };
}


#endif // UNITTESTBOT_SETTINGSCONTEXT_H
