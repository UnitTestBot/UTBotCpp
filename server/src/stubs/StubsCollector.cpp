#include "StubsCollector.h"

#include "visitors/FunctionPointerForStubsVisitor.h"

StubsCollector::StubsCollector(const types::TypesHandler &typesHandler)
    : typesHandler(&typesHandler) {
}

void StubsCollector::collect(tests::TestsMap &testsMap) {
    auto visitor = visitor::FunctionPointerForStubsVisitor(typesHandler);
    for (auto it = testsMap.begin(); it != testsMap.end(); it++) {
        tests::Tests &tests = it.value();
        tests.stubs = visitor.visit(tests);
    }
}
