#include "SettingsContext.h"

#include <protobuf/testgen.grpc.pb.h>

namespace utbot {
    SettingsContext::SettingsContext(bool generateForStaticFunctions,
                    bool verbose,
                    int32_t timeoutPerFunction,
                    int32_t timeoutPerTest,
                    bool useDeterministicSearcher,
                    bool useStubs) : SettingsContext(generateForStaticFunctions,
                                                     verbose,
                                                     timeoutPerFunction,
                                                     timeoutPerTest,
                                                     useDeterministicSearcher,
                                                     useStubs,
                                                     false) {}

    SettingsContext::SettingsContext(bool generateForStaticFunctions,
                                     bool verbose,
                                     int32_t timeoutPerFunction,
                                     int32_t timeoutPerTest,
                                     bool useDeterministicSearcher,
                                     bool useStubs,
                                     bool genSarif)
        : generateForStaticFunctions(generateForStaticFunctions),
          verbose(verbose),
          timeoutPerFunction(timeoutPerFunction > 0
                                 ? std::make_optional(std::chrono::seconds{ timeoutPerFunction })
                                 : std::nullopt),
         timeoutPerTest(timeoutPerTest > 0
         ? std::make_optional(std::chrono::seconds{ timeoutPerTest })
         : std::nullopt),
          useDeterministicSearcher(useDeterministicSearcher), useStubs(useStubs), genSarif(genSarif) {
    }
    SettingsContext::SettingsContext(const testsgen::SettingsContext &settingsContext)
        : SettingsContext(settingsContext.generateforstaticfunctions(),
                          settingsContext.verbose(),
                          settingsContext.timeoutperfunction(),
                          settingsContext.timeoutpertest(),
                          settingsContext.usedeterministicsearcher(),
                          settingsContext.usestubs(),
                          settingsContext.gensarif()) {
    }
}
