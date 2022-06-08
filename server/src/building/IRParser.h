#ifndef UNITTESTBOT_IRPARSER_H
#define UNITTESTBOT_IRPARSER_H

#include "PathSubstitution.h"
#include "Tests.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>


class IRParser {
public:
    bool parseModule(const fs::path &rootBitcode, tests::TestsMap &tests);

private:
    std::unique_ptr<llvm::Module> getModule(const fs::path &rootBitcode,
                                            llvm::LLVMContext &context);
};


#endif // UNITTESTBOT_IRPARSER_H
