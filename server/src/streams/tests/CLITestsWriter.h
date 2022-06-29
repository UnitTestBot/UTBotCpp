#ifndef UNITTESTBOT_CLITESTSWRITER_H
#define UNITTESTBOT_CLITESTSWRITER_H

#include "Tests.h"
#include "TestsWriter.h"
#include "streams/IStreamWriter.h"


class CLITestsWriter : public TestsWriter {
public:
    explicit CLITestsWriter(): TestsWriter(nullptr) {};

    void writeTestsWithProgress(tests::TestsMap &testMap, std::string const &message,
                                const fs::path &testDirPath,
                                std::function<void(tests::Tests &)> &&functor,
                                std::function<void(bool)> &&joinAndWriteSarif) override;

    void writeCodeAnylisisFolder(const fs::path &projectPath) const override;

private:
    static bool writeTestFile(const tests::Tests &tests, const fs::path &testDirPath);
};


#endif // UNITTESTBOT_CLITESTSWRITER_H
