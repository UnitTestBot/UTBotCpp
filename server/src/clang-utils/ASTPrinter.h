#ifndef UNITTESTBOT_ASTPRINTER_H
#define UNITTESTBOT_ASTPRINTER_H

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>

#include <string>

class ASTPrinter {
public:
    ASTPrinter() = default;

    /**
     * Gets the portion of the code that corresponds to given SourceRange, including the
     * last token. Returns expanded macros.
     *
     * @see getSourceTextRaw()
     */
    static std::string getSourceText(clang::SourceRange sourceRange,
                                     const clang::SourceManager &sourceManager,
                                     const clang::LangOptions &langOpts = clang::LangOptions());

    static std::string getSourceTextMacro(clang::SourceRange sourceRange,
                                          const clang::SourceManager &sourceManager,
                                          const clang::LangOptions &langOpts);

    /**
     * Gets the portion of the code that corresponds to given SourceRange exactly as
     * the range is given.
     *
     * @warning The end location of the SourceRange returned by some Clang functions
     * (such as clang::Expr::getSourceRange) might actually point to the first character
     * (the "location") of the last token of the expression, rather than the character
     * past-the-end of the expression like clang::Lexer::getSourceText expects.
     * getSourceTextRaw() does not take this into account. Use getSourceText()
     * instead if you want to get the source text including the last token.
     *
     * @warning This function does not obtain the source of a macro/preprocessor expansion.
     * Use getSourceText() for that.
     */
    static std::string getSourceTextRaw(clang::SourceRange sourceRange,
                                        const clang::SourceManager &sourceManager,
                                        const clang::LangOptions &langOpts = clang::LangOptions());
};

#endif //UNITTESTBOT_ASTPRINTER_H
