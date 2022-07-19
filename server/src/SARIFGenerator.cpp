#include "SARIFGenerator.h"
#include "Paths.h"

#include "loguru.h"

#include <fstream>
#include <regex>
#include <unordered_map>

using namespace tests;

namespace sarif {
    // Some more logic above standard implementation
    //     `fs::relative(src, path)`
    // for the case of src as the relative path.
    // For example:
    //     getInProjectPath("/home/utbot/tmp/UTBotCpp/server/test/suites/object-file",
    //                      "test/suites/object-file/op/source2.c")
    // returns
    //     "op/source2.c"
    fs::path getInProjectPath(const fs::path &path, const fs::path &src) {
        fs::path relToProject;
        auto p = path.begin();
        auto s = src.begin();
        bool foundStartFragment = false;
        while (p != path.end() && s != src.end()) {
            if (*p == *s) {
                foundStartFragment = true;
                ++s;
            } else if (foundStartFragment) {
                break;
            }
            ++p;
        }
        if (foundStartFragment && p == path.end()) {
            while (s != src.end()) {
                relToProject = relToProject / *s;
                ++s;
            }
        }
        return relToProject;
    }

    void sarifAddTestsToResults(const utbot::ProjectContext &projectContext,
                                const Tests &tests,
                                json &results) {
        LOG_S(INFO) << "{stack";
        for (const auto &it : tests.methods) {
            for (const auto &methodTestCase : it.second.testCases) {
                json result;
                const std::vector<std::string> &descriptors = methodTestCase.errorDescriptors;
                std::string key;
                std::string value;
                json stackLocations;
                json codeFlowsLocations;
                json testLocation;
                bool canAddThisTestToSARIF = false;
                for (const std::string &descriptor : descriptors) {
                    std::stringstream streamOfDescriptor(descriptor);
                    std::string lineInDescriptor;
                    bool firstCallInStack = false;
                    while (getline(streamOfDescriptor, lineInDescriptor)) {
                        if (lineInDescriptor.empty() || lineInDescriptor[0] == '#')
                            continue;
                        if (isspace(lineInDescriptor[0])) {
                            if (key == "Stack") {
                                const std::regex stack_regex(
                                    R"regex(\s+#(.*) in ([^ ]*) [(][^)]*[)] at ([^:]*):(\d+))regex");
                                std::smatch stack_match;
                                if (!std::regex_match(lineInDescriptor, stack_match, stack_regex)) {
                                    LOG_S(ERROR) << "wrong `Stack` line: " << lineInDescriptor;
                                } else {
                                    const fs::path &srcPath = fs::path(stack_match[3]);
                                    const fs::path &relPathInProject = getInProjectPath(projectContext.projectPath, srcPath);
                                    const fs::path &fullPathInProject = projectContext.projectPath / relPathInProject;
                                    if (Paths::isSubPathOf(projectContext.buildDir, fullPathInProject)) {
                                        continue;
                                    }
                                    if (!relPathInProject.empty() && fs::exists(fullPathInProject)) {
                                        // stackLocations from project source
                                        json locationWrapper;
                                        locationWrapper["module"] = "project";
                                        {
                                            json location;
                                            location["physicalLocation"]["artifactLocation"]["uri"] = relPathInProject;
                                            location["physicalLocation"]["artifactLocation"]["uriBaseId"] = "%SRCROOT%";
                                            location["physicalLocation"]["region"]["startLine"] = std::stoi(stack_match[4]); // line number
                                            // commented, duplicated in message
                                            // location["logicalLocations"][0]["fullyQualifiedName"] = stack_match[2]; // call name
                                            location["message"]["text"] = stack_match[2].str() + std::string(" (source)"); // info for ANALYSIS STEP
                                            if (firstCallInStack) {
                                                firstCallInStack = false;
                                                result["locations"].push_back(location);
                                                stackLocations["message"]["text"] = "UTBot generated";
                                                codeFlowsLocations["message"]["text"] = "UTBot generated";
                                            }
                                            locationWrapper["location"] = location;
                                        }
                                        stackLocations["frames"].push_back(locationWrapper);
                                        codeFlowsLocations["locations"].push_back(locationWrapper);
                                    } else if (firstCallInStack) {
                                        // stackLocations from runtime, that is called by tested function
                                        json locationWrapper;
                                        locationWrapper["module"] = "external";
                                        {
                                            json location;
                                            location["physicalLocation"]["artifactLocation"] ["uri"] = srcPath.filename(); // just a name
                                            location["physicalLocation"]["artifactLocation"] ["uriBaseId"] = "%PATH%";
                                            location["physicalLocation"]["region"]["startLine"] = std::stoi(stack_match[4]); // line number
                                            // commented, duplicated in message
                                            // location["logicalLocations"][0]["fullyQualifiedName"] = stack_match[2]; // call name
                                            location["message"]["text"] = stack_match[2].str() + std::string(" (external)"); // info for ANALYSIS STEP
                                            locationWrapper["location"] = location;
                                        }
                                        stackLocations["frames"].push_back(locationWrapper);
                                        codeFlowsLocations["locations"].push_back(locationWrapper);
                                    } else {
                                        // the rest is the KLEE calls that are not applicable for navigation
                                        LOG_S(INFO) << "Skip path in stack frame :" << srcPath;
                                    }
                                }
                            }
                        } else {
                            int pos = lineInDescriptor.find(':');
                            if (pos == -1) {
                                LOG_S(ERROR) << "no key:" << lineInDescriptor;
                            } else {
                                if (key == "Stack") {
                                    // Check stack validity
                                    if (firstCallInStack) {
                                        LOG_S(ERROR) << "no visible stack in descriptor:" << descriptor;
                                    } else {
                                        canAddThisTestToSARIF = true;
                                    }
                                }
                                firstCallInStack = true;

                                key = lineInDescriptor.substr(0, pos);
                                value = lineInDescriptor.substr(pos + 1);
                                if (key == "Error") {
                                    result["message"]["text"] = value;
                                    result["level"] = "error";
                                } else if (key == ERROR_ID_KEY) {
                                    result["ruleId"] = value;
                                } else if (key == "Stack") {
                                    stackLocations = json();
                                    codeFlowsLocations = json();
                                } else if (key == TEST_FILE_KEY) {
                                    testLocation = json();
                                    testLocation["physicalLocation"]["artifactLocation"]["uri"] = fs::relative(value, projectContext.projectPath);
                                    testLocation["physicalLocation"]["artifactLocation"]["uriBaseId"] = "%SRCROOT%";
                                } else if (key == TEST_LINE_KEY) {
                                    testLocation["physicalLocation"]["region"]["startLine"] = std::stoi(value); // line number
                                } else if (key == TEST_NAME_KEY) {
                                    // commented, duplicated in message
                                    //  testLocation["logicalLocations"][0]["fullyQualifiedName"] = value; // call name
                                    testLocation["message"]["text"] = value + std::string(" (test)"); // info for ANALYSIS STEP
                                    {
                                        json locationWrapper;
                                        locationWrapper["location"] = testLocation;
                                        locationWrapper["module"] = "test";

                                        stackLocations["frames"].push_back(locationWrapper);
                                        codeFlowsLocations["locations"].push_back(locationWrapper);
                                    }
                                }
                            }
                        }
                    }
                }

                if (canAddThisTestToSARIF) {
                    result["stacks"].push_back(stackLocations);
                    result["codeFlows"][0]["threadFlows"].push_back(codeFlowsLocations);
                    results.push_back(result);
                }
            }
        }
        LOG_S(INFO) << "}stack";
    }

    std::string sarifPackResults(const json &results) {
        // POINT 3
        json sarifJson;
        sarifJson["$schema"] = "https://schemastore.azurewebsites.net/schemas/json/sarif-2.1.0-rtm.5.json";
        sarifJson["version"] = "2.1.0";
        {
            json runs;
            {
                json runAkaTestCase;
                runAkaTestCase["tool"]["driver"]["name"] = "UTBotCpp";
                runAkaTestCase["tool"]["driver"]["informationUri"] = "https://utbot.org";
                runAkaTestCase["results"] = results;
                runs.push_back(runAkaTestCase);
            }
            sarifJson["runs"] = runs;
        }
        return sarifJson.dump(2);
    }
}