/*
 * LLVMTool.h
 *
 *  Created on: Sep 28, 2018
 *      Author: ahueck
 */

#ifndef SRC_PRINTER_LLVMTOOL_H_
#define SRC_PRINTER_LLVMTOOL_H_

#include <clang/Tooling/Tooling.h>

#include <llvm/IR/LLVMContext.h>

#include <memory>

namespace llvm {
class Module;
}  // namespace llvm

namespace clang {
namespace tooling {
class CommonOptionsParser;
}  // namespace tooling
}  // namespace clang

namespace irprinter {

class LLVMTool {
  clang::tooling::ClangTool tool;
  llvm::LLVMContext ctx;
  std::unique_ptr<llvm::Module> m;

 public:
  LLVMTool(clang::tooling::CommonOptionsParser&);

  void execute();

  void setOptFlag(const std::string& opt_flag);

  std::unique_ptr<llvm::Module> takeModule();

  llvm::Module* getModule();

  virtual ~LLVMTool();
};

}  // namespace irprinter

#endif /* SRC_PRINTER_LLVMTOOL_H_ */
