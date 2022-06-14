#include "IncludeFetchSourceFileCallback.h"

#include "Fetcher.h"
#include "utils/ExecUtils.h"
#include "utils/LogUtils.h"

#include "loguru.h"

#include <clang/Frontend/CompilerInstance.h>

using namespace clang;

class IncludeFetchPPCallbacks : public clang::PPCallbacks {
    Fetcher const *const parent;

public:
    explicit IncludeFetchPPCallbacks(clang::CompilerInstance &ci, const Fetcher *parent)
        : compilerInstance(ci), parent(parent) {
    }

    void InclusionDirective(clang::SourceLocation hash_loc,
                            const clang::Token &include_token,
                            llvm::StringRef file_name,
                            bool is_angled,
                            clang::CharSourceRange filename_range,
                            const clang::FileEntry *file,
                            llvm::StringRef search_path,
                            llvm::StringRef relative_path,
                            const clang::Module *imported,
                            clang::SrcMgr::CharacteristicKind fileType) override {
        ExecUtils::throwIfCancelled();

        clang::SourceManager &sourceManager = compilerInstance.getSourceManager();
        if (!sourceManager.isInMainFile(hash_loc)) {
            if (LogUtils::isMaxVerbosity()) {
                auto hashFilename = sourceManager.getFilename(hash_loc).str();
                LOG_S(MAX) << "Found inner include for filename: " << hashFilename
                           << ". Skipping it.";
            }
            return;
        }
        fs::path sourceFilePath = sourceManager.getFileEntryForID(sourceManager.getMainFileID())
                                      ->tryGetRealPathName()
                                      .str();
        LOG_S(MAX) << "Fetched inclusion directive. "
                   << "Source file sourceFilePath: " << sourceFilePath
                   << "\nPath:" << sourceFilePath;
        if (!CollectionUtils::containsKey(*parent->projectTests, sourceFilePath)) {
            LOG_S(DEBUG) << "Project doesn't contain source file: " << sourceFilePath;
            return;
        }
        fs::path headerPath = file_name.str();
        if (headerPath.extension() == ".c") {
            LOG_S(DEBUG) << "Fetched .c file. Skipping it.";
            return;
        }

        if (Paths::isGtest(sourceFilePath)) {
            LOG_S(DEBUG) << "Fetched gtest file. Skipping it.";
            return;
        }
        fs::path includePath = fs::path(search_path) / headerPath;
        LOG_S(MAX) << "Fetched include path: " << includePath;
        auto &projectTests = (*parent->projectTests).at(sourceFilePath);
        auto &mainHeader = projectTests.mainHeader;
        if (Paths::isHeadersEqual(sourceFilePath, includePath)) {
            mainHeader = { is_angled, headerPath };
        } else {
            if (!mainHeader.has_value()) {
                projectTests.headersBeforeMainHeader.emplace_back(is_angled, headerPath);
            }
        }
    }

private:
    clang::CompilerInstance &compilerInstance;
};

IncludeFetchSourceFileCallback::IncludeFetchSourceFileCallback(Fetcher *parent)
    : parent(parent) {
}

bool IncludeFetchSourceFileCallback::handleBeginSource(clang::CompilerInstance &CI) {
    auto findIncludesCallback = std::make_unique<IncludeFetchPPCallbacks>(CI, parent);
    Preprocessor &preprocessor = CI.getPreprocessor();
    preprocessor.addPPCallbacks(std::move(findIncludesCallback));
    return true;
}

void IncludeFetchSourceFileCallback::handleEndSource() {
    SourceFileCallbacks::handleEndSource();
}
