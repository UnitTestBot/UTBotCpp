#ifndef UNITTESTBOT_FETCHER_H
#define UNITTESTBOT_FETCHER_H

#include "FetcherUtils.h"
#include "clang-utils/Matchers.h"
#include "clang-utils/SourceFileChainedCallbacks.h"
#include "exceptions/CompilationDatabaseException.h"
#include "printers/TestsPrinter.h"
#include "types/Types.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <grpcpp/grpcpp.h>

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Fetcher {
    using DeclarationMatcher = clang::ast_matchers::DeclarationMatcher;
    using MatchFinder = clang::ast_matchers::MatchFinder;
    using MatchCallback = clang::ast_matchers::MatchFinder::MatchCallback;

public:
    friend class StructJustDeclMatchCallback;
    friend class TypeDeclsMatchCallback;
    friend class FunctionDeclsMatchCallback;
    friend class GlobalVariableUsageMatchCallback;
    friend class ArraySubscriptFetcherMatchCallback;
    friend class ReturnStmtFetcherMatchCallback;
    friend class IncludeFetchSourceFileCallbacks;
    friend class IncludeFetchPPCallbacks;

    friend class TypesResolver;

    class Options {
    public:
        enum class Value {
            TYPE = (1 << 0),
            FUNCTION = (1 << 1),
            GLOBAL_VARIABLE_USAGE = (1 << 2),
            ARRAY_USAGE = (1 << 3),
            INCLUDE = (1 << 4),
            FUNCTION_NAMES_ONLY = (1 << 5),
            RETURN_TYPE_NAMES_ONLY = (1 << 6),
            WRAPPER = (1 << 7),
            ALL = TYPE | FUNCTION | GLOBAL_VARIABLE_USAGE | ARRAY_USAGE | INCLUDE
        } value;

        Options(Value value); // NOLINT(google-explicit-constructor)

        bool has(Value other) const;
    };

    explicit Fetcher(Options options,
                     const std::shared_ptr<CompilationDatabase> &compilationDatabase,
                     tests::TestsMap &tests,
                     types::TypeMaps *types,
                     size_t *maximumAlignment,
                     const fs::path &compileCommandsJsonPath,
                     bool fetchFunctionBodies);

    void fetch();

    void fetchWithProgress(const ProgressWriter *progressWriter,
                           std::string const &message,
                           bool ignoreDiagnostics = false);

    typedef CollectionUtils::MapFileTo<std::unordered_set<std::string>> FileToStringSet;
private:
    Options options;

    tests::TestsMap *const projectTests;
    types::TypeMaps *const projectTypes;
    size_t *const maximumAlignment;
    fs::path buildRootPath;

    std::shared_ptr<FileToStringSet> structsToDeclare = std::make_shared<FileToStringSet>();
    std::shared_ptr<FileToStringSet> structsDeclared = std::make_shared<FileToStringSet>();
public:
    std::shared_ptr<FileToStringSet> getStructsToDeclare() const;
private:
    // For functions
    bool fetchFunctionBodies;

    // For arrays
    std::set<int64_t> returnVariables;

    ClangToolRunner clangToolRunner;
    std::vector<std::unique_ptr<MatchCallback>> matchCallbacks;
    SourceFileChainedCallbacks sourceFileCallbacks;
    MatchFinder finder;

    template <class Callback, class Matcher, typename... Args>
    void addMatcher(Matcher const &matcher, Args &&...args) {
        auto callback = std::make_unique<Callback>(this, std::forward<Args>(args)...);
        matchCallbacks.push_back(std::move(callback));
        finder.addMatcher(matcher, matchCallbacks.back().get());
    }

    void postProcess() const;
};

inline Fetcher::Options::Value operator|(Fetcher::Options::Value a, Fetcher::Options::Value b) {
    return static_cast<Fetcher::Options::Value>(static_cast<int>(a) | static_cast<int>(b));
}

inline Fetcher::Options::Value operator&(Fetcher::Options::Value a, Fetcher::Options::Value b) {
    return static_cast<Fetcher::Options::Value>(static_cast<int>(a) & static_cast<int>(b));
}

inline Fetcher::Options operator|(Fetcher::Options a, Fetcher::Options b) {
    return Fetcher::Options{ a.value | b.value };
}

inline Fetcher::Options operator&(Fetcher::Options a, Fetcher::Options b) {
    return Fetcher::Options{ a.value & b.value };
}


#endif // UNITTESTBOT_FETCHER_H
