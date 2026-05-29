// Copyright 2025 Google LLC
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

#include "cel-c/error_space.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#include <winsock2.h>
#endif

#include "cel-c/assert.h"
#include "cel-c/config.h"
#include "cel-c/cstring_view.h"
#include "cel-c/status_code.h"

int cel_GenericErrorSpace_CurrentCode() { return errno; }

int cel_SystemErrorSpace_CurrentCode() {
#ifndef _WIN32
  return cel_GenericErrorSpace_CurrentCode();
#else
  return (int)GetLastError();
#endif
}

static bool _cel_CanonicalErrorSpace_OutOfMemory(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code) {
  CEL_ASSERT_EQ(space, cel_CanonicalErrorSpace);

  // The canonical error space does not have an out of memory condition.
  return false;
}

static cel_StatusCode _cel_CanonicalErrorSpace_Canonical(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code) {
  CEL_ASSERT_EQ(space, cel_CanonicalErrorSpace);

  return (cel_StatusCode)code;
}

static int _cel_CanonicalErrorSpace_Message(CEL_NONNULL(const cel_ErrorSpace*)
                                                space,
                                            int code, CEL_NONNULL(char*) buf,
                                            size_t buflen) {
  CEL_ASSERT_EQ(space, cel_CanonicalErrorSpace);
  CEL_ASSERT_NOT_NULL(buf);
  CEL_ASSERT_GT(buflen, 0);

  if (CEL_UNLIKELY(buflen == 0)) {
    return 0;
  }
  if (code == 0) {
    buf[0] = '\0';
    return 0;
  }
  const char* desc = cel_StatusCode_Message(code);
  if (desc == cel_nullptr) {
    buf[0] = '\0';
    return 0;
  }
  size_t desc_len = strlen(desc);
  if (desc_len == 0) {
    buf[0] = '\0';
    return 0;
  }
  size_t to_copy = desc_len >= buflen ? buflen - 1 : desc_len;
  memcpy(buf, desc, to_copy);
  buf[to_copy] = '\0';
  return 0;
}

static const cel_ErrorSpaceVTable _cel_CanonicalErrorSpaceVTable = {
    .name = CEL_CSTRINGVIEW_C("canonical"),
    .OutOfMemory = _cel_CanonicalErrorSpace_OutOfMemory,
    .Canonical = _cel_CanonicalErrorSpace_Canonical,
    .Message = _cel_CanonicalErrorSpace_Message,
};

static const cel_ErrorSpace _cel_CanonicalErrorSpace = {
    .vtable = &_cel_CanonicalErrorSpaceVTable,
};

CEL_NONNULL(const cel_ErrorSpace*)
const cel_CanonicalErrorSpace = &_cel_CanonicalErrorSpace;

static cel_StatusCode _cel_StatusCode_FromPosix(int code) {
  switch (code) {
    case 0:
      return cel_StatusCode_kOk;
    case EINVAL:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENAMETOOLONG:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case E2BIG:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EDESTADDRREQ:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EDOM:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EFAULT:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EILSEQ:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOPROTOOPT:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOTSOCK:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOTTY:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EPROTOTYPE:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ESPIPE:
      return cel_StatusCode_kInvalidArgument;
    case ETIMEDOUT:
      return cel_StatusCode_kDeadlineExceeded;
    case ENODEV:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOENT:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ENOMEDIUM
    case ENOMEDIUM:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ENXIO:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ESRCH:
      return cel_StatusCode_kNotFound;
    case EEXIST:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EADDRNOTAVAIL:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ENOTUNIQ
    case ENOTUNIQ:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EALREADY:
      return cel_StatusCode_kAlreadyExists;
    case EPERM:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EACCES:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ENOKEY
    case ENOKEY:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EROFS:
      return cel_StatusCode_kPermissionDenied;
    case ENOTEMPTY:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EISDIR:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOTDIR:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EADDRINUSE:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EBADF:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef EBADFD
    case EBADFD:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ECHILD:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EISCONN:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef EISNAM
    case EISNAM:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
#ifdef ENOTBLK
    case ENOTBLK:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ENOTCONN:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EPIPE:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ESHUTDOWN
    case ESHUTDOWN:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ETXTBSY:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef EUNATCH
    case EUNATCH:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EBUSY:
      return cel_StatusCode_kFailedPrecondition;
    case ENOSPC:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef EDQUOT
    case EDQUOT:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EMFILE:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EMLINK:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENFILE:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOBUFS:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ENODATA
    case ENODATA:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
#ifdef EUSERS
    case EUSERS:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ENOMEM:
      return cel_StatusCode_kResourceExhausted;
#ifdef ECHRNG
    case ECHRNG:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EFBIG:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EOVERFLOW:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ERANGE:
      return cel_StatusCode_kOutOfRange;
#ifdef ENOPKG
    case ENOPKG:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ENOSYS:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOTSUP:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EAFNOSUPPORT:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef EPFNOSUPPORT
    case EPFNOSUPPORT:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EPROTONOSUPPORT:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EXDEV:
      return cel_StatusCode_kUnimplemented;
    case EAGAIN:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ECOMM
    case ECOMM:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ECONNREFUSED:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ECONNABORTED:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ECONNRESET:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case EINTR:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef EHOSTDOWN
    case EHOSTDOWN:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EHOSTUNREACH:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENETDOWN:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENETRESET:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENETUNREACH:
      CEL_ATTRIBUTE_FALLTHROUGH;
    case ENOLCK:
      CEL_ATTRIBUTE_FALLTHROUGH;
#ifdef ENONET
    case ENONET:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case ENOLINK:
      return cel_StatusCode_kUnavailable;
#ifdef ESTALE
    case ESTALE:
      CEL_ATTRIBUTE_FALLTHROUGH;
#endif
    case EDEADLK:
      return cel_StatusCode_kAborted;
    case ECANCELED:
      return cel_StatusCode_kCancelled;
    default:
      return cel_StatusCode_kUnknown;
  }
}

