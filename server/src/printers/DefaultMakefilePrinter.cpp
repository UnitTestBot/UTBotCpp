#include "DefaultMakefilePrinter.h"

#include "utils/Copyright.h"

namespace printer {

DefaultMakefilePrinter::DefaultMakefilePrinter() {
    writeCopyrightHeader();
}

void DefaultMakefilePrinter::comment(std::string const &message) {
    ss << StringUtils::stringFormat("# %s\n", message);
}

void DefaultMakefilePrinter::declareVariable(std::string const &name, std::string const &value) {
    ss << StringUtils::stringFormat("%s = %s\n", name, value);
}

void DefaultMakefilePrinter::declareVariableIfNotDefined(std::string const &variableName, std::string const &ifNotDefinedValue) {
    ss << StringUtils::stringFormat("ifndef %s\n", variableName);
    ss << TAB;
    declareVariable(variableName, ifNotDefinedValue);
    ss << "endif\n";
}

void DefaultMakefilePrinter::declareVariableWithPriority(std::string const &variableName,
                                                        std::string const &variablePath) {
    std::string pattern = R"(ifneq ("$(wildcard %s)\", ""))";
    ss << StringUtils::stringFormat(pattern + "\n", variablePath);
    ss << TAB;
    declareVariable(variableName, variablePath);
    ss << "endif\n";
}

void DefaultMakefilePrinter::declareAction(const std::string &name) {
    ss << name << "\n";
}
void DefaultMakefilePrinter::declareInclude(const std::string &otherMakefileName) {
    ss << StringUtils::stringFormat("include %s\n", otherMakefileName);
}

void DefaultMakefilePrinter::writeCopyrightHeader() {
    ss << Copyright::GENERATED_MAKEFILE_HEADER << NL;
}

}
