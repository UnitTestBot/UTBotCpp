#include "SettingsContext.h"

#include <protobuf/testgen.grpc.pb.h>
using  namespace ::testsgen;

namespace utbot {
    SettingsContext::SettingsContext(bool generateForStaticFunctions,
                                     bool verbose,
                                     int32_t timeoutPerFunction,
                                     int32_t timeoutPerTest,
                                     bool useDeterministicSearcher,
                                     bool useStubs,
                                     ErrorMode errorMode)
        : generateForStaticFunctions(generateForStaticFunctions),
          verbose(verbose),
          timeoutPerFunction(timeoutPerFunction > 0
                                 ? std::make_optional(std::chrono::seconds{ timeoutPerFunction })
                                 : std::nullopt),
         timeoutPerTest(timeoutPerTest > 0
         ? std::make_optional(std::chrono::seconds{ timeoutPerTest })
         : std::nullopt),
          useDeterministicSearcher(useDeterministicSearcher), useStubs(useStubs),
          errorMode(errorMode) {
    }
    SettingsContext::SettingsContext(const testsgen::SettingsContext &settingsContext)
        : SettingsContext(settingsContext.generateforstaticfunctions(),
                          settingsContext.verbose(),
                          settingsContext.timeoutperfunction(),
                          settingsContext.timeoutpertest(),
                          settingsContext.usedeterministicsearcher(),
                          settingsContext.usestubs(),
                          settingsContext.errormode()) {
    }
}
