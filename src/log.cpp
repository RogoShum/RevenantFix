#include "log.hpp"

namespace revenantfix::internal {

namespace {

bool IsHighFrequencyDiagnostic(const char* format) {
  if (format == nullptr) {
    return false;
  }
  constexpr const char* kPrefixes[] = {
      "BHRC_STATE_TRACE", "BHRC_EFFECT_TRACE", "BHRC_RUNE_TRACE",
      "BHRC_NAME_TRACE", "BHRC_PGD_", "BHRC_RETURN_TRACE",
      "BHRC expedition_probe_", "BHRC record_roster_",
      "BHRC transport_identity_", "BHRC active_friend_admission",
      "startup_title_search", "startup_host_preflight", "record_cache_",
      "identity_", "branch_tree"};
  for (const char* prefix : kPrefixes) {
    if (std::strncmp(format, prefix, std::strlen(prefix)) == 0) {
      return true;
    }
  }
  return false;
}

}  // namespace

// Original src/dllmain.cpp:4495
void LogRaw(const char* text, bool flush_immediately) {
  if (text == nullptr) {
    return;
  }

  OutputDebugStringA(text);

  if (g_log_lock_initialized) {
    EnterCriticalSection(&g_log_lock);
  }

  HANDLE log = g_log;
  if (log != INVALID_HANDLE_VALUE) {
    static ULONGLONG s_last_flush_ms = 0;
    const char* cursor = text;
    size_t remaining = std::strlen(text);
    while (remaining > 0) {
      const DWORD chunk =
          remaining > 0x7ffff000 ? 0x7ffff000 : static_cast<DWORD>(remaining);
      DWORD written = 0;
      if (!WriteFile(log, cursor, chunk, &written, nullptr) || written == 0) {
        break;
      }
      cursor += written;
      remaining -= written;
    }
    const ULONGLONG now = GetTickCount64();
    if (flush_immediately || s_last_flush_ms == 0 ||
        now - s_last_flush_ms >= kLogFlushIntervalMs) {
      FlushFileBuffers(log);
      s_last_flush_ms = now;
    }
  }

  if (g_log_lock_initialized) {
    LeaveCriticalSection(&g_log_lock);
  }
}


// Original src/dllmain.cpp:4540
const char* LogLevelName(LogLevel level) {
  switch (level) {
    case LogLevel::Info:
      return "INFO ";
    case LogLevel::Warn:
      return "WARN ";
    case LogLevel::Error:
      return "ERROR";
  }
  return "INFO ";
}


// Original src/dllmain.cpp:4552
void VLog(LogLevel level, const char* format, va_list args) {
  if (level == LogLevel::Info && !g_config.show_detailed_logs &&
      IsHighFrequencyDiagnostic(format)) {
    return;
  }
  char buffer[4096]{};

  SYSTEMTIME time{};
  GetLocalTime(&time);
  const int prefix_len = std::snprintf(
      buffer,
      sizeof(buffer),
      "%04u-%02u-%02u %02u:%02u:%02u.%03u [%s] [tid=%lu] ",
      time.wYear,
      time.wMonth,
      time.wDay,
      time.wHour,
      time.wMinute,
      time.wSecond,
      time.wMilliseconds,
      LogLevelName(level),
      GetCurrentThreadId());

  if (prefix_len < 0 || prefix_len >= static_cast<int>(sizeof(buffer))) {
    return;
  }

  const int body_capacity = static_cast<int>(sizeof(buffer)) - prefix_len;
  const int body_len = std::vsnprintf(buffer + prefix_len, body_capacity, format, args);

  if (body_len < 0) {
    strcpy_s(buffer + prefix_len, sizeof(buffer) - prefix_len, "<format_error>");
  } else if (body_len >= body_capacity) {
    strncat_s(buffer, " <truncated>", _TRUNCATE);
  }

  strncat_s(buffer, "\r\n", _TRUNCATE);
  LogRaw(buffer, level == LogLevel::Error);
}


// Original src/dllmain.cpp:4588
void Log(const char* format, ...) {
  va_list args;
  va_start(args, format);
  VLog(LogLevel::Info, format, args);
  va_end(args);
}


// Original src/dllmain.cpp:4595
void LogWarn(const char* format, ...) {
  va_list args;
  va_start(args, format);
  VLog(LogLevel::Warn, format, args);
  va_end(args);
}


// Original src/dllmain.cpp:4602
void LogError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  VLog(LogLevel::Error, format, args);
  va_end(args);
}


