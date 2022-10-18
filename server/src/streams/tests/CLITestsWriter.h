#ifndef UNITTESTBOT_CLITESTSWRITER_H
#define UNITTESTBOT_CLITESTSWRITER_H

#include "Tests.h"
#include "TestsWriter.h"
#include "streams/IStreamWriter.h"


class CLITestsWriter : public TestsWriter {
public:
    explicit CLITestsWriter(): TestsWriter(nullptr) {};

    void writeTestsWithProgress(tests::TestsMap &testMap,
                                const std::string &message,
                                const fs::path &testDirPath,
                                std::function<void(tests::Tests &)> &&prepareTests,
                                std::function<void()> &&prepareTotal) override;

    void writeReport(const std::string &content,
                     const std::string &message,
                     const fs::path &pathToStore) const override;

private:
    static bool writeTestFile(const tests::Tests &tests, const fs::path &testDirPath);
};


#endif // UNITTESTBOT_CLITESTSWRITER_H
