/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TIMEEXECSTATISTICS_H
#define UNITTESTBOT_TIMEEXECSTATISTICS_H

#include <chrono>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "utils/path/FileSystemPath.h"

// add this macro to the beginning of the function
#define MEASURE_FUNCTION_EXECUTION_TIME const TimeExecStatistics timeExecStats(__FILE__, __FUNCTION__, __LINE__);

using std::string;
using std::vector;

class TimeExecStatistics {
public:
    TimeExecStatistics(const fs::path& file, const string& function, uint32_t line);

    ~TimeExecStatistics();

    struct Caller {
        string file, function;

        Caller(string  file, string  function);

        [[nodiscard]] string get() const;

        bool operator==(const Caller& other) const;

        friend std::ostream& operator<<(std::ostream& os, const Caller& caller);
    };

    struct CallerHash {
        std::size_t operator()(const Caller &caller) const;
    };

    static void clearStatistic();

    static void printStatistic();

private:
    const std::chrono::steady_clock::time_point begin;
    const Caller currentCaller;

    using SummaryRowType = std::tuple<Caller, double, uint64_t, uint64_t>;

    static SummaryRowType getFunctionSummary(const Caller &function);

    static void printRowDelimiter(std::stringstream& ss, const vector<size_t>& columnsWidth);

    template <std::size_t N> struct num { static const constexpr auto value = N; };

    template <class F, std::size_t... Is> static void tupleFor(F func, std::index_sequence<Is...>) {
        using expander = int[];
        (void)expander{ 0, ((void)func(num<Is>{}), 0)... };
    }

    template <std::size_t N, typename F> static void tupleFor(F func) {
        tupleFor(func, std::make_index_sequence<N>());
    }


};

#endif // UNITTESTBOT_TIMEEXECSTATISTICS_H
