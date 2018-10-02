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
    os << "Match " << ++count << " [" << util::try_demangle(fname) << "]:\n";
    func(f);
    os << "\n";
  }
}
}  // namespace

// using namespace util;

IRNodeFinder::IRNodeFinder(clang::tooling::CommonOptionsParser& op, llvm::raw_ostream& os) : tool(op), os(os) {
}

void IRNodeFinder::parse() {
  tool.execute();
}

void IRNodeFinder::dump() const {
  tool.getModule()->print(os, nullptr);
}

void IRNodeFinder::printFunction(const std::string regex) const {
  const auto* m = tool.getModule();
  applyToMatchingFunction(os, m, regex, [&](const Function* f) { f->print(os); });
}

void IRNodeFinder::listFunction(const std::string regex) const {
  //	listFunctionDecls(visitor.ctx, regex, visitor.os);
  const auto* m = tool.getModule();
  applyToMatchingFunction(os, m, regex, [&](const Function* f) {
    std::string s;
    llvm::raw_string_ostream oss(s);
    f->print(oss);
    oss.flush();

    llvm::Regex r("((;|define)[^{]+){");
    SmallVector<StringRef, 2> match;
    r.match(s, &match);
    if (match.size() > 1) {
      os << *std::next(match.begin()) << "\n";
    }
  });
}

} /* namespace irprinter */