static bool _cel_GenericErrorSpace_OutOfMemory(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code) {
  CEL_ASSERT_EQ(space, cel_GenericErrorSpace);

  return code == ENOMEM;
}

static cel_StatusCode _cel_GenericErrorSpace_Canonical(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code) {
  CEL_ASSERT_EQ(space, cel_GenericErrorSpace);

  return _cel_StatusCode_FromPosix(code);
}

static int _cel_GenericErrorSpace_Message(CEL_NONNULL(const cel_ErrorSpace*)
                                              space,
                                          int code, CEL_NONNULL(char*) buf,
                                          size_t buflen) {
  CEL_ASSERT_EQ(space, cel_GenericErrorSpace);
  CEL_ASSERT_NOT_NULL(buf);
  CEL_ASSERT_GT(buflen, 0);

  if (CEL_UNLIKELY(buflen == 0)) {
    return 0;
  }
  if (code == 0) {
    buf[0] = '\0';
    return 0;
  }
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__) && \
    (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 31))
  const char* desc = strerrordesc_np(code);
  if (desc == cel_nullptr) {
    buf[0] = '\0';
    return 0;
  }
  size_t desc_len = strlen(desc);
  if (desc_len == 0) {
    buf[0] = '\0';
    return 0;
  }
  size_t to_copy = desc_len >= buflen ? buflen - 1 : desc_len;
  memcpy(buf, desc, to_copy);
  buf[to_copy] = '\0';
  return 0;
#elif defined(_WIN32)
  if (code < 0 || code >= _sys_nerr) {
    return EINVAL;
  }
  const char* desc = _sys_errlist[code];
  if (desc == cel_nullptr) {
    buf[0] = '\0';
    return 0;
  }
  size_t desc_len = strlen(desc);
  if (desc_len == 0) {
    buf[0] = '\0';
    return 0;
  }
  size_t to_copy = desc_len >= buflen ? buflen - 1 : desc_len;
  memcpy(buf, desc, to_copy);
  buf[to_copy] = '\0';
  return 0;
#elif defined(__GLIBC__) && defined(_GNU_SOURCE) && _GNU_SOURCE
  // GNU extension
  char* desc = strerror_r(code, buf, buflen);
  if (desc == cel_nullptr) {
    buf[0] = '\0';
    return 0;
  }
  if (desc == buf) {
    return (ptrdiff_t)strnlen(buf, buflen);
  }
  size_t desc_len = strlen(desc);
  if (desc_len == 0) {
    buf[0] = '\0';
    return 0;
  }
  size_t to_copy = desc_len >= buflen ? buflen - 1 : desc_len;
  memcpy(buf, desc, to_copy);
  buf[to_copy] = '\0';
  return 0;
#elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L) && \
    (!defined(_GNU_SOURCE) || !_GNU_SOURCE)
  // XSI compliant.
  int err = strerror_r(code, buf, buflen);
  if (err) {
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__) && \
    (__GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 13))
    if (err == -1) {
      err = errno;
    }
