#include "reward_fix.hpp"

namespace revenantfix::internal {

// Original src/dllmain.cpp:6206
int32_t GetNetModeState(void* net_mode_manager) {
  return net_mode_manager != nullptr
      ? ReadI32(static_cast<const uint8_t*>(net_mode_manager) + 0x0c, -1)
      : -1;
}


// Original src/dllmain.cpp:6212
bool IsRewardAcquireAuthority(int32_t net_mode_state) {
  // FUN_140575810 uses mode 6 as the client request path. Any other valid mode
  // is the side that validates map reward state and broadcasts the result.
  return net_mode_state != 6 && net_mode_state != -1;
}


// Original src/dllmain.cpp:6220
void* SafeFindRewardById(void* map_item_man, uint32_t reward_id) {
  if (g_reward_find_by_id == nullptr || map_item_man == nullptr) {
    return nullptr;
  }
  __try {
    return g_reward_find_by_id(map_item_man, reward_id);
  } __except (LogSehException("RewardFindById original", GetExceptionInformation())) {
    LogWarn(
        "RewardFindById exception mapItemMan=%p rewardId=%u",
        map_item_man,
        reward_id);
    return nullptr;
  }
}


// Original src/dllmain.cpp:6235
uint64_t SafeRewardEntryAvailable(void* gaitem_list) {
  if (g_reward_entry_available == nullptr || gaitem_list == nullptr) {
    return 0;
  }
  __try {
    return g_reward_entry_available(gaitem_list);
  } __except (LogSehException("RewardEntryAvailable original", GetExceptionInformation())) {
    LogWarn("RewardEntryAvailable exception gaitemList=%p", gaitem_list);
    return 0;
  }
}


// Original src/dllmain.cpp:6247
uint32_t RewardNetworkHandleFlag(void* reward) {
  if (reward == nullptr) {
    return 0;
  }
  return (ReadU32(static_cast<const uint8_t*>(reward) + 0x78, 0) >> 4) & 0xffffff01u;
}


// Original src/dllmain.cpp:6254
uint32_t FormatRewardDiscardQueue(void* map_item_man, char* out, size_t out_count) {
  if (out_count != 0) {
    out[0] = '\0';
  }
  if (map_item_man == nullptr) {
    if (out_count != 0) {
      std::snprintf(out, out_count, "mapItemMan=null");
    }
    return 0;
  }

  const uint64_t begin = ReadU64(static_cast<const uint8_t*>(map_item_man) + 0x4a8, 0);
  const uint64_t end = ReadU64(static_cast<const uint8_t*>(map_item_man) + 0x4b0, 0);
  const uint64_t cap = ReadU64(static_cast<const uint8_t*>(map_item_man) + 0x4b8, 0);
  const uint8_t disabled = ReadU8(static_cast<const uint8_t*>(map_item_man) + 0x4c0, 0);
  uint32_t count = 0;
  if (begin != 0 && end >= begin && ((end - begin) % 8) == 0) {
    const uint64_t raw_count = (end - begin) / 8;
    count = raw_count > 0xffffffffull ? 0xffffffffu : static_cast<uint32_t>(raw_count);
  }

  char ids[160]{};
  size_t used = 0;
  const uint32_t limit = count < 6 ? count : 6;
  for (uint32_t i = 0; i < limit && used + 16 < sizeof(ids); ++i) {
    const uint32_t id = ReadU32(reinterpret_cast<const void*>(begin + static_cast<uint64_t>(i) * 8), 0);
    const int written = std::snprintf(ids + used, sizeof(ids) - used, "%s%u", i == 0 ? "" : ",", id);
    if (written <= 0) {
      break;
    }
    used += static_cast<size_t>(written);
  }

  if (out_count != 0) {
    std::snprintf(
        out,
        out_count,
        "begin=%p end=%p cap=%p count=%u disabled4c0=%u ids=[%s]",
        reinterpret_cast<void*>(begin),
        reinterpret_cast<void*>(end),
        reinterpret_cast<void*>(cap),
        count,
        disabled,
        ids);
  }
  return count;
}


// Original src/dllmain.cpp:6302
void LogRewardDiscardRequestState(
    const char* phase,
    void* map_item_man,
    uint32_t reward_id,
    void* net_mode_manager,
    int32_t net_mode_state) {
  void* reward = SafeFindRewardById(map_item_man, reward_id);
  void* gaitem_list = reward != nullptr ? static_cast<uint8_t*>(reward) + 0x20 : nullptr;
  const uint16_t flags_ca = reward != nullptr ? ReadU16(static_cast<const uint8_t*>(reward) + 0xca, 0) : 0;
  const uint32_t flags_78 = reward != nullptr ? ReadU32(static_cast<const uint8_t*>(reward) + 0x78, 0) : 0;
  const uint64_t entry_available = SafeRewardEntryAvailable(gaitem_list);
  const uint8_t entry_count = gaitem_list != nullptr ? ReadU8(gaitem_list, 0) : 0xff;
  const uint32_t first_raw0 = gaitem_list != nullptr && entry_count != 0
      ? ReadU32(static_cast<const uint8_t*>(gaitem_list) + 8, 0)
      : 0;
  const uint32_t first_raw4 = gaitem_list != nullptr && entry_count != 0
      ? ReadU32(static_cast<const uint8_t*>(gaitem_list) + 0xc, 0)
      : 0;
  const uint32_t first_count_or_lot = gaitem_list != nullptr && entry_count != 0
      ? ReadU32(static_cast<const uint8_t*>(gaitem_list) + 0x50, 0)
      : 0;
  char queue[256]{};
  FormatRewardDiscardQueue(map_item_man, queue, sizeof(queue));

  Log(
      "RewardDiscardRequest %s mapItemMan=%p rewardId=%u reward=%p netMgr=%p netState=%d "
      "flagsCA=%04x localDiscardFlagOk=%d handle78=%08x netHandle=%u entryAvailable=%llu "
      "entryCount=%u firstRaw=[%08x,%08x] firstCountOrLot=%u localPending=%d queue={%s}",
      phase != nullptr ? phase : "unknown",
      map_item_man,
      reward_id,
      reward,
      net_mode_manager,
      net_mode_state,
      flags_ca,
      (flags_ca & 0x0180) != 0 ? 1 : 0,
      flags_78,
      RewardNetworkHandleFlag(reward),
      static_cast<unsigned long long>(entry_available),
      entry_count,
      first_raw0,
      first_raw4,
      first_count_or_lot,
      HasRecentLocalRewardDiscardRequest(reward_id) ? 1 : 0,
      queue);
}


// Original src/dllmain.cpp:6349
uint8_t __fastcall HookMapGaitemMarkAcquired(
    void* map_item_man,
    uint32_t map_gaitem_id,
    uint8_t acquired) {
  uint8_t result = g_map_gaitem_mark_acquired(map_item_man, map_gaitem_id, acquired);
  if (!g_config.fix_reward_acquire) {
    return result;
  }

  const uint8_t original_result = result;
  void* net_mode_manager = GetNetModeManager();
  const int32_t net_mode_state = GetNetModeState(net_mode_manager);
  const bool authority = IsRewardAcquireAuthority(net_mode_state);

  if (result == 0 && authority) {
    result = 1;
    LogWarn(
        "RewardAcquireFix forced_success mapItemMan=%p mapGaitemId=%u acquired=%u "
        "originalResult=%u result=%u netMgr=%p netState=%d",
        map_item_man,
        map_gaitem_id,
        acquired,
        original_result,
        result,
        net_mode_manager,
        net_mode_state);
  }

  return result;
}


// Original src/dllmain.cpp:6666
void RememberLocalRewardDiscardRequest(uint32_t reward_id) {
  const ULONGLONG now = GetTickCount64();
  for (auto& request : g_local_reward_discard_requests) {
    if (request.reward_id == reward_id) {
      request.tick = now;
      return;
    }
  }
  g_local_reward_discard_requests[
      g_local_reward_discard_request_cursor % kLocalRewardDiscardRequestSlots] = {reward_id, now};
  ++g_local_reward_discard_request_cursor;
}


// Original src/dllmain.cpp:6679
bool HasRecentLocalRewardDiscardRequest(uint32_t reward_id) {
  const ULONGLONG now = GetTickCount64();
  for (const auto& request : g_local_reward_discard_requests) {
    if (request.reward_id == reward_id &&
        request.tick != 0 &&
        now - request.tick <= kLocalRewardDiscardRequestTtlMs) {
      return true;
    }
  }
  return false;
}


// Original src/dllmain.cpp:6691
void ForgetLocalRewardDiscardRequest(uint32_t reward_id) {
  for (auto& request : g_local_reward_discard_requests) {
    if (request.reward_id == reward_id) {
      request = {};
    }
  }
}


// Original src/dllmain.cpp:6699
bool SpawnDiscardedRewardAsMapNode(void* map_item_man, void* reward, uint32_t reward_id) {
  const bool recent_request = HasRecentLocalRewardDiscardRequest(reward_id);
  if (g_reward_spawn_map_node == nullptr || map_item_man == nullptr || reward == nullptr ||
      !recent_request) {
    LogWarn(
        "RewardDiscardFix skipped world drop rewardId=%u reason=missing_input spawnFn=%p mapItemMan=%p reward=%p recentRequest=%d",
        reward_id,
        reinterpret_cast<void*>(g_reward_spawn_map_node),
        map_item_man,
        reward,
        recent_request ? 1 : 0);
    return false;
  }

  auto* gaitem_list = static_cast<uint8_t*>(reward) + 0x20;
  const uint8_t entry_count = ReadU8(gaitem_list, 0);
  const uint16_t discard_flags = ReadU16(static_cast<const uint8_t*>(reward) + 0xca, 0);
  if (entry_count == 0 || (discard_flags & 0x0180) == 0) {
    LogWarn(
        "RewardDiscardFix skipped world drop rewardId=%u reason=not_eligible entryCount=%u flagsCA=%04x",
        reward_id,
        entry_count,
        discard_flags);
    return false;
  }

  constexpr size_t kGaitemEntrySize = 0x50;
  constexpr uint8_t kMaxSupportedEntries = 8;
  if (entry_count > kMaxSupportedEntries) {
    LogWarn(
        "RewardDiscardFix skipped world drop rewardId=%u reason=too_many_entries count=%u max=%u",
        reward_id,
        entry_count,
        kMaxSupportedEntries);
    return false;
  }

  uint32_t spawned = 0;
  for (uint8_t i = 0; i < entry_count; ++i) {
    alignas(8) uint8_t one_entry_list[8 + kGaitemEntrySize]{};
    one_entry_list[0] = 1;
    auto* entry = gaitem_list + 8 + static_cast<size_t>(i) * kGaitemEntrySize;
    std::memcpy(one_entry_list + 8, entry, kGaitemEntrySize);

    const uint32_t raw0 = ReadU32(entry, 0);
    const uint32_t raw4 = ReadU32(entry + 4, 0);
    const uint32_t count_or_lot = ReadU32(entry + 0x48, 0);
    __try {
      // Use the same map reward creation path as the engine's leftover/overflow
      // reward code. Variant 1 uses a low scatter profile, avoiding the wide
      // horizontal throw used by variant 0.
      g_reward_spawn_map_node(map_item_man, one_entry_list, 1, 0);
      ++spawned;
      Log(
          "RewardDiscardFix spawned world drop rewardId=%u index=%u raw=[%08x,%08x] countOrLot=%u",
          reward_id,
          i,
          raw0,
          raw4,
          count_or_lot);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
      LogError(
          "RewardDiscardFix spawn exception rewardId=%u index=%u raw=[%08x,%08x] countOrLot=%u code=%08x",
          reward_id,
          i,
          raw0,
          raw4,
          count_or_lot,
          GetExceptionCode());
    }
  }

  return spawned != 0;
}


// Original src/dllmain.cpp:6774
void DisableOriginalDiscardRewardNode(void* reward, uint32_t reward_id, const char* phase) {
  if (reward == nullptr) {
    return;
  }

  // Direct reward UI discard is replaced with a normal map reward. Keep the
  // original reward node available for the original cleanup function to process,
  // but remove its own visible/pickable state before and after that cleanup so
  // the client does not keep a residual reward and the host cannot pick a second
  // copy from a remote player's discard.
  constexpr uint16_t kDisableOriginalRewardMask = 0x01c0;
  const auto* reward_bytes = static_cast<const uint8_t*>(reward);
  auto* mutable_reward_bytes = static_cast<uint8_t*>(reward);
  const uint16_t before = ReadU16(reward_bytes + 0xca, 0);
  const uint16_t after = static_cast<uint16_t>(before & ~kDisableOriginalRewardMask);
  if (after == before) {
    return;
  }

  if (WriteU16(mutable_reward_bytes + 0xca, after)) {
    Log(
        "RewardDiscardFix disabled original node phase=%s reward=%p rewardId=%u flagsCA=%04x->%04x",
        phase != nullptr ? phase : "unknown",
        reward,
        reward_id,
        before,
        after);
  } else {
    LogWarn(
        "RewardDiscardFix disable original node failed phase=%s reward=%p rewardId=%u flagsCA=%04x",
        phase != nullptr ? phase : "unknown",
        reward,
        reward_id,
        before);
  }
}


// Original src/dllmain.cpp:6811
void TryClientRewardDiscardFallback(
    void* map_item_man,
    uint32_t reward_id,
    uint32_t local_apply_count_before,
    int32_t net_mode_state) {
  if (net_mode_state != 6) {
    return;
  }
  if (g_reward_local_discard_apply_count != local_apply_count_before) {
    return;
  }
  if (!HasRecentLocalRewardDiscardRequest(reward_id)) {
    return;
  }

  void* reward = SafeFindRewardById(map_item_man, reward_id);
  const uint16_t flags_ca = reward != nullptr
      ? ReadU16(static_cast<const uint8_t*>(reward) + 0xca, 0)
      : 0;
  const uint64_t entry_available = reward != nullptr
      ? SafeRewardEntryAvailable(static_cast<uint8_t*>(reward) + 0x20)
      : 0;
  if (reward == nullptr || (flags_ca & 0x0180) == 0 || entry_available == 0) {
    LogWarn(
        "RewardDiscardFix client_fallback skipped rewardId=%u reason=not_eligible reward=%p flagsCA=%04x entryAvailable=%llu",
        reward_id,
        reward,
        flags_ca,
        static_cast<unsigned long long>(entry_available));
    return;
  }

  const bool spawned = SpawnDiscardedRewardAsMapNode(map_item_man, reward, reward_id);
  if (!spawned) {
    LogWarn(
        "RewardDiscardFix client_fallback failed rewardId=%u reward=%p flagsCA=%04x entryAvailable=%llu",
        reward_id,
        reward,
        flags_ca,
        static_cast<unsigned long long>(entry_available));
    return;
  }

  // Client direct-discard can stall with only a queued 0x2c/0x0b request when
  // the host rejects the reward id/state. In that case we replace the UI reward
  // with a normal synchronized map reward and make the original local node inert
  // so repeated clicks do not create duplicate rewards. The flag write alone is
  // not enough on clients: the visual reward node can remain in the world, and
  // some flag combinations can still leave it selectable. Run the original local
  // cleanup after spawning the replacement so the UI/map node is fully released.
  DisableOriginalDiscardRewardNode(reward, reward_id, "client_fallback_before_original");
  __try {
    g_reward_local_discard_apply(map_item_man, reward);
  } __except (LogSehException("RewardDiscardFix client_fallback local cleanup", GetExceptionInformation())) {
    LogError(
        "RewardDiscardFix client_fallback cleanup_exception rewardId=%u reward=%p flagsCA=%04x",
        reward_id,
        reward,
        flags_ca);
  }
  DisableOriginalDiscardRewardNode(reward, reward_id, "client_fallback_after_original");
  ForgetLocalRewardDiscardRequest(reward_id);
  LogWarn(
      "RewardDiscardFix client_fallback forced_world_drop rewardId=%u reward=%p flagsCA=%04x entryAvailable=%llu",
      reward_id,
      reward,
      flags_ca,
      static_cast<unsigned long long>(entry_available));
}


// Original src/dllmain.cpp:6881
void __fastcall HookRewardDiscardRequest(void* map_item_man, uint32_t reward_id) {
  if (!g_config.fix_reward_discard) {
    g_reward_discard_request(map_item_man, reward_id);
    return;
  }

  void* net_mode_manager = GetNetModeManager();
  const int32_t net_mode_state = GetNetModeState(net_mode_manager);
  LogRewardDiscardRequestState("enter", map_item_man, reward_id, net_mode_manager, net_mode_state);

  RememberLocalRewardDiscardRequest(reward_id);
  const uint32_t local_apply_count_before = g_reward_local_discard_apply_count;
  ++g_reward_discard_request_depth;
  __try {
    g_reward_discard_request(map_item_man, reward_id);
  } __finally {
    --g_reward_discard_request_depth;
  }

  TryClientRewardDiscardFallback(
      map_item_man,
      reward_id,
      local_apply_count_before,
      net_mode_state);
  LogRewardDiscardRequestState("exit", map_item_man, reward_id, net_mode_manager, net_mode_state);
}


// Original src/dllmain.cpp:6908
void __fastcall HookRewardLocalDiscardApply(void* map_item_man, void* reward) {
  if (!g_config.fix_reward_discard) {
    g_reward_local_discard_apply(map_item_man, reward);
    return;
  }

  const uint32_t reward_id = reward != nullptr
      ? ReadU32(static_cast<const uint8_t*>(reward) + 0x78, 0xffffffff)
      : 0xffffffff;
  ++g_reward_local_discard_apply_count;
  const uint16_t flags_before = reward != nullptr ? ReadU16(static_cast<const uint8_t*>(reward) + 0xca, 0) : 0;
  const uint8_t entry_count = reward != nullptr ? ReadU8(static_cast<const uint8_t*>(reward) + 0x20, 0) : 0;
  const uint8_t* first_entry = reward != nullptr && entry_count != 0
      ? static_cast<const uint8_t*>(reward) + 0x28
      : nullptr;
  const uint32_t first_raw0 = first_entry != nullptr ? ReadU32(first_entry, 0) : 0;
  const uint32_t first_raw4 = first_entry != nullptr ? ReadU32(first_entry + 4, 0) : 0;
  const uint32_t first_count_or_lot = first_entry != nullptr ? ReadU32(first_entry + 0x48, 0) : 0;
  const bool had_local_discard_request = HasRecentLocalRewardDiscardRequest(reward_id);
  const bool original_reward_eligible = (flags_before & 0x0180) != 0;

  Log(
      "RewardLocalDiscardApply enter mapItemMan=%p reward=%p rewardId=%u hadRecentRequest=%d entryCount=%u flagsCA=%04x firstRaw=[%08x,%08x] firstCountOrLot=%u depth=%u",
      map_item_man,
      reward,
      reward_id,
      had_local_discard_request ? 1 : 0,
      entry_count,
      flags_before,
      first_raw0,
      first_raw4,
      first_count_or_lot,
      g_reward_discard_request_depth);

  const bool spawned_replacement = SpawnDiscardedRewardAsMapNode(map_item_man, reward, reward_id);

  if (original_reward_eligible) {
    DisableOriginalDiscardRewardNode(reward, reward_id, "before_original");
  }

  g_reward_local_discard_apply(map_item_man, reward);

  if (original_reward_eligible || spawned_replacement) {
    DisableOriginalDiscardRewardNode(reward, reward_id, "after_original");
  }

  if (had_local_discard_request || spawned_replacement) {
    ForgetLocalRewardDiscardRequest(reward_id);
  }

  const uint16_t flags_after = reward != nullptr ? ReadU16(static_cast<const uint8_t*>(reward) + 0xca, 0) : 0;
  Log(
      "RewardLocalDiscardApply exit mapItemMan=%p reward=%p rewardId=%u spawned=%d eligible=%d flagsCA=%04x->%04x pendingAfter=%d",
      map_item_man,
      reward,
      reward_id,
      spawned_replacement ? 1 : 0,
      original_reward_eligible ? 1 : 0,
      flags_before,
      flags_after,
      HasRecentLocalRewardDiscardRequest(reward_id) ? 1 : 0);
}


}  // namespace revenantfix::internal
