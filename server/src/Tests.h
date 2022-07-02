#ifndef UNITTESTBOT_TESTS_H
#define UNITTESTBOT_TESTS_H

#include "Include.h"
#include "LineInfo.h"
#include "types/Types.h"
#include "utils/CollectionUtils.h"
#include "utils/PrinterUtils.h"
#include "utils/ErrorInfo.h"

#include <klee/KTest.h>
#include <klee/TestCase.h>
#include "json.hpp"
#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>
#include "Paths.h"

#include <iterator>
#include <memory>
#include <utility>
#include <queue>
#include <optional>

namespace tests {
    class StructValueView;
}

namespace printer {
    class TestsPrinter;
    struct MultiLinePrinter {
        static std::string print(TestsPrinter *printer, const tests::StructValueView *view);
    };
}

namespace tests {

    const std::string LAZYNAME = "unnamed";

    using MapAddressName = std::unordered_map<size_t , std::string>;

    bool isUnnamed(char *name);

    struct UTBotKTestObject {
        std::string name;
        std::vector<char> bytes;
        std::vector<Offset> offsets;
        size_t address;
        bool is_lazy = false;

        UTBotKTestObject(std::string name,
                         std::vector<char> bytes,
                         std::vector<Offset> offsets,
                         size_t address,
                         bool is_lazy);

        explicit UTBotKTestObject(const ConcretizedObject &kTestObject);
    };
    struct UTBotKTest {
        enum class Status {
            SUCCESS,
            FAILED
        };
        std::vector<UTBotKTestObject> objects;
        Status status;
        ErrorInfo errorInfo;

        UTBotKTest(std::vector<UTBotKTestObject> objects, Status status)
            : objects(std::move(objects)), status(status) {
        }

        UTBotKTest(std::vector<UTBotKTestObject> objects, Status status,
                    ErrorInfo errorInfo_)
            : objects(std::move(objects)), status(status),
            errorInfo(std::move(errorInfo_)) {
        }
    };
    using UTBotKTestList = std::vector<UTBotKTest>;

    /**
     * This function checks if value string representation is a floating-point special value.
     * @param value - string value representation
     * @return whether value is a floating-point special value
     */
    bool isFPSpecialValue(const std::string &value);

    /**
     * This function transforms floating-point special values to C++ representation.
     * @param value - string value representation
     * @return string value representation in C++
     */
    std::string processFPSpecialValue(const std::string &value);

    /**
    * Abstract representation of some value.
    */
    struct AbstractValueView {
        AbstractValueView() = default;

        ~AbstractValueView() = default;
    public:
        /**
         * Returns string representation of the value.
         */
        [[nodiscard]] virtual std::string getEntryValue(printer::TestsPrinter *printer) const = 0;

        virtual bool containsFPSpecialValue() {
            return false;
        }

        /**
         * Returns subviews of this view.
         */
        [[nodiscard]] virtual const std::vector<std::shared_ptr<AbstractValueView>> &getSubViews() const {
            return this->subViews;
        };


    protected:
        explicit AbstractValueView(std::vector<std::shared_ptr<AbstractValueView>> subViews) : subViews(std::move(subViews)) {}

        std::vector<std::shared_ptr<AbstractValueView>> subViews{};
    };

    /**
    * It's value is stored as a string. Subviews are always empty.
    */
    struct JustValueView : AbstractValueView {
        explicit JustValueView(std::string value) : AbstractValueView(), entryValue(std::move(value)) {}

        [[nodiscard]] std::string getEntryValue(printer::TestsPrinter *printer) const override {
            return entryValue;
        }

        bool containsFPSpecialValue() override {
            return tests::isFPSpecialValue(entryValue);
        }

    private:
        std::string entryValue;
    };

    /**
     * Value of a void type. Subviews are always empty, entry value is empty.
     */
    struct VoidValueView : AbstractValueView {
        explicit VoidValueView() = default;

        [[nodiscard]] std::string getEntryValue(printer::TestsPrinter *printer) const override {
            return "";
        }

        bool containsFPSpecialValue() override {
            return false;
        }
    };

    /**
    * Representation of primitive value.
    */
    struct PrimitiveValueView : JustValueView {
        explicit PrimitiveValueView(std::string value) : JustValueView(std::move(value)) {}
    };

    /**
    * Representation of string value.
    */
    struct StringValueView : JustValueView {
    public:
        explicit StringValueView(std::string value) : JustValueView(std::move(value)) {}
    };

