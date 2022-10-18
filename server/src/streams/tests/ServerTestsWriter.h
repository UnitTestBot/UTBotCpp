#ifndef UNITTESTBOT_VSCODETESTSWRITER_H
#define UNITTESTBOT_VSCODETESTSWRITER_H

#include "Tests.h"
#include "TestsWriter.h"
#include "streams/IStreamWriter.h"

#include <utils/FileSystemUtils.h>

class ServerTestsWriter : public TestsWriter {
public:
    explicit ServerTestsWriter(grpc::ServerWriter<testsgen::TestsResponse> *writer,
                               bool synchronizeCode)
        : TestsWriter(writer), synchronizeCode(synchronizeCode)  {};

    void writeTestsWithProgress(tests::TestsMap &testMap,
                                const std::string &message,
                                const fs::path &testDirPath,
                                std::function<void(tests::Tests &)> &&prepareTests,
                                std::function<void()> &&prepareTotal) override;

    void writeReport(const std::string &content,
                     const std::string &message,
                     const fs::path &pathToStore) const override;

private:
    [[nodiscard]] virtual bool writeFileAndSendResponse(const tests::Tests &tests,
                                                        const fs::path &testDirPath,
                                                        const std::string &message,
                                                        double percent,
                                                        bool isCompleted) const;

    bool synchronizeCode;
};


#endif // UNITTESTBOT_VSCODETESTSWRITER_H
