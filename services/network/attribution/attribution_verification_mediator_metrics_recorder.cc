// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string_view>

#include "services/network/attribution/attribution_verification_mediator_metrics_recorder.h"
#include "base/check.h"
#include "base/metrics/histogram_functions.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "services/network/attribution/attribution_verification_mediator.h"

namespace network {

namespace {

using Step = AttributionVerificationMediator::Step;
using GetHeadersStatus = AttributionVerificationMediator::GetHeadersStatus;
using ProcessVerificationStatus =
    AttributionVerificationMediator::ProcessVerificationStatus;

enum TimeSpan {
  kGetKeyCommitment,
  kInitializeCryptographer,
  kBlindMessage,
  kSignBlindMessage,
  kUnblindSignature,
  kTotal,
};

void RecordTiming(TimeSpan time_span, bool is_success, base::TimeDelta delta) {
  // These must stay in sync with the corresponding histogram suffixes in
  // histograms.xml.
  std::string_view time_span_string;
  switch (time_span) {
    case kGetKeyCommitment:
      time_span_string = "GetKeyCommitment";
      break;
    case kInitializeCryptographer:
      time_span_string = "InitializeCryptographer";
      break;
    case kBlindMessage:
      time_span_string = "BlindMessage";
      break;
    case kSignBlindMessage:
      time_span_string = "SignBlindMessage";
      break;
    case kUnblindSignature:
      time_span_string = "UnblindSignature";
      break;
    case kTotal:
      time_span_string = "Total";
      break;
  }
  std::string_view outcome_string = is_success ? "Success" : "Failure";

  base::UmaHistogramTimes(
      base::JoinString({"Conversions.ReportVerification.Duration",
                        time_span_string, outcome_string},
                       "."),
      delta);
}

}  // namespace

AttributionVerificationMediatorMetricsRecorder::
    AttributionVerificationMediatorMetricsRecorder() = default;
AttributionVerificationMediatorMetricsRecorder::
    ~AttributionVerificationMediatorMetricsRecorder() = default;

void AttributionVerificationMediatorMetricsRecorder::Start() {
  DCHECK(get_headers_start_.is_null());
  get_headers_start_ = base::TimeTicks::Now();
}

void AttributionVerificationMediatorMetricsRecorder::Complete(Step step) {
  switch (step) {
    case Step::kGetKeyCommitment:
      DCHECK(!get_headers_start_.is_null());
      DCHECK(get_key_commitment_end_.is_null());
      get_key_commitment_end_ = base::TimeTicks::Now();
      break;
    case Step::kInitializeCryptographer:
      DCHECK(!get_key_commitment_end_.is_null());
      DCHECK(crypto_initialization_end_.is_null());
      crypto_initialization_end_ = base::TimeTicks::Now();
      break;
    case Step::kBlindMessage:
      DCHECK(!crypto_initialization_end_.is_null());
      DCHECK(blind_message_end_.is_null());
      blind_message_end_ = base::TimeTicks::Now();
      break;
    case Step::kSignBlindMessage:
      DCHECK(!blind_message_end_.is_null());
      DCHECK(sign_blind_message_end_.is_null());
      sign_blind_message_end_ = base::TimeTicks::Now();
      break;
    case Step::kUnblindMessage:
      DCHECK(!sign_blind_message_end_.is_null());
      DCHECK(unblind_message_end_.is_null());
      unblind_message_end_ = base::TimeTicks::Now();
      break;
  }
}

void AttributionVerificationMediatorMetricsRecorder::FinishGetHeadersWith(
    GetHeadersStatus status) {
  DCHECK(!get_headers_start_.is_null());

  if (!get_key_commitment_end_.is_null()) {
    bool is_success =
        status !=
        AttributionVerificationMediator::GetHeadersStatus::kIssuerNotRegistered;
    RecordTiming(TimeSpan::kGetKeyCommitment, is_success,
                 /*delta=*/get_key_commitment_end_ - get_headers_start_);
  }

  if (!crypto_initialization_end_.is_null()) {
    bool is_success =
        status != GetHeadersStatus::kUnableToAddKeysOnCryptographer &&
        status != GetHeadersStatus::kUnableToInitializeCryptographer;
    RecordTiming(
        TimeSpan::kInitializeCryptographer, is_success,
        /*delta=*/crypto_initialization_end_ - get_key_commitment_end_);
  }

  if (!blind_message_end_.is_null()) {
    bool is_success = status != GetHeadersStatus::kUnableToBlindMessage;
    RecordTiming(TimeSpan::kBlindMessage, is_success,
                 /*delta=*/blind_message_end_ - crypto_initialization_end_);
  }

  base::UmaHistogramEnumeration(
      "Conversions.ReportVerification.GetHeadersStatus", status);
}

void AttributionVerificationMediatorMetricsRecorder::
    FinishProcessVerificationWith(ProcessVerificationStatus status) {
  DCHECK(!blind_message_end_.is_null());

  auto finish_end = base::TimeTicks::Now();

  if (!sign_blind_message_end_.is_null()) {
    bool is_success =
        status != ProcessVerificationStatus::kNoSignatureReceivedFromIssuer;
    RecordTiming(TimeSpan::kSignBlindMessage, is_success,
                 /*delta=*/sign_blind_message_end_ - blind_message_end_);
  }

  if (!unblind_message_end_.is_null()) {
    bool is_success =
        status != ProcessVerificationStatus::kUnableToUnblindSignature;
    RecordTiming(TimeSpan::kUnblindSignature, is_success,
                 /*delta=*/unblind_message_end_ - sign_blind_message_end_);
  }

  RecordTiming(TimeSpan::kTotal,
               /*is_success=*/status == ProcessVerificationStatus::kSuccess,
               /*delta=*/finish_end - get_headers_start_);

  base::UmaHistogramEnumeration(
      "Conversions.ReportVerification.ProcessVerificationStatus", status);
}

}  // namespace network
