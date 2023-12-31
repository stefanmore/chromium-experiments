// Copyright 2010 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SANDBOX_WIN_SRC_PROCESS_THREAD_DISPATCHER_H_
#define SANDBOX_WIN_SRC_PROCESS_THREAD_DISPATCHER_H_

#include <stdint.h>

#include "sandbox/win/src/crosscall_server.h"
#include "sandbox/win/src/ipc_tags.h"
#include "sandbox/win/src/sandbox_policy_base.h"

namespace sandbox {

// This class handles process and thread-related IPC calls.
class ThreadProcessDispatcher : public Dispatcher {
 public:
  explicit ThreadProcessDispatcher();

  ThreadProcessDispatcher(const ThreadProcessDispatcher&) = delete;
  ThreadProcessDispatcher& operator=(const ThreadProcessDispatcher&) = delete;

  ~ThreadProcessDispatcher() override {}

  // Dispatcher interface.
  bool SetupService(InterceptionManager* manager, IpcTag service) override;

 private:
  // Processes IPC requests coming from calls to NtOpenThread() in the target.
  bool NtOpenThread(IPCInfo* ipc, uint32_t desired_access, uint32_t thread_id);

  // Processes IPC requests from calls to NtOpenProcessTokenEx() in the target.
  bool NtOpenProcessTokenEx(IPCInfo* ipc,
                            HANDLE process,
                            uint32_t desired_access,
                            uint32_t attributes);

  // Processes IPC requests coming from calls to CreateThread() in the target.
  bool CreateThread(IPCInfo* ipc,
                    SIZE_T stack_size,
                    LPTHREAD_START_ROUTINE start_address,
                    LPVOID parameter,
                    DWORD creation_flags);
};

}  // namespace sandbox

#endif  // SANDBOX_WIN_SRC_PROCESS_THREAD_DISPATCHER_H_
