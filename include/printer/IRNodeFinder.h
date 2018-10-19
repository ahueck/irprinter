/*
 * IRNodeFinder.h
 *
 *  Created on: Sep 13, 2018
 *      Author: ahueck
 */

#ifndef INCLUDE_PRINTER_IRNODEFINDER_H_
#define INCLUDE_PRINTER_IRNODEFINDER_H_

#include <printer/LLVMTool.h>

#include <llvm/Support/raw_ostream.h>

#include <string>

namespace clang {
namespace tooling {
class CommonOptionsParser;
}  // namespace tooling
}  // namespace clang

namespace irprinter {

class IRNodeFinder {
 private:
  LLVMTool tool;
  llvm::raw_ostream& os;

 public:
  explicit IRNodeFinder(clang::tooling::CommonOptionsParser& op, llvm::raw_ostream& os = llvm::outs());

  void parse();

  void dump() const;

  void setOptFlag(StringRef flag);

  void printFunction(const std::string regex = ".*") const;

  void listFunction(const std::string regex = ".*") const;
};

} /* namespace irprinter */

#endif /* INCLUDE_PRINTER_IRNODEFINDER_H_ */
