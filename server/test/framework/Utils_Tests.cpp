/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "gtest/gtest.h"

#include "TestUtils.h"
#include "utils/CollectionUtils.h"
#include "utils/CompilationUtils.h"
#include "utils/ExecUtils.h"
#include "utils/StringUtils.h"

#include <algorithm>

namespace {
    using std::vector;
    using std::string;
    auto projectPath = fs::current_path().parent_path() / testUtils::getRelativeTestSuitePath("server");

    TEST(Utils_Test, Split) {
        string s = "a,b,c,d,";
        vector<string> vs = StringUtils::split(s, ',');
        EXPECT_EQ(vector<string>({"a", "b", "c", "d"}), vs);
    }

    TEST(Utils_Test, Ltrim) {
        string s = " aaaa  ";
        StringUtils::ltrim(s);
        EXPECT_EQ("aaaa  ", s);
    }

    TEST(Utils_Test, Rtrim) {
        string s = " aaaa  ";
        StringUtils::rtrim(s);
        EXPECT_EQ(" aaaa", s);
    }

    TEST(Utils_Test, Trim) {
        string s = " aaaa  ";
        StringUtils::trim(s);
        EXPECT_EQ("aaaa", s);
    }

    TEST(Utils_Test, GetKeys) {
        std::unordered_map<string, string> map;
        map["a"] = "z";
        map["b"] = "y";
        map["c"] = "x";
        auto keys = CollectionUtils::getKeys(map);
        std::sort(keys.begin(), keys.end());
        EXPECT_EQ(vector<string>({"a", "b", "c"}), keys);
    }

    TEST(Utils_Test, JoinWith) {
        vector<string> vs({"xx", "yy", "zz"});
        string joined = StringUtils::joinWith(vs, "::");
        EXPECT_EQ("xx::yy::zz", joined);
    }

    TEST(Utils_Test, IsNumber) {
        string s1 = "123";
        string s2 = "q123";
        EXPECT_EQ(true, StringUtils::isNumber(s1));
        EXPECT_EQ(false, StringUtils::isNumber(s2));
    }

    TEST(Utils_Test, FilterPathsByDirNames) {
        vector<fs::path> paths{
                                     projectPath / "basic_functions.c",
                                     projectPath / "types.c",
                                     projectPath / "basic_functions.h",
                                     projectPath / "zzz/snippet.c"
                             };
        vector<fs::path> dirNames{ projectPath };
        auto filteredPaths = Paths::filterPathsByDirNames(paths, dirNames, { ".c" });
        EXPECT_EQ(vector<fs::path>({
                                           projectPath / "basic_functions.c",
                                           projectPath / "types.c"}),
                  filteredPaths);
    }

    TEST(Utils_Test, Exec) {
        auto execResult = ShellExecTask::runShellCommandTask(ShellExecTask::ExecutionParameters("pwd", {}), projectPath);
        EXPECT_EQ(projectPath.string() + "\n", execResult.output);
        EXPECT_EQ(0, execResult.status);
    }

    TEST(Utils_Test, Exec_Timeout) {
        using namespace std::chrono_literals;
        auto task = ShellExecTask::getShellCommandTask("sleep", {"5h"}, std::chrono::seconds(2));
        auto start = std::chrono::system_clock::now();
        auto execResult = task.run();
        auto end = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        EXPECT_LE(diff.count(), 10.);
    }

    TEST(Utils_Test, AddExt) {
        fs::path filePath = projectPath / "basic_functions.c";
        EXPECT_EQ(projectPath / "basic_functions.bc", Paths::replaceExtension(filePath, ".bc"));
    }

    TEST(Utils_Test, StringFormat) {
        CompilationUtils::CompilerName compilerName = CompilationUtils::CompilerName::CLANG;
        const char *subProjectName = "executable";
        auto format = StringUtils::stringFormat("%s_%s", to_string(compilerName), subProjectName);
        EXPECT_EQ("CLANG_executable", format);
    }

    TEST(Utils_Test, LongestCommonPath) {
        fs::path a = "/home/utbot/tmp/JollyFish/git-2.29/t/helper";
        fs::path b = "/home/utbot/tmp/JollyFish/git-2.29/libgit.bc";
        fs::path commonPrefixPath = Paths::longestCommonPrefixPath(a, b);
        EXPECT_EQ(fs::path{"/home/utbot/tmp/JollyFish/git-2.29"}, commonPrefixPath);
    }

    TEST(Utils_Test, LongestCommonPath2) {
        fs::path a = "a";
        fs::path b = "b";
        fs::path commonPrefixPath = Paths::longestCommonPrefixPath(a, b);
        EXPECT_EQ(fs::path{""}, commonPrefixPath);
    }

    TEST(Utils_Test, FileSystemPathIsNormalized) {
        EXPECT_EQ(fs::path("/a/b/./c/../x").string(), "/a/b/x");
    }

    TEST(Utils_Test, FileSystemPathOperatorEquals) {
        EXPECT_EQ(fs::path("/a/../b/./c/../x"), fs::path("/a/b/x/../../../b/x"));
    }

    TEST(Utils_Test, FileSystemPathOperatorDivide) {
        EXPECT_EQ(fs::path("/a/b") / fs::path("../c"), fs::path("/a/c"));
    }

    TEST(Utils_Test, FileSystemPathOperatorAssignDivision) {
        EXPECT_EQ(fs::path("/a/b") /= fs::path("../c"), fs::path("/a/c"));
    }

    TEST(Utils_Test, AddExtension) {
        EXPECT_EQ(Paths::addExtension("/a/b", ".cpp"), "/a/b.cpp");
    }
}