// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base.test.params;

/**
 * Generator to use generate arguments for parameterized test methods.
 *
 * @see ParameterAnnotations.UseMethodParameter
 */
public interface ParameterProvider {
    Iterable<ParameterSet> getParameters();
}
