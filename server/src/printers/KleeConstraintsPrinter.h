#ifndef UNITTESTBOT_KLEECONSTRAINTSPRINTER_H
#define UNITTESTBOT_KLEECONSTRAINTSPRINTER_H

#include "Printer.h"

using tests::Tests;

namespace printer {
    class KleeConstraintsPrinter: public Printer {
    public:
        explicit KleeConstraintsPrinter(const types::TypesHandler *typesHandler, utbot::Language srcLanguage);

        utbot::Language getLanguage() const override;

        Stream genConstraints(const std::string &name, const types::Type& type, const std::vector<std::string>& names={});

        Stream genConstraints(const Tests::MethodParam &param, const std::vector<std::string>& names={});

        void setTabsDepth(const size_t depth) {
            tabsDepth = depth;
        }

    private:
        types::TypesHandler const *typesHandler;

        struct ConstraintsState {
            std::string paramName;
            std::string curElement;
            types::Type curType;
            bool endString;
            int depth = 0;
        };

        void genConstraintsForPrimitive(const ConstraintsState &state, const std::vector<std::string>& names={});

//        void genConstraintsForPointerOrArray(const ConstraintsState &state);

        void genConstraintsForMultiPointerOrArray(const ConstraintsState &state,
                                                  std::vector<size_t> sizes);

        void genConstraintsForStruct(const ConstraintsState &state);

        void genConstraintsForEnum(const ConstraintsState &state);

        void genConstraintsForPointerInStruct(const ConstraintsState &state);

        static std::string cexConstraints(const std::string &name, const types::Type &type, const std::vector<std::string>& names={});

        void noConstraints(const std::string &cause);
    };
}



#endif // UNITTESTBOT_KLEECONSTRAINTSPRINTER_H
