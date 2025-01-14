//===-------------- PassBuilder bindings for LLVM-C -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file defines the C bindings to the new pass manager
///
//===----------------------------------------------------------------------===//

#include "llvm-c/Transforms/PassBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/CBindingWrapping.h"

using namespace llvm;

namespace llvm {
/// Helper struct for holding a set of builder options for LLVMRunPasses. This
/// structure is used to keep LLVMRunPasses backwards compatible with future
/// versions in case we modify the options the new Pass Manager utilizes.
class LLVMPassBuilderOptions {
public:
  explicit LLVMPassBuilderOptions(
      bool DebugLogging = false, bool VerifyEach = false,
      PipelineTuningOptions PTO = PipelineTuningOptions())
      : DebugLogging(DebugLogging), VerifyEach(VerifyEach), PTO(PTO) {}

  bool DebugLogging;
  bool VerifyEach;
  PipelineTuningOptions PTO;
};
} // namespace llvm

static TargetMachine *unwrap(LLVMTargetMachineRef P) {
  return reinterpret_cast<TargetMachine *>(P);
}

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(LLVMPassBuilderOptions,
                                   LLVMPassBuilderOptionsRef)

LLVMErrorRef LLVMRunPasses(LLVMModuleRef M, const char *Passes,
                           LLVMTargetMachineRef TM,
                           LLVMPassBuilderOptionsRef Options) {
  TargetMachine *Machine = unwrap(TM);
  LLVMPassBuilderOptions *PassOpts = unwrap(Options);
  bool Debug = PassOpts->DebugLogging;
  bool VerifyEach = PassOpts->VerifyEach;

  Module *Mod = unwrap(M);
  PassInstrumentationCallbacks PIC;
  PassBuilder PB(Debug, Machine, PassOpts->PTO, None, &PIC);

  LoopAnalysisManager LAM(Debug);
  FunctionAnalysisManager FAM(Debug);
  CGSCCAnalysisManager CGAM(Debug);
  ModuleAnalysisManager MAM(Debug);
  PB.registerLoopAnalyses(LAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerModuleAnalyses(MAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  StandardInstrumentations SI(Debug, VerifyEach);
  SI.registerCallbacks(PIC, &FAM);
  ModulePassManager MPM(Debug);
  if (VerifyEach) {
    MPM.addPass(VerifierPass());
  }
  if (auto Err = PB.parsePassPipeline(MPM, Passes)) {
    return wrap(std::move(Err));
  }

  MPM.run(*Mod, MAM);
  return LLVMErrorSuccess;
}

LLVMPassBuilderOptionsRef LLVMCreatePassBuilderOptions() {
  return wrap(new LLVMPassBuilderOptions());
}

void LLVMPassBuilderOptionsSetVerifyEach(LLVMPassBuilderOptionsRef Options,
                                         LLVMBool VerifyEach) {
  unwrap(Options)->VerifyEach = VerifyEach;
}

void LLVMPassBuilderOptionsSetDebugLogging(LLVMPassBuilderOptionsRef Options,
                                           LLVMBool DebugLogging) {
  unwrap(Options)->DebugLogging = DebugLogging;
}

void LLVMPassBuilderOptionsSetLoopInterleaving(
    LLVMPassBuilderOptionsRef Options, LLVMBool LoopInterleaving) {
  unwrap(Options)->PTO.LoopInterleaving = LoopInterleaving;
}

void LLVMPassBuilderOptionsSetLoopVectorization(
    LLVMPassBuilderOptionsRef Options, LLVMBool LoopVectorization) {
  unwrap(Options)->PTO.LoopVectorization = LoopVectorization;
}

void LLVMPassBuilderOptionsSetSLPVectorization(
    LLVMPassBuilderOptionsRef Options, LLVMBool SLPVectorization) {
  unwrap(Options)->PTO.SLPVectorization = SLPVectorization;
}

void LLVMPassBuilderOptionsSetLoopUnrolling(LLVMPassBuilderOptionsRef Options,
                                            LLVMBool LoopUnrolling) {
  unwrap(Options)->PTO.LoopUnrolling = LoopUnrolling;
}

void LLVMPassBuilderOptionsSetForgetAllSCEVInLoopUnroll(
    LLVMPassBuilderOptionsRef Options, LLVMBool ForgetAllSCEVInLoopUnroll) {
  unwrap(Options)->PTO.ForgetAllSCEVInLoopUnroll = ForgetAllSCEVInLoopUnroll;
}

void LLVMPassBuilderOptionsSetCoroutines(LLVMPassBuilderOptionsRef Options,
                                         LLVMBool Coroutines) {
  unwrap(Options)->PTO.Coroutines = Coroutines;
}

void LLVMPassBuilderOptionsSetLicmMssaOptCap(LLVMPassBuilderOptionsRef Options,
                                             unsigned LicmMssaOptCap) {
  unwrap(Options)->PTO.LicmMssaOptCap = LicmMssaOptCap;
}

void LLVMPassBuilderOptionsSetLicmMssaNoAccForPromotionCap(
    LLVMPassBuilderOptionsRef Options, unsigned LicmMssaNoAccForPromotionCap) {
  unwrap(Options)->PTO.LicmMssaNoAccForPromotionCap =
      LicmMssaNoAccForPromotionCap;
}

void LLVMPassBuilderOptionsSetCallGraphProfile(
    LLVMPassBuilderOptionsRef Options, LLVMBool CallGraphProfile) {
  unwrap(Options)->PTO.CallGraphProfile = CallGraphProfile;
}

void LLVMPassBuilderOptionsSetMergeFunctions(LLVMPassBuilderOptionsRef Options,
                                             LLVMBool MergeFunctions) {
  unwrap(Options)->PTO.MergeFunctions = MergeFunctions;
}

void LLVMDisposePassBuilderOptions(LLVMPassBuilderOptionsRef Options) {
  delete unwrap(Options);
}
