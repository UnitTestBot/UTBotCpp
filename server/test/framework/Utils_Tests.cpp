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

    TEST(ReadBytesAsValueTest, Unsigned1) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 15;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "8196");
    }

    TEST(ReadBytesAsValueTest, Unsigned2) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "8196");
    }

    TEST(ReadBytesAsValueTest, Signed1) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "-8188");
    }

    TEST(ReadBytesAsValueTest, Signed2) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 3;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "-7676");
    }

    TEST(ReadBytesAsValueTest, Signed3) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 0;
        size_t len = 14;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "4128");
    }

    TEST(ReadBytesAsValueTest, Signed4) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 4;
        size_t len = 3;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "2");
    }

    TEST(ReadBytesAsValueTest, Signed5) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 4;
        size_t len = 2;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "-2");
    }

    TEST(ReadBytesAsValueTest, Signed6) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[2] ^= 1 << 0;
        bytes[1] ^= 1 << 4;
        bytes[0] ^= 1 << 5;
        size_t offset = 4;
        size_t len = 1;
        EXPECT_EQ(tests::readBytesAsValue<signed int>(bytes, offset, len), "0");
    }

    TEST(ReadBytesAsValueTest, Signed7) {
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

    TEST(ReadBytesAsValueTest, Signed8) {
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

    TEST(ReadBytesAsValueTest, Signed9) {
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

    TEST(ReadBytesAsValueTest, Unsigned3) {
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

    TEST(ReadBytesAsValueTest, Unsigned4) {
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

    TEST(ReadBytesAsValueTest, Unsigned5) {
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

    TEST(ReadBytesAsValueTest, Unsigned6) {
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

    TEST(ReadBytesAsValueTest, Unsigned7) {
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

    TEST(ReadBytesAsValueTest, Unsigned8) {
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

    TEST(ReadBytesAsValueTest, Unsigned9) {
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

    TEST(ReadBytesAsValueTest, Bool1) {
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

    TEST(ReadBytesAsValueTest, Bool2) {
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

    TEST(ReadBytesAsValueTest, Bool3) {
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

    TEST(ReadBytesAsValueTest, ByteOffsetInt) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[3] = 1;
        bytes[2] = bytes[1] = bytes[0] = -1;
        size_t offset = 0;
        size_t len = 24;
        EXPECT_EQ(tests::readBytesAsValue<int>(bytes, offset, len), "-1");
    }

    TEST(ReadBytesAsValueTest, ByteOffsetInt2) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[3] = 1;
        bytes[2] = bytes[1] = bytes[0] = -1;
        size_t offset = 8;
        size_t len = 24;
        EXPECT_EQ(tests::readBytesAsValue<int>(bytes, offset, len), "131071");
    }

    TEST(ReadBytesAsValueTest, ByteOffsetUnsignedInt) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[3] = 1;
        bytes[2] = bytes[1] = bytes[0] = -1;
        size_t offset = 0;
        size_t len = 24;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "16777215");
    }

    TEST(ReadBytesAsValueTest, ByteOffsetUnsignedInt2) {
        size_t const LEN = 4;
        std::vector<char> bytes(LEN);
        bytes[3] = 1;
        bytes[2] = bytes[1] = bytes[0] = -1;
        size_t offset = 8;
        size_t len = 24;
        EXPECT_EQ(tests::readBytesAsValue<unsigned int>(bytes, offset, len), "131071");
    }

    template<typename T>
    void readBytesAsValueTestTemplate(T val) {
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

    TEST(ReadBytesAsValueTest, CommonInt) {
        readBytesAsValueTestTemplate<int>(13);
        readBytesAsValueTestTemplate<int>(26);
        readBytesAsValueTestTemplate<int>(42);
        readBytesAsValueTestTemplate<int>(0);
        readBytesAsValueTestTemplate<int>(1);
        readBytesAsValueTestTemplate<int>(-13);
        readBytesAsValueTestTemplate<int>(-26);
        readBytesAsValueTestTemplate<int>(-42);
        readBytesAsValueTestTemplate<int>(-1);
        readBytesAsValueTestTemplate<int>(std::numeric_limits<int>::max());
        readBytesAsValueTestTemplate<int>(std::numeric_limits<int>::min());
    }

    TEST(ReadBytesAsValueTest, CommonUnsignedInt) {
        readBytesAsValueTestTemplate<unsigned int>(13);
        readBytesAsValueTestTemplate<unsigned int>(26);
        readBytesAsValueTestTemplate<unsigned int>(42);
        readBytesAsValueTestTemplate<unsigned int>(0);
        readBytesAsValueTestTemplate<unsigned int>(1);
        readBytesAsValueTestTemplate<unsigned int>(std::numeric_limits<unsigned int>::min());
        readBytesAsValueTestTemplate<unsigned int>(std::numeric_limits<unsigned int>::max());
    }

    TEST(ReadBytesAsValueTest, CommonChar) {
        readBytesAsValueTestTemplate<char>(13);
        readBytesAsValueTestTemplate<char>(26);
        readBytesAsValueTestTemplate<char>(42);
        readBytesAsValueTestTemplate<char>(0);
        readBytesAsValueTestTemplate<char>(1);
        readBytesAsValueTestTemplate<char>(-13);
        readBytesAsValueTestTemplate<char>(-26);
        readBytesAsValueTestTemplate<char>(-42);
        readBytesAsValueTestTemplate<char>(-1);
        readBytesAsValueTestTemplate<char>(std::numeric_limits<char>::min());
        readBytesAsValueTestTemplate<char>(std::numeric_limits<char>::max());
    }

    TEST(ReadBytesAsValueTest, CommonUnsignedChar) {
        readBytesAsValueTestTemplate<unsigned char>(13);
        readBytesAsValueTestTemplate<unsigned char>(26);
        readBytesAsValueTestTemplate<unsigned char>(42);
        readBytesAsValueTestTemplate<unsigned char>(0);
        readBytesAsValueTestTemplate<unsigned char>(1);
        readBytesAsValueTestTemplate<unsigned char>(std::numeric_limits<unsigned char>::min());
        readBytesAsValueTestTemplate<unsigned char>(std::numeric_limits<unsigned char>::max());
    }

    TEST(ReadBytesAsValueTest, CommonShort) {
        readBytesAsValueTestTemplate<short>(13);
        readBytesAsValueTestTemplate<short>(26);
        readBytesAsValueTestTemplate<short>(42);
        readBytesAsValueTestTemplate<short>(0);
        readBytesAsValueTestTemplate<short>(1);
        readBytesAsValueTestTemplate<short>(-13);
        readBytesAsValueTestTemplate<short>(-26);
        readBytesAsValueTestTemplate<short>(-42);
        readBytesAsValueTestTemplate<short>(-1);
        readBytesAsValueTestTemplate<short>(std::numeric_limits<short>::min());
        readBytesAsValueTestTemplate<short>(std::numeric_limits<short>::max());
    }

    TEST(ReadBytesAsValueTest, CommonUnsignedShort) {
        readBytesAsValueTestTemplate<unsigned short>(13);
        readBytesAsValueTestTemplate<unsigned short>(26);
        readBytesAsValueTestTemplate<unsigned short>(42);
        readBytesAsValueTestTemplate<unsigned short>(0);
        readBytesAsValueTestTemplate<unsigned short>(1);
        readBytesAsValueTestTemplate<unsigned short>(-13);
        readBytesAsValueTestTemplate<unsigned short>(-26);
        readBytesAsValueTestTemplate<unsigned short>(-42);
        readBytesAsValueTestTemplate<unsigned short>(-1);
        readBytesAsValueTestTemplate<unsigned short>(std::numeric_limits<unsigned short>::min());
        readBytesAsValueTestTemplate<unsigned short>(std::numeric_limits<unsigned short>::max());
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
