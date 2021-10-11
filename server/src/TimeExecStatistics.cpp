/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "TimeExecStatistics.h"

#include "utils/CollectionUtils.h"
#include "utils/StringUtils.h"

#include "loguru.h"

#include <utility>

static thread_local double maxDurationMs = 0;
static const string SUMMARY_DELIMITER = " | ";
thread_local std::
    unordered_map<TimeExecStatistics::Caller, std::vector<double>, TimeExecStatistics::CallerHash>
        statistic;
static const vector<string> HEADERS({ "Function", "% of overall", "Total time (ms)",
                                      "Times called" });
static const size_t COLUMNS_NUMBER = 4;

TimeExecStatistics::Caller::Caller(string file, string function)
    : file(std::move(file)), function(std::move(function)) {
}

string TimeExecStatistics::Caller::get() const {
    return file + " " + function;
}

bool TimeExecStatistics::Caller::operator==(const TimeExecStatistics::Caller &other) const {
    return this->get() == other.get();
}

std::ostream &operator<<(std::ostream &os, const TimeExecStatistics::Caller &caller) {
    os << caller.get();
    return os;
}

std::size_t
TimeExecStatistics::CallerHash::operator()(const TimeExecStatistics::Caller &caller) const {
    return std::hash<string>()(caller.get());
}

TimeExecStatistics::TimeExecStatistics(const fs::path &file, const string &function, uint32_t line)
    : currentCaller({ file.filename().string() + ":" + std::to_string(line), function }),
      begin(std::chrono::steady_clock::now()) {
}

TimeExecStatistics::~TimeExecStatistics() {
    const auto duration = std::chrono::steady_clock::now() - begin;
    double durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    if (durationMs > maxDurationMs) {
        maxDurationMs = durationMs;
    }
    LOG_S(MAX) << "Execution Time for " << currentCaller.get() << " is " << durationMs << " ms";
    statistic[currentCaller].push_back(durationMs);
}

void TimeExecStatistics::clearStatistic() {
    statistic.clear();
}

void TimeExecStatistics::printStatistic() {
    vector<SummaryRowType> summaryTable;
    vector<size_t> columnsWidth(COLUMNS_NUMBER);
    for (size_t i = 0; i < COLUMNS_NUMBER; i++) {
        columnsWidth[i] = HEADERS[i].size();
    }
    for (const auto &[name, durations] : statistic) {
        auto currentSummary = getFunctionSummary(name);
        tupleFor<COLUMNS_NUMBER>([&](auto i) {
            std::stringstream ss;
            ss << std::get<i.value>(currentSummary);
            columnsWidth[i.value] = std::max(columnsWidth[i.value], ss.str().size());
        });
        summaryTable.push_back(currentSummary);
    }
    std::sort(summaryTable.begin(), summaryTable.end(), [](const auto &row1, const auto &row2) {
        return std::get<1>(row1) > std::get<1>(row2);
    });
    std::stringstream statsStream;
    statsStream << "Time execution statistic report:\n";
    printRowDelimiter(statsStream, columnsWidth);
    statsStream << SUMMARY_DELIMITER;
    for (size_t i = 0; i < COLUMNS_NUMBER; i++) {
        statsStream << std::setw(columnsWidth[i]) << HEADERS[i];
        statsStream << SUMMARY_DELIMITER;
    }
    statsStream << "\n";
    printRowDelimiter(statsStream, columnsWidth);
    for (const auto &row : summaryTable) {
        statsStream << SUMMARY_DELIMITER;
        tupleFor<COLUMNS_NUMBER>([&](auto i) {
            switch (i.value) {
            case 0:
                statsStream << std::setw(columnsWidth[0])
                            << (std::get<0>(row).file +
                                StringUtils::repeat(" ", columnsWidth[0] -
                                                             std::get<0>(row).file.size() -
                                                             std::get<0>(row).function.size()) +
                                std::get<0>(row).function);
                break;
            case 1:
                statsStream << std::setw(columnsWidth[1]) << std::fixed << std::setprecision(2)
                            << std::get<1>(row);
                statsStream.unsetf(std::ios_base::fixed);
                break;
            default:
                statsStream << std::setw(columnsWidth[i.value]) << std::get<i.value>(row);
            }
            statsStream << SUMMARY_DELIMITER;
        });
        statsStream << "\n";
    }
    printRowDelimiter(statsStream, columnsWidth);
    LOG_S(DEBUG) << statsStream.str();
}

std::tuple<TimeExecStatistics::Caller, double, uint64_t, uint64_t>
TimeExecStatistics::getFunctionSummary(const Caller &function) {
    double totalDuration = 0, pctOfTotalExecutionTime, timesCalled;
    if (!CollectionUtils::containsKey(statistic, function) || statistic[function].size() == 0) {
        return { Caller("", ""), 0, 0, 0 };
    }
    const auto &durations = statistic[function];
    for (auto d : durations) {
        totalDuration += d;
    }
    timesCalled = durations.size();
    pctOfTotalExecutionTime = (totalDuration / maxDurationMs) * 100.;
    return { function, pctOfTotalExecutionTime, totalDuration, timesCalled };
}

void TimeExecStatistics::printRowDelimiter(std::stringstream &ss,
                                           const vector<size_t> &columnsWidth) {
    ss << SUMMARY_DELIMITER;
    for (size_t i = 0; i < COLUMNS_NUMBER; i++) {
        ss << StringUtils::repeat("_", columnsWidth[i]);
        ss << SUMMARY_DELIMITER;
    }
    ss << "\n";
}