#endif
    return err;
  }
  return 0;
#else
  // No other alternative, just use strerror and hope it is thread-safe.
  const char* desc = strerror(code);
  if (desc == cel_nullptr) {
    buf[0] = '\0';
    return 0;
  }
  size_t desc_len = strlen(desc);
  if (desc_len == 0) {
    buf[0] = '\0';
    return 0;
  }
  size_t to_copy = desc_len >= buflen ? buflen - 1 : desc_len;
  memcpy(buf, desc, to_copy);
  buf[to_copy] = '\0';
  return 0;
#endif
}

static const cel_ErrorSpaceVTable _cel_GenericErrorSpaceVTable = {
    .name = CEL_CSTRINGVIEW_C("generic"),
    .OutOfMemory = _cel_GenericErrorSpace_OutOfMemory,
    .Canonical = _cel_GenericErrorSpace_Canonical,
    .Message = _cel_GenericErrorSpace_Message,
};

static const cel_ErrorSpace _cel_GenericErrorSpace = {
    .vtable = &_cel_GenericErrorSpaceVTable,
};

CEL_NONNULL(const cel_ErrorSpace*)
const cel_GenericErrorSpace = &_cel_GenericErrorSpace;

static bool _cel_SystemErrorSpace_OutOfMemory(CEL_NONNULL(const cel_ErrorSpace*)
                                                  space,
                                              int code) {
  CEL_ASSERT_EQ(space, cel_SystemErrorSpace);

#ifndef _WIN32
  return cel_ErrorSpace_OutOfMemory(cel_GenericErrorSpace, code);
#else
  return code == ERROR_NOT_ENOUGH_MEMORY || code == ERROR_OUTOFMEMORY;
#endif
}

