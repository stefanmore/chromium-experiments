// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is of the same format as file that generated by
//     third_party/jni_zero/jni_zero.py
// For
//     com/google/vr/internal/controller/NativeCallbacks

// Local modification includes:
// 1. Remove all implementation, only keep definition.
// 2. Use absolute path instead of relative path.
// 3. Removed all helper functions such as: Create.
// 4. Removed external functions that don't have implementation in shim file.
// 5. Replace all nativeHandle to handle. This is because jni_generator.py
// require jni functions start with "native" prefix. So we add the prefix to
// generate the file. But the real jni functions in the static library
// doesn't have the prefix.
// 6. Added function RegisterNativeCallbacksNatives at the end of this file.
// 7. Added "vr" as an argument to base::android::LazyGetClass.

#ifndef com_google_vr_internal_controller_NativeCallbacks_JNI
#define com_google_vr_internal_controller_NativeCallbacks_JNI

#include "base/android/jni_android.h"
// ----------------------------------------------------------------------------
// Native JNI methods
// ----------------------------------------------------------------------------
#include <jni.h>
#include <atomic>
#include <type_traits>

#include "third_party/jni_zero/jni_int_wrapper.h"
#include "third_party/jni_zero/jni_zero_helper.h"

// Step 1: forward declarations.
namespace {
const char kNativeCallbacksClassPath[] =
    "com/google/vr/internal/controller/NativeCallbacks";
// Leaking this jclass as we cannot use LazyInstance from some threads.
std::atomic<jclass> g_NativeCallbacks_clazz __attribute__((unused)) (nullptr);
#define NativeCallbacks_clazz(env)                                  \
  base::android::LazyGetClass(env, kNativeCallbacksClassPath, "vr", \
                              &g_NativeCallbacks_clazz)

}  // namespace

namespace NativeCallbacks {
// Step 2: method stubs.

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleStateChanged(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jint newState);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleControllerRecentered(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jfloat qx,
    jfloat qy,
    jfloat qz,
    jfloat qw);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleTouchEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jint action,
    jfloat x,
    jfloat y);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleOrientationEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jfloat qx,
    jfloat qy,
    jfloat qz,
    jfloat qw);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleButtonEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jint buttonCode,
    jboolean down);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleAccelEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jfloat x,
    jfloat y,
    jfloat z);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleGyroEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jfloat x,
    jfloat y,
    jfloat z);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handlePositionEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jfloat x,
    jfloat y,
    jfloat z);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleBatteryEvent(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint controllerIndex,
    jlong timestampNanos,
    jboolean isCharging,
    jint batteryLevelBucket);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceInitFailed(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint failureReason);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceFailed(
    JNIEnv* env,
    jobject jcaller,
    jlong userData);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceUnavailable(
    JNIEnv* env,
    jobject jcaller,
    jlong userData);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceConnected(
    JNIEnv* env,
    jobject jcaller,
    jlong userData,
    jint flags);

JNI_BOUNDARY_EXPORT void
Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceDisconnected(
    JNIEnv* env,
    jobject jcaller,
    jlong userData);

// Step 3: RegisterNatives.

static const JNINativeMethod kMethodsNativeCallbacks[] = {
    {"handleStateChanged",
     "("
     "J"
     "I"
     "I"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleStateChanged)},
    {"handleControllerRecentered",
     "("
     "J"
     "I"
     "J"
     "F"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleControllerRecentered)},
    {"handleTouchEvent",
     "("
     "J"
     "I"
     "J"
     "I"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleTouchEvent)},
    {"handleOrientationEvent",
     "("
     "J"
     "I"
     "J"
     "F"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleOrientationEvent)},
    {"handleButtonEvent",
     "("
     "J"
     "I"
     "J"
     "I"
     "Z"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleButtonEvent)},
    {"handleAccelEvent",
     "("
     "J"
     "I"
     "J"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleAccelEvent)},
    {"handleGyroEvent",
     "("
     "J"
     "I"
     "J"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleGyroEvent)},
    {"handlePositionEvent",
     "("
     "J"
     "I"
     "J"
     "F"
     "F"
     "F"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handlePositionEvent)},
    {"handleBatteryEvent",
     "("
     "J"
     "I"
     "J"
     "Z"
     "I"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleBatteryEvent)},
    {"handleServiceInitFailed",
     "("
     "J"
     "I"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceInitFailed)},
    {"handleServiceFailed",
     "("
     "J"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceFailed)},
    {"handleServiceUnavailable",
     "("
     "J"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceUnavailable)},
    {"handleServiceConnected",
     "("
     "J"
     "I"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceConnected)},
    {"handleServiceDisconnected",
     "("
     "J"
     ")"
     "V",
     reinterpret_cast<void*>(
         Java_com_google_vr_internal_controller_NativeCallbacks_handleServiceDisconnected)},
};

static bool RegisterNativesImpl(JNIEnv* env) {
  const int kMethodsNativeCallbacksSize =
      std::extent<decltype(kMethodsNativeCallbacks)>();

  if (env->RegisterNatives(NativeCallbacks_clazz(env), kMethodsNativeCallbacks,
                           kMethodsNativeCallbacksSize) < 0) {
    jni_generator::HandleRegistrationError(env, NativeCallbacks_clazz(env),
                                           __FILE__);
    return false;
  }

  return true;
}

static bool RegisterNativeCallbacksNatives(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace NativeCallbacks

#endif  // com_google_vr_internal_controller_NativeCallbacks_JNI
