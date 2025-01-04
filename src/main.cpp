#include <printer/IRNodeFinder.h>

#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/Regex.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Module.h>
#include <llvm/LineEditor/LineEditor.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory IRPrinter("IR Printer Sample");
// static cl::opt<bool> colors("use-color", cl::init(true), cl::desc("Enable or disable color output"),
// cl::cat(IRPrinter));

namespace {
StringRef lexWord(StringRef word) {
  StringRef::iterator begin{word.begin()};
  StringRef::iterator end{word.end()};
  while (true) {
    if (begin == end) {
      return StringRef(begin, 0);
    }
    if (!isWhitespace(*begin)) {
      break;
    }
    ++begin;
  }
  StringRef::iterator wordbegin = begin;
  while (true) {
    ++begin;

    if (begin == end || isWhitespace(*begin)) {
      return StringRef(wordbegin, begin - wordbegin);
    }
  }
}
}  // namespace

int main(int argc, const char** argv) {
#if LLVM_VERSION_MAJOR < 14
  CommonOptionsParser op(argc, argv, IRPrinter);
  irprinter::IRNodeFinder ir(op);
#else
  auto op = CommonOptionsParser::create(argc, argv, IRPrinter);
  if (!op) {
    llvm::outs() << "Erroneous input";
    return 1;
  }
  irprinter::IRNodeFinder ir(op.get());
#endif


  auto ret = ir.parse();
  if (ret != 0) {
    llvm::outs() << "Error parsing. Quitting...\n";
    return ret;
  }

  llvm::LineEditor le("ir-printer");
  while (auto line = le.readLine()) {
    StringRef ref = *line;
    auto cmd      = lexWord(ref);

    if (cmd == "q" || cmd == "quit") {
      break;
    } else if (cmd == "g" || cmd == "generate") {
      ir.parse();
    } else if (cmd == "f" || cmd == "flag") {
      auto str = lexWord(StringRef(cmd.end(), ref.end() - cmd.end()));
      llvm::outs() << "Set flag to " << str << ". Re-generating module...\n";
      ir.setOptFlag(str);
      ir.parse();
    } else if (cmd == "dump") {
      ir.dump();
    } else if (cmd == "l" || cmd == "list" || cmd == "p" || cmd == "print") {
      auto str = lexWord(StringRef(cmd.end(), ref.end() - cmd.end()));
      if (str == "") {
        str = ".*";
      } else {
        std::string error;
        llvm::Regex r(str);
        if (!r.isValid(error)) {
          llvm::outs() << "Invalid regex (" << error << "): \"" << str << "\"\n";
          continue;
        }
      }
      if (cmd == "p" || cmd == "print") {
        ir.printFunction(std::string{str});
      } else {
        ir.listFunction(std::string{str});
      }
    } else if (cmd == "d" || cmd == "demangle") {
      auto str            = lexWord(StringRef(cmd.end(), ref.end() - cmd.end()));
      auto demangled_name = irprinter::IRNodeFinder::demangle(std::string{str});
      llvm::outs() << "Demangled name: " << demangled_name << "\n";
    }

    llvm::outs().flush();
  }

  return 0;
}
