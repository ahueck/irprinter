/*
 * IRNodeFinder.cpp
 *
 *  Created on: Sep 13, 2018
 *      Author: ahueck
 */

#include "printer/IRNodeFinder.h"
#include "Util.h"

#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

using namespace llvm;

namespace irprinter {

namespace {
template <typename F>
void applyToMatchingFunction(llvm::raw_ostream& os, const llvm::Module* m, const std::string& regex, F&& func) {
  const auto fvec = util::regex_find(*m, regex);
  unsigned count{0};
  for (auto f : fvec) {
    auto fname = f->getName();
    os << "Match " << ++count << " [" << util::try_demangle(fname) << "]:";
    func(f);
    os << "\n";
  }
}
}  // namespace

// using namespace util;

IRNodeFinder::IRNodeFinder(clang::tooling::CommonOptionsParser& op, llvm::raw_ostream& os) : tool(op), os(os) {
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

void IRNodeFinder::listFunction(const std::string& regex) const {
  const auto* m = tool.getModule();
  applyToMatchingFunction(os, m, regex, [&](const Function* f) {
    std::string s;
    llvm::raw_string_ostream oss(s);
    f->print(oss);
    oss.flush();

    if (f->isDeclaration()) {
      os << "\n" << s.substr(1);
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
  return util::try_demangle(name);
}

} /* namespace irprinter */
