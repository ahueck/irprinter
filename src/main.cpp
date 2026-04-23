#include <printer/IRNodeFinder.h>

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Module.h>
#include <llvm/LineEditor/LineEditor.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory IRPrinter("IR Printer Sample");
// static cl::opt<bool> colors("use-color", cl::init(true), cl::desc("Enable or disable color output"),
// cl::cat(IRPrinter));

namespace {
SmallVector<StringRef, 8> tokenize_words(StringRef line) {
  SmallVector<StringRef, 8> tokens;
  while (!line.empty()) {
    line = line.ltrim();
    if (line.empty()) {
      break;
    }

    const auto word_end = line.find_first_of(" \t\n\r\f\v");
    if (word_end == StringRef::npos) {
      tokens.push_back(line);
      break;
    }

    tokens.push_back(line.take_front(word_end));
    line = line.drop_front(word_end);
  }
  return tokens;
}
}  // namespace

int main(int argc, const char** argv) {
#if LLVM_VERSION_MAJOR < 14
  CommonOptionsParser op(argc, argv, IRPrinter);
  const auto& sources      = op.getSourcePathList();
  const auto& compilations = op.getCompilations();
#else
  auto op_res = CommonOptionsParser::create(argc, argv, IRPrinter);
  if (!op_res) {
    llvm::outs() << "Erroneous input";
    return 1;
  }
  auto& op                 = op_res.get();
  const auto& sources      = op.getSourcePathList();
  const auto& compilations = op.getCompilations();
#endif

  irprinter::IRNodeFinder ir(compilations, sources);

  auto ret = ir.parse();
  if (ret != 0) {
    llvm::errs() << "Error parsing. Quitting...\n";
    return ret;
  }

  llvm::LineEditor le("ir-printer");
  irprinter::IRPrintingFlags printing_flags;
  llvm::outs().enable_colors(true);
  while (auto line = le.readLine()) {
    const auto tokens = tokenize_words(*line);
    if (tokens.empty()) {
      continue;
    }

    const auto cmd  = tokens.front();
    const auto args = ArrayRef<StringRef>(tokens).drop_front();

    if (cmd == "q" || cmd == "quit") {
      break;
    } else if (unsigned start; !cmd.getAsInteger(10, start)) {
      unsigned end{start};
      if (!args.empty()) {
        if (args.front().getAsInteger(10, end)) {
          const auto end_ref = args.front();
          llvm::outs() << "Invalid end location: " << end_ref << "\n";
          continue;
        }
      }
      if (end < start) {
        llvm::outs() << "Error: end location (" << end << ") is less than start location (" << start << ")\n";
      } else {
        ir.printByLocation(start, end, printing_flags);
      }
    } else if (cmd == "deps") {
      printing_flags.setIncludeDependencies(!printing_flags.include_dependencies);
      llvm::outs() << "Dependency expansion " << (printing_flags.include_dependencies ? "enabled" : "disabled") << "\n";
    } else if (cmd == "lines") {
      printing_flags.setPrintLine(!printing_flags.print_line);
      llvm::outs() << "Line prefixes " << (printing_flags.print_line ? "enabled" : "disabled") << "\n";
    } else if (cmd == "g" || cmd == "generate") {
      ir.parse();
    } else if (cmd == "f" || cmd == "flag") {
      std::string joined_flags;
      for (const auto flag : args) {
        if (!joined_flags.empty()) {
          joined_flags += " ";
        }
        joined_flags += flag.str();
        ir.setOptFlag(flag);
      }

      if (args.empty()) {
        llvm::outs() << "No flag provided. Usage: f <flag...>\n";
        continue;
      }

      llvm::outs() << "Set flag" << (args.size() > 1 ? "s" : "") << " to " << joined_flags
                   << ". Re-generating module...\n";
      ir.parse();
    } else if (cmd == "dump") {
      ir.dump();
    } else if (cmd == "l" || cmd == "list" || cmd == "p" || cmd == "print") {
      StringRef str = args.empty() ? StringRef(".*") : args.front();
      if (!args.empty()) {
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
      auto str            = args.empty() ? StringRef{} : args.front();
      auto demangled_name = irprinter::IRNodeFinder::demangle(std::string{str});
      llvm::outs() << "Demangled name: " << demangled_name << "\n";
    }
    llvm::outs().flush();
  }

  return 0;
}
