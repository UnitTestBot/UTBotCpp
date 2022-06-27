#include "NameDecorator.h"

#include "utils/CollectionUtils.h"
#include "utils/StringUtils.h"

#include <iterator>

std::string NameDecorator::decorate(std::string_view name) {
    std::string result;
    std::regex_replace(std::back_inserter(result), name.begin(), name.end(),
                       CPP_ONLY_KEYWORDS_REGEX, "$&_");
    // add underscore at the end of name
    return result;
}

std::string NameDecorator::defineWcharT(std::string_view canonicalName) {
    return StringUtils::stringFormat("#define wchar_t %.*s", canonicalName.length(),
                                     canonicalName.data());
}

const std::string NameDecorator::UNDEF_WCHAR_T = "#ifdef wchar_t\n"
                                                 "#undef wchar_t\n"
                                                 "#endif\n";

// https://en.cppreference.com/w/c/keyword
const std::unordered_set<std::string> NameDecorator::C_KEYWORDS = {
    "auto",    "break",  "case",     "char",   "const",    "continue", "default",
    "do",      "double", "else",     "enum",   "extern",   "float",    "for",
    "goto",    "if",     "inline",   "int",    "long",     "register", "restrict",
    "return",  "short",  "signed",   "sizeof", "static",   "struct",   "switch",
    "typedef", "union",  "unsigned", "void",   "volatile", "while",
};

// https://en.cppreference.com/w/cpp/keyword
const std::unordered_set<std::string> NameDecorator::CPP_KEYWORDS = {
    "alignas",
    "alignof",
    "and",
    "and_eq",
    "asm",
    "atomic_cancel",
    "atomic_commit",
    "atomic_noexcept",
    "auto",
    "bitand",
    "bitor",
//    "bool",
    "break",
    "case",
    "catch",
    "char",
    "char8_t",
    "char16_t",
    "char32_t",
    "class",
    "compl",
    "concept",
    "const",
    "consteval",
    "constexpr",
    "constinit",
    "const_cast",
    "continue",
    "co_await",
    "co_return",
    "co_yield",
    "decltype",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "not_eq",
    "nullptr",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "reflexpr",
    "register",
    "reinterpret_cast",
    "requires",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "synchronized",
    "template",
    "this",
    "thread_local",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
//    "wchar_t",
    "while",
    "xor",
    "xor_eq",
};


const std::unordered_set<std::string> NameDecorator::CPP_ONLY_KEYWORDS = CollectionUtils::filterOut(
        CPP_KEYWORDS, [](std::string const &s) { return CollectionUtils::contains(C_KEYWORDS, s); });

const std::regex NameDecorator::CPP_ONLY_KEYWORDS_REGEX{
    "\\b(" + StringUtils::joinWith(CPP_ONLY_KEYWORDS, "|") + ")\\b"
};


const std::unordered_set<std::string> NameDecorator::CPP_OPERATORS = {
    "xor", "xor_eq", "or", "not", "or_eq", "not_eq", "compl", "bitor", "bitand", "and_eq", "and"
};

const std::unordered_set<std::string> NameDecorator::TO_DEFINE =
        CollectionUtils::filterOut(NameDecorator::CPP_ONLY_KEYWORDS, [](std::string const &it) {
            return CollectionUtils::contains(NameDecorator::CPP_OPERATORS, it) || it == "bool";
        });

const std::vector<std::string> NameDecorator::DEFINES =
    CollectionUtils::transformTo<std::vector<std::string>>(TO_DEFINE, [](std::string const &it) {
        return StringUtils::stringFormat("#define %s %s", it, decorate(it));
    });

const std::string NameDecorator::DEFINES_CODE = StringUtils::joinWith(DEFINES, "\n");

const std::vector<std::string> NameDecorator::UNDEFS =
    CollectionUtils::transformTo<std::vector<std::string>>(TO_DEFINE, [](std::string const &it) {
        return StringUtils::stringFormat("#undef %s", it);
    });

const std::string NameDecorator::UNDEFS_CODE = StringUtils::joinWith(UNDEFS, "\n");
