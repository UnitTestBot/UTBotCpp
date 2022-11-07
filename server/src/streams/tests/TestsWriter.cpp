#include "TestsWriter.h"

#include "SARIFGenerator.h"
#include "utils/FileSystemUtils.h"

#include "loguru.h"


TestsWriter::TestsWriter(grpc::ServerWriter<testsgen::TestsResponse> *writer): ServerWriter(writer) {}

void TestsWriter::writeCompleted(const tests::TestsMap &testMap, int totalTestsCounter) {
    std::string finalMessage;
    if (totalTestsCounter == 1) {
        finalMessage = StringUtils::stringFormat("%d test file generated.", totalTestsCounter);
    } else {
        finalMessage = StringUtils::stringFormat("%d test files generated.", totalTestsCounter);
    }
    writeProgress(finalMessage, 100.0, true);
}

void TestsWriter::writeReport(const std::string &content,
                              const std::string &message,
                              const fs::path &pathToStore) const
{
    try {
        backupIfExists(pathToStore);
    } catch (const std::exception &e) {
        LOG_S(ERROR) << e.what()
                     << ": problem in `writeReport` with "
                     << pathToStore;
    }
    FileSystemUtils::writeToFile(pathToStore, content);
}

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

void TestsWriter::backupIfExists(const fs::path &filePath) {
    if (fs::exists(filePath)) {
        std::filesystem::file_time_type ftime = fs::last_write_time(filePath);
        time_t tt = to_time_t(ftime);
        tm *gmt = localtime(&tt);

        std::stringstream nfn;
        nfn << filePath.stem().c_str() << "-" << std::put_time(gmt, "%Y%m%d%H%M%S")
            << filePath.extension().c_str();

        LOG_S(INFO) << "Backup previous report to " << nfn.str();
        fs::rename(filePath, filePath.parent_path() / nfn.str());
    }
}

void TestsWriter::writeFile(const std::string& content, const std::string& message, const std::string& filePath) const {
    FileSystemUtils::writeToFile(filePath, content);
}