static cel_StatusCode _cel_SystemErrorSpace_Canonical(
    CEL_NONNULL(const cel_ErrorSpace*) space, int code) {
  CEL_ASSERT_EQ(space, cel_SystemErrorSpace);

#ifndef _WIN32
  return cel_ErrorSpace_Canonical(cel_GenericErrorSpace, code);
#else
  // Borrowed from std::system_category() in MSVC STL.
  switch (code) {
    case 0:
      return cel_StatusCode_kOk;
    case ERROR_INVALID_FUNCTION:
      return _cel_StatusCode_FromPosix(ENOSYS);
    case ERROR_FILE_NOT_FOUND:
      return _cel_StatusCode_FromPosix(ENOENT);
    case ERROR_PATH_NOT_FOUND:
      return _cel_StatusCode_FromPosix(ENOENT);
    case ERROR_TOO_MANY_OPEN_FILES:
      return _cel_StatusCode_FromPosix(ENFILE);
    case ERROR_ACCESS_DENIED:
      return _cel_StatusCode_FromPosix(EACCES);
    case ERROR_INVALID_HANDLE:
      return _cel_StatusCode_FromPosix(EINVAL);
    case ERROR_NOT_ENOUGH_MEMORY:
      return _cel_StatusCode_FromPosix(ENOMEM);
    case ERROR_INVALID_ACCESS:
      return _cel_StatusCode_FromPosix(EACCES);
    case ERROR_OUTOFMEMORY:
      return _cel_StatusCode_FromPosix(ENOMEM);
    case ERROR_INVALID_DRIVE:
      return _cel_StatusCode_FromPosix(ENODEV);
    case ERROR_CURRENT_DIRECTORY:
      return _cel_StatusCode_FromPosix(EACCES);
    case ERROR_NOT_SAME_DEVICE:
      return _cel_StatusCode_FromPosix(EXDEV);
    case ERROR_WRITE_PROTECT:
      return _cel_StatusCode_FromPosix(EACCES);
    case ERROR_BAD_UNIT:
      return _cel_StatusCode_FromPosix(ENODEV);
    case ERROR_NOT_READY:
      return _cel_StatusCode_FromPosix(EAGAIN);
    case ERROR_SEEK:
      return _cel_StatusCode_FromPosix(EIO);
    case ERROR_WRITE_FAULT:
      return _cel_StatusCode_FromPosix(EIO);
    case ERROR_READ_FAULT:
      return _cel_StatusCode_FromPosix(EIO);
    case ERROR_SHARING_VIOLATION:
      return _cel_StatusCode_FromPosix(EACCES);
    case ERROR_LOCK_VIOLATION:
      return _cel_StatusCode_FromPosix(ENOLCK);
    case ERROR_HANDLE_DISK_FULL:
      return _cel_StatusCode_FromPosix(ENOSPC);
    case ERROR_NOT_SUPPORTED:
      return _cel_StatusCode_FromPosix(ENOTSUP);
    case ERROR_BAD_NETPATH:
      return _cel_StatusCode_FromPosix(ENOENT);
    case ERROR_DEV_NOT_EXIST:
      return _cel_StatusCode_FromPosix(ENODEV);
    case ERROR_BAD_NET_NAME:
      return _cel_StatusCode_FromPosix(ENOENT);
    case ERROR_FILE_EXISTS:
      return _cel_StatusCode_FromPosix(EEXIST);
    case ERROR_CANNOT_MAKE:
      return _cel_StatusCode_FromPosix(EACCES);
    case ERROR_INVALID_PARAMETER:
      return _cel_StatusCode_FromPosix(EINVAL);
    case ERROR_BROKEN_PIPE:
      return _cel_StatusCode_FromPosix(EPIPE);
    case ERROR_OPEN_FAILED:
      return _cel_StatusCode_FromPosix(EIO);
    case ERROR_BUFFER_OVERFLOW:
      return _cel_StatusCode_FromPosix(ENAMETOOLONG);
    case ERROR_DISK_FULL:
      return _cel_StatusCode_FromPosix(ENOSPC);
    case ERROR_SEM_TIMEOUT:
      return _cel_StatusCode_FromPosix(ETIMEDOUT);
    case ERROR_INVALID_NAME:
      return _cel_StatusCode_FromPosix(ENOENT);
    case ERROR_NEGATIVE_SEEK:
      return _cel_StatusCode_FromPosix(EINVAL);
    case ERROR_BUSY_DRIVE:
      return _cel_StatusCode_FromPosix(EBUSY);
    case ERROR_DIR_NOT_EMPTY:
      return _cel_StatusCode_FromPosix(ENOTEMPTY);
    case ERROR_BUSY:
      return _cel_StatusCode_FromPosix(EBUSY);
    case ERROR_ALREADY_EXISTS:
      return _cel_StatusCode_FromPosix(EEXIST);
    case ERROR_FILENAME_EXCED_RANGE:
      return _cel_StatusCode_FromPosix(ENAMETOOLONG);
    case ERROR_LOCKED:
      return _cel_StatusCode_FromPosix(ENOLCK);
    case WAIT_TIMEOUT:
      return _cel_StatusCode_FromPosix(ETIMEDOUT);
    case ERROR_DIRECTORY:
      return _cel_StatusCode_FromPosix(EINVAL);
    case ERROR_OPERATION_ABORTED:
      return _cel_StatusCode_FromPosix(ECANCELED);
    case ERROR_NOACCESS:
      return _cel_StatusCode_FromPosix(EACCES);
    case ERROR_CANTOPEN:
      return _cel_StatusCode_FromPosix(EIO);
    case ERROR_CANTREAD:
      return _cel_StatusCode_FromPosix(EIO);
    case ERROR_CANTWRITE:
      return _cel_StatusCode_FromPosix(EIO);
    case ERROR_RETRY:
      return _cel_StatusCode_FromPosix(EAGAIN);
    case ERROR_TIMEOUT:
      return _cel_StatusCode_FromPosix(ETIMEDOUT);
    case ERROR_OPEN_FILES:
      return _cel_StatusCode_FromPosix(EBUSY);
    case ERROR_DEVICE_IN_USE:
      return _cel_StatusCode_FromPosix(EBUSY);
    case ERROR_REPARSE_TAG_INVALID:
      return _cel_StatusCode_FromPosix(EINVAL);
    case WSAEINTR:
      return _cel_StatusCode_FromPosix(EINTR);
    case WSAEBADF:
      return _cel_StatusCode_FromPosix(EBADF);
    case WSAEACCES:
      return _cel_StatusCode_FromPosix(EACCES);
    case WSAEFAULT:
      return _cel_StatusCode_FromPosix(EFAULT);
    case WSAEINVAL:
      return _cel_StatusCode_FromPosix(EINVAL);
    case WSAEMFILE:
      return _cel_StatusCode_FromPosix(EMFILE);
    case WSAEWOULDBLOCK:
      return _cel_StatusCode_FromPosix(EWOULDBLOCK);
    case WSAEINPROGRESS:
      return _cel_StatusCode_FromPosix(EINPROGRESS);
    case WSAEALREADY:
      return _cel_StatusCode_FromPosix(EALREADY);
    case WSAENOTSOCK:
      return _cel_StatusCode_FromPosix(ENOTSOCK);
    case WSAEDESTADDRREQ:
      return _cel_StatusCode_FromPosix(EDESTADDRREQ);
    case WSAEMSGSIZE:
      return _cel_StatusCode_FromPosix(EMSGSIZE);
    case WSAEPROTOTYPE:
      return _cel_StatusCode_FromPosix(EPROTOTYPE);
    case WSAENOPROTOOPT:
      return _cel_StatusCode_FromPosix(ENOPROTOOPT);
    case WSAEPROTONOSUPPORT:
      return _cel_StatusCode_FromPosix(EPROTONOSUPPORT);
    case WSAEOPNOTSUPP:
      return _cel_StatusCode_FromPosix(EOPNOTSUPP);
    case WSAEAFNOSUPPORT:
      return _cel_StatusCode_FromPosix(EAFNOSUPPORT);
    case WSAEADDRINUSE:
      return _cel_StatusCode_FromPosix(EADDRINUSE);
    case WSAEADDRNOTAVAIL:
      return _cel_StatusCode_FromPosix(EADDRNOTAVAIL);
    case WSAENETDOWN:
      return _cel_StatusCode_FromPosix(ENETDOWN);
    case WSAENETUNREACH:
      return _cel_StatusCode_FromPosix(ENETUNREACH);
    case WSAENETRESET:
      return _cel_StatusCode_FromPosix(ENETRESET);
    case WSAECONNABORTED:
      return _cel_StatusCode_FromPosix(ECONNABORTED);
    case WSAECONNRESET:
      return _cel_StatusCode_FromPosix(ECONNRESET);
    case WSAENOBUFS:
      return _cel_StatusCode_FromPosix(ENOBUFS);
    case WSAEISCONN:
      return _cel_StatusCode_FromPosix(EISCONN);
    case WSAENOTCONN:
      return _cel_StatusCode_FromPosix(ENOTCONN);
    case WSAETIMEDOUT:
      return _cel_StatusCode_FromPosix(ETIMEDOUT);
    case WSAECONNREFUSED:
      return _cel_StatusCode_FromPosix(ECONNREFUSED);
    case WSAENAMETOOLONG:
      return _cel_StatusCode_FromPosix(ENAMETOOLONG);
    case WSAEHOSTUNREACH:
      return _cel_StatusCode_FromPosix(EHOSTUNREACH);
    default:
      return cel_StatusCode_kUnknown;
  }
#endif
}

