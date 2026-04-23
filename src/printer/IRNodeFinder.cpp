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
  const llvm::BasicBlock* block;
  unsigned line;
  bool is_dependency{false};

  LineInstruction(const llvm::Instruction* inst, const llvm::BasicBlock* block, unsigned line,
                  bool is_dependency = false)
      : inst(inst), block(block), line(line), is_dependency(is_dependency) {
  }
};

struct FunctionInstructions {
  const llvm::Function* func;
  std::vector<LineInstruction> instructions;

  FunctionInstructions(const llvm::Function* func, std::vector<LineInstruction> instructions)
      : func(func), instructions(std::move(instructions)) {
  }
};

using LineMap         = std::vector<FunctionInstructions>;
using InstructionSet  = std::unordered_set<const llvm::Instruction*>;
using InstructionList = std::vector<const llvm::Instruction*>;

std::optional<std::string> get_main_file_path(const llvm::Module* module) {
  auto* CUs = module->getNamedMetadata("llvm.dbg.cu");
  if (!CUs || CUs->getNumOperands() == 0) {
    return std::nullopt;
  }

  auto* cu = llvm::cast<llvm::DICompileUnit>(CUs->getOperand(0));
  llvm::SmallString<128> path;
  if (!llvm::sys::fs::real_path(cu->getFilename(), path)) {
    return path.str().str();
  }
  return std::nullopt;
}

class LineMapBuilder {
  const llvm::Module* module;
  std::optional<std::string> main_file_path;
  unsigned line_start;
  unsigned search_end;
  bool include_deps;

  InstructionSet seed_instructions;
  InstructionList seed_worklist;
  InstructionSet dependency_instructions;
  LineMap ordered_line_map;
  std::unordered_map<const llvm::Function*, std::size_t> function_to_index;

  bool is_relevant_function(const llvm::Function& func) const {
    const auto* sub = func.getSubprogram();
    if (!sub) {
      return false;
    }

    // TODO: investigate function location filtering w.r.t. inlining.
    if (main_file_path) {
      llvm::SmallString<128> func_path;
      if (!llvm::sys::fs::real_path(sub->getFilename(), func_path)) {
        return func_path == *main_file_path;
      }
    }
    return true;
  }

  template <typename F>
  void for_each_relevant_instruction(F&& visitor) const {
    const auto relevant_functions =
        llvm::make_filter_range(*module, [&](const llvm::Function& func) { return is_relevant_function(func); });

    for (const auto& func : relevant_functions) {
      for (const auto& block : func) {
        for (const auto& inst : block) {
          visitor(func, block, inst);
        }
      }
    }
  }

  static unsigned instruction_line_or_zero(const llvm::Instruction& inst) {
    const auto& loc = inst.getDebugLoc();
    return loc ? loc.getLine() : 0;
  }

  void collect_seed_instructions() {
    for_each_relevant_instruction([&](const llvm::Function&, const llvm::BasicBlock&, const llvm::Instruction& inst) {
      auto line = instruction_line_or_zero(inst);
      if (line == 0) {
        return;
      }
      if (line >= line_start && line <= search_end) {
        if (seed_instructions.emplace(&inst).second) {
          seed_worklist.push_back(&inst);
        }
      }
    });
  }

  void collect_backward_dependencies() {
    if (!include_deps) {
      return;
    }

    InstructionSet visited = seed_instructions;
    auto worklist          = seed_worklist;
    while (!worklist.empty()) {
      const llvm::Instruction* current = worklist.back();
      worklist.pop_back();

      for (const llvm::Use& operand_use : current->operands()) {
        const auto* operand_inst = llvm::dyn_cast<llvm::Instruction>(operand_use.get());
        if (!operand_inst) {
          continue;
        }
        if (!visited.emplace(operand_inst).second) {
          continue;
        }

        dependency_instructions.emplace(operand_inst);
        worklist.push_back(operand_inst);
      }
    }
  }

  FunctionInstructions& ensure_function_bucket(const llvm::Function* func) {
    const auto [it, inserted] = function_to_index.emplace(func, ordered_line_map.size());
    if (inserted) {
      ordered_line_map.emplace_back(func, std::vector<LineInstruction>{});
    }
    return ordered_line_map[it->second];
  }

  void build_ordered_line_map() {
    for_each_relevant_instruction(
        [&](const llvm::Function& func, const llvm::BasicBlock& block, const llvm::Instruction& inst) {
          const bool is_seed = seed_instructions.find(&inst) != seed_instructions.end();
          const bool is_dep  = dependency_instructions.find(&inst) != dependency_instructions.end();
          if (!is_seed && !is_dep) {
            return;
          }

          ensure_function_bucket(&func).instructions.emplace_back(&inst, &block, instruction_line_or_zero(inst),
                                                                  !is_seed && is_dep);
        });
  }

 public:
  LineMapBuilder(const llvm::Module* module, const std::optional<std::string>& main_file_path, unsigned line_start,
                 unsigned search_end, bool include_deps)
      : module(module),
        main_file_path(main_file_path),
        line_start(line_start),
        search_end(search_end),
        include_deps(include_deps) {
  }

  LineMap run() {
    collect_seed_instructions();
    collect_backward_dependencies();
    build_ordered_line_map();
    return std::move(ordered_line_map);
  }
};

LineMap collect_line_map(const llvm::Module* module, unsigned line_start, unsigned search_end,
                         const std::optional<std::string>& main_file_path, bool include_deps) {
  return LineMapBuilder(module, main_file_path, line_start, search_end, include_deps).run();
}

void print_line_map(llvm::raw_ostream& out, const LineMap& line_map, bool print_line_prefix) {
  std::size_t line_number_width = [&print_line_prefix](const auto& line_map) {
    unsigned width{0};
    if (print_line_prefix) {
      unsigned max_line = 0;
      for (const auto& [_, instructions] : line_map) {
        for (const auto& line_instruction : instructions) {
          max_line = std::max(max_line, line_instruction.line);
        }
      }
      width = std::to_string(max_line).size();
    }
    return width;
  }(line_map);

  for (const auto& [func, instructions] : line_map) {
    out << func->getName() << ":\n";
    const llvm::BasicBlock* current_block{nullptr};
    for (const auto& line_instruction : instructions) {
      if (line_instruction.block != current_block) {
        current_block = line_instruction.block;
        if (!current_block) {
          continue;
        }
        out.indent(2);
        if (current_block->hasName()) {
          out << current_block->getName() << ":\n";
        } else {
          current_block->printAsOperand(out, false);
          out << ":\n";
        }
      }
      out.indent(2);
      if (print_line_prefix) {
        out << (line_instruction.is_dependency ? '+' : ' ');
        const std::string line = std::to_string(line_instruction.line);
        if (line.size() < line_number_width) {
          out.indent(line_number_width - line.size());
        }
        out << line << " | ";
      }
      line_instruction.inst->print(out);
      out << "\n";
    }
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

void IRNodeFinder::printByLocation(unsigned line_start, unsigned line_end, const IRPrintingFlags& flags) const {
  const unsigned search_end = std::max(line_start, line_end);
  const auto* module        = tool.getModule();
  const auto main_file_path = get_main_file_path(module);
  const LineMap line_map = collect_line_map(module, line_start, search_end, main_file_path, flags.include_dependencies);

  if (!line_map.empty()) {
    print_line_map(os, line_map, flags.print_line);
    os << "\n";
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
