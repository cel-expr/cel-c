// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cel-c/assert.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __ANDROID__
#include <android/api-level.h>
#if __ANDROID_API__ >= 21
#include <syslog.h>
extern void android_set_abort_message(const char* msg);
#else
#include <assert.h>
#endif
#endif

#include "cel-c/config.h"

#if defined(__APPLE__) && CEL_HAVE_INCLUDE(<CrashReporterClient.h>)
#include <CrashReporterClient.h>
#endif

#ifdef _MSC_VER
#include <intrin.h>
#endif

void cel_AssertionFailed(CEL_NONNULL(const char*) file, int line,
                         CEL_NONNULL(const char*) cond) {
  CEL_USED(file);
  CEL_USED(line);
  CEL_USED(cond);
#ifndef NDEBUG
  fprintf(stderr, "%s:%d: Assertion `%s' failed\n", file, line, cond);
  fflush(stderr);
#ifdef __ANDROID__
  CEL_NULLABLE(char*) amsg = cel_nullptr;
  asprintf(&amsg, "%s:%d: Assertion `%s' failed\n", file, line, cond);
  if (amsg == cel_nullptr) {
    amsg = (CEL_NULLABLE(char*))"";
  }
#if __ANDROID_API__ >= 21
  android_set_abort_message(amsg);
  openlog("cel", 0, 0);
  syslog(LOG_CRIT, "%s", amsg);
  closelog();
#else
  __assert2(file, line, __func__, amsg);
#endif
#elif defined(__APPLE__) && CEL_HAVE_INCLUDE(<CrashReporterClient.h>)
  CEL_NULLABLE(char*) amsg = cel_nullptr;
  asprintf(&amsg, "%s:%d: Assertion `%s' failed\n", file, line, cond);
  if (amsg == cel_nullptr) {
    amsg = (CEL_NULLABLE(char*))"";
  }
  CRSetCrashLogMessage(amsg);
#endif
#endif
#if !defined(NDEBUG) && CEL_HAVE_BUILTIN(__builtin_debugtrap)
  __builtin_debugtrap();
#elif !defined(NDEBUG) && defined(_MSC_VER)
  __debugbreak();
#endif
  abort();
}
