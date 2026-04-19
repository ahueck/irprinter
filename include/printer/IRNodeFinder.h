/*
 * IRNodeFinder.h
 *
 *  Created on: Sep 13, 2018
 *      Author: ahueck
 */

#ifndef INCLUDE_PRINTER_IRNODEFINDER_H_
#define INCLUDE_PRINTER_IRNODEFINDER_H_

#include <printer/LLVMTool.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

#include <string>

namespace clang::tooling {
class CommonOptionsParser;
}  // namespace clang::tooling

namespace irprinter {

class IRNodeFinder {
 private:
  LLVMTool tool;
  llvm::raw_ostream& os;

 public:
  explicit IRNodeFinder(clang::tooling::CommonOptionsParser& op, llvm::raw_ostream& os = llvm::outs());
  IRNodeFinder(const clang::tooling::CompilationDatabase& compilation_database, llvm::ArrayRef<std::string> SourcePaths,
               llvm::raw_ostream& os = llvm::outs());

  int parse();

  void dump() const;

  void setOptFlag(llvm::StringRef flag);

  void printFunction(const std::string& regex = ".*") const;

  void printByLocation(unsigned line_start_, unsigned line_end_ = 0, bool include_deps = true) const;

  void listFunction(const std::string& regex = ".*") const;

  static std::string demangle(const std::string& name);
};

} /* namespace irprinter */

#endif /* INCLUDE_PRINTER_IRNODEFINDER_H_ */
