// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PERFORMANCE_MANAGER_METRICS_CPU_PROBE_PRESSURE_SAMPLE_H_
#define CHROME_BROWSER_PERFORMANCE_MANAGER_METRICS_CPU_PROBE_PRESSURE_SAMPLE_H_

namespace performance_manager::metrics {

// Represents availability of compute resources measured over a period of time.
struct PressureSample {
  // Average utilization of all CPU cores.
  //
  // Values use a scale from 0.0 (no utilization) to 1.0 (100% utilization).
  double cpu_utilization;
};

}  // namespace performance_manager::metrics

#endif  // CHROME_BROWSER_PERFORMANCE_MANAGER_METRICS_CPU_PROBE_PRESSURE_SAMPLE_H_
