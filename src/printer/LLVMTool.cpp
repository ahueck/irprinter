/*
 * LLVMTool.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: ahueck
 */

#include "printer/LLVMTool.h"
#include "Util.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Support/CommandLine.h>

#include <clang/CodeGen/BackendUtil.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <llvm/IR/Module.h>

#include <vector>

namespace irprinter {

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

namespace adjuster {
ArgumentsAdjuster getStripOptFlagAdjuster() {
  return [](const CommandLineArguments& Args, StringRef /*unused*/) {
    CommandLineArguments AdjustedArgs;
    for (size_t i = 0, e = Args.size(); i < e; ++i) {
      StringRef Arg = Args[i];
      if (!util::starts_with_any_of(Arg, "-O", "-g")) {
        AdjustedArgs.push_back(Args[i]);
      }
    }
    return AdjustedArgs;
  };
}
}  // namespace adjuster

namespace action {
class ExtractorAction : public CodeGenAction {
  std::unique_ptr<llvm::Module>& m;

 public:
  ExtractorAction(LLVMContext& ctx, std::unique_ptr<llvm::Module>& m) : CodeGenAction(Backend_EmitNothing, &ctx), m(m) {
  }

  bool BeginSourceFileAction(CompilerInstance& CI) {
    // FIXME workaround for error: "clang: Not enough positional command line arguments specified!"
    // when using clangtool, the commandline parser is executed twice, this removes a leftover causing the above error
#if LLVM_VERSION_MAJOR < 19
    llvm::cl::TopLevelSubCommand->PositionalOpts.clear();
#endif
    return CodeGenAction::BeginSourceFileAction(CI);
  }

  void EndSourceFileAction() {
    CodeGenAction::EndSourceFileAction();
    std::unique_ptr<llvm::Module> Mod(takeModule());
    if (Mod) {
      m = std::move(Mod);
    }
  }
};

class ExtractorActionFactory : public tooling::FrontendActionFactory {
  LLVMContext& ctx;
  std::unique_ptr<llvm::Module>& Mods;

 public:
  ExtractorActionFactory(LLVMContext& ctx, std::unique_ptr<llvm::Module>& Mods) : ctx(ctx), Mods(Mods) {
  }

  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<ExtractorAction>(ctx, Mods);
  }
};

std::unique_ptr<tooling::FrontendActionFactory> CreateExtractorActionFactory(LLVMContext& VMC,
                                                                             std::unique_ptr<llvm::Module>& Mods) {
  return std::make_unique<ExtractorActionFactory>(VMC, Mods);
}

}  // namespace action

LLVMTool::LLVMTool(CommonOptionsParser& op) : LLVMTool(op.getCompilations(), op.getSourcePathList()) {
}

LLVMTool::LLVMTool(const CompilationDatabase& compilation_database, ArrayRef<std::string> source_path)
    : tool(compilation_database, source_path) {
#ifdef IRPRINTER_CLANG_RESOURCE_DIR
  const std::string resource_dir = IRPRINTER_CLANG_RESOURCE_DIR;
  if (!resource_dir.empty()) {
    const std::string resource_arg = "-resource-dir=" + resource_dir;
    tool.appendArgumentsAdjuster(
        clang::tooling::getInsertArgumentAdjuster(resource_arg.c_str(), clang::tooling::ArgumentInsertPosition::BEGIN));
  }
#endif
}

int LLVMTool::execute() {
  commitUserArgs();
  auto action = action::CreateExtractorActionFactory(ctx, m);
  return tool.run(action.get());
}

std::unique_ptr<llvm::Module> LLVMTool::takeModule() {
  return std::move(m);
}

const llvm::Module* LLVMTool::getModule() const {
  return m.get();
}

void LLVMTool::setFlag(StringRef flag) {
  user_args.emplace_back(flag.data());
}

void LLVMTool::removeFlag(StringRef flag) {
  user_args.erase(llvm::remove_if(user_args, [&flag](const auto& str) { return flag == str; }), user_args.end());
}

void LLVMTool::clearUserFlags() {
  user_args.clear();
  tool.clearArgumentsAdjusters();
  tool.appendArgumentsAdjuster(getClangStripOutputAdjuster());
  tool.appendArgumentsAdjuster(getClangSyntaxOnlyAdjuster());
  tool.appendArgumentsAdjuster(getClangStripDependencyFileAdjuster());
}

void LLVMTool::commitUserArgs() {
  for (auto& arg : user_args) {
    StringRef flag = arg;
    if (util::starts_with_any_of(flag, "-O", "-g")) {
      tool.appendArgumentsAdjuster(combineAdjusters(
          adjuster::getStripOptFlagAdjuster(), getInsertArgumentAdjuster(flag.data(), ArgumentInsertPosition::END)));
    } else {
      tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(flag.data()));
    }
  }
}

LLVMTool::~LLVMTool() = default;

}  // namespace irprinter