    /**
    * Representation of pointer to function.
    */
    struct FunctionPointerView : JustValueView {
    public:
        explicit FunctionPointerView(std::string value) : JustValueView(std::move(value)) {}
    };

    /**
    * Representation of enum.
    */
    struct EnumValueView : JustValueView {
    public:
        explicit EnumValueView(std::string value) : JustValueView(std::move(value)) {}
    };

    /**
     * Representation of array value. It's value is stored as a string. Subviews of the ArrayValueView are its elements.
     */
    struct ArrayValueView : AbstractValueView {
        explicit ArrayValueView(std::vector<std::shared_ptr<AbstractValueView>> &subViews)
            : AbstractValueView(subViews) {}

        [[nodiscard]] std::string getEntryValue(printer::TestsPrinter *printer) const override {
            std::vector<std::string> entries;
            for (const auto &subView : subViews) {
                entries.push_back(subView->getEntryValue(printer));
            }

            return "{" + StringUtils::joinWith(entries, ", ") + "}";
        }

        bool containsFPSpecialValue() override {
            for (const auto &subView : subViews) {
                if (subView->containsFPSpecialValue()) {
                    return true;
                }
            }
            return false;
        }
    };

    /**
     * Representation of struct value. It's value is stored as a string. Subviews of StructValueView are its fields.
     */
    struct StructValueView : AbstractValueView {
        explicit StructValueView(bool _isCLike,
                                 std::vector<std::string> _fields,
                                 std::vector<std::shared_ptr<AbstractValueView>> subViews,
                                 std::optional<std::string> entryValue)
            : AbstractValueView(std::move(subViews)), entryValue(std::move(entryValue)),
              isCLike(_isCLike),
              fields(std::move(_fields)){
        }

        [[nodiscard]] const std::vector<std::shared_ptr<AbstractValueView>> &getSubViews() const override {
            return this->subViews;
        }

        [[nodiscard]] std::string getEntryValue(printer::TestsPrinter *printer) const override {
            if (entryValue.has_value()) {
                return entryValue.value();
            }

            if (printer != nullptr) {
                return printer::MultiLinePrinter::print(printer, this);
            }

            std::vector<std::string> entries;
            for (const auto &subView : subViews) {
                entries.push_back(subView->getEntryValue(nullptr));
            }

            return "{" + StringUtils::joinWith(entries, ", ") + "}";
        }

