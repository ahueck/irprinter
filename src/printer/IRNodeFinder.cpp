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

void IRNodeFinder::printByLocation(unsigned line_start_, unsigned line_end_) const {
  line_end_ = std::max(line_start_, line_end_);
  std::string matches;
  llvm::raw_string_ostream local_oss{matches};
  bool first_match{false};
  const auto* m = tool.getModule();
  for (const auto& f : *m) {
    first_match = false;
    for (const auto& bb : f) {
      for (const auto& inst : bb) {
        const auto& loc = inst.getDebugLoc();
        if (loc) {
          const auto line = loc.getLine();
          if (line >= line_start_ && line <= line_end_) {
            if (!first_match) {
              local_oss << f.getName() << ":\n";
              first_match = true;
            }
            inst.print(local_oss);
            local_oss << "\n";
          }
        }
      }
    }
  }
  if (!local_oss.str().empty()) {
    os << local_oss.str() << "\n";
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
