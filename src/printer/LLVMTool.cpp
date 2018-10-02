/*
 * LLVMTool.cpp
 *
 *  Created on: Sep 28, 2018
 *      Author: ahueck
 */

#include "printer/LLVMTool.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/CompilationDatabase.h>

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
      if (!Arg.startswith("-O") && !Arg.startswith("-g")) {
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
    llvm::cl::TopLevelSubCommand->PositionalOpts.clear();
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

  FrontendAction* create() override {
    return new ExtractorAction(ctx, Mods);
  }
};

std::unique_ptr<tooling::FrontendActionFactory> CreateExtractorActionFactory(LLVMContext& VMC,
                                                                             std::unique_ptr<llvm::Module>& Mods) {
  return std::make_unique<ExtractorActionFactory>(VMC, Mods);
}

}  // namespace action

LLVMTool::LLVMTool(CommonOptionsParser& op) : tool(op.getCompilations(), op.getSourcePathList()) {
}

void LLVMTool::execute() {
  commitUserArgs();
  auto action = action::CreateExtractorActionFactory(ctx, m);
  tool.run(action.get());
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
  this->user_args.erase(llvm::remove_if(user_args, [&flag](const auto& str) { return flag == str; }), user_args.end());
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
    if (flag.startswith("-O") || flag.startswith("-g")) {
      tool.appendArgumentsAdjuster(combineAdjusters(
          adjuster::getStripOptFlagAdjuster(), getInsertArgumentAdjuster(flag.data(), ArgumentInsertPosition::END)));
    } else {
      tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(flag.data()));
    }
  }
}

LLVMTool::~LLVMTool() = default;

}  // namespace irprinter