static int _cel_SystemErrorSpace_Message(CEL_NONNULL(const cel_ErrorSpace*)
                                             space,
                                         int code, CEL_NONNULL(char*) buf,
                                         size_t buflen) {
  CEL_ASSERT_EQ(space, cel_SystemErrorSpace);
  CEL_ASSERT_NOT_NULL(buf);
  CEL_ASSERT_GT(buflen, 0);

#ifndef _WIN32
  return cel_ErrorSpace_Message(cel_GenericErrorSpace, code, buf, buflen);
#else
  if (CEL_UNLIKELY(buflen == 0)) {
    return 0;
  }
  if (code == 0) {
    buf[0] = '\0';
    return 0;
  }
  DWORD ret = FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, cel_nullptr,
      (DWORD)(unsigned int)code, 0, buf, (DWORD)buflen, cel_nullptr);
  if (ret == 0) {
    buf[0] = '\0';
    return (int)GetLastError();
  }
  CEL_ASSERT_LE(ret, buflen);
  if (ret == buflen) {
    --ret;
  }
  buf[ret] = '\0';
  return 0;
#endif
}

static const cel_ErrorSpaceVTable _cel_SystemErrorSpaceVTable = {
    .name = CEL_CSTRINGVIEW_C("system"),
    .OutOfMemory = _cel_SystemErrorSpace_OutOfMemory,
    .Canonical = _cel_SystemErrorSpace_Canonical,
    .Message = _cel_SystemErrorSpace_Message,
};

static const cel_ErrorSpace _cel_SystemErrorSpace = {
    .vtable = &_cel_SystemErrorSpaceVTable,
};

CEL_NONNULL(const cel_ErrorSpace*)
const cel_SystemErrorSpace = &_cel_SystemErrorSpace;
