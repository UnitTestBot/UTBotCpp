#ifndef UNITTESTBOT_SARIF_H
#define UNITTESTBOT_SARIF_H

#include <string>

namespace sarif {
    struct Sarif {
    public:
        const static inline std::string sarif_klee_prefix = "__sarif_";
        const static inline std::string sarif_klee_extension = ".json";
        const static inline std::string prefix_for_json_path = "// THIS LINE SHOULDN'T BE AT END, path of klee-sarif: ";
    };
}

#endif //UNITTESTBOT_SARIF_H
