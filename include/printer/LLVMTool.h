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
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/LLVMContext.h>

#include <memory>

namespace llvm {
class Module;
}  // namespace llvm

namespace clang::tooling {
class CommonOptionsParser;
}  // namespace clang::tooling

namespace irprinter {

class LLVMTool {
  clang::tooling::ClangTool tool;
  llvm::LLVMContext ctx;
  std::unique_ptr<llvm::Module> m;
  clang::tooling::CommandLineArguments user_args;

 public:
  explicit LLVMTool(clang::tooling::CommonOptionsParser&);

  int execute();

  void setFlag(llvm::StringRef flag);
  void removeFlag(llvm::StringRef flag);
  void clearUserFlags();

  std::unique_ptr<llvm::Module> takeModule();

  [[nodiscard]] const llvm::Module* getModule() const;

  virtual ~LLVMTool();

 private:
  void commitUserArgs();
};

}  // namespace irprinter

#endif /* SRC_PRINTER_LLVMTOOL_H_ */
