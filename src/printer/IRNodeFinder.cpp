/*
 * IRNodeFinder.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: ahueck
 */

#include "printer/IRNodeFinder.h"
#include "Util.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace irprinter {

namespace {
std::string strip_parens(std::string name) {
  if (name.size() > 2 && name.substr(name.size() - 2) == "()") {
    name.erase(name.size() - 2);
  }
  return name;
}

template <typename F>
void applyToMatchingFunction(llvm::raw_ostream& os, const llvm::Module* m, const std::string& regex, F&& func) {
  const auto fvec = util::regex_find(*m, regex);
  unsigned count{0};
  for (auto f : fvec) {
    auto fname = f->getName();
    os << "Match " << ++count << " [" << strip_parens(util::try_demangle(fname)) << "]:";
    func(f);
    os << "\n";
  }
}
}  // namespace

// using namespace util;

IRNodeFinder::IRNodeFinder(clang::tooling::CommonOptionsParser& op, llvm::raw_ostream& os)
    : IRNodeFinder(op.getCompilations(), op.getSourcePathList(), os) {
}

IRNodeFinder::IRNodeFinder(const clang::tooling::CompilationDatabase& compilation_database,
                           llvm::ArrayRef<std::string> source_path, llvm::raw_ostream& os)
    : tool(compilation_database, source_path), os(os) {
}

int IRNodeFinder::parse() {
  return tool.execute();
}

void IRNodeFinder::dump() const {
  tool.getModule()->print(os, nullptr);
}

void IRNodeFinder::setOptFlag(StringRef flag) {
  tool.setFlag(flag);
}

void IRNodeFinder::printFunction(const std::string& regex) const {
  const auto* m = tool.getModule();
  applyToMatchingFunction(os, m, regex, [&](const Function* f) { f->print(os); });
}

void IRNodeFinder::printByLocation(unsigned line_start, unsigned line_end) const {
  const unsigned search_end = std::max(line_start, line_end);
  const auto* module        = tool.getModule();
  const auto main_file_path = [&](const llvm::Module* m) -> std::optional<std::string> {
    auto* CUs = m->getNamedMetadata("llvm.dbg.cu");
    if (!CUs || CUs->getNumOperands() == 0) {
      return std::nullopt;
    }
    auto* cu = llvm::cast<llvm::DICompileUnit>(CUs->getOperand(0));
    llvm::SmallString<128> path;
    if (!llvm::sys::fs::real_path(cu->getFilename(), path)) {
      return path.str().str();
    }
    return std::nullopt;
  }(module);

  const auto is_relevant_function = [&](const llvm::Function& func) -> bool {
    const auto* sub = func.getSubprogram();
    if (!sub) {
      return false;
    }
    // TODO: investigate w.r.t. inlining?
    // if (sub->getLine() > search_end) {
    //   return false;
    // }
    if (main_file_path) {
      llvm::SmallString<128> func_path;
      if (!llvm::sys::fs::real_path(sub->getFilename(), func_path)) {
        return func_path == *main_file_path;
      }
    }
    return true;
  };

  std::string matches_buffer;
  llvm::raw_string_ostream buffer_stream{matches_buffer};

  for (const auto& func : llvm::make_filter_range(*module, is_relevant_function)) {
    bool function_header_printed = false;
    for (const auto& block : func) {
      for (const auto& inst : block) {
        const auto& loc = inst.getDebugLoc();
        if (!loc) {
          continue;
        }
        const unsigned line = loc.getLine();

        if (line >= line_start && line <= search_end) {
          if (!function_header_printed) {
            buffer_stream << func.getName() << ":\n";
            function_header_printed = true;
          }
          inst.print(buffer_stream);
          buffer_stream << "\n";
        }
      }
    }
  }

  if (!buffer_stream.str().empty()) {
    os << buffer_stream.str() << "\n";
  }
}

void IRNodeFinder::listFunction(const std::string& regex) const {
  const auto* m = tool.getModule();
  applyToMatchingFunction(os, m, regex, [&](const Function* f) {
    std::string s;
    llvm::raw_string_ostream oss(s);
    f->print(oss);
    oss.flush();

    if (f->isDeclaration()) {
      os << "\n" << s;
    } else {
      llvm::Regex r("((;|define)[^{]+){");
      SmallVector<StringRef, 2> match;
      r.match(s, &match);
      if (match.size() > 1) {
        os << "\n" << *std::next(match.begin()) << "\n";
      }
    }
  });
}

std::string IRNodeFinder::demangle(const std::string& name) {
  return strip_parens(util::try_demangle(name));
}

} /* namespace irprinter */
