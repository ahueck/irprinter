/*
 * LLVMTool.h
 *
 *  Created on: Sep 28, 2018
 *      Author: ahueck
 */

#ifndef SRC_PRINTER_LLVMTOOL_H_
#define SRC_PRINTER_LLVMTOOL_H_

#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/Tooling.h>

#include <llvm/ADT/SmallVector.h>
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
  clang::tooling::CommandLineArguments user_args;

 public:
  LLVMTool(clang::tooling::CommonOptionsParser&);

  void execute();

  void setFlag(StringRef flag);
  void removeFlag(StringRef flag);
  void clearUserFlags();

  std::unique_ptr<llvm::Module> takeModule();

  const llvm::Module* getModule() const;

  virtual ~LLVMTool();

 private:
  void commitUserArgs();
};

}  // namespace irprinter

#endif /* SRC_PRINTER_LLVMTOOL_H_ */