// Original src/dllmain.cpp:4609
const char* SehExceptionName(DWORD code) {
  switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
      return "EXCEPTION_ACCESS_VIOLATION";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_BREAKPOINT:
      return "EXCEPTION_BREAKPOINT";
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      return "EXCEPTION_DATATYPE_MISALIGNMENT";
    case EXCEPTION_FLT_DENORMAL_OPERAND:
      return "EXCEPTION_FLT_DENORMAL_OPERAND";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_FLT_INEXACT_RESULT:
      return "EXCEPTION_FLT_INEXACT_RESULT";
    case EXCEPTION_FLT_INVALID_OPERATION:
      return "EXCEPTION_FLT_INVALID_OPERATION";
    case EXCEPTION_FLT_OVERFLOW:
      return "EXCEPTION_FLT_OVERFLOW";
    case EXCEPTION_FLT_STACK_CHECK:
      return "EXCEPTION_FLT_STACK_CHECK";
    case EXCEPTION_FLT_UNDERFLOW:
      return "EXCEPTION_FLT_UNDERFLOW";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      return "EXCEPTION_ILLEGAL_INSTRUCTION";
    case EXCEPTION_IN_PAGE_ERROR:
      return "EXCEPTION_IN_PAGE_ERROR";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      return "EXCEPTION_INT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_OVERFLOW:
      return "EXCEPTION_INT_OVERFLOW";
    case EXCEPTION_INVALID_DISPOSITION:
      return "EXCEPTION_INVALID_DISPOSITION";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
    case EXCEPTION_PRIV_INSTRUCTION:
      return "EXCEPTION_PRIV_INSTRUCTION";
    case EXCEPTION_SINGLE_STEP:
      return "EXCEPTION_SINGLE_STEP";
    case EXCEPTION_STACK_OVERFLOW:
      return "EXCEPTION_STACK_OVERFLOW";
  }
  return "UNKNOWN_SEH_EXCEPTION";
}


// Original src/dllmain.cpp:4655
const char* AccessViolationOperationName(ULONG_PTR operation) {
  switch (operation) {
    case 0:
      return "read";
    case 1:
      return "write";
    case 8:
      return "execute";
  }
  return "unknown";
}


// Original src/dllmain.cpp:4667
int LogSehException(const char* where, EXCEPTION_POINTERS* info) {
  const char* label = where != nullptr ? where : "<unknown>";
  if (info == nullptr || info->ExceptionRecord == nullptr) {
    LogError("%s exception details_unavailable", label);
    return EXCEPTION_EXECUTE_HANDLER;
  }

  const EXCEPTION_RECORD* record = info->ExceptionRecord;
  LogError(
      "%s exception code=0x%08lx name=%s address=%p flags=0x%08lx params=%lu",
      label,
      record->ExceptionCode,
      SehExceptionName(record->ExceptionCode),
      record->ExceptionAddress,
      record->ExceptionFlags,
      record->NumberParameters);

  if ((record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
       record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) &&
      record->NumberParameters >= 2) {
    LogError(
        "%s memory_fault operation=%llu(%s) target=%p",
        label,
        static_cast<unsigned long long>(record->ExceptionInformation[0]),
        AccessViolationOperationName(record->ExceptionInformation[0]),
        reinterpret_cast<void*>(record->ExceptionInformation[1]));
  }

#if defined(_M_X64)
  if (info->ContextRecord != nullptr) {
    const CONTEXT* ctx = info->ContextRecord;
    LogError(
        "%s context rip=%p rsp=%p rbp=%p rax=%p rbx=%p rcx=%p rdx=%p r8=%p r9=%p r10=%p r11=%p",
        label,
        reinterpret_cast<void*>(ctx->Rip),
        reinterpret_cast<void*>(ctx->Rsp),
        reinterpret_cast<void*>(ctx->Rbp),
        reinterpret_cast<void*>(ctx->Rax),
        reinterpret_cast<void*>(ctx->Rbx),
        reinterpret_cast<void*>(ctx->Rcx),
        reinterpret_cast<void*>(ctx->Rdx),
        reinterpret_cast<void*>(ctx->R8),
        reinterpret_cast<void*>(ctx->R9),
        reinterpret_cast<void*>(ctx->R10),
        reinterpret_cast<void*>(ctx->R11));
  }
#endif

  return EXCEPTION_EXECUTE_HANDLER;
}


// Original src/dllmain.cpp:4718
void OpenLog() {
  wchar_t dll_path[MAX_PATH]{};
  GetModuleFileNameW(g_module, dll_path, MAX_PATH);

  wchar_t* slash = std::wcsrchr(dll_path, L'\\');
  if (slash != nullptr) {
    *(slash + 1) = L'\0';
  }

  wcscpy_s(g_dll_dir, dll_path);
  wcscat_s(dll_path, L"RevenantFix.log");

  g_log = CreateFileW(
      dll_path,
      GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);
}


}  // namespace revenantfix::internal
