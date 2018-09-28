#include <printer/LLVMTool.h>

#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Module.h>
#include <llvm/LineEditor/LineEditor.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory IRPrinter("IR Printer Sample");
static cl::opt<bool> colors("color", cl::init(true), cl::desc("Enable or disable color output"), cl::cat(IRPrinter));

int main(int argc, const char** argv) {
  CommonOptionsParser op(argc, argv, IRPrinter);
  irprinter::LLVMTool tool(op);
  //  tool.setOptFlag("-O2");
  tool.execute();
  auto mod = tool.getModule();
  if (mod) {
    mod->dump();
  }
  return 0;
}
