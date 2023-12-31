// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/diagnostics/diagnostics_controller.h"

#include <memory>
#include <string>

#include "base/command_line.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/diagnostics/diagnostics_model.h"
#include "chrome/browser/diagnostics/diagnostics_test.h"
#include "chrome/browser/diagnostics/diagnostics_writer.h"

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "ash/constants/ash_switches.h"
#endif

namespace diagnostics {

DiagnosticsController* DiagnosticsController::GetInstance() {
  return base::Singleton<DiagnosticsController>::get();
}

DiagnosticsController::DiagnosticsController() : writer_(nullptr) {}

DiagnosticsController::~DiagnosticsController() {}

const DiagnosticsModel& DiagnosticsController::GetResults() const {
  return *model_;
}

bool DiagnosticsController::HasResults() {
  return (model_.get() && model_->GetTestRunCount() > 0);
}

void DiagnosticsController::ClearResults() { model_.reset(); }

void DiagnosticsController::RecordRegularStartup() {
#if BUILDFLAG(IS_CHROMEOS_ASH)  // Only collecting UMA stats on ChromeOS
  // For each of the test types, record a normal start (no diagnostics run), so
  // we have a common denominator.
  for (int i = 0; i < DIAGNOSTICS_TEST_ID_COUNT; ++i) {
    RecordUMARecoveryResult(static_cast<DiagnosticsTestId>(i), RESULT_NOT_RUN);
    RecordUMATestResult(static_cast<DiagnosticsTestId>(i), RESULT_NOT_RUN);
  }
#endif
}

// This entry point is called from early in startup when very few things have
// been initialized, so be careful what you use.
int DiagnosticsController::Run(const base::CommandLine& command_line,
                               DiagnosticsWriter* writer) {
  writer_ = writer;

  model_.reset(MakeDiagnosticsModel(command_line));
  model_->RunAll(writer_);

  return 0;
}

// This entry point is called from early in startup when very few things have
// been initialized, so be careful what you use.
int DiagnosticsController::RunRecovery(const base::CommandLine& command_line,
                                       DiagnosticsWriter* writer) {
  if (!HasResults()) {
    if (writer) {
      writer->WriteInfoLine("No diagnostics have been run.");
      writer->OnAllRecoveryDone(model_.get());
    }
    return -1;
  }

  writer_ = writer;

  model_->RecoverAll(writer_);
  return 0;
}

}  // namespace diagnostics
