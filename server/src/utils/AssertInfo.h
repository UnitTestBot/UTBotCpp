//
// Created by mWX1153800 on 6/28/2022.
//

#ifndef UNITTESTBOT_ASSERTINFO_H
#define UNITTESTBOT_ASSERTINFO_H
#include <string>
#include <optional>

struct AssertInfo {
    bool failedAssert;
    std::optional <std::string> assertBody;
    std::optional <std::string> fileWithFailedAssert;
    std::optional <std::string> lineWithFailedAssert;

    [[nodiscard]] AssertInfo(bool failedAssert_, std::string assertBody_, std::string file, std::string line) : failedAssert(failedAssert_),
          assertBody(std::move(assertBody_)), fileWithFailedAssert(std::move(file)), lineWithFailedAssert(std::move(line)) {}

    [[nodiscard]] AssertInfo() : failedAssert(false), assertBody(std::nullopt), fileWithFailedAssert(std::nullopt), lineWithFailedAssert(std::nullopt) {}
};

#endif // UNITTESTBOT_ASSERTINFO_H
