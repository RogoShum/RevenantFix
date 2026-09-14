#include "core.hpp"

namespace revenantfix::internal {

// Original src/dllmain.cpp:4114
uintptr_t RvaOf(const void* address) {
  const uintptr_t value = reinterpret_cast<uintptr_t>(address);
  if (g_game_base == 0 || value < g_game_base) {
    return 0;
  }
  return value - g_game_base;
}


// Original src/dllmain.cpp:4122
uint32_t ReadU32(const void* ptr, uint32_t fallback) {
  __try {
    return *static_cast<const uint32_t*>(ptr);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return fallback;
  }
}


// Original src/dllmain.cpp:4130
uint64_t ReadU64(const void* ptr, uint64_t fallback) {
  __try {
    return *static_cast<const uint64_t*>(ptr);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return fallback;
  }
}


// Original src/dllmain.cpp:4138
uint16_t ReadU16(const void* ptr, uint16_t fallback) {
  __try {
    return *static_cast<const uint16_t*>(ptr);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return fallback;
  }
}


// Original src/dllmain.cpp:4146
int32_t ReadI32(const void* ptr, int32_t fallback) {
  __try {
    return *static_cast<const int32_t*>(ptr);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return fallback;
  }
}


// Original src/dllmain.cpp:4154
uint8_t ReadU8(const void* ptr, uint8_t fallback) {
  __try {
    return *static_cast<const uint8_t*>(ptr);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return fallback;
  }
}


// Original src/dllmain.cpp:4162
bool TryReadU64(const void* ptr, uint64_t* out) {
  if (out == nullptr) {
    return false;
  }
  __try {
    *out = *static_cast<const uint64_t*>(ptr);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = 0;
    return false;
  }
}


// Original src/dllmain.cpp:4175
bool TryReadU32(const void* ptr, uint32_t* out) {
  if (out == nullptr) {
    return false;
  }
  __try {
    *out = *static_cast<const uint32_t*>(ptr);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = 0;
    return false;
  }
}


// Original src/dllmain.cpp:4188
bool TryCompareExchangeU32(
    void* ptr,
    uint32_t desired,
    uint32_t expected,
    uint32_t* observed) {
  if (ptr == nullptr || observed == nullptr) {
    return false;
  }
  __try {
    *observed = static_cast<uint32_t>(InterlockedCompareExchange(
        reinterpret_cast<volatile LONG*>(ptr),
        static_cast<LONG>(desired),
        static_cast<LONG>(expected)));
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *observed = UINT32_MAX;
    return false;
  }
}


// Original src/dllmain.cpp:4208
bool TryReadU8(const void* ptr, uint8_t* out) {
  if (out == nullptr) {
    return false;
  }
  __try {
    *out = *static_cast<const uint8_t*>(ptr);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = 0xff;
    return false;
  }
}


// Original src/dllmain.cpp:4221
bool WriteU8(void* ptr, uint8_t value) {
  __try {
    *static_cast<uint8_t*>(ptr) = value;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:4230
bool WriteU16(void* ptr, uint16_t value) {
  __try {
    *static_cast<uint16_t*>(ptr) = value;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:4239
bool WriteI32(void* ptr, int32_t value) {
  __try {
    *static_cast<int32_t*>(ptr) = value;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:4248
bool WriteU32(void* ptr, uint32_t value) {
  __try {
    *static_cast<uint32_t*>(ptr) = value;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:4257
bool WriteU64(void* ptr, uint64_t value) {
  __try {
    *static_cast<uint64_t*>(ptr) = value;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:4266
void FormatPartyMemberSlots(void* party_member_info, char* slot_text, size_t slot_text_size) {
  if (slot_text == nullptr || slot_text_size == 0) {
    return;
  }
  slot_text[0] = '\0';
  if (party_member_info == nullptr) {
    std::snprintf(slot_text, slot_text_size, "<null>");
    return;
  }

  size_t used = 0;
  for (int i = 0; i < kObservedPartyMemberSlots; ++i) {
    const uint8_t* base =
        static_cast<const uint8_t*>(party_member_info) + kPartyMemberSlotBaseOffset + i * kPartyMemberSlotStride;
    const int32_t player_id = ReadI32(base, -1);
    const int32_t mode = ReadI32(base + 8, -1);
    const int32_t state = ReadI32(base + 0xc, -1);
    const int32_t net_value = ReadI32(base + 0x10, -1);
    const uint8_t flag24 = ReadU8(base + 0x24, 0);
    const int written = std::snprintf(
        slot_text + used,
        slot_text_size - used,
        " [%d:id=%d mode=%d state=%d net=%d f24=%02x]",
        i,
        player_id,
        mode,
        state,
        net_value,
        flag24);
    if (written <= 0) {
      break;
    }
    const size_t remaining = slot_text_size - used;
    const size_t appended = static_cast<size_t>(written);
    used += appended < remaining ? appended : remaining;
    if (used >= slot_text_size) {
      slot_text[slot_text_size - 1] = '\0';
      break;
    }
  }
}


// Original src/dllmain.cpp:4308
void* GetGlobalPartyMemberInfo() {
  const uint64_t game_man = ReadU64(reinterpret_cast<const void*>(g_game_base + kGameManPtrRva), 0);
  if (game_man == 0) {
    return nullptr;
  }
  return reinterpret_cast<void*>(ReadU64(reinterpret_cast<const void*>(game_man + 0xdc8), 0));
}


// Original src/dllmain.cpp:4316
bool ReadPartyMemberSlot(void* party_member_info, int index, uint8_t* out_slot) {
  if (party_member_info == nullptr || out_slot == nullptr || index < 0 || index >= kObservedPartyMemberSlots) {
    return false;
  }
  __try {
    const uint8_t* slot =
        static_cast<const uint8_t*>(party_member_info) + kPartyMemberSlotBaseOffset + index * kPartyMemberSlotStride;
    std::memcpy(out_slot, slot, kPartyMemberSlotStride);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:4330
bool WritePartyMemberSlot(void* party_member_info, int index, const uint8_t* slot_data) {
  if (party_member_info == nullptr || slot_data == nullptr || index < 0 || index >= kObservedPartyMemberSlots) {
    return false;
  }
  __try {
    uint8_t* slot =
        static_cast<uint8_t*>(party_member_info) + kPartyMemberSlotBaseOffset + index * kPartyMemberSlotStride;
    std::memcpy(slot, slot_data, kPartyMemberSlotStride);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:4344
bool PartyMemberSlotHasPlayer(const uint8_t* slot_data) {
  return slot_data != nullptr && ReadI32(slot_data, -1) != -1;
}


// Original src/dllmain.cpp:4348
void NormalizePartyMemberInfoAfterRemove(
    void* party_member_info,
    uintptr_t caller_rva,
    uint64_t member_id,
    uint64_t reason,
    uint64_t result) {
  if (party_member_info == nullptr || result == 0) {
    return;
  }

  const int32_t old_count =
      ReadI32(static_cast<const uint8_t*>(party_member_info) + kPartyMemberCountOffset, -1);
  const int32_t old_native_count_1c =
      ReadI32(static_cast<const uint8_t*>(party_member_info) + kPartyMemberNativeCount1cOffset, -1);
  const int32_t old_native_count_20 =
      ReadI32(static_cast<const uint8_t*>(party_member_info) + kPartyMemberNativeCount20Offset, -1);
  if (old_count < 0 || old_count > kObservedPartyMemberSlots) {
    return;
  }

  uint8_t slots[kObservedPartyMemberSlots][kPartyMemberSlotStride]{};
  int valid_count = 0;
  bool saw_empty = false;
  bool has_slot_hole = false;
  bool read_ok = true;
  for (int i = 0; i < kObservedPartyMemberSlots; ++i) {
    if (!ReadPartyMemberSlot(party_member_info, i, slots[i])) {
      read_ok = false;
      break;
    }
    if (PartyMemberSlotHasPlayer(slots[i])) {
      has_slot_hole = has_slot_hole || saw_empty;
      ++valid_count;
    } else {
      saw_empty = true;
    }
  }
  if (!read_ok || valid_count > kObservedPartyMemberSlots) {
    return;
  }

  const bool needs_count_repair = old_count != valid_count;
  if (!has_slot_hole && !needs_count_repair) {
    // +0x1c/+0x20 are recomputed by the native session/update path. They may
    // legitimately lag this remove transaction and are observation-only here.
    if (old_native_count_1c != valid_count || old_native_count_20 != valid_count) {
      Log(
          "PartyMemberNormalize native_counts_untouched info=%p count=%d valid=%d nativeCount1c=%d nativeCount20=%d member=%016llx reason=%016llx result=%llu callerRva=%llx",
          party_member_info,
          old_count,
          valid_count,
          old_native_count_1c,
          old_native_count_20,
          static_cast<unsigned long long>(member_id),
          static_cast<unsigned long long>(reason),
          static_cast<unsigned long long>(result),
          static_cast<unsigned long long>(caller_rva));
    }
    return;
  }

  char before_slots[512]{};
  char after_slots[512]{};
  FormatPartyMemberSlots(party_member_info, before_slots, sizeof(before_slots));

  bool write_ok = true;
  if (has_slot_hole) {
    uint8_t compacted[kObservedPartyMemberSlots][kPartyMemberSlotStride]{};
    int destination = 0;
    for (int pass = 0; pass < 2; ++pass) {
      const bool want_player = pass == 0;
      for (int i = 0; i < kObservedPartyMemberSlots; ++i) {
        if (PartyMemberSlotHasPlayer(slots[i]) == want_player) {
          std::memcpy(compacted[destination], slots[i], kPartyMemberSlotStride);
          ++destination;
        }
      }
    }
    for (int i = 0; i < kObservedPartyMemberSlots; ++i) {
      if (!WritePartyMemberSlot(party_member_info, i, compacted[i])) {
        write_ok = false;
        break;
      }
    }
  }
  if (needs_count_repair) {
    write_ok = WriteI32(static_cast<uint8_t*>(party_member_info) + kPartyMemberCountOffset, valid_count) && write_ok;
  }

  FormatPartyMemberSlots(party_member_info, after_slots, sizeof(after_slots));
  if (write_ok) {
    Log(
        "PartyMemberNormalize physical_repair info=%p slotsCompacted=%d count=%d->%d nativeCount1c=%d untouched nativeCount20=%d untouched member=%016llx reason=%016llx result=%llu callerRva=%llx before=%s after=%s",
        party_member_info,
        has_slot_hole ? 1 : 0,
        old_count,
        valid_count,
        old_native_count_1c,
        old_native_count_20,
        static_cast<unsigned long long>(member_id),
        static_cast<unsigned long long>(reason),
        static_cast<unsigned long long>(result),
        static_cast<unsigned long long>(caller_rva),
        before_slots,
        after_slots);
  } else {
    LogWarn(
        "PartyMemberNormalize write_failed info=%p member=%016llx reason=%016llx result=%llu callerRva=%llx before=%s after=%s",
        party_member_info,
        static_cast<unsigned long long>(member_id),
        static_cast<unsigned long long>(reason),
        static_cast<unsigned long long>(result),
        static_cast<unsigned long long>(caller_rva),
        before_slots,
        after_slots);
  }
}


// Original src/dllmain.cpp:4466
uint8_t NormalizeBuddyPersistentFlags(uint8_t current_flags) {
  const uint8_t persistent = current_flags & kSlotFlagsPersistentMask;
  return persistent != 0 ? persistent : kSlotFlagsPersistentDefault;
}


// Original src/dllmain.cpp:4471
bool WriteSlotFlags(void* state, int buddy_slot, uint8_t flags) {
  if (state == nullptr || buddy_slot < 0 || buddy_slot >= kBuddySlotCount) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(state);
  const int offset = buddy_slot * kBuddySlotSize;
  return WriteU8(base + kSlotFlagsOffset + offset, flags);
}


// Original src/dllmain.cpp:4481
bool WriteBuddySlotControl(void* state, int buddy_slot, uint8_t state_byte, uint8_t pending, uint8_t flags) {
  if (state == nullptr || buddy_slot < 0 || buddy_slot >= kBuddySlotCount) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(state);
  const int offset = buddy_slot * kBuddySlotSize;
  bool ok = true;
  ok = WriteU8(base + kSlotStateOffset + offset, state_byte) && ok;
  ok = WriteU8(base + kSlotPendingOffset + offset, pending) && ok;
  ok = WriteU8(base + kSlotFlagsOffset + offset, flags) && ok;
  return ok;
}


// Original src/dllmain.cpp:4740
void BuildDllSidecarPath(const wchar_t* file_name, wchar_t* out_path, size_t out_count) {
  if (out_path == nullptr || out_count == 0) {
    return;
  }
  out_path[0] = L'\0';
  if (g_dll_dir[0] == L'\0' || file_name == nullptr) {
    return;
  }
  wcscpy_s(out_path, out_count, g_dll_dir);
  wcscat_s(out_path, out_count, file_name);
}


// Original src/dllmain.cpp:4752
void EnsureDefaultConfigFile(const wchar_t* config_path) {
  if (config_path == nullptr || config_path[0] == L'\0') {
    return;
  }

  HANDLE file = CreateFileW(
      config_path,
      GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr,
      CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    return;
  }

  constexpr char kDefaultConfig[] =
      "; RevenantFix configuration\r\n"
      "; Boolean values: 0 = off, 1 = on.\r\n"
      "\r\n"
      "[Fixes]\r\n"
      "; Remote owner gate fallback summon repair mode:\r\n"
      "; 0 = call original FUN_1403895e0 and FUN_1403897c0\r\n"
      "; 1 = manually advance 00/pending to 0a, then call original FUN_1403897c0\r\n"
      "; 2 = manually advance both 00->0a and 0a->0b\r\n"
      "remoteOwnerGateFallbackMode=0\r\n"
      "; Fix host-side reward acquire validation for 4+ player reward slots.\r\n"
      "fixRewardAcquire=1\r\n"
      "; Fix reward UI direct discard by spawning a normal synced map reward and disabling the original reward node.\r\n"
      "fixRewardDiscard=1\r\n"
      "; Expand local world-map/minimap/head-indicator remote-player marker paths from 2 remotes to 5 remotes.\r\n"
      "fixPlayerMapMarkers=1\r\n"
      "; Compact PartyMemberInfo after a player leaves so 4+ player lists do not leave gaps outside the UI scan range.\r\n"
      "fixPlayerListCompaction=1\r\n"
      "; Install NRSC peer discovery/control and RevenantFix SteamNetworkingMessages test packets.\r\n"
      "enableNrscSteamControl=1\r\n"
      "; Suppress the matched players already departed dialog. This does not start any join or recovery flow.\r\n"
      "enableMatchedPlayersUiSuppress=1\r\n"
      "\r\n"
      "[Debug]\r\n"
      "; Show high-frequency reconnect, identity, effect, rune and transport diagnostics.\r\n"
      "showDetailedLogs=0\r\n";
  DWORD written = 0;
  WriteFile(file, kDefaultConfig, static_cast<DWORD>(std::strlen(kDefaultConfig)), &written, nullptr);
  CloseHandle(file);
}


// Original src/dllmain.cpp:4796
void EnsureIniKeyDefault(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    const wchar_t* value) {
  wchar_t buffer[16]{};
  DWORD read = GetPrivateProfileStringW(
      section,
      key,
      L"",
      buffer,
      static_cast<DWORD>(sizeof(buffer) / sizeof(buffer[0])),
      config_path);
  if (read == 0) {
    WritePrivateProfileStringW(section, key, value, config_path);
  }
}


// Original src/dllmain.cpp:4814
void EnsureConfigDefaults(const wchar_t* config_path) {
  if (config_path == nullptr || config_path[0] == L'\0') {
    return;
  }
  EnsureIniKeyDefault(config_path, L"Fixes", L"remoteOwnerGateFallbackMode", L"0");
  EnsureIniKeyDefault(config_path, L"Fixes", L"fixRewardAcquire", L"1");
  EnsureIniKeyDefault(config_path, L"Fixes", L"fixRewardDiscard", L"1");
  EnsureIniKeyDefault(config_path, L"Fixes", L"fixPlayerMapMarkers", L"1");
  EnsureIniKeyDefault(config_path, L"Fixes", L"fixPlayerListCompaction", L"1");
  EnsureIniKeyDefault(config_path, L"Fixes", L"enableNrscSteamControl", L"1");
  EnsureIniKeyDefault(config_path, L"Fixes", L"enableMatchedPlayersUiSuppress", L"1");
  EnsureIniKeyDefault(config_path, L"Debug", L"showDetailedLogs", L"0");
  WritePrivateProfileStringW(L"Debug", L"debugBuddyToggleRemotePlayerDataFault", nullptr, config_path);
  WritePrivateProfileStringW(L"Debug", L"debugRemotePlayerFaultMode", nullptr, config_path);
  WritePrivateProfileStringW(L"Debug", L"debugRemotePlayerDataFaultHoldMs", nullptr, config_path);
}


// Original src/dllmain.cpp:4830
void LoadConfig() {
  wchar_t config_path[MAX_PATH]{};
  BuildDllSidecarPath(L"RevenantFix.ini", config_path, MAX_PATH);
  EnsureDefaultConfigFile(config_path);
  EnsureConfigDefaults(config_path);

  g_config.fix_reward_acquire =
      GetPrivateProfileIntW(
          L"Fixes",
          L"fixRewardAcquire",
          1,
          config_path) != 0;
  g_config.fix_reward_discard =
      GetPrivateProfileIntW(
          L"Fixes",
          L"fixRewardDiscard",
          1,
          config_path) != 0;
  g_config.fix_player_map_markers =
      GetPrivateProfileIntW(
          L"Fixes",
          L"fixPlayerMapMarkers",
          1,
          config_path) != 0;
  g_config.fix_player_list_compaction =
      GetPrivateProfileIntW(
          L"Fixes",
          L"fixPlayerListCompaction",
          1,
          config_path) != 0;
  g_config.enable_nrsc_steam_control =
      GetPrivateProfileIntW(
          L"Fixes",
          L"enableNrscSteamControl",
          1,
          config_path) != 0;
  g_config.enable_nrsc_matched_players_ui_suppress =
      GetPrivateProfileIntW(
          L"Fixes",
          L"enableMatchedPlayersUiSuppress",
          1,
          config_path) != 0;
  g_config.remote_owner_gate_fallback_mode =
      GetPrivateProfileIntW(
          L"Fixes",
          L"remoteOwnerGateFallbackMode",
          kRemoteOwnerGateFallbackModeOriginal,
          config_path);
  if (g_config.remote_owner_gate_fallback_mode < kRemoteOwnerGateFallbackModeOriginal ||
      g_config.remote_owner_gate_fallback_mode > kRemoteOwnerGateFallbackModeManualAll) {
    g_config.remote_owner_gate_fallback_mode = kRemoteOwnerGateFallbackModeOriginal;
  }
  g_config.hold_authorized_expedition_b_leave =
      GetPrivateProfileIntW(
          L"Fixes",
          L"holdAuthorizedExpeditionBLeave",
          1,
          config_path) != 0;
  g_config.show_detailed_logs =
      GetPrivateProfileIntW(
          L"Debug",
          L"showDetailedLogs",
          0,
          config_path) != 0;
}


// Original src/dllmain.cpp:7762
bool WriteCodeByte(void* target, uint8_t value, const char* name) {
  DWORD old_protect = 0;
  if (!VirtualProtect(target, 1, PAGE_EXECUTE_READWRITE, &old_protect)) {
    LogError(
        "%s failed reason=virtual_protect target=%p error=%lu",
        name,
        target,
        GetLastError());
    return false;
  }

  const bool wrote = WriteU8(target, value);
  FlushInstructionCache(GetCurrentProcess(), target, 1);

  DWORD ignored = 0;
  VirtualProtect(target, 1, old_protect, &ignored);
  if (!wrote) {
    LogError("%s failed reason=write_exception target=%p value=%02x", name, target, value);
  }
  return wrote;
}


// Original src/dllmain.cpp:7784
bool WriteCodeBytes(void* target, const uint8_t* bytes, size_t size, const char* name) {
  if (target == nullptr || bytes == nullptr || size == 0 || size > 16) {
    LogError("%s failed reason=invalid_args target=%p size=%llu", name, target, static_cast<unsigned long long>(size));
    return false;
  }

  DWORD old_protect = 0;
  if (!VirtualProtect(target, size, PAGE_EXECUTE_READWRITE, &old_protect)) {
    LogError(
        "%s failed reason=virtual_protect target=%p size=%llu error=%lu",
        name,
        target,
        static_cast<unsigned long long>(size),
        GetLastError());
    return false;
  }

  bool wrote = true;
  __try {
    std::memcpy(target, bytes, size);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    wrote = false;
  }
  FlushInstructionCache(GetCurrentProcess(), target, size);

  DWORD ignored = 0;
  VirtualProtect(target, size, old_protect, &ignored);
  if (!wrote) {
    LogError("%s failed reason=write_exception target=%p size=%llu", name, target, static_cast<unsigned long long>(size));
  }
  return wrote;
}


// Original src/dllmain.cpp:7817
bool ReadCodeBytes(void* target, uint8_t* out, size_t size) {
  if (target == nullptr || out == nullptr || size == 0 || size > 16) {
    return false;
  }
  __try {
    std::memcpy(out, target, size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:7829
void FormatBytes(const uint8_t* bytes, size_t size, char* out, size_t out_size) {
  if (out == nullptr || out_size == 0) {
    return;
  }
  out[0] = '\0';
  if (bytes == nullptr) {
    return;
  }
  size_t used = 0;
  for (size_t i = 0; i < size && used + 4 < out_size; ++i) {
    const int written = std::snprintf(out + used, out_size - used, "%s%02x", i == 0 ? "" : " ", bytes[i]);
    if (written <= 0) {
      break;
    }
    used += static_cast<size_t>(written);
  }
}


// Original src/dllmain.cpp:7847
bool InstallCodePatch(
    CodePatch& patch,
    uintptr_t rva,
    const uint8_t* expected,
    const uint8_t* replacement,
    size_t size,
    const char* name,
    bool allow_call_overwrite,
    const uint8_t* alternate_expected) {
  if (g_game_base == 0 || expected == nullptr || replacement == nullptr || size == 0 || size > 16) {
    LogError("%s failed reason=invalid_args rva=%llx size=%llu", name, static_cast<unsigned long long>(rva), static_cast<unsigned long long>(size));
    return false;
  }

  auto* target = reinterpret_cast<void*>(g_game_base + rva);
  uint8_t current[16]{};
  if (!ReadCodeBytes(target, current, size)) {
    LogError("%s failed reason=read_exception target=%p rva=%llx size=%llu", name, target, static_cast<unsigned long long>(rva), static_cast<unsigned long long>(size));
    return false;
  }

  if (std::memcmp(current, replacement, size) == 0) {
    patch.target = target;
    std::memcpy(patch.original, expected, size);
    std::memcpy(patch.replacement, replacement, size);
    patch.size = size;
    patch.installed = true;
    patch.name = name;
    Log("%s already target=%p rva=%llx size=%llu", name, target, static_cast<unsigned long long>(rva), static_cast<unsigned long long>(size));
    return true;
  }

  if (std::memcmp(current, expected, size) != 0) {
    char current_text[64]{};
    char expected_text[64]{};
    FormatBytes(current, size, current_text, sizeof(current_text));
    FormatBytes(expected, size, expected_text, sizeof(expected_text));
    if (alternate_expected != nullptr && std::memcmp(current, alternate_expected, size) == 0) {
      char alternate_text[64]{};
      char replacement_text[64]{};
      FormatBytes(alternate_expected, size, alternate_text, sizeof(alternate_text));
      FormatBytes(replacement, size, replacement_text, sizeof(replacement_text));
      LogWarn(
          "%s accepting_alternate target=%p rva=%llx current=%s expected=%s new=%s",
          name,
          target,
          static_cast<unsigned long long>(rva),
          alternate_text,
          expected_text,
          replacement_text);
      if (!WriteCodeBytes(target, replacement, size, name)) {
        return false;
      }
      patch.target = target;
      std::memcpy(patch.original, current, size);
      std::memcpy(patch.replacement, replacement, size);
      patch.size = size;
      patch.installed = true;
      patch.name = name;
      return true;
    }
    if (allow_call_overwrite && size == 5 && current[0] == 0xe8 && replacement[0] == 0xb8) {
      char replacement_text[64]{};
      FormatBytes(replacement, size, replacement_text, sizeof(replacement_text));
      LogWarn(
          "%s overwriting existing call patch target=%p rva=%llx current=%s expected=%s new=%s",
          name,
          target,
          static_cast<unsigned long long>(rva),
          current_text,
          expected_text,
          replacement_text);
      if (!WriteCodeBytes(target, replacement, size, name)) {
        return false;
      }
      patch.target = target;
      std::memcpy(patch.original, current, size);
      std::memcpy(patch.replacement, replacement, size);
      patch.size = size;
      patch.installed = true;
      patch.name = name;
      return true;
    }
    LogError(
        "%s failed reason=unexpected_bytes target=%p rva=%llx current=%s expected=%s",
        name,
        target,
        static_cast<unsigned long long>(rva),
        current_text,
        expected_text);
    return false;
  }

  if (!WriteCodeBytes(target, replacement, size, name)) {
    return false;
  }

  patch.target = target;
  std::memcpy(patch.original, expected, size);
  std::memcpy(patch.replacement, replacement, size);
  patch.size = size;
  patch.installed = true;
  patch.name = name;

  char expected_text[64]{};
  char replacement_text[64]{};
  FormatBytes(expected, size, expected_text, sizeof(expected_text));
  FormatBytes(replacement, size, replacement_text, sizeof(replacement_text));
  Log(
      "%s ok target=%p rva=%llx old=%s new=%s",
      name,
      target,
      static_cast<unsigned long long>(rva),
      expected_text,
      replacement_text);
  return true;
}


// Original src/dllmain.cpp:7965
void RemoveCodePatch(CodePatch& patch) {
  if (!patch.installed || patch.target == nullptr || patch.size == 0) {
    return;
  }
  if (WriteCodeBytes(patch.target, patch.original, patch.size, patch.name != nullptr ? patch.name : "CodePatch")) {
    char original_text[64]{};
    FormatBytes(patch.original, patch.size, original_text, sizeof(original_text));
    Log(
        "CodePatch removed %s target=%p restored=%s",
        patch.name != nullptr ? patch.name : "<unnamed>",
        patch.target,
        original_text);
  }
  patch = CodePatch{};
}


// Original src/dllmain.cpp:7990
bool CopyBytesSafe(void* dst, const void* src, size_t size) {
  if (dst == nullptr || src == nullptr || size == 0) {
    return false;
  }
  __try {
    std::memcpy(dst, src, size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:8002
bool ZeroBytesSafe(void* dst, size_t size) {
  if (dst == nullptr || size == 0) {
    return false;
  }
  __try {
    std::memset(dst, 0, size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}


// Original src/dllmain.cpp:8544
void RemoveBytePatch(BytePatch& patch) {
  if (!patch.installed || patch.target == nullptr) {
    return;
  }
  if (WriteCodeByte(patch.target, patch.original, patch.name != nullptr ? patch.name : "BytePatch")) {
    Log(
        "BytePatch removed %s target=%p restored=%02x replacement=%02x",
        patch.name != nullptr ? patch.name : "<unnamed>",
        patch.target,
        patch.original,
        patch.replacement);
  }
  patch = BytePatch{};
}


// Original src/dllmain.cpp:11644
uint64_t ParseIniHexU64(const wchar_t* text, uint64_t fallback) {
  if (text == nullptr) {
    return fallback;
  }

  const wchar_t* cursor = text;
  while (*cursor != L'\0' && std::iswspace(*cursor) != 0) {
    ++cursor;
  }
  if (*cursor == L'\0') {
    return fallback;
  }

  if (cursor[0] == L'0' && (cursor[1] == L'x' || cursor[1] == L'X')) {
    cursor += 2;
  }
  if (*cursor == L'\0') {
    return fallback;
  }

  wchar_t* end = nullptr;
  const uint64_t value = _wcstoui64(cursor, &end, 16);
  while (end != nullptr && *end != L'\0' && std::iswspace(*end) != 0) {
    ++end;
  }
  return end != cursor && end != nullptr && *end == L'\0' ? value : fallback;
}


// Original src/dllmain.cpp:11672
uint64_t ReadIniU64(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    uint64_t fallback) {
  wchar_t buffer[64]{};
  GetPrivateProfileStringW(
      section,
      key,
      L"",
      buffer,
      static_cast<DWORD>(sizeof(buffer) / sizeof(buffer[0])),
      config_path);
  return ParseIniHexU64(buffer, fallback);
}


// Original src/dllmain.cpp:11688
void WriteIniU64Hex(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    uint64_t value) {
  wchar_t buffer[32]{};
  swprintf_s(buffer, L"0x%016llx", static_cast<unsigned long long>(value));
  WritePrivateProfileStringW(section, key, buffer, config_path);
}


// Original src/dllmain.cpp:11711
bool WriteIniHexBytes(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    const uint8_t* bytes,
    size_t size) {
  if (config_path == nullptr || section == nullptr || key == nullptr ||
      bytes == nullptr || size == 0 ||
      size > kBhrcNativeDescriptorMaxPayloadSize) {
    return false;
  }
  wchar_t text[kBhrcNativeDescriptorMaxPayloadSize * 2 + 1]{};
  constexpr wchar_t kHex[] = L"0123456789abcdef";
  for (size_t i = 0; i < size; ++i) {
    text[i * 2] = kHex[(bytes[i] >> 4) & 0x0f];
    text[i * 2 + 1] = kHex[bytes[i] & 0x0f];
  }
  text[size * 2] = L'\0';
  return WritePrivateProfileStringW(section, key, text, config_path) != FALSE;
}


// Original src/dllmain.cpp:11732
bool ReadIniHexBytes(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    uint8_t* out,
    size_t size) {
  if (config_path == nullptr || section == nullptr || key == nullptr ||
      out == nullptr || size == 0 ||
      size > kBhrcNativeDescriptorMaxPayloadSize) {
    return false;
  }
  wchar_t text[kBhrcNativeDescriptorMaxPayloadSize * 2 + 1]{};
  const DWORD count = GetPrivateProfileStringW(
      section, key, L"", text, static_cast<DWORD>(_countof(text)),
      config_path);
  if (count != size * 2) {
    std::memset(out, 0, size);
    return false;
  }
  for (size_t i = 0; i < size; ++i) {
    const int high = BhrcHexNibble(text[i * 2]);
    const int low = BhrcHexNibble(text[i * 2 + 1]);
    if (high < 0 || low < 0) {
      std::memset(out, 0, size);
      return false;
    }
    out[i] = static_cast<uint8_t>((high << 4) | low);
  }
  return true;
}


// Original src/dllmain.cpp:11767
void WriteIniI32(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    int32_t value) {
  wchar_t buffer[32]{};
  swprintf_s(buffer, L"%d", value);
  WritePrivateProfileStringW(section, key, buffer, config_path);
}


// Original src/dllmain.cpp:11777
int32_t ReadIniI32(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    int32_t fallback) {
  wchar_t buffer[64]{};
  GetPrivateProfileStringW(
      section,
      key,
      L"",
      buffer,
      static_cast<DWORD>(sizeof(buffer) / sizeof(buffer[0])),
      config_path);
  if (buffer[0] == L'\0') {
    return fallback;
  }
  const wchar_t* cursor = buffer;
  while (*cursor != L'\0' && std::iswspace(*cursor) != 0) {
    ++cursor;
  }
  wchar_t* end = nullptr;
  const long value = std::wcstol(cursor, &end, 10);
  while (end != nullptr && *end != L'\0' && std::iswspace(*end) != 0) {
    ++end;
  }
  return end != cursor && end != nullptr && *end == L'\0' ? static_cast<int32_t>(value) : fallback;
}


// Original src/dllmain.cpp:11846
void* ReadGameGlobalPtr(uintptr_t rva) {
  if (g_game_base == 0 || rva == 0) {
    return nullptr;
  }
  return reinterpret_cast<void*>(ReadU64(reinterpret_cast<const void*>(g_game_base + rva), 0));
}


// Original src/dllmain.cpp:44533
void WriteAbsoluteJump(uint8_t* address, void* destination) {
  // jmp qword ptr [rip+0]; dq destination
  address[0] = 0xFF;
  address[1] = 0x25;
  *reinterpret_cast<uint32_t*>(address + 2) = 0;
  *reinterpret_cast<uint64_t*>(address + 6) = reinterpret_cast<uint64_t>(destination);
}


// Original src/dllmain.cpp:44541
void WriteAbsoluteJumpViaRax(uint8_t* address, void* destination) {
  // mov rax, destination; jmp rax
  address[0] = 0x48;
  address[1] = 0xB8;
  *reinterpret_cast<uint64_t*>(address + 2) = reinterpret_cast<uint64_t>(destination);
  address[10] = 0xFF;
  address[11] = 0xE0;
}


// Original src/dllmain.cpp:44550
bool InstallInlineHook(
    InlineHook& hook,
    uintptr_t target,
    void* detour,
    size_t patch_size,
    void** original,
    const char* name) {
  if (hook.target != nullptr || hook.detour != nullptr ||
      hook.trampoline != nullptr || hook.patch_size != 0) {
    LogError(
        "InstallInlineHook occupied slot rejected name=%s existingTarget=%p existingDetour=%p existingTrampoline=%p existingPatchSize=%llu",
        name,
        hook.target,
        hook.detour,
        hook.trampoline,
        static_cast<unsigned long long>(hook.patch_size));
    return false;
  }
  if (target == 0 || detour == nullptr) {
    LogError(
        "InstallInlineHook invalid target name=%s target=%p detour=%p",
        name,
        reinterpret_cast<void*>(target),
        detour);
    return false;
  }
  if (patch_size < 12 || patch_size > sizeof(hook.original) ||
      patch_size == 13) {
    LogError("InstallInlineHook invalid patch size name=%s size=%llu", name, patch_size);
    return false;
  }

  hook.target = reinterpret_cast<void*>(target);
  hook.detour = detour;
  hook.patch_size = patch_size;
  std::memcpy(hook.original, hook.target, patch_size);
  auto reset_failed_install = [&]() {
    if (hook.trampoline != nullptr) {
      VirtualFree(hook.trampoline, 0, MEM_RELEASE);
    }
    hook = InlineHook{};
    if (original != nullptr) {
      *original = nullptr;
    }
  };

  if (original != nullptr) {
    const bool ends_with_rel_jcc = patch_size >= 6 &&
        hook.original[patch_size - 6] == 0x0f &&
        (hook.original[patch_size - 5] & 0xf0) == 0x80;
    const bool ends_with_rel_jmp =
        patch_size >= 5 && hook.original[patch_size - 5] == 0xE9;
    const size_t trampoline_capacity = patch_size + 32;
    auto* trampoline = static_cast<uint8_t*>(
        VirtualAlloc(nullptr, trampoline_capacity, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (trampoline == nullptr) {
      LogError("VirtualAlloc trampoline failed name=%s error=%lu", name, GetLastError());
      reset_failed_install();
      return false;
    }
    hook.trampoline = trampoline;

    if (ends_with_rel_jcc) {
      const size_t rel_jcc_offset = patch_size - 6;
      const int32_t rel = *reinterpret_cast<const int32_t*>(
          hook.original + rel_jcc_offset + 2);
      auto* target_rel_next =
          static_cast<uint8_t*>(hook.target) + patch_size;
      void* rel_destination = target_rel_next + rel;
      std::memcpy(trampoline, hook.original, rel_jcc_offset);
      // Invert the original condition and skip the absolute taken-branch jump.
      // The following absolute jump then preserves the original fall-through.
      trampoline[rel_jcc_offset] = 0x0f;
      trampoline[rel_jcc_offset + 1] =
          hook.original[rel_jcc_offset + 1] ^ 0x01;
      *reinterpret_cast<int32_t*>(trampoline + rel_jcc_offset + 2) = 14;
      WriteAbsoluteJump(
          trampoline + rel_jcc_offset + 6, rel_destination);
      WriteAbsoluteJump(
          trampoline + rel_jcc_offset + 20,
          static_cast<uint8_t*>(hook.target) + patch_size);
      Log(
          "InstallInlineHook relocated terminal rel32 condition name=%s relOffset=%llu destination=%p fallthrough=%p",
          name,
          static_cast<unsigned long long>(rel_jcc_offset),
          rel_destination,
          static_cast<uint8_t*>(hook.target) + patch_size);
    } else if (ends_with_rel_jmp) {
      const size_t rel_jmp_offset = patch_size - 5;
      const int32_t rel = *reinterpret_cast<const int32_t*>(hook.original + rel_jmp_offset + 1);
      auto* target_rel_next = static_cast<uint8_t*>(hook.target) + rel_jmp_offset + 5;
      void* rel_destination = target_rel_next + rel;
      std::memcpy(trampoline, hook.original, rel_jmp_offset);
      WriteAbsoluteJump(trampoline + rel_jmp_offset, rel_destination);
      Log(
          "InstallInlineHook relocated terminal rel32 jump name=%s relOffset=%llu destination=%p",
          name,
          static_cast<unsigned long long>(rel_jmp_offset),
          rel_destination);
    } else {
      // Expand interior RIP-relative lea/mov into absolute forms. Trampoline
      // pages can be >2GB from nightreign.exe, so rewriting only the disp32
      // fails. Forms handled:
      //   48 8B 05 disp32  -> mov rax,imm64 ; mov rax,[rax]
      //   48 8B 0D disp32  -> mov rcx,imm64 ; mov rcx,[rcx]
      //   48 8D 0D disp32  -> mov rcx,imm64
      size_t src = 0;
      size_t dst = 0;
      bool expanded = false;
      while (src < patch_size) {
        const bool is_lea_rcx_rip = src + 7 <= patch_size &&
            hook.original[src] == 0x48 && hook.original[src + 1] == 0x8d &&
            hook.original[src + 2] == 0x0d;
        const bool is_mov_rax_rip = src + 7 <= patch_size &&
            hook.original[src] == 0x48 && hook.original[src + 1] == 0x8b &&
            hook.original[src + 2] == 0x05;
        const bool is_mov_rcx_rip = src + 7 <= patch_size &&
            hook.original[src] == 0x48 && hook.original[src + 1] == 0x8b &&
            hook.original[src + 2] == 0x0d;
        if (is_lea_rcx_rip || is_mov_rax_rip || is_mov_rcx_rip) {
          const int32_t rel =
              *reinterpret_cast<const int32_t*>(hook.original + src + 3);
          auto* old_next = static_cast<uint8_t*>(hook.target) + src + 7;
          const uint64_t absolute =
              reinterpret_cast<uint64_t>(old_next) +
              static_cast<int64_t>(rel);
          // mov reg, imm64 is 10 bytes; the load forms add 3 bytes.
          const size_t need =
              (is_mov_rax_rip || is_mov_rcx_rip) ? 13u : 10u;
          if (dst + need + 14 > trampoline_capacity) {
            LogError(
                "InstallInlineHook trampoline overflow name=%s src=%llu",
                name, static_cast<unsigned long long>(src));
            reset_failed_install();
            return false;
          }
          if (is_lea_rcx_rip || is_mov_rcx_rip) {
            trampoline[dst + 0] = 0x48;
            trampoline[dst + 1] = 0xb9;  // mov rcx, imm64
            *reinterpret_cast<uint64_t*>(trampoline + dst + 2) = absolute;
            dst += 10;
            if (is_mov_rcx_rip) {
              trampoline[dst + 0] = 0x48;
              trampoline[dst + 1] = 0x8b;
              trampoline[dst + 2] = 0x09;  // mov rcx, [rcx]
              dst += 3;
            }
          } else {
            trampoline[dst + 0] = 0x48;
            trampoline[dst + 1] = 0xb8;  // mov rax, imm64
            *reinterpret_cast<uint64_t*>(trampoline + dst + 2) = absolute;
            trampoline[dst + 10] = 0x48;
            trampoline[dst + 11] = 0x8b;
            trampoline[dst + 12] = 0x00;  // mov rax, [rax]
            dst += 13;
          }
          Log(
              "InstallInlineHook expanded RIP-relative name=%s src=%llu kind=%s abs=%p",
              name, static_cast<unsigned long long>(src),
              is_lea_rcx_rip
                  ? "lea_rcx"
                  : (is_mov_rcx_rip ? "mov_rcx_load" : "mov_rax_load"),
              reinterpret_cast<void*>(absolute));
          src += 7;
          expanded = true;
          continue;
        }
        if (dst + 1 + 14 > trampoline_capacity) {
          LogError(
              "InstallInlineHook trampoline overflow name=%s byteCopy", name);
          reset_failed_install();
          return false;
        }
        trampoline[dst++] = hook.original[src++];
      }
      WriteAbsoluteJump(
          trampoline + dst, static_cast<uint8_t*>(hook.target) + patch_size);
      if (!expanded) {
        // ordinary path; keep silence
      }
    }
    *original = trampoline;
  } else {
    hook.trampoline = nullptr;
  }

  DWORD old_protect = 0;
  if (!VirtualProtect(hook.target, patch_size, PAGE_EXECUTE_READWRITE, &old_protect)) {
    LogError("VirtualProtect target failed name=%s error=%lu", name, GetLastError());
    reset_failed_install();
    return false;
  }

  uint8_t patch[32]{};
  size_t jump_size = 0;
  if (patch_size >= 14) {
    WriteAbsoluteJump(patch, detour);
    jump_size = 14;
  } else if (patch_size == 12) {
    WriteAbsoluteJumpViaRax(patch, detour);
    jump_size = 12;
  } else {
    LogError("InstallInlineHook unsupported short patch size name=%s size=%llu", name, patch_size);
    DWORD ignored = 0;
    VirtualProtect(hook.target, patch_size, old_protect, &ignored);
    reset_failed_install();
    return false;
  }
  for (size_t i = jump_size; i < patch_size; ++i) {
    patch[i] = 0x90;
  }

  std::memcpy(hook.target, patch, patch_size);
  FlushInstructionCache(GetCurrentProcess(), hook.target, patch_size);

  DWORD ignored = 0;
  VirtualProtect(hook.target, patch_size, old_protect, &ignored);

  Log("Hooked %s target=%p trampoline=%p patchSize=%llu", name, hook.target, hook.trampoline, patch_size);
  return true;
}


// Original src/dllmain.cpp:44772
bool InstallInlineHookRelocatedCallAtOffset9(
    InlineHook& hook,
    uintptr_t target,
    void* detour,
    void** original,
    const char* name) {
  constexpr size_t kPatchSize = 16;
  constexpr size_t kCallOffset = 9;
  constexpr size_t kCallLength = 5;
  if (hook.target != nullptr || hook.detour != nullptr ||
      hook.trampoline != nullptr || hook.patch_size != 0) {
    LogError(
        "InstallInlineHookRelocatedCallAtOffset9 occupied slot rejected name=%s existingTarget=%p existingDetour=%p existingTrampoline=%p existingPatchSize=%llu",
        name,
        hook.target,
        hook.detour,
        hook.trampoline,
        static_cast<unsigned long long>(hook.patch_size));
    return false;
  }
  if (target == 0 || detour == nullptr) {
    LogError(
        "InstallInlineHookRelocatedCallAtOffset9 invalid target name=%s target=%p detour=%p",
        name,
        reinterpret_cast<void*>(target),
        detour);
    return false;
  }
  if (original == nullptr) {
    LogError("InstallInlineHookRelocatedCallAtOffset9 original missing name=%s", name);
    return false;
  }
  hook.target = reinterpret_cast<void*>(target);
  hook.detour = detour;
  hook.patch_size = kPatchSize;
  std::memcpy(hook.original, hook.target, kPatchSize);
  if (hook.original[kCallOffset] != 0xe8) {
    LogError(
        "InstallInlineHookRelocatedCallAtOffset9 opcode mismatch name=%s opcode=%02x",
        name, static_cast<unsigned>(hook.original[kCallOffset]));
    hook = InlineHook{};
    return false;
  }

  auto* trampoline = static_cast<uint8_t*>(VirtualAlloc(
      nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
  if (trampoline == nullptr) {
    LogError(
        "InstallInlineHookRelocatedCallAtOffset9 VirtualAlloc failed name=%s error=%lu",
        name, GetLastError());
    hook = InlineHook{};
    return false;
  }
  const int32_t relative = *reinterpret_cast<const int32_t*>(
      hook.original + kCallOffset + 1);
  auto* const relative_next = static_cast<uint8_t*>(hook.target) +
      kCallOffset + kCallLength;
  void* const call_destination = relative_next + relative;
  std::memcpy(trampoline, hook.original, kCallOffset);
  // call qword ptr [rip+2]; jmp +8; dq original call destination
  trampoline[9] = 0xff;
  trampoline[10] = 0x15;
  *reinterpret_cast<uint32_t*>(trampoline + 11) = 2;
  trampoline[15] = 0xeb;
  trampoline[16] = 0x08;
  *reinterpret_cast<uint64_t*>(trampoline + 17) =
      reinterpret_cast<uint64_t>(call_destination);
  std::memcpy(trampoline + 25, hook.original + 14, 2);
  WriteAbsoluteJump(
      trampoline + 27, static_cast<uint8_t*>(hook.target) + kPatchSize);
  hook.trampoline = trampoline;
  *original = trampoline;

  DWORD old_protect = 0;
  if (!VirtualProtect(
          hook.target, kPatchSize, PAGE_EXECUTE_READWRITE, &old_protect)) {
    LogError(
        "InstallInlineHookRelocatedCallAtOffset9 VirtualProtect failed name=%s error=%lu",
        name, GetLastError());
    VirtualFree(trampoline, 0, MEM_RELEASE);
    *original = nullptr;
    hook = InlineHook{};
    return false;
  }
  uint8_t patch[kPatchSize]{};
  WriteAbsoluteJump(patch, detour);
  patch[14] = 0x90;
  patch[15] = 0x90;
  std::memcpy(hook.target, patch, kPatchSize);
  FlushInstructionCache(GetCurrentProcess(), hook.target, kPatchSize);
  DWORD ignored = 0;
  VirtualProtect(hook.target, kPatchSize, old_protect, &ignored);
  Log(
      "Hooked %s target=%p trampoline=%p patchSize=%llu relocatedCall=%p",
      name, hook.target, hook.trampoline,
      static_cast<unsigned long long>(kPatchSize), call_destination);
  return true;
}


// Original src/dllmain.cpp:44871
bool InstallInlineHookRelocatedConditionAtOffset2(
    InlineHook& hook,
    uintptr_t target,
    void* detour,
    void** original,
    const char* name) {
  constexpr size_t kPatchSize = 18;
  constexpr size_t kConditionOffset = 2;
  constexpr size_t kConditionLength = 6;
  if (hook.target != nullptr || hook.detour != nullptr ||
      hook.trampoline != nullptr || hook.patch_size != 0) {
    LogError(
        "InstallInlineHookRelocatedConditionAtOffset2 occupied slot rejected name=%s",
        name);
    return false;
  }
  if (target == 0 || detour == nullptr || original == nullptr) {
    LogError(
        "InstallInlineHookRelocatedConditionAtOffset2 invalid input name=%s target=%p detour=%p original=%p",
        name, reinterpret_cast<void*>(target), detour, original);
    return false;
  }

  hook.target = reinterpret_cast<void*>(target);
  hook.detour = detour;
  hook.patch_size = kPatchSize;
  std::memcpy(hook.original, hook.target, kPatchSize);
  if (hook.original[kConditionOffset] != 0x0f ||
      (hook.original[kConditionOffset + 1] & 0xf0) != 0x80) {
    LogError(
        "InstallInlineHookRelocatedConditionAtOffset2 opcode mismatch name=%s opcode=%02x%02x",
        name, static_cast<unsigned>(hook.original[kConditionOffset]),
        static_cast<unsigned>(hook.original[kConditionOffset + 1]));
    hook = InlineHook{};
    return false;
  }

  auto* trampoline = static_cast<uint8_t*>(VirtualAlloc(
      nullptr, 80, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
  if (trampoline == nullptr) {
    LogError(
        "InstallInlineHookRelocatedConditionAtOffset2 VirtualAlloc failed name=%s error=%lu",
        name, GetLastError());
    hook = InlineHook{};
    return false;
  }
  const int32_t relative = *reinterpret_cast<const int32_t*>(
      hook.original + kConditionOffset + 2);
  auto* const relative_next = static_cast<uint8_t*>(hook.target) +
      kConditionOffset + kConditionLength;
  void* const taken_destination = relative_next + relative;

  std::memcpy(trampoline, hook.original, kConditionOffset);
  trampoline[kConditionOffset] = 0x0f;
  trampoline[kConditionOffset + 1] =
      hook.original[kConditionOffset + 1] ^ 0x01;
  *reinterpret_cast<int32_t*>(trampoline + kConditionOffset + 2) = 14;
  WriteAbsoluteJump(
      trampoline + kConditionOffset + kConditionLength,
      taken_destination);
  constexpr size_t kRelocatedFallthroughOffset =
      kConditionOffset + kConditionLength + 14;
  std::memcpy(
      trampoline + kRelocatedFallthroughOffset,
      hook.original + kConditionOffset + kConditionLength,
      kPatchSize - kConditionOffset - kConditionLength);
  WriteAbsoluteJump(
      trampoline + kRelocatedFallthroughOffset +
          (kPatchSize - kConditionOffset - kConditionLength),
      static_cast<uint8_t*>(hook.target) + kPatchSize);
  hook.trampoline = trampoline;
  *original = trampoline;

  DWORD old_protect = 0;
  if (!VirtualProtect(
          hook.target, kPatchSize, PAGE_EXECUTE_READWRITE, &old_protect)) {
    LogError(
        "InstallInlineHookRelocatedConditionAtOffset2 VirtualProtect failed name=%s error=%lu",
        name, GetLastError());
    VirtualFree(trampoline, 0, MEM_RELEASE);
    *original = nullptr;
    hook = InlineHook{};
    return false;
  }
  uint8_t patch[kPatchSize]{};
  WriteAbsoluteJump(patch, detour);
  for (size_t i = 14; i < kPatchSize; ++i) {
    patch[i] = 0x90;
  }
  std::memcpy(hook.target, patch, kPatchSize);
  FlushInstructionCache(GetCurrentProcess(), hook.target, kPatchSize);
  DWORD ignored = 0;
  VirtualProtect(hook.target, kPatchSize, old_protect, &ignored);
  Log(
      "Hooked %s target=%p trampoline=%p patchSize=%llu relocatedCondition=%p",
      name, hook.target, hook.trampoline,
      static_cast<unsigned long long>(kPatchSize), taken_destination);
  return true;
}


// Original src/dllmain.cpp:44971
void RemoveInlineHook(InlineHook& hook) {
  if (hook.target == nullptr || hook.patch_size == 0) {
    return;
  }

  DWORD old_protect = 0;
  if (VirtualProtect(hook.target, hook.patch_size, PAGE_EXECUTE_READWRITE, &old_protect)) {
    std::memcpy(hook.target, hook.original, hook.patch_size);
    FlushInstructionCache(GetCurrentProcess(), hook.target, hook.patch_size);
    DWORD ignored = 0;
    VirtualProtect(hook.target, hook.patch_size, old_protect, &ignored);
  }

  if (hook.trampoline != nullptr) {
    VirtualFree(hook.trampoline, 0, MEM_RELEASE);
  }

  hook = InlineHook{};
}


// Original src/dllmain.cpp:45296
bool BuildRelativeCall(
    uintptr_t callsite,
    uintptr_t destination,
    uint8_t out_call[5]) {
  if (out_call == nullptr) {
    return false;
  }
  const int64_t delta =
      static_cast<int64_t>(destination) -
      static_cast<int64_t>(callsite + 5);
  if (delta < INT32_MIN || delta > INT32_MAX) {
    return false;
  }
  out_call[0] = 0xe8;
  const int32_t relative = static_cast<int32_t>(delta);
  std::memcpy(out_call + 1, &relative, sizeof(relative));
  return true;
}


// Original src/dllmain.cpp:45315
void* AllocateRelayNear(uintptr_t callsite, size_t size) {
  if (callsite == 0 || size == 0) {
    return nullptr;
  }

  SYSTEM_INFO system_info{};
  GetSystemInfo(&system_info);
  const uintptr_t granularity =
      static_cast<uintptr_t>(system_info.dwAllocationGranularity);
  const uintptr_t minimum_application =
      reinterpret_cast<uintptr_t>(system_info.lpMinimumApplicationAddress);
  const uintptr_t maximum_application =
      reinterpret_cast<uintptr_t>(system_info.lpMaximumApplicationAddress);
  if (granularity == 0 || maximum_application < size) {
    return nullptr;
  }

  const uintptr_t next = callsite + 5;
  uintptr_t lower = next > 0x80000000ull
      ? next - 0x80000000ull
      : minimum_application;
  uintptr_t upper = next <= maximum_application - INT32_MAX
      ? next + INT32_MAX
      : maximum_application;
  if (lower < minimum_application) {
    lower = minimum_application;
  }
  if (upper > maximum_application - size) {
    upper = maximum_application - size;
  }
  lower = (lower + granularity - 1) & ~(granularity - 1);

  for (uintptr_t cursor = lower; cursor <= upper;) {
    MEMORY_BASIC_INFORMATION region{};
    if (VirtualQuery(
            reinterpret_cast<const void*>(cursor),
            &region,
            sizeof(region)) == 0) {
      break;
    }
    const uintptr_t region_base =
        reinterpret_cast<uintptr_t>(region.BaseAddress);
    const uintptr_t region_end =
        region.RegionSize <= UINTPTR_MAX - region_base
        ? region_base + region.RegionSize
        : UINTPTR_MAX;
    if (region.State == MEM_FREE) {
      uintptr_t candidate =
          region_base < lower ? lower : region_base;
      candidate =
          (candidate + granularity - 1) & ~(granularity - 1);
      if (candidate <= upper && candidate < region_end &&
          size <= region_end - candidate) {
        void* relay = VirtualAlloc(
            reinterpret_cast<void*>(candidate),
            size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE);
        uint8_t call[5]{};
        if (relay != nullptr &&
            BuildRelativeCall(
                callsite,
                reinterpret_cast<uintptr_t>(relay),
                call)) {
          return relay;
        }
        if (relay != nullptr) {
          VirtualFree(relay, 0, MEM_RELEASE);
        }
      }
    }
    if (region_end <= cursor || region_end > upper) {
      break;
    }
    cursor = region_end;
  }
  return nullptr;
}


// Original src/dllmain.cpp:47861
DWORD WINAPI WorkerThread(void*) {
  OpenLog();
  LoadConfig();
  Log("RevenantFix loading");
  Log(
      "RevenantFix config delayedSwitchTicks=%u regenFullSeconds=%d remoteOwnerGateFallbackMode=%d fixRewardAcquire=%d fixRewardDiscard=%d fixPlayerMapMarkers=%d fixPlayerListCompaction=%d enableNrscSteamControl=%d enableMatchedPlayersUiSuppress=%d holdAuthorizedExpeditionBLeave=%d showDetailedLogs=%d",
      kDelayedSwitchCleanupTicks,
      kZeroHpFallbackRegenFullSeconds,
      g_config.remote_owner_gate_fallback_mode,
      g_config.fix_reward_acquire ? 1 : 0,
      g_config.fix_reward_discard ? 1 : 0,
      g_config.fix_player_map_markers ? 1 : 0,
      g_config.fix_player_list_compaction ? 1 : 0,
      g_config.enable_nrsc_steam_control ? 1 : 0,
      g_config.enable_nrsc_matched_players_ui_suppress ? 1 : 0,
      g_config.hold_authorized_expedition_b_leave ? 1 : 0,
      g_config.show_detailed_logs ? 1 : 0);

  g_game_base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
  Log("gameBase=%p", reinterpret_cast<void*>(g_game_base));

  g_unsummon_buddy = reinterpret_cast<UnsummonBuddyFn>(g_game_base + kUnsummonBuddyRva);
  g_try_use_buddy = reinterpret_cast<TryUseBuddyFn>(g_game_base + kTryUseBuddyRva);
  g_get_buddy_hp_rate = reinterpret_cast<GetBuddyHpRateFn>(g_game_base + kGetBuddyHpRateRva);
  g_buddy_module_update = reinterpret_cast<BuddyUpdateFn>(g_game_base + kBuddyModuleUpdateRva);
  g_buddy_update = reinterpret_cast<BuddyUpdateFn>(g_game_base + kBuddyUpdateRva);
  g_is_local_owner = reinterpret_cast<ChrGateFn>(g_game_base + kIsLocalOwnerRva);
  g_remote_owner_gate = reinterpret_cast<ChrGateFn>(g_game_base + kRemoteOwnerGateRva);
  g_default_player_game_data =
      reinterpret_cast<DefaultPlayerGameDataFn>(g_game_base + kDefaultPlayerGameDataRva);
  g_net_send_core = reinterpret_cast<NetSendCoreFn>(g_game_base + kNetSendCoreRva);
  g_world_chr_authority = reinterpret_cast<WorldChrAuthorityFn>(
      g_game_base + kWorldChrAuthorityRva);
  g_world_chr_status_mark_dirty = reinterpret_cast<WorldChrStatusMarkDirtyFn>(
      g_game_base + kWorldChrStatusMarkDirtyRva);
  g_party_member_remove = reinterpret_cast<PartyMemberRemoveFn>(g_game_base + kPartyMemberRemoveRva);
  g_map_gaitem_mark_acquired =
      reinterpret_cast<MapGaitemMarkAcquiredFn>(g_game_base + kMapGaitemMarkAcquiredRva);
  g_reward_spawn_map_node =
      reinterpret_cast<RewardSpawnMapNodeFn>(g_game_base + kRewardSpawnMapNodeRva);
  g_reward_entry_available =
      reinterpret_cast<RewardEntryAvailableFn>(g_game_base + kRewardEntryAvailableRva);
  g_reward_discard_request =
      reinterpret_cast<RewardDiscardRequestFn>(g_game_base + kRewardDiscardRequestRva);
  g_reward_local_discard_apply =
      reinterpret_cast<RewardLocalDiscardApplyFn>(g_game_base + kRewardLocalDiscardApplyBodyRva);
  g_reward_find_by_id = reinterpret_cast<RewardFindByIdFn>(g_game_base + kRewardFindByIdRva);
  g_buddy_death_recovery_update =
      reinterpret_cast<BuddyUpdateFn>(g_game_base + kBuddyDeathRecoveryUpdateRva);
  g_hp_set_current = reinterpret_cast<HpSetCurrentFn>(g_game_base + kHpSetCurrentRva);
  g_apply_sp_effect = reinterpret_cast<ApplySpEffectFn>(g_game_base + kApplySpEffectRva);
  g_player_game_data_alloc =
      reinterpret_cast<PlayerGameDataAllocFn>(g_game_base + kPlayerGameDataAllocRva);
  g_player_game_data_ctor =
      reinterpret_cast<PlayerGameDataCtorFn>(g_game_base + kPlayerGameDataCtorRva);
  g_player_ins_controller_eligibility = reinterpret_cast<PlayerInsControllerEligibilityFn>(
      g_game_base + kPlayerInsControllerEligibilityRva);
  g_world_chr_register = reinterpret_cast<WorldChrRegisterFn>(g_game_base + kWorldChrRegisterRva);
  g_world_owner_group_register =
      reinterpret_cast<WorldOwnerGroupRegisterFn>(g_game_base + kWorldOwnerGroupRegisterRva);
  g_world_owner_group_set_entity_active =
      reinterpret_cast<WorldOwnerGroupSetEntityActiveFn>(g_game_base + kWorldOwnerGroupSetEntityActiveRva);
  g_entity_lookup = reinterpret_cast<EntityLookupFn>(g_game_base + kEntityLookupRva);
  g_world_chr_lookup_by_game_data_id =
      reinterpret_cast<WorldChrManLookupByGameDataIdFn>(g_game_base + kWorldChrManLookupByGameDataIdRva);
  g_current_map_family_read =
      reinterpret_cast<CurrentMapFamilyReadFn>(
          g_game_base + kCurrentMapFamilyReadRva);
  g_world_map_remote_marker_update =
      reinterpret_cast<WorldMapRemoteMarkerUpdateFn>(g_game_base + kWorldMapRemoteMarkerUpdateRva);
  g_chr_icon_id = reinterpret_cast<ChrIconIdFn>(g_game_base + kChrIconIdRva);
  g_world_map_set_icon_id = reinterpret_cast<WorldMapSetIconIdFn>(g_game_base + kWorldMapSetIconIdRva);
  g_world_map_set_icon_type = reinterpret_cast<WorldMapSetIconTypeFn>(g_game_base + kWorldMapSetIconTypeRva);
  InstallInlineHook(
      g_hooks[64],
      g_game_base + kSessionEventBroadcastRva,
      reinterpret_cast<void*>(HookSessionEventBroadcast),
      15,
      reinterpret_cast<void**>(&g_session_event_broadcast),
      "FUN_140df24d0/native event broadcast bridge");
  InstallInlineHook(
      g_hooks[74],
      g_game_base + kSessionEventBroadcastToTargetsRva,
      reinterpret_cast<void*>(HookSessionEventBroadcastToTargets),
      15,
      reinterpret_cast<void**>(&g_session_event_broadcast_to_targets),
      "FUN_140df2380/native targeted event broadcast bridge");
  InstallVerifiedBhrcNativeStatusReadHook();
  InstallBhrcNativeEventResultHooks();
  InstallInlineHook(
      g_hooks[68],
      g_game_base + kPlayerStatusSerializePayloadRva,
      reinterpret_cast<void*>(HookPlayerStatusSerializePayload),
      12,
      reinterpret_cast<void**>(&g_player_status_serialize_payload),
      "FUN_14020bde0/native 0x0c PlayerGameData status capture");
  InstallInlineHook(
      g_hooks[69],
      g_game_base + kPlayerStatusPayloadValidateRva,
      reinterpret_cast<void*>(HookPlayerStatusPayloadValidate),
      17,
      reinterpret_cast<void**>(&g_player_status_payload_validate),
      "FUN_14020c350/native 0x0c payload validation capture");
  g_player_game_data_full_serialize =
      reinterpret_cast<PlayerGameDataFullSerializeFn>(
          g_game_base + kPlayerGameDataFullSerializeRva);
  g_player_game_data_full_deserialize =
      reinterpret_cast<PlayerGameDataFullDeserializeFn>(
          g_game_base + kPlayerGameDataFullDeserializeRva);
  g_player_game_data_codec_version =
      reinterpret_cast<PlayerGameDataCodecVersionFn>(
          g_game_base + kPlayerGameDataCodecVersionRva);
  g_player_game_data_post_load_refresh =
      reinterpret_cast<PlayerGameDataPostLoadFn>(
          g_game_base + kPlayerGameDataPostLoadRefreshRva);
  g_player_game_data_post_load_rebuild =
      reinterpret_cast<PlayerGameDataPostLoadFn>(
          g_game_base + kPlayerGameDataPostLoadRebuildRva);
  g_native_write_stream_init = reinterpret_cast<NativeWriteStreamInitFn>(
      g_game_base + kNativeWriteStreamInitRva);
  g_native_write_stream_remaining =
      reinterpret_cast<NativeWriteStreamMetricFn>(
          g_game_base + kNativeWriteStreamRemainingRva);
  g_native_write_stream_capacity =
      reinterpret_cast<NativeWriteStreamMetricFn>(
          g_game_base + kNativeWriteStreamCapacityRva);
  g_native_write_stream_destroy =
      reinterpret_cast<NativeWriteStreamDestroyFn>(
          g_game_base + kNativeWriteStreamDestroyRva);
  g_native_read_stream_init = reinterpret_cast<NativeReadStreamInitFn>(
      g_game_base + kNativeReadStreamInitRva);
  g_native_read_stream_destroy =
      reinterpret_cast<NativeReadStreamDestroyFn>(
          g_game_base + kNativeReadStreamDestroyRva);
  Log(
      "BHRC full_pgd_native_codec resolved serialize=%p deserialize=%p version=%p writer=%p/%p/%p/%p reader=%p/%p postLoad=%p/%p hotOnly=1 coldStorage=0",
      reinterpret_cast<void*>(g_player_game_data_full_serialize),
      reinterpret_cast<void*>(g_player_game_data_full_deserialize),
      reinterpret_cast<void*>(g_player_game_data_codec_version),
      reinterpret_cast<void*>(g_native_write_stream_init),
      reinterpret_cast<void*>(g_native_write_stream_remaining),
      reinterpret_cast<void*>(g_native_write_stream_capacity),
      reinterpret_cast<void*>(g_native_write_stream_destroy),
      reinterpret_cast<void*>(g_native_read_stream_init),
      reinterpret_cast<void*>(g_native_read_stream_destroy),
      reinterpret_cast<void*>(g_player_game_data_post_load_refresh),
      reinterpret_cast<void*>(g_player_game_data_post_load_rebuild));
  InstallInlineHook(
      g_hooks[72],
      g_game_base + kPlayerStatusCompleteRva,
      reinterpret_cast<void*>(HookPlayerStatusComplete),
      18,
      nullptr,
      "FUN_1401feab0/native player status completion fallback");

  if (!InstallBhrcNativeDescriptorRetentionHook()) {
    LogWarn(
        "BHRC native_descriptor_retention unavailable activeFriendAdmissionRemainsFailClosed=1");
  }
  const bool reconnect_hooks_ready = InstallBhrcFunctionalReconnectHooks();
  if (reconnect_hooks_ready &&
      !InstallBhrcReturningEvent1dTransactionHooks()) {
    LogWarn(
        "BHRC returning_event_1d_transaction_install outcome=unavailable immediateConsumeDisabled=1");
  }
  if (!InstallBhrcAutomaticMetadataHooks()) {
    LogWarn(
        "BHRC_AUTO_RECONNECT_METADATA install outcome=unavailable");
  }
  if (!InstallBhrcReconnectEffectDiagnostics()) {
    LogWarn("BHRC_EFFECT_TRACE install outcome=unavailable snapshotsStillEnabled=1");
  }
  const bool old_member_adapter_ready =
      reconnect_hooks_ready && InstallBhrcOldMemberHandoffAdapter();
  if (!old_member_adapter_ready) {
    LogWarn(
        "BHRC active_friend_admission edge=old_member_adapter_dependency outcome=unavailable reconnectHooksReady=%d originalMatchingUnchanged=1",
        reconnect_hooks_ready ? 1 : 0);
  }
  InstallMatchedPlayersUiHooks();
  if (g_config.enable_nrsc_steam_control) {
    const bool title_provider_ready =
        InstallBhrcNativeTitleReconnectProviderVtablePatches();
    if (!title_provider_ready) {
      LogWarn(
          "BHRC title_provider_vtable_install outcome=failed startupProbeAvailable=0 activeFriendNotificationIndependent=1");
    } else if (old_member_adapter_ready &&
               !InstallBhrcNativeTitlePostConfirmHooks()) {
      LogWarn(
          "BHRC title_post_confirm_hooks outcome=unavailable");
    }
    HANDLE nrsc_thread = CreateThread(nullptr, 0, NrscSteamControlInstallThread, nullptr, 0, nullptr);
    if (nrsc_thread != nullptr) {
      CloseHandle(nrsc_thread);
    } else {
      LogWarn("NRSC Steam control skipped reason=create_thread_error error=%lu", GetLastError());
    }
    if (InterlockedCompareExchange(&g_nrsc_control_thread_started, 1, 0) == 0) {
      HANDLE control_thread = CreateThread(nullptr, 0, RevenantFixSteamControlThread, nullptr, 0, nullptr);
      if (control_thread != nullptr) {
        CloseHandle(control_thread);
      } else {
        InterlockedExchange(&g_nrsc_control_thread_started, 0);
        LogWarn("RevenantFix Steam control skipped reason=create_thread_error error=%lu", GetLastError());
      }
    }
  } else {
    Log("Skipped NRSC Steam control by config");
  }

  InstallBuddyReadyRangePatch();
  if (g_config.fix_player_map_markers) {
    InstallPlayerMapMarkerPatches();
    if (!EnsureRemotePlayerGameDataCapacity("startup")) {
      HANDLE expansion_thread = CreateThread(nullptr, 0, RemotePlayerGameDataExpansionThread, nullptr, 0, nullptr);
      if (expansion_thread != nullptr) {
        CloseHandle(expansion_thread);
      } else {
        LogWarn("RemotePlayerGameDataExpand failed reason=create_thread_error error=%lu", GetLastError());
      }
    }
  } else {
    Log("Skipped player map/marker patches by config");
  }

  InstallInlineHook(
      g_hooks[0],
      g_game_base + kTryUseBuddyRva,
      reinterpret_cast<void*>(HookTryUseBuddy),
      15,
      reinterpret_cast<void**>(&g_try_use_buddy),
      "FUN_1403860d0/TryUseBuddy gate");
  InstallInlineHook(
      g_hooks[1],
      g_game_base + kChangeBuddyStateRva,
      reinterpret_cast<void*>(HookChangeBuddyState),
      14,
      reinterpret_cast<void**>(&g_change_buddy_state),
      "FUN_140460d00/ChangeBuddyState target");
  InstallInlineHook(
      g_hooks[2],
      g_game_base + kRequestUnsummonRva,
      reinterpret_cast<void*>(HookRequestUnsummon),
      16,
      reinterpret_cast<void**>(&g_request_unsummon),
      "FUN_140460d10/RequestUnsummon target");
  InstallInlineHook(
      g_hooks[3],
      g_game_base + kBuddyModuleUpdateRva,
      reinterpret_cast<void*>(HookBuddyModuleUpdate),
      15,
      reinterpret_cast<void**>(&g_buddy_module_update),
      "FUN_1403874f0/BuddyModuleUpdate custom buddy maintenance");
  InstallInlineHook(
      g_hooks[4],
      g_game_base + kCreateBuddyRva,
      reinterpret_cast<void*>(HookCreateBuddy),
      14,
      reinterpret_cast<void**>(&g_create_buddy),
      "FUN_140389000/CreateBuddy custom buddy tracking");
  InstallBuddyControllerHooks();
  InstallInlineHook(
      g_hooks[14],
      g_game_base + kApplySpEffectRva,
      reinterpret_cast<void*>(HookApplySpEffect),
      19,
      reinterpret_cast<void**>(&g_apply_sp_effect),
      "FUN_1403c7fd0/ApplySpEffect native buddy deploy boundary");
  InstallInlineHook(
      g_hooks[5],
      g_game_base + kCreateEntityForOwnerRva,
      reinterpret_cast<void*>(HookCreateEntityForOwner),
      14,
      reinterpret_cast<void**>(&g_create_entity_for_owner),
      "FUN_1404abdf0/CreateEntityForOwner custom entity-slot remap");
  if (g_config.fix_player_map_markers) {
    InstallInlineHook(
        g_hooks[7],
        g_game_base + kWorldMapRemoteMarkerUpdateRva,
        reinterpret_cast<void*>(HookWorldMapRemoteMarkerUpdate),
        14,
        reinterpret_cast<void**>(&g_world_map_remote_marker_update),
        "FUN_14090ee00/WorldMapRemoteMarkerUpdate map marker avatar repair");
  } else {
    Log("Skipped world map marker avatar repair hook by config");
  }
  if (g_config.fix_reward_discard) {
    InstallInlineHook(
        g_hooks[10],
        g_game_base + kRewardDiscardRequestRva,
        reinterpret_cast<void*>(HookRewardDiscardRequest),
        24,
        reinterpret_cast<void**>(&g_reward_discard_request),
        "FUN_140575810/RewardDiscardRequest reward discard fix");
    InstallInlineHook(
        g_hooks[11],
        g_game_base + kRewardLocalDiscardApplyBodyRva,
        reinterpret_cast<void*>(HookRewardLocalDiscardApply),
        15,
        reinterpret_cast<void**>(&g_reward_local_discard_apply),
        "FUN_14057cf30+9/RewardLocalDiscardApply reward discard fix");
  } else {
    Log("Skipped reward discard fix hooks by config");
  }
  InstallInlineHook(
      g_hooks[42],
      g_game_base + kPlayerGameDataAllocRva,
      reinterpret_cast<void*>(HookPlayerGameDataAlloc),
      15,
      reinterpret_cast<void**>(&g_player_game_data_alloc),
      "FUN_1401fe590/PlayerGameDataAlloc custom buddy fixed pool");
  InstallInlineHook(
      g_hooks[43],
      g_game_base + kWorldChrRegisterRva,
      reinterpret_cast<void*>(HookWorldChrRegister),
      19,
      reinterpret_cast<void**>(&g_world_chr_register),
      "FUN_14051de60/WorldChrRegister base owner group registration");
  if (g_config.fix_reward_acquire) {
    InstallInlineHook(
        g_hooks[44],
        g_game_base + kMapGaitemMarkAcquiredRva,
        reinterpret_cast<void*>(HookMapGaitemMarkAcquired),
        20,
        reinterpret_cast<void**>(&g_map_gaitem_mark_acquired),
        "FUN_14057d400/MapGaitemMarkAcquired reward acquire fix");
  } else {
    Log("Skipped reward acquire fix hook by config");
  }
  if (g_config.fix_player_list_compaction) {
    InstallInlineHook(
        g_hooks[53],
        g_game_base + kPartyMemberRemoveRva,
        reinterpret_cast<void*>(HookPartyMemberRemove),
        15,
        reinterpret_cast<void**>(&g_party_member_remove),
        "FUN_140aedd00/PartyMemberRemove compact fix");
  } else {
    Log("Skipped party member compaction hook by config");
  }
  if (g_config.enable_nrsc_steam_control) {
    InstallInlineHook(
        g_hooks[46],
        g_game_base + kNetSendCoreRva,
        reinterpret_cast<void*>(HookNetSendCore),
        26,
        reinterpret_cast<void**>(&g_net_send_core),
        "FUN_140df2270/NetSendCore native reconnect event capture");
  }
  Log("RevenantFix core fixes installed");

  return 0;
}

BOOL HandleDllProcessEvent(HMODULE module, DWORD reason) {
  if (reason == DLL_PROCESS_ATTACH) {
    g_module = module;
    InitializeCriticalSection(&g_log_lock);
    g_log_lock_initialized = true;
    DisableThreadLibraryCalls(module);

    HANDLE thread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
    if (thread != nullptr) {
      CloseHandle(thread);
    }
  } else if (reason == DLL_PROCESS_DETACH) {
    InterlockedExchange(&g_bhrc_functional_reconnect_hooks_ready, 0);
    InterlockedExchange(&g_bhrc_native_descriptor_retention_hook_ready, 0);
    InterlockedExchange(&g_bhrc_old_member_handoff_ready, 0);
    InterlockedExchange(&g_bhrc_old_member_route_ready, 0);
    RemoveInlineHook(g_hooks[kPlayerInsControllerEligibilityHookSlot]);
    g_player_ins_controller_eligibility =
        reinterpret_cast<PlayerInsControllerEligibilityFn>(
            g_game_base + kPlayerInsControllerEligibilityRva);
    RemovePlayerInsControllerAuthorityCallPatch();
    InterlockedExchange(&g_bhrc_title_post_confirm_trace_nonce, 0);
    ClearBhrcTitlePlayerChrLoginBridge();
    RemoveCodePatch(
        g_native_title_player_chr_login_completion_vtable_patch);
    RemoveCodePatch(
        g_native_title_chaos_rate_result_copy_vtable_patch);
    RemoveCodePatch(
        g_native_title_chaos_rank_result_copy_vtable_patch);
    RemoveCodePatch(
        g_native_title_player_chr_login_result_copy_vtable_patch);
    RemoveCodePatch(
        g_native_title_player_chr_login_factory_invoke_vtable_patch);
    RemoveCodePatch(
        g_native_title_reconnect_task_result_gate_vtable_patch);
    RemoveCodePatch(
        g_native_title_post_confirm_dispatcher_invoke_vtable_patch);
    RemoveCodePatch(
        g_native_title_positive_confirm_callback_vtable_patch);
    RemoveCodePatch(g_native_title_online_mode_selector_patch);
    RemoveCodePatch(g_native_title_parent_online_gate_vtable_patch);
    RemoveCodePatch(g_native_title_master_online_gate_vtable_patch);
    RemoveCodePatch(g_native_title_descriptor_gate_vtable_patch);
    RemoveCodePatch(
        g_native_title_existence_provider_invoke_vtable_patch);
    RemoveCodePatch(
        g_native_title_job78_existence_provider_invoke_vtable_patch);
    RemoveCodePatch(
        g_native_title_search_start_vtable_patch);
    RemoveCodePatch(
        g_native_title_check_penalty_result_copy_vtable_patch);
    RemoveCodePatch(
        g_native_title_check_penalty_poll_vtable_patch);
    RemoveCodePatch(
        g_native_title_check_penalty_submit_vtable_patch);
    RemoveCodePatch(g_player_indicator_refresh_loop_range_patch);
    RemoveCodePatch(g_player_indicator_refresh_valid_range_patch);
    RemoveCodePatch(g_remote_player_data_by_peer_range_patch);
    RemoveCodePatch(g_remote_player_data_search_by_entity_range2_patch);
    RemoveCodePatch(g_remote_player_data_search_by_entity_range1_patch);
    RemoveCodePatch(g_remote_player_data_find_by_entity_range2_patch);
    RemoveCodePatch(g_remote_player_data_find_by_entity_range1_patch);
    RemoveCodePatch(g_remote_player_data_get_by_index_range_patch);
    RemoveCodePatch(g_remote_player_data_flag_clear_range_patch);
    RemoveCodePatch(g_remote_player_data_clear_range_patch);
    RemoveCodePatch(g_remote_player_data_write_range_patch);
    RemoveCodePatch(g_world_map_remote_marker_update_count_patch);
    RemoveCodePatch(g_world_map_player_list_count_patch);
    RemoveCodePatch(g_player_icon_avatar_build_count_patch);
    RemoveCodePatch(g_world_map_remote_data_count_patch2);
    RemoveCodePatch(g_world_map_remote_data_count_patch1);
    RemoveBytePatch(g_buddy_ready_range_patch);
    for (int i = static_cast<int>(sizeof(g_hooks) / sizeof(g_hooks[0])) - 1; i >= 0; --i) {
      RemoveInlineHook(g_hooks[i]);
    }
    if (g_log != INVALID_HANDLE_VALUE) {
      Log("NightreignRevenantFix unloading");
      if (g_log_lock_initialized) {
        EnterCriticalSection(&g_log_lock);
      }
      FlushFileBuffers(g_log);
      CloseHandle(g_log);
      g_log = INVALID_HANDLE_VALUE;
      if (g_log_lock_initialized) {
        LeaveCriticalSection(&g_log_lock);
      }
    }
    if (g_log_lock_initialized) {
      DeleteCriticalSection(&g_log_lock);
      g_log_lock_initialized = false;
    }
  }

  return TRUE;
}


}  // namespace revenantfix::internal
