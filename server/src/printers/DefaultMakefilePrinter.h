#ifndef UNITTESTBOT_DEFAULTMAKEFILEPRINTER_H
#define UNITTESTBOT_DEFAULTMAKEFILEPRINTER_H

#include "Printer.h"

#include <string>
#include <vector>

namespace printer {
class DefaultMakefilePrinter : public Printer {
public:
    static const std::string TARGET_ALL;
    static const std::string TARGET_BUILD;
    static const std::string TARGET_RUN;
    static const std::string TARGET_FORCE;

    DefaultMakefilePrinter();

    ~DefaultMakefilePrinter() override = default;

    void comment(std::string const &message);

    virtual void declareVariable(std::string const &name, std::string const &value);

    void declareAction(std::string const &name);

    void declareVariableIfNotDefined(std::string const &variableName, std::string const &ifNotDefinedValue);

    void declareVariableWithPriority(std::string const &variableName, std::string const &variablePath);

    template<class ContainerD = std::initializer_list<std::string>,
            class ContainerA = std::initializer_list<std::string>>
    void declareTarget(std::string const &name,
                       ContainerD &&dependencies,
                       ContainerA &&actions) {
        ss << StringUtils::stringFormat(
                "%s : %s\n\t%s\n", name,
                StringUtils::joinWith(std::forward<ContainerD>(dependencies), " "),
                StringUtils::joinWith(std::forward<ContainerA>(actions), "\n\t"));
    }

    void declareInclude(const std::string &otherMakefileName);

protected:
    void writeCopyrightHeader() final;
};
}


#endif //UNITTESTBOT_DEFAULTMAKEFILEPRINTER_H
