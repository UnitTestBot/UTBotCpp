#include "gtest/gtest.h"

#include "TestUtils.h"
#include "utils/CollectionUtils.h"
#include "utils/CompilationUtils.h"
#include "utils/ExecUtils.h"
#include "utils/StringUtils.h"

#include <algorithm>
#include <climits>
#include <limits>
#include <random>
#include <string>

namespace {
    auto projectPath = fs::current_path().parent_path() / testUtils::getRelativeTestSuitePath("server");

    TEST(readBytesAsValue, unsigned1) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 15;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "8196");
    }

    TEST(readBytesAsValue, unsigned2) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "8196");
    }

    TEST(readBytesAsValue, signed1) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "-8188");
    }

    TEST(readBytesAsValue, signed2) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "-7676");
    }

    TEST(readBytesAsValue, signed3) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 0;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "4128");
    }

    TEST(readBytesAsValue, signed4) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 4;
        size_t len = 3;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "2");
    }

    TEST(readBytesAsValue, signed5) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 4;
        size_t len = 2;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "-2");
    }

    TEST(readBytesAsValue, signed6) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 4;
        size_t len = 1;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "0");
    }

    TEST(readBytesAsValue, signed7) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 2;
        size_t len = 2;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "-2");
    }

    TEST(readBytesAsValue, signed8) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 2;
        size_t len = 6;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "10");
    }

    TEST(readBytesAsValue, signed9) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 1;
        size_t len = 15;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "2068");
    }

    TEST(readBytesAsValue, unsigned3) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 1;
        size_t len = 15;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "2068");
    }

    TEST(readBytesAsValue, unsigned4) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 3;
        size_t len = 15;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "8709");
    }

    TEST(readBytesAsValue, unsigned5) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 3;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "8709");
    }

    TEST(readBytesAsValue, unsigned6) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 3;
        size_t len = 16;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "41477");
    }

    TEST(readBytesAsValue, unsigned7) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 3;
        size_t len = 17;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "41477");
    }

    TEST(readBytesAsValue, unsigned8) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 3;
        size_t len = 26;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "41477");
    }

    TEST(readBytesAsValue, unsigned9) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 3;
        size_t len = 26;
        EXPECT_EQ(tests::readBytesAsValue<unsigned long long>(bytes, offset, len), "41477");
    }

    TEST(readBytesAsValue, unsigned10) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 15;
        size_t len = 2;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "2");
    }

    TEST(readBytesAsValue, bool1) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 15;
        size_t len = 1;
        EXPECT_EQ(tests::readBytesAsValue<bool>(bytes, offset, len), "0");
    }

    TEST(readBytesAsValue, bool2) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 16;
        size_t len = 1;
        EXPECT_EQ(tests::readBytesAsValue<bool>(bytes, offset, len), "1");
    }

    TEST(readBytesAsValue, bool3) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 2;
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        bytes[0] ^= 1 << 3;
        size_t offset = 12;
        size_t len = 1;
        EXPECT_EQ(tests::readBytesAsValue<bool>(bytes, offset, len), "1");
    }

    template<typename T>
    void readBytesAsValue_test_template(T val) {
        srand(42);
        for (size_t tcount = 0; tcount < 5; ++tcount) {
//            std::cout << "\ttest #" << tcount << ", val = " << val;
            auto add = static_cast<size_t>(rand() % 10);
            auto start = static_cast<size_t>(rand() % add);
//            std::cout << "additional bytes: " << add << ", start position: " << start << std::endl;
            size_t const len = sizeof(T);
            std::vector<char> bytes(len + add);
            for (size_t i = 1; i <= len; ++i) {
                bytes[start + i - 1] = (val & ((1LL << (CHAR_BIT * i)) - 1)) >> (CHAR_BIT * (i - 1));
//            std::cout << (unsigned)bytes[start + i - 1] << " ";
            }
//        std::cout << std::endl;
//        for (char c : bytes) {
//            std::cout << (unsigned)c << " ";
//        }
//        std::cout << std::endl;
            EXPECT_EQ(tests::readBytesAsValue<T>(bytes, start * CHAR_BIT, len * CHAR_BIT), std::to_string(val));
        }
    }

    TEST(readBytesAsValue, common_int) {
        readBytesAsValue_test_template<int>(13);
        readBytesAsValue_test_template<int>(26);
        readBytesAsValue_test_template<int>(42);
        readBytesAsValue_test_template<int>(0);
        readBytesAsValue_test_template<int>(1);
        readBytesAsValue_test_template<int>(-13);
        readBytesAsValue_test_template<int>(-26);
        readBytesAsValue_test_template<int>(-42);
        readBytesAsValue_test_template<int>(-1);
        readBytesAsValue_test_template<int>(std::numeric_limits<int>::max());
        readBytesAsValue_test_template<int>(std::numeric_limits<int>::min());
    }

    TEST(readBytesAsValue, common_uint) {
        readBytesAsValue_test_template<unsigned int>(13);
        readBytesAsValue_test_template<unsigned int>(26);
        readBytesAsValue_test_template<unsigned int>(42);
        readBytesAsValue_test_template<unsigned int>(0);
        readBytesAsValue_test_template<unsigned int>(1);
        readBytesAsValue_test_template<unsigned int>(std::numeric_limits<unsigned int>::min());
        readBytesAsValue_test_template<unsigned int>(std::numeric_limits<unsigned int>::max());
    }

    TEST(readBytesAsValue, common_char) {
        readBytesAsValue_test_template<char>(13);
        readBytesAsValue_test_template<char>(26);
        readBytesAsValue_test_template<char>(42);
        readBytesAsValue_test_template<char>(0);
        readBytesAsValue_test_template<char>(1);
        readBytesAsValue_test_template<char>(-13);
        readBytesAsValue_test_template<char>(-26);
        readBytesAsValue_test_template<char>(-42);
        readBytesAsValue_test_template<char>(-1);
        readBytesAsValue_test_template<char>(std::numeric_limits<char>::min());
        readBytesAsValue_test_template<char>(std::numeric_limits<char>::max());
    }

    TEST(readBytesAsValue, common_short) {
        readBytesAsValue_test_template<short>(13);
        readBytesAsValue_test_template<short>(26);
        readBytesAsValue_test_template<short>(42);
        readBytesAsValue_test_template<short>(0);
        readBytesAsValue_test_template<short>(1);
        readBytesAsValue_test_template<short>(-13);
        readBytesAsValue_test_template<short>(-26);
        readBytesAsValue_test_template<short>(-42);
        readBytesAsValue_test_template<short>(-1);
        readBytesAsValue_test_template<short>(std::numeric_limits<short>::min());
        readBytesAsValue_test_template<short>(std::numeric_limits<short>::max());
    }

    TEST(readBytesAsValue, common_ushort) {
        readBytesAsValue_test_template<unsigned short>(13);
        readBytesAsValue_test_template<unsigned short>(26);
        readBytesAsValue_test_template<unsigned short>(42);
        readBytesAsValue_test_template<unsigned short>(0);
        readBytesAsValue_test_template<unsigned short>(1);
        readBytesAsValue_test_template<unsigned short>(-13);
        readBytesAsValue_test_template<unsigned short>(-26);
        readBytesAsValue_test_template<unsigned short>(-42);
        readBytesAsValue_test_template<unsigned short>(-1);
        readBytesAsValue_test_template<unsigned short>(std::numeric_limits<unsigned short>::min());
        readBytesAsValue_test_template<unsigned short>(std::numeric_limits<unsigned short>::max());
    }

    TEST(Utils_Test, Split) {
        std::string s = "a,b,c,d,";
        std::vector<std::string> vs = StringUtils::split(s, ',');
        EXPECT_EQ(std::vector<std::string>({"a", "b", "c", "d"}), vs);
    }

    TEST(Utils_Test, Ltrim) {
        std::string s = " aaaa  ";
        StringUtils::ltrim(s);
        EXPECT_EQ("aaaa  ", s);
    }

    TEST(Utils_Test, Rtrim) {
        std::string s = " aaaa  ";
        StringUtils::rtrim(s);
        EXPECT_EQ(" aaaa", s);
    }

    TEST(Utils_Test, Trim) {
        std::string s = " aaaa  ";
        StringUtils::trim(s);
        EXPECT_EQ("aaaa", s);
    }

    TEST(Utils_Test, GetKeys) {
        std::unordered_map<std::string, std::string> map;
        map["a"] = "z";
        map["b"] = "y";
        map["c"] = "x";
        auto keys = CollectionUtils::getKeys(map);
        std::sort(keys.begin(), keys.end());
        EXPECT_EQ(std::vector<std::string>({"a", "b", "c"}), keys);
    }

    TEST(Utils_Test, JoinWith) {
        std::vector<std::string> vs({"xx", "yy", "zz"});
        std::string joined = StringUtils::joinWith(vs, "::");
        EXPECT_EQ("xx::yy::zz", joined);
    }

    TEST(Utils_Test, IsNumber) {
        std::string s1 = "123";
        std::string s2 = "q123";
        EXPECT_EQ(true, StringUtils::isNumber(s1));
        EXPECT_EQ(false, StringUtils::isNumber(s2));
    }

    TEST(Utils_Test, FilterPathsByDirNames) {
        CollectionUtils::FileSet paths{
                                     projectPath / "basic_functions.c",
                                     projectPath / "types.c",
                                     projectPath / "basic_functions.h",
                                     projectPath / "zzz/snippet.c"
                             };
        std::vector<fs::path> dirNames{ projectPath };
        auto filteredPaths = Paths::filterPathsByDirNames(paths, dirNames, Paths::isCFile);
        EXPECT_EQ(CollectionUtils::FileSet({
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
