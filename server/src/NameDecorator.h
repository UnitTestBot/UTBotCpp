/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_NAMEDECORATOR_H
#define UNITTESTBOT_NAMEDECORATOR_H

#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

class NameDecorator {
public:
    static std::string decorate(std::string_view name);

    static std::string defineWcharT(std::string_view canonicalName);

    static const std::string UNDEF_WCHAR_T;

    static const std::unordered_set<std::string> C_KEYWORDS;

    static const std::unordered_set<std::string> CPP_KEYWORDS;

    static const std::unordered_set<std::string> CPP_ONLY_KEYWORDS;

    static const std::regex CPP_ONLY_KEYWORDS_REGEX;

    static const std::unordered_set<std::string> CPP_OPERATORS;

    static const std::vector<std::string> DEFINES;

    static const std::string DEFINES_CODE;

    static const std::vector<std::string> UNDEFS;

    static const std::string UNDEFS_CODE;

private:
    static const std::unordered_set<std::string> TO_DEFINE;
};


#endif // UNITTESTBOT_NAMEDECORATOR_H
