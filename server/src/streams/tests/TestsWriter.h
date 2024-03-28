#ifndef UNITTESTBOT_TESTSWRITER_H
#define UNITTESTBOT_TESTSWRITER_H

#include "Tests.h"
#include "streams/BaseWriter.h"
#include "streams/IStreamWriter.h"
#include "streams/ServerWriter.h"
#include "stubs/Stubs.h"
#include "utils/ExecUtils.h"

#include <protobuf/testgen.grpc.pb.h>

class TestsWriter : public utbot::ServerWriter<testsgen::TestsResponse> {
public:
    explicit TestsWriter(grpc::ServerWriter<testsgen::TestsResponse> *writer);

    virtual void writeTestsWithProgress(tests::TestsMap &testMap,
                                        const std::string &message,
                                        const fs::path &testDirRelPath,
                                        std::function<void(tests::Tests &)> &&prepareTests,
                                        std::function<void()> &&prepareTotal) = 0;

    virtual void writeReport(const std::string &content,
                             const std::string &message,
                             const fs::path &pathToStore) const;

    static void backupIfExists(const fs::path &filePath);

protected:
    void writeCompleted(tests::TestsMap const &testMap, int totalTestsCounter);

};


#endif // UNITTESTBOT_TESTSWRITER_H