        bool containsFPSpecialValue() override {
            for (const auto &subView : subViews) {
                if (subView->containsFPSpecialValue()) {
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] std::string getFieldPrefix(int i) const {
            std::string prefix = "." + fields[i] + " = ";
            if (isCLike) {
                return prefix;
            }
            // it is not C Struct-initialization, but C++ List-initialization.
            // The `designation` isn't allowed.
            // https://en.cppreference.com/w/c/language/struct_initialization
            // https://en.cppreference.com/w/cpp/language/list_initialization
            return  "/*" + prefix + "*/";
        }

    private:
        bool isCLike;
        std::vector<std::string> fields;
        std::optional<std::string> entryValue;
    };

    /**
    * Representation of union.
    */
    struct UnionValueView : AbstractValueView {
    public:
        explicit UnionValueView(const std::string &typeName,
                                const std::shared_ptr<AbstractValueView> &rawDataView,
                                std::vector<std::shared_ptr<AbstractValueView>,
                                std::allocator<std::shared_ptr<AbstractValueView>>> subViews);

        [[nodiscard]] std::string getEntryValue(printer::TestsPrinter *printer) const override {
            return entryValue;
        }

        bool containsFPSpecialValue() override {
            return false;
        }

    private:
        std::string entryValue;
    };

    struct InitReference {
        std::string varName;
        std::string refName;
        std::string typeName;
        InitReference(std::string varName, std::string refName, std::string typeName)
            : varName(std::move(varName)), refName(std::move(refName)), typeName(std::move(typeName)) {
        }
    };

    struct Tests {

        struct TypeAndVarName {
            types::Type type;
            std::string varName;

            TypeAndVarName(types::Type type, std::string varName)
                : type(std::move(type)), varName(std::move(varName)) {
            }
        };

        struct MethodParam {
            types::Type type;
            std::string name;
            std::optional<uint64_t> alignment;

            bool hasIncompleteType = false;

            MethodParam(types::Type type,
                        std::string name,
                        std::optional<uint64_t> alignment,
                        bool hasIncompleteType = false)
                : type(std::move(type)), name(std::move(name)), alignment(std::move(alignment)),
                  hasIncompleteType(hasIncompleteType) {

            }

            std::string underscoredName() const {
                return "_" + name;
            }

            bool isChangeable() const {
                if((type.isObjectPointer() || type.isLValueReference()) &&
                    !type.isTypeContainsFunctionPointer() &&
                    !type.isConstQualifiedValue() && !types::TypesHandler::baseTypeIsVoid(type)) {
                    return true;
                }
                return false;
            }

            std::string dataVariableName() const {
                return this->type.isTwoDimensionalPointer() ?
                       this->underscoredName() :
                       this->name;
            }
        };

        struct TestCaseParamValue {
            std::string name;
            std::optional<uint64_t> alignment;
            std::shared_ptr<AbstractValueView> view;
            std::vector<MethodParam> lazyParams;
            std::vector<TestCaseParamValue> lazyValues;
            TestCaseParamValue() = default;
            TestCaseParamValue(std::string name,
                               std::optional<uint64_t> alignment,
                               std::shared_ptr<AbstractValueView> view)
                : name(std::move(name)), alignment(alignment), view(std::move(view)) {};
        };

        struct TestCaseDescription {
            std::string suiteName;

            std::vector<TestCaseParamValue> globalPreValues;
            std::vector<TestCaseParamValue> globalPostValues;
            std::vector<UTBotKTestObject> objects;

            std::vector<MethodParam> stubValuesTypes;
            std::vector<TestCaseParamValue> stubValues;

            MapAddressName lazyAddressToName;
            std::vector<InitReference> lazyReferences;

            std::vector<TestCaseParamValue> funcParamValues;
            std::vector<TestCaseParamValue> paramPostValues;
            TestCaseParamValue returnValue;
            TestCaseParamValue functionReturnNotNullValue;
            TestCaseParamValue kleePathFlagSymbolicValue;
            std::optional <TestCaseParamValue> stdinValue = std::nullopt;
            std::optional<TestCaseParamValue> classPreValues;
            std::optional<TestCaseParamValue> classPostValues;
            ErrorInfo errorInfo;
        };

        struct MethodTestCase {
            std::string suiteName;

            std::vector<TestCaseParamValue> globalPreValues;
            std::vector<TestCaseParamValue> globalPostValues;
            std::optional <TestCaseParamValue> stdinValue;
            std::vector<InitReference> lazyReferences;
            std::vector<UTBotKTestObject> objects;

            MapAddressName lazyAddressToName;

            std::vector<MethodParam> stubValuesTypes;
            std::vector<TestCaseParamValue> stubValues;

            std::vector<TestCaseParamValue> paramValues;
            std::vector<TestCaseParamValue> paramPostValues;
            std::vector<TestCaseParamValue> stubParamValues;
            std::vector<MethodParam> stubParamTypes;
            TestCaseParamValue returnValue;
            std::optional<TestCaseParamValue> classPreValues;
            std::optional<TestCaseParamValue> classPostValues;
            ErrorInfo errorInfo;

            [[nodiscard]] bool isError() const;
        };

        struct Modifiers {
            bool isStatic;
            bool isExtern;
            bool isInline;
        };

        struct MethodDescription {
            std::optional<MethodParam> classObj;
            std::string name;
            typedef std::unordered_map<std::string, std::string> SuiteNameToCodeTextMap;
            std::string stubsText;
            SuiteNameToCodeTextMap codeText;
            std::string paramsString;

            types::Type returnType;
            bool hasIncompleteReturnType = false;

            std::optional<std::string> sourceBody;
            Modifiers modifiers;
            bool isVariadic = false;
            std::vector<MethodParam> globalParams;
            std::vector<MethodParam> params;

            typedef std::unordered_map<std::string, std::shared_ptr<types::FunctionInfo>> FPointerMap;
            FPointerMap functionPointers;
            std::vector<MethodTestCase> testCases;
            typedef std::unordered_map<std::string, std::vector<MethodTestCase>> SuiteNameToTestCasesMap;
            SuiteNameToTestCasesMap suiteTestCases;

            bool operator==(const MethodDescription &other) const;

            MethodDescription();

            [[nodiscard]] std::vector<types::Type> getParamTypes() const {
                return CollectionUtils::transform(params, [](auto const& param) {
                    return param.type;
                });
            }

            [[nodiscard]] std::vector<std::string> getParamNames() const {
                return CollectionUtils::transform(params, [](MethodParam const& param) {
                    return param.name;
                });
            }

            [[nodiscard]] types::FunctionInfo toFunctionInfo() {
                types::FunctionInfo fInfo;
                fInfo.isArray = false;
                fInfo.name = name;
                fInfo.returnType = returnType;
                for (const auto& param: params) {
                    fInfo.params.push_back({param.type, param.name});
                }
                return fInfo;
            }

            [[nodiscard]] static MethodDescription fromFunctionInfo(const types::FunctionInfo& fInfo) {
                MethodDescription method;
                method.name = fInfo.name;
                method.returnType = fInfo.returnType;
                for (const auto& param: fInfo.params) {
                    method.params.emplace_back(param.type, param.name, std::nullopt);
                }
                return method;
            }

            [[nodiscard]] bool hasChangeable() const {
                for(const auto& i : params) {
                    if (i.isChangeable()) {
                        return true;
                    }
                }
                return false;
            }

            [[nodiscard]] bool isClassMethod() const {
                return classObj.has_value();
            }

            [[nodiscard]] std::optional<std::string> getClassName() const {
                if (isClassMethod()) {
                    return std::make_optional(classObj->name);
                }
                return std::nullopt;
            }

            [[nodiscard]] std::optional<std::string> getClassTypeName() const {
                if (isClassMethod()) {
                    return std::make_optional(classObj->type.typeName());
                }
                return std::nullopt;
            }
        };

        struct MethodDescriptionToStringEqual {
            using is_transparent [[maybe_unused]] = void;
        };
        struct MethodDescriptionHash {
            std::size_t operator()(const MethodDescription &methodDescription) const;
        };

        using MethodsMap = tsl::ordered_map<std::string, MethodDescription>;

        static const std::string DEFAULT_SUITE_NAME;
        static const std::string ERROR_SUITE_NAME;
        static const MethodParam &getStdinMethodParam();

        fs::path sourceFilePath;
        std::string sourceFileNameNoExt; // without extension
        fs::path relativeFileDir;   // relative to project root dir
        std::string testFilename;
        fs::path testHeaderFilePath;
        fs::path testSourceFilePath;

        std::vector<Include> srcFileHeaders;
        std::vector<Include> headersBeforeMainHeader;
        std::optional<Include> mainHeader;
        MethodsMap methods; // method's name -> description
        std::string code;       // contains final code of test file
        std::string headerCode; // contains code of header
        std::vector<std::string> commentBlocks{};
        std::string stubs; // language-independent stubs definitions

        std::uint32_t errorMethodsNumber;
        std::uint32_t regressionMethodsNumber;

        bool isFilePresentedInCommands = true;
        bool isFilePresentedInArtifact = true;
    };

    typedef CollectionUtils::OrderedMapFileTo<Tests> TestsMap;

    struct TestMethod {
        std::string methodName;
        fs::path bitcodeFilePath;
        fs::path sourceFilePath;
        bool operator==(const TestMethod &rhs) const;
        bool operator!=(const TestMethod &rhs) const;

        TestMethod(std::string methodName, fs::path bitcodeFile, fs::path sourceFilename);
    };

    using MethodKtests = std::unordered_map<TestMethod, UTBotKTestList, HashUtils::TestMethodHash>;

    /**
     * This class provides functionality to parse Klee output given in
     * KTestObject format.
     */
    class KTestObjectParser {
    public:
        explicit KTestObjectParser(types::TypesHandler &typesHandler)
            : typesHandler(typesHandler){};

        /**
         * Parses given klee objects, reads result ot the testsMap.
         * @param batch
         * @param tests
         * @param filterByLineFlag
         * @param predicateInfo
         * @param verbose
         */
        void parseKTest(const MethodKtests &batch,
                        tests::Tests &tests,
                        const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                        bool filterByLineFlag,
                        std::shared_ptr<LineInfo> lineInfo);
    private:
        types::TypesHandler &typesHandler;

        struct RawKleeParam {
            std::string paramName;
            std::vector<char> rawData;

            RawKleeParam(std::string paramName, std::vector<char> rawData)
                : paramName(std::move(paramName)), rawData(std::move(rawData)) {
            }

            [[nodiscard]] [[maybe_unused]] bool hasPrefix(const std::string &prefix) const {
                return StringUtils::startsWith(paramName, prefix);
            }
        };

        struct JsonIndAndParam {
            size_t jsonInd;
            Tests::MethodParam param;
            Tests::TestCaseParamValue& paramValue;
            JsonIndAndParam(size_t jsonInd, Tests::MethodParam param,
                            Tests::TestCaseParamValue& paramValue) : jsonInd(jsonInd),
                  param(std::move(param)), paramValue(paramValue) {
            }
        };

        /**
         * Parses KTestObject that represents test cases generated by Klee.
         * @param cases
         * @param filterByLineFlag
         * @param predicateInfo
         * @param methodDescription
         */
        void parseTestCases(const UTBotKTestList &cases,
                            bool filterByLineFlag,
                            Tests::MethodDescription &methodDescription,
                            const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                            std::shared_ptr<LineInfo> lineInfo);
        /**
         * Parses parameters that are stored in given objects. Then parameters
         * are written into paramValues.
         * @param testCases
         * @param filterByLineFlag
         * @param predicateInfo
         * @param methodDescription
         * @param traceStream
         */
        Tests::TestCaseDescription
        parseTestCaseParameters(const UTBotKTest &testCases,
                                Tests::MethodDescription &methodDescription,
                                const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                                std::stringstream &traceStream);

        std::shared_ptr<AbstractValueView>
        testParameterView(const RawKleeParam &kleeParam,
                          const Tests::TypeAndVarName &param,
                          types::PointerUsage usage,
                          const MapAddressName &fromAddressToName,
                          std::vector<InitReference> &initReferences,
                          const std::optional<const Tests::MethodDescription> &testingMethod = std::nullopt);

        std::shared_ptr<ArrayValueView> multiArrayView(const std::vector<char> &byteArray,
                                                       const types::Type &type,
                                                       size_t arraySize,
                                                       size_t offset,
                                                       types::PointerUsage usage);

        std::shared_ptr<ArrayValueView> arrayView(const std::vector<char> &byteArray,
                                                  const types::Type &type,
                                                  size_t arraySize,
                                                  unsigned int offset,
                                                  types::PointerUsage usage);

        static std::shared_ptr<StringValueView> stringLiteralView(const std::vector<char> &byteArray,
                                                                  size_t length = 0);

        std::shared_ptr<FunctionPointerView> functionPointerView(const std::optional<std::string>& scopeName,
                                                                 const std::string &methodName,
                                                                 const std::string &paramName);

        std::shared_ptr<FunctionPointerView> functionPointerView(const std::string &structName,
                                                                 const std::string &fieldName);

        std::shared_ptr<UnionValueView> unionView(const std::vector<char> &byteArray,
                                                  types::UnionInfo &unionInfo,
                                                  unsigned int offset,
                                                  types::PointerUsage usage);

        std::shared_ptr<StructValueView> structView(const std::vector<char> &byteArray,
                                                    types::StructInfo &curStruct,
                                                    unsigned int offset,
                                                    types::PointerUsage usage);

        std::shared_ptr<StructValueView> structView(const std::vector<char> &byteArray,
                                                    types::StructInfo &curStruct,
                                                    unsigned int offset,
                                                    types::PointerUsage usage,
                                                    const std::optional<const Tests::MethodDescription> &testingMethod,
                                                    const std::string &name,
                                                    const MapAddressName &fromAddressToName,
                                                    std::vector<InitReference> &initReferences);

        std::shared_ptr<PrimitiveValueView> primitiveView(const std::vector<char> &byteArray,
                                                          const types::Type &type,
                                                          size_t offset,
                                                          size_t len);
        static std::shared_ptr<EnumValueView> enumView(const std::vector<char> &byteArray,
                                                       types::EnumInfo &enumInfo,
                                                       size_t offset,
                                                       size_t len);

        std::string primitiveCharView(const types::Type &type, std::string value);
        static std::string primitiveBoolView(const std::string &value);

        constexpr static const char *const KLEE_PATH_FLAG = "kleePathFlag";

        const std::string PointerWidthType = "unsigned long long";
        const size_t PointerWidthSize = 8;

        constexpr static const char *const KLEE_PATH_FLAG_SYMBOLIC = "kleePathFlagSymbolic";
        static std::vector<RawKleeParam>::const_iterator
        getKleeParam(const std::vector<RawKleeParam> &rawKleeParams, std::string name);
        static RawKleeParam getKleeParamOrThrow(const std::vector<RawKleeParam> &rawKleeParams, const std::string &name);

        Tests::TestCaseDescription
        parseTestCaseParams(const UTBotKTest &ktest,
                            const Tests::MethodDescription &methodDescription,
                            const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                            const std::stringstream &traceStream);
        std::vector<std::shared_ptr<AbstractValueView>> collectUnionSubViews(const std::vector<char> &byteArray,
                                                                             const types::UnionInfo &info,
                                                                             unsigned int offset,
                                                                             types::PointerUsage usage);
        void processGlobalParamPreValue(Tests::TestCaseDescription &testCaseDescription,
                                        const Tests::MethodParam &globalParam,
                                        std::vector<RawKleeParam> &rawKleeParams);

        void processSymbolicStdin(Tests::TestCaseDescription &testCaseDescription,
                                  std::vector<RawKleeParam> &rawKleeParams);

        void processGlobalParamPostValue(Tests::TestCaseDescription &testCaseDescription,
                                         const Tests::MethodParam &globalParam,
                                         std::vector<RawKleeParam> &rawKleeParams);

        void processClassPostValue(Tests::TestCaseDescription &testCaseDescription,
                                   const Tests::MethodParam &param,
                                   std::vector<RawKleeParam> &rawKleeParams);

        void processParamPostValue(Tests::TestCaseDescription &testCaseDescription,
                                   const Tests::MethodParam &param,
                                   std::vector<RawKleeParam> &rawKleeParams);

        void processStubParamValue(Tests::TestCaseDescription &testCaseDescription,
                                   const std::unordered_map<std::string, types::Type>& methodNameToReturnTypeMap,
                                   std::vector<RawKleeParam> &rawKleeParams);

        static void addToOrder(const std::vector<UTBotKTestObject> &objects,
                               const std::string &paramName,
                               const types::Type &paramType,
                               Tests::TestCaseParamValue &paramValue,
                               std::vector<bool> &visited,
                               std::queue<JsonIndAndParam>& order);

        void assignTypeUnnamedVar(Tests::MethodTestCase &testCase,
                                  const Tests::MethodDescription &methodDescription);

        void assignTypeStubVar(Tests::MethodTestCase &testCase,
                               const Tests::MethodDescription &methodDescription);

        size_t findFieldIndex(const types::StructInfo &structInfo, size_t offset);

        types::Type traverseLazyInStruct(std::vector<bool> &visited,
                                         const types::Type &curVarType,
                                         size_t offset,
                                         const Tests::MethodTestCase &testCase,
                                         const Tests::MethodDescription &methodDescription);

        std::shared_ptr<AbstractValueView> getLazyPointerView(const MapAddressName &fromAddressToName,
                                                              std::vector<InitReference> &initReferences,
                                                              const std::string &name,
                                                              std::string res,
                                                              const types::Type &paramType) const;
    };
    /**
     * @brief This function is used for converting primiive value of a specific type
     * To a string value which we can print to .cpp file.
     */
    template <typename T>
    std::enable_if_t<!std::is_floating_point<T>::value, std::string>
    primitiveValueToString(T value) {
        return std::to_string(value);
    }

    template <typename T>
    std::enable_if_t<std::is_floating_point<T>::value, std::string>
    primitiveValueToString(T value) {
        std::stringstream ss;
        ss << std::scientific;
        ss << value;
        return ss.str();
    }
    /**
     * This function is used for converting sequence of bytes to specific type.
     * Returns string representation of a value recorded in byteArray.
     * @tparam T - type of value
     * @param byteArray
     * @param offset - initial position of a value in byteArray
     * @param len - size of T
     * @return string representation of value
     */
    template <typename T>
    std::string readBytesAsValue(const std::vector<char> &byteArray, size_t offset, size_t len) {
        char bytes[len];
        for (int j = 0; j < len; j++) {
            bytes[j] = byteArray[offset + j];
        }
        T *pTypeValue = (T *)bytes;
        T pValue = *pTypeValue;
        return primitiveValueToString<T>(pValue);
    }
    /**
     * Same as readBytesAsValue, but returns result of readBytesAsValue that is already
     * parametrized by type that corresponds to given typeName.
     * @param typeName - string name of type
     * @param byteArray - array of bytes
     * @param offset - initial position
     * @param len - size of type that corresponds to typeName
     * @return string representation of value.
     */
    std::string readBytesAsValueForType(const std::vector<char> &byteArray,
                                        const std::string &typeName,
                                        unsigned int offset,
                                        unsigned int len);
} // tests
#endif // UNITTESTBOT_TESTS_H
