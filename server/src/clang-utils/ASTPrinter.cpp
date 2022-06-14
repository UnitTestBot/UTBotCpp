#include "ASTPrinter.h"

#include <clang/Lex/Lexer.h>

std::string ASTPrinter::getSourceText(clang::SourceRange sourceRange,
                                      const clang::SourceManager &sourceManager,
                                      const clang::LangOptions &langOpts) {
    /*
     * NOTE: sm.getSpellingLoc() used in case the range corresponds to a macro/preprocessed source.
    */
    auto start_loc = sourceManager.getExpansionLoc(sourceRange.getBegin());
    auto last_token_loc = sourceManager.getExpansionLoc(sourceRange.getEnd());
    auto end_loc = clang::Lexer::getLocForEndOfToken(last_token_loc, 0, sourceManager, langOpts);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    return getSourceTextRaw(printable_range, sourceManager, langOpts);
}

std::string ASTPrinter::getSourceTextMacro(clang::SourceRange sourceRange,
                                      const clang::SourceManager &sourceManager,
                                      const clang::LangOptions &langOpts) {
    auto charRange = clang::Lexer::getAsCharRange(sourceRange, sourceManager, langOpts);
    auto stringRep = clang::Lexer::getSourceText(charRange, sourceManager, langOpts);
    return stringRep.str();
}

std::string ASTPrinter::getSourceTextRaw(clang::SourceRange sourceRange,
                                         const clang::SourceManager &sourceManager,
                                         const clang::LangOptions &langOpts) {

    return clang::Lexer::getSourceText(clang::CharSourceRange::getCharRange(sourceRange), sourceManager, langOpts);
}
