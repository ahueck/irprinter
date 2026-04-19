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

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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

struct LineInstruction {
  const llvm::Instruction* inst;
  unsigned line;
  bool is_dependency{false};

  LineInstruction(const llvm::Instruction* inst, unsigned line, bool is_dependency = false)
      : inst(inst), line(line), is_dependency(is_dependency) {
  }
};

struct FunctionInstructions {
  const llvm::Function* func;
  std::vector<LineInstruction> instructions;

  FunctionInstructions(const llvm::Function* func, std::vector<LineInstruction> instructions)
      : func(func), instructions(std::move(instructions)) {
  }
};

using LineMap = std::vector<FunctionInstructions>;

LineMap collect_line_map(const llvm::Module* module, unsigned line_start, unsigned search_end,
                         const std::optional<std::string>& main_file_path, bool include_deps) {
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

  std::unordered_set<const llvm::Instruction*> seed_instructions;
  std::vector<const llvm::Instruction*> seed_worklist;

  // Phase 1: collect direct line matches (seeds).
  for (const auto& func : llvm::make_filter_range(*module, is_relevant_function)) {
    for (const auto& block : func) {
      for (const auto& inst : block) {
        const auto& loc = inst.getDebugLoc();
        if (!loc) {
          continue;
        }
        const unsigned line = loc.getLine();
        if (line >= line_start && line <= search_end) {
          if (seed_instructions.emplace(&inst).second) {
            seed_worklist.push_back(&inst);
          }
        }
      }
    }
  }

  // Phase 2: collect recursive backward dependencies from seeds.
  std::unordered_set<const llvm::Instruction*> dep_instructions;
  if (include_deps) {
    std::unordered_set<const llvm::Instruction*> visited_instructions = seed_instructions;
    auto worklist                                                     = seed_worklist;
    while (!worklist.empty()) {
      const llvm::Instruction* current = worklist.back();
      worklist.pop_back();

      for (const llvm::Use& operand_use : current->operands()) {
        const auto* operand_inst = llvm::dyn_cast<llvm::Instruction>(operand_use.get());
        if (!operand_inst) {
          continue;
        }
        if (!visited_instructions.emplace(operand_inst).second) {
          continue;
        }

        dep_instructions.emplace(operand_inst);
        worklist.push_back(operand_inst);
      }
    }
  }

  // Phase 3: build ordered line map from seeds + deps.
  LineMap matches;
  std::unordered_map<const llvm::Function*, std::size_t> function_to_index;
  const auto ensure_function_bucket = [&](const llvm::Function* func) -> FunctionInstructions& {
    const auto [it, inserted] = function_to_index.emplace(func, matches.size());
    if (inserted) {
      matches.emplace_back(func, std::vector<LineInstruction>{});
    }
    return matches[it->second];
  };

  for (const auto& func : llvm::make_filter_range(*module, is_relevant_function)) {
    for (const auto& block : func) {
      for (const auto& inst : block) {
        const bool is_seed = seed_instructions.find(&inst) != seed_instructions.end();
        const bool is_dep  = dep_instructions.find(&inst) != dep_instructions.end();
        if (!is_seed && !is_dep) {
          continue;
        }

        const auto& loc = inst.getDebugLoc();
        const unsigned line = loc ? loc.getLine() : 0;
        ensure_function_bucket(&func).instructions.emplace_back(&inst, line, !is_seed && is_dep);
      }
    }
  }

  return matches;
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

void IRNodeFinder::printByLocation(unsigned line_start, unsigned line_end, bool include_deps) const {
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

  std::string matches_buffer;
  llvm::raw_string_ostream buffer_stream{matches_buffer};

  const auto print_line_map = [&](const LineMap& line_map) {
    for (const auto& [func, instructions] : line_map) {
      buffer_stream << func->getName() << ":\n";
      for (const auto& line_instruction : instructions) {
        line_instruction.inst->print(buffer_stream);
        buffer_stream << "\n";
      }
    }
  };

  const LineMap line_map = collect_line_map(module, line_start, search_end, main_file_path, include_deps);
  print_line_map(line_map);

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
