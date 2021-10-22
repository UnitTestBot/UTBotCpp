/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "IRParser.h"

#include "utils/KleeUtils.h"

#include "loguru.hpp"

#include <llvm/BinaryFormat/Magic.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <PathSubstitution.h>

bool IRParser::parseModule(const fs::path &rootBitcode, tests::TestsMap &tests) {
    try {
        llvm::LLVMContext context;
        auto module = getModule(rootBitcode, context);
        if (module) {
            for (auto it = tests.begin(); it != tests.end(); it++) {
                fs::path const &sourceFile = it.key();
                tests::Tests &test = it.value();
                test.isFilePresentedInArtifact = true;
                for (const auto &[methodName, methodDescription] : test.methods) {
                    std::string entryPointFunction = KleeUtils::entryPointFunction(test, methodName, true);
                    string methodDebugInfo =
                        StringUtils::stringFormat("Method: '%s', file: '%s'", methodName, sourceFile);
                    if (llvm::Function *pFunction = module->getFunction(entryPointFunction)) {
                        continue;
                    } else {
                        LOG_S(DEBUG) << "llvm::Function is null: " << methodDebugInfo;
                        test.isFilePresentedInArtifact = false;
                    }
                }
            }
            return true;
        } else {
            LOG_S(ERROR) << "llvm::Module is null: " << rootBitcode;
            return false;
        }
    } catch (...) {
        LOG_S(ERROR) << "Failed to parse module: LLVM ERROR";
        return false;
    }
}
std::unique_ptr<llvm::Module> IRParser::getModule(const fs::path &rootBitcode,
                                                  llvm::LLVMContext &context) {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> bufferErr =
        llvm::MemoryBuffer::getFileOrSTDIN(rootBitcode.string());
    std::error_code ec = bufferErr.getError();
    if (ec) {
        LOG_S(ERROR) << StringUtils::stringFormat("Loading file %s failed: %s", rootBitcode,
                                                  ec.message());
        return nullptr;
    }

    llvm::MemoryBufferRef Buffer = bufferErr.get()->getMemBufferRef();

    llvm::file_magic magic = identify_magic(Buffer.getBuffer());

    if (magic == llvm::file_magic::bitcode) {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> module = llvm::parseIR(Buffer, Err, context);
        if (!module) {
            LOG_S(ERROR) << StringUtils::stringFormat("Loading file %s failed: %s", rootBitcode,
                                                      Err.getMessage().str());
            return nullptr;
        }
        return module;
    }

    if (magic == llvm::file_magic::archive) {
        LOG_S(ERROR) << "Archive found: " << rootBitcode;
        return nullptr;
    }
    return nullptr;
}
