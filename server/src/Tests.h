/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TESTS_H
#define UNITTESTBOT_TESTS_H

#include "Include.h"
#include "LineInfo.h"
#include "types/Types.h"
#include "utils/CollectionUtils.h"
#include "utils/PrinterUtils.h"

#include <klee/KTest.h>
#include <klee/TestCase.h>
#include "json.hpp"
#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include <iterator>
#include <memory>
#include <utility>
#include <queue>
#include <optional>

namespace tests {
    using std::string;
    using std::vector;
    using std::shared_ptr;

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
        vector<UTBotKTestObject> objects;
        Status status;

        UTBotKTest(std::vector<UTBotKTestObject> objects, Status status)
            : objects(std::move(objects)), status(status) {
        }
    };
    using UTBotKTestList = vector<UTBotKTest>;

    /**
     * This function checks if value string representation is a floating-point special value.
     * @param value - string value representation
     * @return whether value is a floating-point special value
     */
    bool isFPSpecialValue(const string &value);

    /**
     * This function transforms floating-point special values to C++ representation.
     * @param value - string value representation
     * @return string value representation in C++
     */
    string processFPSpecialValue(const string &value);

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
        [[nodiscard]] virtual string getEntryValue() const = 0;

        virtual bool containsFPSpecialValue() {
            return false;
        }

        /**
         * Returns subviews of this view.
         */
        [[nodiscard]] virtual const vector<shared_ptr<AbstractValueView>> &getSubViews() const {
            return this->subViews;
        };

        /**
         * For StructValueView should return vector of string representations of its field values.
         */
        virtual vector<string> fieldEntryValues() {
            return {};
        }

    protected:
        explicit AbstractValueView(vector<shared_ptr<AbstractValueView>> subViews) : subViews(std::move(subViews)) {}

        vector<shared_ptr<AbstractValueView>> subViews{};
    };

    /**
    * It's value is stored as a string. Subviews are always empty.
    */
    struct JustValueView : AbstractValueView {
        explicit JustValueView(string value) : AbstractValueView(), entryValue(std::move(value)) {}

        [[nodiscard]] string getEntryValue() const override {
            return entryValue;
        }

        bool containsFPSpecialValue() override {
            return tests::isFPSpecialValue(entryValue);
        }

    private:
        string entryValue;
    };

    /**
     * Value of a void type. Subviews are always empty, entry value is empty.
     */
    struct VoidValueView : AbstractValueView {
        explicit VoidValueView() = default;

        [[nodiscard]] string getEntryValue() const override {
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
        explicit PrimitiveValueView(string value) : JustValueView(std::move(value)) {}
    };

    /**
    * Representation of string value.
    */
    struct StringValueView : JustValueView {
    public:
        explicit StringValueView(string value) : JustValueView(std::move(value)) {}
    };

    /**
    * Representation of pointer to function.
    */
    struct FunctionPointerView : JustValueView {
    public:
        explicit FunctionPointerView(string value) : JustValueView(std::move(value)) {}
    };

    /**
    * Representation of enum.
    */
    struct EnumValueView : JustValueView {
    public:
        explicit EnumValueView(string value) : JustValueView(std::move(value)) {}
    };

    /**
     * Representation of array value. It's value is stored as a string. Subviews of the ArrayValueView are its elements.
     */
    struct ArrayValueView : AbstractValueView {
        explicit ArrayValueView(vector<shared_ptr<AbstractValueView>> &subViews)
            : AbstractValueView(subViews) {}

        [[nodiscard]] string getEntryValue() const override {
            vector<string> entries;
            for (const auto &subView : subViews) {
                entries.push_back(subView->getEntryValue());
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
     * In order to get fields and subfields values (leaves in terms of trees) method fieldEntryValues().
     */
    struct StructValueView : AbstractValueView {
        explicit StructValueView(vector<shared_ptr<AbstractValueView>> subViews,
                                 std::optional<std::string> entryValue)
            : AbstractValueView(std::move(subViews)), entryValue(std::move(entryValue)) {
        }

        [[nodiscard]] const vector<shared_ptr<AbstractValueView>> &getSubViews() const override {
            return this->subViews;
        }

        [[nodiscard]] string getEntryValue() const override {
            if (entryValue.has_value()) {
                return entryValue.value();
            }

            vector<string> entries;
            for (const auto &subView : subViews) {
                entries.push_back(subView->getEntryValue());
            }

            return "{" + StringUtils::joinWith(entries, ", ") + "}";
        }

        vector<string> fieldEntryValues() override {
            vector<string> result;
            for (const auto &subView : subViews) {
                vector<string> subFieldEntryValues = subView->fieldEntryValues();
                CollectionUtils::extend(result, subFieldEntryValues);
                if (subFieldEntryValues.empty()) {
                    result.push_back(subView->getEntryValue());
                }
            }

            return result;
        }

        bool containsFPSpecialValue() override {
            for (const auto &subView : subViews) {
                if (subView->containsFPSpecialValue()) {
                    return true;
                }
            }
            return false;
        }
    private:
        std::optional<std::string> entryValue;
    };

    /**
    * Representation of union.
    */
    struct UnionValueView : AbstractValueView {
    public:
        explicit UnionValueView(const string &typeName,
                                const shared_ptr<AbstractValueView> &rawDataView,
                                vector<shared_ptr<AbstractValueView>,
                                std::allocator<shared_ptr<AbstractValueView>>> subViews);

        [[nodiscard]] string getEntryValue() const override {
            return entryValue;
        }

        bool containsFPSpecialValue() override {
            return false;
        }

        vector<string> fieldEntryValues() override {
            return { getEntryValue() };
        }

    private:
        std::string entryValue;
    };

    struct InitReference {
        std::string varName;
        std::string refName;
        InitReference(std::string varName, std::string refName)
            : varName(std::move(varName)), refName(std::move(refName)) {
        }
    };

    struct Tests {

        struct TypeAndVarName {
            types::Type type;
            string varName;

            TypeAndVarName(types::Type type, std::string varName)
                : type(std::move(type)), varName(std::move(varName)) {
            }
        };
        struct MethodParam {
            types::Type type;
            string name;
            std::optional<uint64_t> alignment;

            bool hasIncompleteType = false;

            MethodParam(types::Type type,
                        string name,
                        std::optional<uint64_t> alignment,
                        bool hasIncompleteType = false)
                : type(std::move(type)), name(std::move(name)), alignment(std::move(alignment)),
                  hasIncompleteType(hasIncompleteType) {

            }

            string underscoredName() const {
                return "_" + name;
            }

            bool isChangeable() const {
                if((type.isObjectPointer() || type.isLValueReference()) &&
                    !type.isTypeContainsFunctionPointer() &&
                    !type.isConstQualifiedValue()) {
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
            string name;
            std::optional<uint64_t> alignment;
            shared_ptr<AbstractValueView> view;
            TestCaseParamValue() = default;
            TestCaseParamValue(string name,
                               std::optional<uint64_t> alignment,
                               shared_ptr<AbstractValueView> view)
                : name(std::move(name)), alignment(alignment), view(std::move(view)) {};
        };

        struct TestCaseDescription {
            string suiteName;

            vector<TestCaseParamValue> globalPreValues;
            vector<TestCaseParamValue> globalPostValues;
            vector<UTBotKTestObject> objects;

            vector<MethodParam> stubValuesTypes;
            vector<TestCaseParamValue> stubValues;

            MapAddressName fromAddressToName;
            vector<InitReference> lazyReferences;

            vector<TestCaseParamValue> funcParamValues;
            vector<TestCaseParamValue> paramPostValues;
            TestCaseParamValue returnValue;
            TestCaseParamValue functionReturnNotNullValue;
            TestCaseParamValue kleePathFlagSymbolicValue;
            std::optional <TestCaseParamValue> stdinValue = std::nullopt;
            std::optional<TestCaseParamValue> classPreValues;
            std::optional<TestCaseParamValue> classPostValues;
        };

        struct MethodTestCase {
            string suiteName;

            vector<TestCaseParamValue> globalPreValues;
            vector<TestCaseParamValue> globalPostValues;
            std::optional <TestCaseParamValue> stdinValue;
            vector<TypeAndVarName> lazyVariables;
            vector<InitReference> lazyReferences;
            vector<UTBotKTestObject> objects;

            MapAddressName fromAddressToName;

            vector<MethodParam> stubValuesTypes;
            vector<TestCaseParamValue> stubValues;

            vector<TestCaseParamValue> paramValues;
            vector<TestCaseParamValue> paramPostValues;
            vector<TestCaseParamValue> lazyValues;
            vector<TestCaseParamValue> stubParamValues;
            vector<MethodParam> stubParamTypes;
            shared_ptr<AbstractValueView> returnValueView;
            std::optional<TestCaseParamValue> classPreValues;
            std::optional<TestCaseParamValue> classPostValues;

            bool isError() const;
        };

        struct Modifiers {
            bool isStatic;
            bool isExtern;
            bool isInline;
        };

        struct MethodDescription {
            std::optional<MethodParam> classObj;
            std::string name;
            typedef std::unordered_map<string, string> SuiteNameToCodeTextMap;
            std::string stubsText;
            SuiteNameToCodeTextMap codeText;
            std::string paramsString;

            types::Type returnType;
            bool hasIncompleteReturnType = false;

            std::optional<string> sourceBody;
            Modifiers modifiers;
            bool isVariadic = false;
            std::vector<MethodParam> globalParams;
            std::vector<MethodParam> params;

            typedef std::unordered_map<string, std::shared_ptr<types::FunctionInfo>> FPointerMap;
            FPointerMap functionPointers;
            vector<MethodTestCase> testCases;
            typedef std::unordered_map<string, vector<MethodTestCase>> SuiteNameToTestCasesMap;
            SuiteNameToTestCasesMap suiteTestCases;

            bool operator==(const MethodDescription &other) const;

            MethodDescription();

            [[nodiscard]] vector<types::Type> getParamTypes() const {
                return CollectionUtils::transform(params, [](auto const& param) {
                    return param.type;
                });
            }

            [[nodiscard]] vector<string> getParamNames() const {
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

            bool hasChangeable() const {
                for(const auto& i : params) {
                    if (i.isChangeable()) {
                        return true;
                    }
                }
                return false;
            }

            bool isClassMethod() const {
                return classObj.has_value();
            }

            std::optional<std::string> getClassName() const {
                if (isClassMethod()) {
                    return std::make_optional(classObj->name);
                }
                return std::nullopt;
            }

            std::optional<std::string> getClassTypeName() const {
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

        static const string DEFAULT_SUITE_NAME;
        static const string ERROR_SUITE_NAME;
        static const MethodParam &getStdinMethodParam();

        fs::path sourceFilePath;
        string sourceFileNameNoExt; // without extension
        fs::path relativeFileDir;   // relative to project root dir
        string testFilename;
        fs::path testHeaderFilePath;
        fs::path testSourceFilePath;

        vector<Include> srcFileHeaders;
        vector<Include> headersBeforeMainHeader;
        std::optional<Include> mainHeader;
        MethodsMap methods; // method's name -> description
        string code;       // contains final code of test file
        string headerCode; // contains code of header
        std::vector<string> commentBlocks{};
        string stubs; // language-independent stubs definitions

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

        TestMethod(string methodName, fs::path bitcodeFile, fs::path sourceFilename);
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
                        const std::unordered_map<string, types::Type>& methodNameToReturnTypeMap,
                        bool filterByLineFlag,
                        shared_ptr<LineInfo> lineInfo);
    private:
        types::TypesHandler &typesHandler;

        struct RawKleeParam {
            string paramName;
            vector<char> rawData;

            RawKleeParam(string paramName, vector<char> rawData)
                : paramName(std::move(paramName)), rawData(std::move(rawData)) {
            }

            [[nodiscard]] [[maybe_unused]] bool hasPrefix(const string &prefix) const {
                return StringUtils::startsWith(paramName, prefix);
            }
        };

        struct JsonNumAndType {
            int num;
            types::Type type;
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
                            const std::unordered_map<string, types::Type>& methodNameToReturnTypeMap,
                            shared_ptr<LineInfo> lineInfo);
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
                                const std::unordered_map<string, types::Type>& methodNameToReturnTypeMap,
                                std::stringstream &traceStream);

        shared_ptr<AbstractValueView>
        testParameterView(const RawKleeParam &kleeParam,
                          const Tests::TypeAndVarName &param,
                          types::PointerUsage usage,
                          const MapAddressName &fromAddressToName,
                          std::vector<InitReference> &initReferences,
                          const std::optional<const Tests::MethodDescription> &testingMethod = std::nullopt);

        shared_ptr<ArrayValueView> multiArrayView(const vector<char> &byteArray,
                                                  const types::Type &type,
                                                  size_t arraySize,
                                                  size_t offset,
                                                  types::PointerUsage usage);

        shared_ptr<ArrayValueView> arrayView(const vector<char> &byteArray,
                                             const types::Type &type,
                                             size_t arraySize,
                                             unsigned int offset,
                                             types::PointerUsage usage);

        static shared_ptr<StringValueView> stringLiteralView(const vector<char> &byteArray,
                                                             size_t length = 0);

        shared_ptr<FunctionPointerView> functionPointerView(const std::optional<string>& scopeName,
                                                            const string &methodName,
                                                            const string &paramName);

        shared_ptr<FunctionPointerView> functionPointerView(const string &structName,
                                                            const string &fieldName);

        shared_ptr<UnionValueView> unionView(const vector<char> &byteArray,
                                             types::UnionInfo &unionInfo,
                                             unsigned int offset,
                                             types::PointerUsage usage);

        shared_ptr<StructValueView> structView(const vector<char> &byteArray,
                                               types::StructInfo &curStruct,
                                               unsigned int offset,
                                               types::PointerUsage usage);

        shared_ptr<StructValueView> structView(const vector<char> &byteArray,
                                               types::StructInfo &curStruct,
                                               unsigned int offset,
                                               types::PointerUsage usage,
                                               const std::optional<const Tests::MethodDescription> &testingMethod,
                                               const std::string &name,
                                               const MapAddressName &fromAddressToName,
                                               std::vector<InitReference> &initReferences);

        shared_ptr<PrimitiveValueView> primitiveView(const vector<char> &byteArray,
                                                     const types::Type &type,
                                                     size_t offset,
                                                     size_t len);
        static shared_ptr<EnumValueView> enumView(const vector<char> &byteArray,
                                                  types::EnumInfo &enumInfo,
                                                  size_t offset,
                                                  size_t len);

        string primitiveCharView(const types::Type &type, string value);
        static string primitiveBoolView(const string &value);

        constexpr static const char *const KLEE_PATH_FLAG = "kleePathFlag";

        const std::string PointerWidthType = "unsigned long long";
        const size_t PointerWidthSize = 8;

        constexpr static const char *const KLEE_PATH_FLAG_SYMBOLIC = "kleePathFlagSymbolic";
        static vector<RawKleeParam>::const_iterator
        getKleeParam(const vector<RawKleeParam> &rawKleeParams, std::string name);
        static RawKleeParam getKleeParamOrThrow(const vector<RawKleeParam> &rawKleeParams, const std::string &name);

        Tests::TestCaseDescription
        parseTestCaseParams(const UTBotKTest &ktest,
                            const Tests::MethodDescription &methodDescription,
                            const std::unordered_map<string, types::Type>& methodNameToReturnTypeMap,
                            const std::stringstream &traceStream);
        vector<shared_ptr<AbstractValueView>> collectUnionSubViews(const vector<char> &byteArray,
                                                                   const types::UnionInfo &info,
                                                                   unsigned int offset,
                                                                   types::PointerUsage usage);
        void processGlobalParamPreValue(Tests::TestCaseDescription &testCaseDescription,
                                        const Tests::MethodParam &globalParam,
                                        vector<RawKleeParam> &rawKleeParams);

        void processSymbolicStdin(Tests::TestCaseDescription &testCaseDescription,
                                  vector<RawKleeParam> &rawKleeParams);

        void processGlobalParamPostValue(Tests::TestCaseDescription &testCaseDescription,
                                     const Tests::MethodParam &globalParam,
                                     vector<RawKleeParam> &rawKleeParams);

        void processClassPostValue(Tests::TestCaseDescription &testCaseDescription,
                                                      const Tests::MethodParam &param,
                                                      vector<RawKleeParam> &rawKleeParams);

        void processParamPostValue(Tests::TestCaseDescription &testCaseDescription,
                                       const Tests::MethodParam &param,
                                       vector<RawKleeParam> &rawKleeParams);

        void processStubParamValue(Tests::TestCaseDescription &testCaseDescription,
                                   const std::unordered_map<string, types::Type>& methodNameToReturnTypeMap,
                                   vector<RawKleeParam> &rawKleeParams);

        void assignTypeUnnamedVar(Tests::MethodTestCase &testCase,
                                  const Tests::MethodDescription &methodDescription);

        void assignTypeStubVar(Tests::MethodTestCase &testCase,
                               const Tests::MethodDescription &methodDescription);

        void workWithStructInBFS(std::queue<JsonNumAndType> &order, std::vector<bool> &visited,
                                 const Offset &off, std::vector<UTBotKTestObject> &objects, const types::StructInfo &structInfo);

        int findFieldIndex(const types::StructInfo &structInfo, size_t offset);

        int findObjectIndex(const std::vector<UTBotKTestObject> &objects, const std::string &name);

        types::Type traverseStruct(const types::StructInfo &structInfo, size_t offset);
    };
    /**
     * @brief This function is used for converting primiive value of a specific type
     * To a string value which we can print to .cpp file.
     */
    template <typename T>
    std::enable_if_t<!std::is_floating_point<T>::value, string>
    primitiveValueToString(T value) {
        return std::to_string(value);
    }

    template <typename T>
    std::enable_if_t<std::is_floating_point<T>::value, string>
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
    string readBytesAsValue(const vector<char> &byteArray, size_t offset, size_t len) {
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
    string readBytesAsValueForType(const vector<char> &byteArray,
                                   const string &typeName,
                                   unsigned int offset,
                                   unsigned int len);
}
#endif // UNITTESTBOT_TESTS_H