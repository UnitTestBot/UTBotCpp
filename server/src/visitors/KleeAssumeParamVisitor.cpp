/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "KleeAssumeParamVisitor.h"
#include "utils/KleeUtils.h"
#include "utils/PrinterUtils.h"

namespace visitor {
  KleeAssumeParamVisitor::KleeAssumeParamVisitor(
          const types::TypesHandler *typesHandler,
          printer::KleePrinter *printer)
          : KleeAssumeVisitor(typesHandler, printer) {
      usage = types::PointerUsage::PARAMETER;
  }

  static thread_local std::string outVariable;

  void KleeAssumeParamVisitor::visit(const Tests::MethodParam &param,
                                     const std::string &_outVariable) {
      outVariable = _outVariable;
      std::string name = param.dataVariableName();
      auto paramType = param.type.arrayCloneMultiDim(usage);
      visitAny(paramType, name, nullptr, PrinterUtils::DEFAULT_ACCESS, 0);
      outVariable = {};
  }

  void KleeAssumeParamVisitor::visitGlobal(const Tests::MethodParam &globalParam,
                                           const std::string &_outVariable) {
      outVariable = _outVariable;
      visitAny(globalParam.type, globalParam.name, nullptr, PrinterUtils::DEFAULT_ACCESS, 0);
      outVariable = {};
  }

  void KleeAssumeParamVisitor::visitPrimitive(const types::Type &type,
                                                    const std::string &name,
                                                    const tests::AbstractValueView *view,
                                                    const std::string &access,
                                                    int depth,
                                                    tests::Tests::ConstructorInfo constructorInfo) {
      kleeAssume(PrinterUtils::getEqualString(name, PrinterUtils::fillVarName(access, outVariable)));
  }

  void KleeAssumeParamVisitor::visitPointer(const types::Type &type,
                                            const std::string &name,
                                            const tests::AbstractValueView *view,
                                            const std::string &access,
                                            int depth) {
      if (depth == 0) {
          auto sizes = type.arraysSizes(usage);
          std::string newName = PrinterUtils::getDereferencePointer(name, sizes.size());
          KleeAssumeVisitor::visitAny(type.baseTypeObj(), newName, view, access, depth);
      }
      //TODO add assert visit struct.pointer
  }

  void KleeAssumeParamVisitor::visitArray(const types::Type &type,
                                                const std::string &name,
                                                const tests::AbstractValueView *view,
                                                const std::string &access,
                                                size_t size,
                                                int depth,
                                                tests::Tests::ConstructorInfo constructorInfo) {
      if (depth == 0) {
          if (type.isObjectPointer()) {
              return visitPointer(type, name, view, access, depth);
          }
      }
      std::vector<size_t> sizes = type.arraysSizes(usage);
      bool assignPointersToNull = type.isTypeContainsPointer() && depth > 0;
      if (assignPointersToNull) {
          int pointerIndex = type.indexOfFirstPointerInTypeKinds();
          sizes = std::vector<size_t>(sizes.begin(), sizes.begin() + pointerIndex);
      }
      const auto iterators = printer->printForLoopsAndReturnLoopIterators(sizes);
      const auto indexing = printer::Printer::constrMultiIndex(iterators);
      if (assignPointersToNull) {
          kleeAssume(PrinterUtils::getEqualString(name + indexing,  PrinterUtils::C_NULL));
      } else {
          visitAny(type.baseTypeObj(), name + indexing, view, access + indexing, depth + sizes.size(), constructorInfo);
      }
      printer->closeBrackets(sizes.size());
  }
}