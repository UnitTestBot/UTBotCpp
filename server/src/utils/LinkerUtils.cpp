#include "LinkerUtils.h"

#include "Paths.h"
#include "StringUtils.h"
#include "exceptions/UnImplementedException.h"

namespace LinkerUtils {
    static inline const std::string STUB_SUFFIX = "_stub";

    fs::path applySuffix(const fs::path &output,
                         BuildResult::Type unitType,
                         const std::string &suffixForParentOfStubs) {
        switch (unitType) {
        case BuildResult::Type::ALL_STUBS:
            return Paths::addSuffix(output, STUB_SUFFIX);
        case BuildResult::Type::ANY_STUBS:
            return Paths::addSuffix(output, suffixForParentOfStubs);
        case BuildResult::Type::NO_STUBS:
            return output;
        case BuildResult::Type::NONE:
            throw UnImplementedException(StringUtils::stringFormat(
                "Applying suffix for file %s which has invalid type", output));
        }
    }
}
