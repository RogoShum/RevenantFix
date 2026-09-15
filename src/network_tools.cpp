#include "network_tools.hpp"

namespace revenantfix::internal {

// 7a66e71 src/dllmain.cpp:6056
void __fastcall HookNetSendCore(
    void* net_manager,
    void* peer,
    uint32_t msg_id,
    const void* packet,
  int size) {
  const uintptr_t caller_rva = RvaOf(_ReturnAddress());

  bool original_ok = false;
  __try {
    g_net_send_core(net_manager, peer, msg_id, packet, size);
    original_ok = true;
  } __except (LogSehException("NetSendCore original", GetExceptionInformation())) {
    LogError(
        "NetSendCore original_exception net=%p peer=%p msgId=0x%02x size=%d callerRva=%llx",
        net_manager,
        peer,
        msg_id & 0xffu,
        size,
        static_cast<unsigned long long>(caller_rva));
  }

  if (original_ok && msg_id == 0x17) {
    RepublishAuthoritativePlayerStatusAfterNativeRegistration(
        peer, packet, size, caller_rva);
  }

  if (original_ok && msg_id == 0x1d &&
      size == static_cast<int>(kBhrcNativeEvent1dPayloadSize) &&
      packet != nullptr && IsLocalNrscHost()) {
    CaptureBhrcPendingHostNativeEvent1d(
        net_manager,
        packet,
        static_cast<uint32_t>(size),
        "FUN_140df2270_original_0x1d");
    CaptureBhrcHostNativeEvent(
        msg_id,
        ResolveBhrcHostLiveSourceRecordKey(),
        packet,
        static_cast<uint32_t>(size),
        "FUN_140df2270_original_0x1d");
  }

}


// 7a66e71 src/dllmain.cpp:6101
int __fastcall HookNativeEventPairRead(
    void* record_pair,
    uint8_t event_id,
    void* out_payload,
    uint32_t payload_size,
    uint8_t* out_flags) {
  const uintptr_t direct_caller_rva = RvaOf(_ReturnAddress());
  int result = 0;
  __try {
    result = g_native_event_pair_read(
        record_pair, event_id, out_payload, payload_size, out_flags);
  } __except (LogSehException(
      "BHRC FUN_1422d75a0 native event pair read",
      GetExceptionInformation())) {
    LogError(
        "BHRC hook_exception function=FUN_1422d75a0 record=%p event=0x%02x out=%p capacity=%u",
        record_pair, static_cast<unsigned>(event_id), out_payload, payload_size);
    return 0;
  }
  if (result > 0 && event_id == 0x94 && out_payload != nullptr &&
      payload_size == kBhrcNativeEvent94MaxPayloadSize) {
    CaptureBhrcMigratableNativeEvent94(
        out_payload, static_cast<uint32_t>(result),
        "FUN_1422d75a0_original_success");
  }
  if (result != 0 || out_payload == nullptr) {
    return result;
  }

  if (direct_caller_rva != kBhrcNativeEvent94PairReadCallerReturnRva ||
      event_id != 0x94 ||
      payload_size != kBhrcNativeEvent94MaxPayloadSize) {
    return result;
  }

  // The active old-member FriendJoin belongs to the saved-descriptor /
  // previous-session owner. Letting the generic state-7 reader take its
  // one-shot candidate would recreate the lobby GuestReady/Matching owner
  // before the cancel -> idle -> previous-session adapter handoff can run.
  const BhrcExpeditionProbeAttempt probe = SnapshotBhrcExpeditionProbe();
  if (BhrcProbeForbidsGenericState7Event94(probe)) {
    return 0;
  }

  const uint64_t record_key =
      ValidateBhrcTeamSessionEventRecord(record_pair);
  uint32_t bridged_size = 0;
  if (!ConsumeBhrcInboundNativeEvent(
          event_id,
          record_key,
          out_payload,
          payload_size,
          &bridged_size,
          "FUN_140dfe070_original_empty")) {
    return 0;
  }
  if (out_flags != nullptr) {
    __try {
      *out_flags = 0;
    } __except (LogSehException(
        "BHRC FUN_140dfe070 bridged flags",
        GetExceptionInformation())) {
      return 0;
    }
  }
  return static_cast<int>(bridged_size);
}


// 7a66e71 src/dllmain.cpp:6208
int __fastcall HookNativeEventReadAnyRecord(
    void* session_mgr,
    uint32_t event_id,
    void* out_payload,
    uint32_t payload_size) {
  int result = 0;
  __try {
    result = g_native_event_read_any_record(
        session_mgr, event_id, out_payload, payload_size);
  } __except (LogSehException(
      "BHRC FUN_140df1f20 native event read any record",
      GetExceptionInformation())) {
    LogError(
        "BHRC hook_exception function=FUN_140df1f20 manager=%p event=0x%02x out=%p size=%u",
        session_mgr, event_id, out_payload, payload_size);
    return 0;
  }
  if (result == 0 && event_id == 0x1d && out_payload != nullptr &&
      payload_size == kBhrcNativeEvent1dPayloadSize && !IsLocalNrscHost()) {
    const BhrcExpeditionProbeAttempt probe = SnapshotBhrcExpeditionProbe();
    const BhrcNativeExpeditionAuthorityState authority =
        SnapshotNativeExpeditionAuthority();
    const BhrcNativeManagerRegistryLookup authority_lookup =
        ReadBhrcNativeManagerRegistry(session_mgr, authority.manager_id);
    const bool returning_same_b_route_ready =
        (probe.trigger ==
             BhrcExpeditionProbeTrigger::StartupPreviousSession &&
         !probe.previous_session_owner_only) ||
        (probe.trigger == BhrcExpeditionProbeTrigger::FriendJoin &&
         probe.previous_session_owner_only);
    const bool returning_same_b_mode1_ready =
        IsBhrcAuthorizedClientProbe(probe) &&
        returning_same_b_route_ready &&
        probe.stage == BhrcExpeditionProbeStage::HandedOff &&
        probe.host_phase == BhrcExpeditionHostPhase::RunningExpedition &&
        probe.membership == BhrcExpeditionMembership::ReturningSameB &&
        probe.native_attempt_running_authorized &&
        !probe.native_attempt_transport_lost &&
        authority.valid &&
        authority.commit_seq == probe.authority_commit_seq &&
        authority.master_lobby_id == probe.master_lobby_id &&
        authority.lobby_id == probe.expedition_lobby_id &&
        authority.owner_steam_id == probe.host_steam_id &&
        authority.local_steam_id ==
            g_nrsc_steam_control_state.local_steam_id &&
        authority.native_manager != nullptr &&
        session_mgr == ReadGameGlobalPtr(kNetModeManagerPtrRva) &&
        !authority_lookup.read_fault &&
        authority_lookup.manager == authority.native_manager &&
        authority_lookup.provider != nullptr;
    uint32_t bridged_size = 0;
    if (probe.host_steam_id != 0 && probe.native_owner_ready &&
        returning_same_b_mode1_ready &&
        (probe.native_event_sent_mask & kBhrcNativeEventMask0c) != 0) {
      if (TryReserveBhrcReturningSameBEvent1d(
              session_mgr, probe.host_steam_id, out_payload, payload_size,
              &bridged_size)) {
        result = static_cast<int>(bridged_size);
      }
    }
  }
  return result;
}


// 7a66e71 src/dllmain.cpp:11117
const char* DescribeNrscLobbyEvent(uint32_t event_type) {
  if (event_type == 1) {
    return "NrscPeerJoin";
  }
  if (((0x10114u >> (event_type & 0x1f)) & 1u) != 0) {
    return "NrscPeerLost";
  }
  return "NrscPeerEvent";
}


// 7a66e71 src/dllmain.cpp:11168
void RememberNrscPeerSteamId(uint64_t steam_id) {
  if (steam_id == 0 || steam_id == g_nrsc_steam_control_state.local_steam_id) {
    return;
  }

  for (uint64_t known : g_known_nrsc_peer_steam_ids) {
    if (known == steam_id) {
      return;
    }
  }

  for (uint64_t& known : g_known_nrsc_peer_steam_ids) {
    if (known == 0) {
      known = steam_id;
      return;
    }
  }

  g_known_nrsc_peer_steam_ids[g_known_nrsc_peer_cursor++ % _countof(g_known_nrsc_peer_steam_ids)] = steam_id;
}


// FUN_1800647a0 uses scan_context-0x70 for the native peer vector. A
// retained active peer makes FUN_1800645a0 return without RegisterPeer, so
// the callback-maintained shadow alone cannot describe a manual rejoin.
bool TryReadNrscNativePeerActive(uint64_t steam_id, bool* active) {
  *active = false;
  const bool member_context = g_nrsc_steam_control_state.member_context != nullptr;
  const auto* context = static_cast<const uint8_t*>(member_context
      ? g_nrsc_steam_control_state.member_context : g_nrsc_steam_control_state.scan_context);
  if (context == nullptr || g_nrsc_module == nullptr) return false;
  const size_t state_offset = member_context ? 0x88 : 0x78;
  const size_t vector_offset = member_context ? 0x80 : 0x70;
  BhrcNrscOwnershipSnapshot state{};
  state.state = ReadU64(context - state_offset, 0);
  state.master_lobby = ReadU64(reinterpret_cast<const void*>(state.state + 0x120), 0);
  state.local_steam_id = ReadU64(reinterpret_cast<const void*>(state.state + 8), 0);
  state.owner_steam_id = ReadU64(reinterpret_cast<const void*>(state.state + 0x128), 0);
  if (state.state == 0 || state.master_lobby == 0 ||
      state.master_lobby != GetCurrentNrscMasterLobbyId() ||
      state.local_steam_id != g_nrsc_steam_control_state.local_steam_id ||
      state.owner_steam_id != g_nrsc_steam_control_state.owner_steam_id) return false;
  uint64_t vector = 0, begin = 0, end = 0;
  if (!TryReadU64(context - vector_offset, &vector) || vector == 0 ||
      !TryReadU64(reinterpret_cast<const void*>(vector), &begin) ||
      !TryReadU64(reinterpret_cast<const void*>(vector + 8), &end) ||
      end < begin || (end - begin) % 8 != 0 || (end - begin) / 8 > 16)
    return false;
  bool found = false;
  for (uint64_t cursor = begin; cursor != end; cursor += 8) {
    uint64_t peer = 0, key = 0;
    uint8_t flags = 0;
    if (!TryReadU64(reinterpret_cast<const void*>(cursor), &peer)) return false;
    if (peer == 0) continue;
    if (!TryReadU64(reinterpret_cast<const void*>(peer + 8), &key) ||
        !TryReadU8(reinterpret_cast<const void*>(peer + 0x128), &flags)) return false;
    if (key == steam_id && (flags & 1) != 0) found = true;
  }
  if (ReadU64(context - state_offset, 0) != state.state ||
      ReadU64(reinterpret_cast<const void*>(state.state + 0x120), 0) != state.master_lobby ||
      ReadU64(reinterpret_cast<const void*>(state.state + 8), 0) != state.local_steam_id ||
      ReadU64(reinterpret_cast<const void*>(state.state + 0x128), 0) != state.owner_steam_id ||
      ReadU64(context - vector_offset, 0) != vector ||
      ReadU64(reinterpret_cast<const void*>(vector), UINT64_MAX) != begin ||
      ReadU64(reinterpret_cast<const void*>(vector + 8), UINT64_MAX) != end) return false;
  *active = found;
  return true;
}

// 7a66e71 src/dllmain.cpp:11189
bool IsNrscPeerActive(uint64_t steam_id) {
  if (steam_id == 0 || steam_id == g_nrsc_steam_control_state.local_steam_id) {
    return false;
  }
  bool active = false;
  AcquireSRWLockShared(&g_active_nrsc_peer_lock);
  for (uint64_t peer : g_active_nrsc_peer_steam_ids) {
    if (peer == steam_id) {
      active = true;
      break;
    }
  }
  ReleaseSRWLockShared(&g_active_nrsc_peer_lock);
  return active;
}


// 7a66e71 src/dllmain.cpp:11205
void SetNrscPeerActive(uint64_t steam_id, bool active, const char* reason) {
  if (steam_id == 0 || steam_id == g_nrsc_steam_control_state.local_steam_id) {
    return;
  }
  bool changed = false;
  uint64_t displaced_peer = 0;
  AcquireSRWLockExclusive(&g_active_nrsc_peer_lock);
  if (active) {
    bool present = false;
    for (uint64_t peer : g_active_nrsc_peer_steam_ids) {
      if (peer == steam_id) {
        present = true;
        break;
      }
    }
    if (!present) {
      for (uint64_t& peer : g_active_nrsc_peer_steam_ids) {
        if (peer == 0) {
          peer = steam_id;
          present = true;
          changed = true;
          break;
        }
      }
      if (!present) {
        displaced_peer = g_active_nrsc_peer_steam_ids[0];
        g_active_nrsc_peer_steam_ids[0] = steam_id;
        changed = true;
      }
    }
  } else {
    for (uint64_t& peer : g_active_nrsc_peer_steam_ids) {
      if (peer == steam_id) {
        peer = 0;
        changed = true;
      }
    }
  }
  ReleaseSRWLockExclusive(&g_active_nrsc_peer_lock);
  if (displaced_peer != 0) {
    TraceBhrcPgdPeerChange(displaced_peer, false, true, "active_peer_slot_replaced", _ReturnAddress());
  }
  if (changed || !active) {
    // Capture admission/record evidence before the existing cleanup below.
    TraceBhrcPgdPeerChange(steam_id, active, changed, reason, _ReturnAddress());
  }
  if (changed) {
    Log(
        "BHRC nrsc_active_peer steamId=%016llx active=%d changed=%d reason=%s",
        static_cast<unsigned long long>(steam_id), active ? 1 : 0,
        changed ? 1 : 0, reason != nullptr ? reason : "unknown");
  }
  if (active && changed) {
    MarkBhrcFullPgdSnapshotsForPublish("nrsc_peer_became_active");
  }
  if (!active) {
    ClearBhrcNativeAdmissionForPeer(steam_id, reason);
  }
}


// 7a66e71 src/dllmain.cpp:11256
void ClearNrscActivePeers(const char* reason) {
  uint32_t cleared = 0;
  uint64_t cleared_peers[kObservedPartyMemberSlots]{};
  AcquireSRWLockExclusive(&g_active_nrsc_peer_lock);
  for (uint64_t& peer : g_active_nrsc_peer_steam_ids) {
    if (peer != 0) {
      if (cleared < kObservedPartyMemberSlots) {
        cleared_peers[cleared] = peer;
      }
      ++cleared;
      peer = 0;
    }
  }
  ReleaseSRWLockExclusive(&g_active_nrsc_peer_lock);
  Log(
      "BHRC nrsc_active_peers_cleared count=%u reason=%s",
      cleared, reason != nullptr ? reason : "unknown");
  if (cleared == 0) TraceBhrcPgdPeerChange(0, false, false, reason, _ReturnAddress());
  for (uint32_t i = 0; i < cleared && i < kObservedPartyMemberSlots; ++i) {
    TraceBhrcPgdPeerChange(cleared_peers[i], false, true, reason, _ReturnAddress());
  }
  ClearBhrcNativeAdmissionState(reason);
  const uint32_t rearm_count =
      cleared < kObservedPartyMemberSlots ? cleared : kObservedPartyMemberSlots;
  for (uint32_t i = 0; i < rearm_count; ++i) {
    ClearBhrcNativeAdmissionForPeer(cleared_peers[i], reason);
  }
}


// 7a66e71 src/dllmain.cpp:11701
void __fastcall HookSessionEventBroadcast(
    void* session_mgr,
    uint32_t event_type,
    const void* payload,
    uint32_t payload_size) {
  __try {
    g_session_event_broadcast(session_mgr, event_type, payload, payload_size);
  } __except (LogSehException("BHRC FUN_140df24d0 native event broadcast", GetExceptionInformation())) {
    LogError(
        "BHRC hook_exception function=FUN_140df24d0 eventType=0x%02x sessionMgr=%p payload=%p payloadSize=%u",
        event_type,
        session_mgr,
        payload,
        payload_size);
    return;
  }
  if (event_type == 0x67) {
    CaptureBhrcPendingHostNativeEvent67(
        session_mgr, payload, payload_size,
        "FUN_140df24d0_original_0x67");
  }
  if (event_type == 0x67 && IsLocalNrscHost() &&
      payload_size == kBhrcNativeEvent67PayloadSize) {
    CaptureBhrcHostNativeEvent(
        event_type,
        ResolveBhrcHostLiveSourceRecordKey(),
        payload,
        payload_size,
        "FUN_140df24d0_original_0x67");
  }
}


// 7a66e71 src/dllmain.cpp:11728
uint64_t __fastcall HookSessionEventBroadcastToTargets(
    void* session_mgr,
    void* target_list,
    uint32_t event_type,
    const void* payload,
    uint64_t payload_size) {
  uint64_t result = 0;
  __try {
    result = g_session_event_broadcast_to_targets(
        session_mgr,
        target_list,
        event_type,
        payload,
        payload_size);
  } __except (LogSehException("BHRC FUN_140df2380 native event broadcast targets", GetExceptionInformation())) {
    LogError(
        "BHRC hook_exception function=FUN_140df2380 eventType=0x%02x sessionMgr=%p targets=%p payload=%p payloadSizeRaw=%llu",
        event_type,
        session_mgr,
        target_list,
        payload,
        static_cast<unsigned long long>(payload_size));
    return 0;
  }
  if (event_type == 0x67) {
    CaptureBhrcPendingHostNativeEvent67(
        session_mgr, payload, static_cast<uint32_t>(payload_size),
        "FUN_140df2380_original_0x67");
  }
  if (event_type == 0x67 && IsLocalNrscHost() &&
      payload_size == kBhrcNativeEvent67PayloadSize) {
    CaptureBhrcHostNativeEvent(
        event_type,
        ResolveBhrcHostLiveSourceRecordKey(),
        payload,
        static_cast<uint32_t>(payload_size),
        "FUN_140df2380_original_0x67");
  }
  return result;
}


// 7a66e71 src/dllmain.cpp:11764
int __fastcall HookSessionRecordEventRead(
    void* session_mgr,
    const void* record,
    uint32_t event_type,
    void* out_payload,
    uint32_t payload_size,
    uint8_t pending,
    uint8_t removing) {
  BeginBhrcNameRecordRead(session_mgr, record, event_type, payload_size);
  const uint64_t record_key = record != nullptr
      ? ReadU64(static_cast<const uint8_t*>(record) + 0x10, 0)
      : 0;
  int result = 0;
  __try {
    result = g_session_record_event_read(
        session_mgr,
        record,
        event_type,
        out_payload,
        payload_size,
        pending,
        removing);
  } __except (LogSehException("BHRC FUN_140df6da0 native event read", GetExceptionInformation())) {
    LogError(
        "BHRC native_0c_read_exception eventType=0x%02x sessionMgr=%p record=%p payloadSize=%u pending=%u removing=%u",
        event_type,
        session_mgr,
        record,
        payload_size,
        pending,
        removing);
    return 0;
  }
  const int original_result = result;
  const uint8_t* live_record = record_key != 0
      ? FindSessionRecordByKey(session_mgr, record_key)
      : nullptr;
  const uint8_t live_membership = live_record != nullptr
      ? ReadU8(live_record + 0xe5, 0xff)
      : 0xff;
  const bool native_event_membership_candidate =
      live_membership == 0 ||
      (event_type == 0x0c && IsLocalNrscHost() && live_membership == 1);
  const bool genuine_provider_bound_record =
      live_record != nullptr && live_record == record &&
      ReadU64(live_record + 0x00, 0) != 0 &&
      ReadU64(live_record + 0x08, 0) != 0 &&
      native_event_membership_candidate;
  if (result == 0 && out_payload != nullptr && record_key != 0 &&
      genuine_provider_bound_record &&
      ((event_type == 0x0c &&
        payload_size == kBhrcPlayerStatusPayloadSize) ||
       (event_type == 0x67 &&
        payload_size == kBhrcNativeEvent67PayloadSize))) {
    uint32_t bridged_size = 0;
    if (ConsumeBhrcInboundNativeEvent(
            event_type,
            record_key,
            out_payload,
            payload_size,
            &bridged_size,
            "FUN_140df6da0_original_empty")) {
      result = static_cast<int>(bridged_size);
    }
  }
  if (event_type == 0x0c &&
      result == static_cast<int>(kBhrcPlayerStatusPayloadSize) &&
      out_payload != nullptr &&
      record != nullptr) {
    CaptureBhrcStatusPayloadForRecordKey(record_key, out_payload, "native_0c_read_original");
    // Bridge to validate: apply hook is skipped, so validated remote 0x1f8 is
    // attributed here by record_key/steam id (option B host-relay capture).
    RememberBhrcTlsLast0cRead(record_key, out_payload);
  }
  CaptureBhrcNameRecordRead(session_mgr, record, event_type, out_payload,
      payload_size, original_result, result);
  return result;
}


// 7a66e71 src/dllmain.cpp:17594
void CaptureNrscLobbyScanState(const char* reason, void* context, uint64_t observed_lobby) {
  if (context == nullptr) {
    return;
  }

  auto* ctx = static_cast<uint8_t*>(context);
  void* steam_matchmaking = reinterpret_cast<void*>(ReadU64(ctx - 0xa0, 0));
  void* nrsc_state = reinterpret_cast<void*>(ReadU64(ctx - 0x78, 0));
  const uint64_t stored_lobby =
      nrsc_state != nullptr ? ReadU64(static_cast<uint8_t*>(nrsc_state) + 0x120, 0) : 0;
  const uint64_t owner_steam_id =
      nrsc_state != nullptr ? ReadU64(static_cast<uint8_t*>(nrsc_state) + 0x128, 0) : 0;
  const uint64_t local_steam_id =
      nrsc_state != nullptr ? ReadU64(static_cast<uint8_t*>(nrsc_state) + 0x8, 0) : 0;
  g_nrsc_steam_control_state.scan_context = context;
  if (steam_matchmaking != nullptr) {
    g_nrsc_steam_control_state.steam_matchmaking = steam_matchmaking;
  }
  g_nrsc_steam_control_state.current_master_lobby_id = stored_lobby;
  if (stored_lobby != 0) {
    g_nrsc_steam_control_state.last_master_lobby_id = stored_lobby;
  }
  if (observed_lobby != 0) {
    g_nrsc_steam_control_state.last_observed_lobby_id = observed_lobby;
  }
  if (owner_steam_id != 0) {
    g_nrsc_steam_control_state.owner_steam_id = owner_steam_id;
    if (g_nrsc_steam_control_state.last_owner_steam_id == 0) {
      g_nrsc_steam_control_state.last_owner_steam_id = owner_steam_id;
    }
  } else if (stored_lobby == 0) {
    g_nrsc_steam_control_state.owner_steam_id = 0;
  }
  if (local_steam_id != 0) {
    g_nrsc_steam_control_state.local_steam_id = local_steam_id;
  }
  g_nrsc_steam_control_state.last_seen_ms = GetTickCount();

  PersistNrscSteamControlState(reason);
}


// 7a66e71 src/dllmain.cpp:17635
void CaptureNrscLobbyEventState(const char* reason, void* context, uint64_t lobby_id, uint64_t steam_id) {
  if (context != nullptr) {
    auto* ctx = static_cast<uint8_t*>(context);
    void* nrsc_state = reinterpret_cast<void*>(ReadU64(ctx - 0x88, 0));
    const uint64_t stored_lobby =
        nrsc_state != nullptr ? ReadU64(static_cast<uint8_t*>(nrsc_state) + 0x120, 0) : 0;
    const uint64_t owner_steam_id =
        nrsc_state != nullptr ? ReadU64(static_cast<uint8_t*>(nrsc_state) + 0x128, 0) : 0;
    const uint64_t local_steam_id =
        nrsc_state != nullptr ? ReadU64(static_cast<uint8_t*>(nrsc_state) + 0x8, 0) : 0;
    g_nrsc_steam_control_state.current_master_lobby_id = stored_lobby;
    if (stored_lobby != 0) {
      g_nrsc_steam_control_state.last_master_lobby_id = stored_lobby;
    }
    if (owner_steam_id != 0) {
      g_nrsc_steam_control_state.owner_steam_id = owner_steam_id;
      if (g_nrsc_steam_control_state.last_owner_steam_id == 0) {
        g_nrsc_steam_control_state.last_owner_steam_id = owner_steam_id;
      }
    } else if (stored_lobby == 0) {
      g_nrsc_steam_control_state.owner_steam_id = 0;
    }
    if (local_steam_id != 0) {
      g_nrsc_steam_control_state.local_steam_id = local_steam_id;
    }
  }

  if (lobby_id != 0) {
    g_nrsc_steam_control_state.last_observed_lobby_id = lobby_id;
  }
  g_nrsc_steam_control_state.last_seen_ms = GetTickCount();
  const uint64_t tracked_master_lobby =
      g_nrsc_steam_control_state.current_master_lobby_id;
  if (tracked_master_lobby != 0 &&
      (lobby_id == 0 || lobby_id == tracked_master_lobby)) {
    RefreshBhrcMasterLobbyOwnerFromSteam(tracked_master_lobby, reason);
  }

  PersistNrscSteamControlState(reason);
}


// 7a66e71 src/dllmain.cpp:17682
const char* RevenantFixSteamControlPacketTypeName(uint16_t type) {
  switch (type) {
    case kRevenantFixSteamControlPacketTypeTestPing:
      return "test_ping";
    case kRevenantFixSteamControlPacketTypeTestPong:
      return "test_pong";
    case kRevenantFixSteamControlPacketTypeNativeEvent:
      return "native_event";
    case kRevenantFixSteamControlPacketTypeRecordRosterAnnounce:
      return "record_roster_announce";
    case kRevenantFixSteamControlPacketTypeExpeditionProbeRequest:
      return "expedition_probe_request";
    case kRevenantFixSteamControlPacketTypeExpeditionProbeResponse:
      return "expedition_probe_response";
    case kRevenantFixSteamControlPacketTypeBuddyRecall:
      return "buddy_recall";
    case kRevenantFixSteamControlPacketTypeFullPgdChunk:
      return "full_pgd_chunk";
    case kRevenantFixSteamControlPacketTypeStartupHostPreflightRequest:
      return "startup_host_preflight_request";
    case kRevenantFixSteamControlPacketTypeStartupHostPreflightResponse:
      return "startup_host_preflight_response";
    default:
      return "unknown";
  }
}


// 7a66e71 src/dllmain.cpp:17717
uint64_t GetSavedNrscSteamLobbyId() {
  const BhrcNativeExpeditionAuthorityState native_authority =
      SnapshotNativeExpeditionAuthority();
  if (native_authority.valid && native_authority.lobby_id != 0) {
    return native_authority.lobby_id;
  }
  PersistedReconnectIdentity old_player{};
  LoadPersistedReconnectIdentity(&old_player);
  if (old_player.lobby_id != 0) {
    return old_player.lobby_id;
  }
  if (g_nrsc_steam_control_state.expedition_lobby_id != 0) {
    return g_nrsc_steam_control_state.expedition_lobby_id;
  }
  const uint64_t saved_expedition_lobby = LoadPersistedExpeditionLobbyId();
  if (saved_expedition_lobby != 0) {
    return saved_expedition_lobby;
  }
  return 0;
}


// 7a66e71 src/dllmain.cpp:17738
uint64_t GetSavedNrscMasterLobbyId() {
  return g_nrsc_steam_control_state.last_master_lobby_id != 0
      ? g_nrsc_steam_control_state.last_master_lobby_id
      : LoadPersistedNrscMasterLobbyId();
}


// 7a66e71 src/dllmain.cpp:17744
uint64_t GetCurrentNrscMasterLobbyId() {
  return g_nrsc_steam_control_state.current_master_lobby_id;
}


// 7a66e71 src/dllmain.cpp:17773
RevenantFixSteamPeerTestState* FindRevenantFixSteamPeerTestState(uint64_t steam_id) {
  if (steam_id == 0) {
    return nullptr;
  }
  for (RevenantFixSteamPeerTestState& state : g_revenantfix_steam_test_states) {
    if (state.steam_id == steam_id) {
      return &state;
    }
  }
  return nullptr;
}


// 7a66e71 src/dllmain.cpp:17785
RevenantFixSteamPeerTestState* AllocateRevenantFixSteamPeerTestState(uint64_t steam_id) {
  if (steam_id == 0) {
    return nullptr;
  }
  if (RevenantFixSteamPeerTestState* existing = FindRevenantFixSteamPeerTestState(steam_id)) {
    return existing;
  }

  for (RevenantFixSteamPeerTestState& state : g_revenantfix_steam_test_states) {
    if (state.steam_id == 0) {
      state.steam_id = steam_id;
      return &state;
    }
  }

  RevenantFixSteamPeerTestState* oldest = &g_revenantfix_steam_test_states[0];
  for (RevenantFixSteamPeerTestState& state : g_revenantfix_steam_test_states) {
    if (state.sent_ms < oldest->sent_ms) {
      oldest = &state;
    }
  }
  *oldest = RevenantFixSteamPeerTestState{};
  oldest->steam_id = steam_id;
  return oldest;
}


// 7a66e71 src/dllmain.cpp:17811
uint32_t GenerateRevenantFixSteamTestNonce(uint64_t peer_steam_id) {
  LARGE_INTEGER qpc{};
  QueryPerformanceCounter(&qpc);
  const uint64_t local = g_nrsc_steam_control_state.local_steam_id;
  const uint64_t mixed =
      static_cast<uint64_t>(qpc.QuadPart) ^
      static_cast<uint64_t>(__rdtsc()) ^
      peer_steam_id ^
      (local << 7) ^
      (local >> 9) ^
      static_cast<uint64_t>(GetTickCount());
  return static_cast<uint32_t>(mixed ^ (mixed >> 32));
}


// 7a66e71 src/dllmain.cpp:17825
uint32_t GenerateDistinctRevenantFixSteamTestNonce(
    uint64_t peer_steam_id, uint32_t prior_nonce) {
  uint32_t nonce = GenerateRevenantFixSteamTestNonce(peer_steam_id);
  if (nonce == 0 || nonce == prior_nonce) {
    nonce = prior_nonce + 1;
    if (nonce == 0) {
      nonce = 1;
    }
  }
  return nonce;
}


// 7a66e71 src/dllmain.cpp:17837
void FillSteamNetworkingIdentitySteamId(SteamNetworkingIdentityLite* identity, uint64_t steam_id) {
  if (identity == nullptr) {
    return;
  }
  *identity = SteamNetworkingIdentityLite{};
  identity->type = kSteamNetworkingIdentityTypeSteamID;
  identity->size = sizeof(steam_id);
  std::memcpy(identity->data, &steam_id, sizeof(steam_id));
}


// 7a66e71 src/dllmain.cpp:17847
uint64_t ReadSteamNetworkingIdentitySteamId(const SteamNetworkingIdentityLite& identity) {
  if (identity.type != kSteamNetworkingIdentityTypeSteamID || identity.size < static_cast<int>(sizeof(uint64_t))) {
    return 0;
  }
  uint64_t steam_id = 0;
  std::memcpy(&steam_id, identity.data, sizeof(steam_id));
  return steam_id;
}


// 7a66e71 src/dllmain.cpp:17856
bool EnsureBhrcStartupP2PCallbackRegistered(const char* reason) {
  const LONG state =
      InterlockedCompareExchange(&g_bhrc_startup_p2p_callback_state, 0, 0);
  if (state == 2) {
    return true;
  }
  if (g_bhrc_startup_p2p == nullptr ||
      g_bhrc_startup_p2p_accept == nullptr ||
      g_bhrc_steam_api_register_callback == nullptr) {
    return false;
  }
  if (InterlockedCompareExchange(
          &g_bhrc_startup_p2p_callback_state, 1, 0) != 0) {
    return InterlockedCompareExchange(
               &g_bhrc_startup_p2p_callback_state, 0, 0) == 2;
  }

  bool registered = false;
  g_bhrc_startup_p2p_session_request_callback.PrepareForRegistration(
      kSteamP2PSessionRequestCallbackId);
  __try {
    g_bhrc_steam_api_register_callback(
        &g_bhrc_startup_p2p_session_request_callback,
        kSteamP2PSessionRequestCallbackId);
    registered = true;
  } __except (LogSehException(
      "BHRC legacy P2P callback registration",
      GetExceptionInformation())) {
    registered = false;
  }
  InterlockedExchange(
      &g_bhrc_startup_p2p_callback_state, registered ? 2 : 0);
  if (registered) {
    Log(
        "BHRC startup_host_preflight edge=p2p_callback_registered callbackId=%d channel=%d transport=steam_legacy_p2p nrscSessionMutation=0 requestReason=%s",
        kSteamP2PSessionRequestCallbackId, kRevenantFixStartupP2pChannel,
        reason != nullptr ? reason : "unknown");
  } else {
    LogError(
        "BHRC startup_host_preflight edge=p2p_callback_register_failed callbackId=%d channel=%d transport=steam_legacy_p2p requestReason=%s",
        kSteamP2PSessionRequestCallbackId, kRevenantFixStartupP2pChannel,
        reason != nullptr ? reason : "unknown");
  }
  return registered;
}


// 7a66e71 src/dllmain.cpp:17902
bool ResolveBhrcStartupP2PTransport(const char* reason) {
  if (g_bhrc_startup_p2p != nullptr &&
      g_bhrc_startup_p2p_send != nullptr &&
      g_bhrc_startup_p2p_is_available != nullptr &&
      g_bhrc_startup_p2p_read != nullptr &&
      g_bhrc_startup_p2p_accept != nullptr &&
      EnsureBhrcStartupP2PCallbackRegistered(reason)) {
    return true;
  }

  HMODULE steam_api = GetModuleHandleW(L"steam_api64.dll");
  if (steam_api == nullptr) {
    const DWORD now = GetTickCount();
    if (g_bhrc_startup_p2p_last_failure_log_ms == 0 ||
        now - g_bhrc_startup_p2p_last_failure_log_ms > 10000) {
      g_bhrc_startup_p2p_last_failure_log_ms = now;
      LogWarn(
          "BHRC startup_host_preflight edge=p2p_resolve_failed reason=no_steam_api64 requestReason=%s",
          reason != nullptr ? reason : "unknown");
    }
    return false;
  }

  auto* get_networking = reinterpret_cast<SteamApiNetworkingFn>(
      GetProcAddress(steam_api, "SteamAPI_SteamNetworking_v006"));
  g_bhrc_startup_p2p_send =
      reinterpret_cast<SteamNetworkingSendP2PPacketFn>(
          GetProcAddress(
              steam_api, "SteamAPI_ISteamNetworking_SendP2PPacket"));
  g_bhrc_startup_p2p_is_available =
      reinterpret_cast<SteamNetworkingIsP2PPacketAvailableFn>(
          GetProcAddress(
              steam_api,
              "SteamAPI_ISteamNetworking_IsP2PPacketAvailable"));
  g_bhrc_startup_p2p_read =
      reinterpret_cast<SteamNetworkingReadP2PPacketFn>(
          GetProcAddress(
              steam_api, "SteamAPI_ISteamNetworking_ReadP2PPacket"));
  g_bhrc_startup_p2p_accept =
      reinterpret_cast<SteamNetworkingAcceptP2PSessionWithUserFn>(
          GetProcAddress(
              steam_api,
              "SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser"));
  g_bhrc_steam_api_register_callback =
      reinterpret_cast<SteamApiRegisterCallbackFn>(
          GetProcAddress(steam_api, "SteamAPI_RegisterCallback"));

  void* networking = nullptr;
  if (get_networking != nullptr) {
    __try {
      networking = get_networking();
    } __except (LogSehException(
        "BHRC SteamAPI_SteamNetworking_v006",
        GetExceptionInformation())) {
      networking = nullptr;
    }
  }
  g_bhrc_startup_p2p = networking;
  const bool resolved =
      networking != nullptr &&
      g_bhrc_startup_p2p_send != nullptr &&
      g_bhrc_startup_p2p_is_available != nullptr &&
      g_bhrc_startup_p2p_read != nullptr &&
      g_bhrc_startup_p2p_accept != nullptr &&
      g_bhrc_steam_api_register_callback != nullptr;
  if (!resolved) {
    const DWORD now = GetTickCount();
    if (g_bhrc_startup_p2p_last_failure_log_ms == 0 ||
        now - g_bhrc_startup_p2p_last_failure_log_ms > 10000) {
      g_bhrc_startup_p2p_last_failure_log_ms = now;
      LogWarn(
          "BHRC startup_host_preflight edge=p2p_resolve_failed reason=missing_legacy_p2p_export interface=%p send=%p available=%p read=%p accept=%p registerCallback=%p requestReason=%s",
          networking,
          reinterpret_cast<void*>(g_bhrc_startup_p2p_send),
          reinterpret_cast<void*>(g_bhrc_startup_p2p_is_available),
          reinterpret_cast<void*>(g_bhrc_startup_p2p_read),
          reinterpret_cast<void*>(g_bhrc_startup_p2p_accept),
          reinterpret_cast<void*>(g_bhrc_steam_api_register_callback),
          reason != nullptr ? reason : "unknown");
    }
    return false;
  }
  if (!EnsureBhrcStartupP2PCallbackRegistered(reason)) {
    return false;
  }
  if (!g_bhrc_startup_p2p_resolved_logged) {
    g_bhrc_startup_p2p_resolved_logged = true;
    Log(
        "BHRC startup_host_preflight edge=p2p_transport_ready interface=%p send=%p read=%p accept=%p channel=%d transport=steam_legacy_p2p sessionOwner=revenantfix nrscMessagesTouched=0 requestReason=%s",
        networking,
        reinterpret_cast<void*>(g_bhrc_startup_p2p_send),
        reinterpret_cast<void*>(g_bhrc_startup_p2p_read),
        reinterpret_cast<void*>(g_bhrc_startup_p2p_accept),
        kRevenantFixStartupP2pChannel,
        reason != nullptr ? reason : "unknown");
  }
  return true;
}


// 7a66e71 src/dllmain.cpp:18001
void HandleBhrcStartupP2PSessionRequest(uint64_t remote_steam_id) {
  const uint64_t local_steam_id =
      g_nrsc_steam_control_state.local_steam_id;
  if (!g_config.enable_nrsc_steam_control ||
      remote_steam_id == 0 || local_steam_id == 0 ||
      remote_steam_id == local_steam_id || !IsLocalNrscHost() ||
      g_bhrc_startup_p2p == nullptr ||
      g_bhrc_startup_p2p_accept == nullptr) {
    return;
  }

  BhrcSteamRecordCacheEntry cached{};
  const BhrcNativeExpeditionAuthorityState authority =
      SnapshotNativeExpeditionAuthority();
  const bool known_cached_member =
      SnapshotCachedReconnectRecordBySteamId(remote_steam_id, &cached) &&
      cached.steam_id == remote_steam_id &&
      (cached.record_key == 0 || cached.record_key == remote_steam_id);
  const bool retained_member = authority.valid &&
      HasBhrcReturningMemberRetentionCredential(
          remote_steam_id, remote_steam_id, authority);
  if (!known_cached_member && !retained_member) {
    LogWarn(
        "BHRC startup_host_preflight edge=p2p_session_rejected remote=%016llx local=%016llx reason=unknown_previous_member cacheFound=0 retention=%d transport=steam_legacy_p2p nrscSessionMutation=0",
        static_cast<unsigned long long>(remote_steam_id),
        static_cast<unsigned long long>(local_steam_id),
        retained_member ? 1 : 0);
    return;
  }

  bool accepted = false;
  __try {
    accepted = g_bhrc_startup_p2p_accept(
        g_bhrc_startup_p2p, remote_steam_id);
  } __except (LogSehException(
      "BHRC AcceptP2PSessionWithUser",
      GetExceptionInformation())) {
    accepted = false;
  }
  Log(
      "BHRC startup_host_preflight edge=p2p_session_accept remote=%016llx local=%016llx accepted=%d cacheFound=%d retention=%d channel=%d transport=steam_legacy_p2p nrscSessionMutation=0",
      static_cast<unsigned long long>(remote_steam_id),
      static_cast<unsigned long long>(local_steam_id),
      accepted ? 1 : 0, known_cached_member ? 1 : 0,
      retained_member ? 1 : 0, kRevenantFixStartupP2pChannel);
}


// 7a66e71 src/dllmain.cpp:18048
bool TrySendBhrcStartupP2PPacket(
    uint64_t target_steam_id,
    const void* payload,
    uint32_t payload_size,
    uint16_t expected_type,
    const char* reason) {
  if (!g_config.enable_nrsc_steam_control ||
      target_steam_id == 0 || payload == nullptr ||
      payload_size < sizeof(RevenantFixSteamControlPacket) ||
      payload_size > kRevenantFixStartupP2pMaxPacketSize) {
    return false;
  }
  const auto* packet =
      static_cast<const RevenantFixSteamControlPacket*>(payload);
  if (packet->magic != kRevenantFixSteamControlPacketMagic ||
      packet->version != kRevenantFixSteamControlPacketVersion ||
      packet->type != expected_type ||
      packet->target_steam_id != target_steam_id ||
      (expected_type !=
           kRevenantFixSteamControlPacketTypeStartupHostPreflightRequest &&
       expected_type !=
           kRevenantFixSteamControlPacketTypeStartupHostPreflightResponse)) {
    return false;
  }
  if (!ResolveBhrcStartupP2PTransport(reason)) {
    return false;
  }

  bool sent = false;
  __try {
    sent = g_bhrc_startup_p2p_send(
        g_bhrc_startup_p2p, target_steam_id, payload, payload_size,
        kRevenantFixStartupP2pSendReliable, kRevenantFixStartupP2pChannel);
  } __except (LogSehException(
      "BHRC SendP2PPacket startup preflight",
      GetExceptionInformation())) {
    sent = false;
  }
  return sent;
}


// 7a66e71 src/dllmain.cpp:18089
bool ResolveSteamNetworkingMessagesInterface(const char* reason) {
  if (g_steam_networking_messages != nullptr &&
      g_steam_messages_send != nullptr &&
      g_steam_messages_receive != nullptr) {
    return true;
  }

  HMODULE steam_api = GetModuleHandleW(L"steam_api64.dll");
  if (steam_api == nullptr) {
    const DWORD now = GetTickCount();
    if (g_steam_networking_messages_last_failure_log_ms == 0 ||
        now - g_steam_networking_messages_last_failure_log_ms > 10000) {
      g_steam_networking_messages_last_failure_log_ms = now;
      LogWarn(
          "RevenantFixSteamControl resolve_failed reason=no_steam_api64 requestReason=%s",
          reason != nullptr ? reason : "unknown");
    }
    return false;
  }

  g_steam_messages_send = reinterpret_cast<SteamNetworkingMessagesSendMessageToUserFn>(
      GetProcAddress(steam_api, "SteamAPI_ISteamNetworkingMessages_SendMessageToUser"));
  g_steam_messages_receive = reinterpret_cast<SteamNetworkingMessagesReceiveMessagesOnChannelFn>(
      GetProcAddress(steam_api, "SteamAPI_ISteamNetworkingMessages_ReceiveMessagesOnChannel"));
  constexpr const char* kDirectExports[] = {
      "SteamAPI_SteamNetworkingMessages_SteamAPI_v002",
      "SteamAPI_SteamNetworkingMessages_SteamAPI_v001",
      "SteamAPI_SteamNetworkingMessages_v002",
      "SteamAPI_SteamNetworkingMessages_v001",
      "SteamAPI_SteamNetworkingMessages",
  };
  for (const char* name : kDirectExports) {
    auto* fn = reinterpret_cast<SteamApiNetworkingMessagesFn>(GetProcAddress(steam_api, name));
    if (fn == nullptr) {
      continue;
    }

    void* messages = nullptr;
    __try {
      messages = fn();
    } __except (LogSehException("SteamAPI_SteamNetworkingMessages export", GetExceptionInformation())) {
      messages = nullptr;
    }

    if (messages != nullptr) {
      g_steam_networking_messages = messages;
      if (!g_steam_networking_messages_resolved_logged) {
        g_steam_networking_messages_resolved_logged = true;
        Log(
            "RevenantFixSteamControl resolved source=%s interface=%p send=%p recv=%p sessionOwner=nrsc requestReason=%s",
            name,
            messages,
            reinterpret_cast<void*>(g_steam_messages_send),
            reinterpret_cast<void*>(g_steam_messages_receive),
            reason != nullptr ? reason : "unknown");
      }
      return g_steam_messages_send != nullptr && g_steam_messages_receive != nullptr;
    }
  }

  auto* get_user = reinterpret_cast<SteamApiGetHSteamUserFn>(
      GetProcAddress(steam_api, "SteamAPI_GetHSteamUser"));
  auto* find_user_interface = reinterpret_cast<SteamInternalFindOrCreateUserInterfaceFn>(
      GetProcAddress(steam_api, "SteamInternal_FindOrCreateUserInterface"));
  if (get_user != nullptr && find_user_interface != nullptr) {
    int hsteam_user = 0;
    __try {
      hsteam_user = get_user();
    } __except (LogSehException("SteamAPI_GetHSteamUser SteamNetworkingMessages", GetExceptionInformation())) {
      hsteam_user = 0;
    }

    constexpr const char* kInterfaceVersions[] = {
        "SteamNetworkingMessages002",
        "SteamNetworkingMessages001",
    };
    for (const char* version : kInterfaceVersions) {
      void* messages = nullptr;
      __try {
        messages = find_user_interface(hsteam_user, version);
      } __except (LogSehException("SteamInternal_FindOrCreateUserInterface SteamNetworkingMessages", GetExceptionInformation())) {
        messages = nullptr;
      }

      if (messages != nullptr) {
        g_steam_networking_messages = messages;
        if (!g_steam_networking_messages_resolved_logged) {
          g_steam_networking_messages_resolved_logged = true;
          Log(
              "RevenantFixSteamControl resolved source=SteamInternal_FindOrCreateUserInterface version=%s hsteamUser=%d interface=%p send=%p recv=%p sessionOwner=nrsc requestReason=%s",
              version,
              hsteam_user,
              messages,
              reinterpret_cast<void*>(g_steam_messages_send),
              reinterpret_cast<void*>(g_steam_messages_receive),
              reason != nullptr ? reason : "unknown");
        }
        return g_steam_messages_send != nullptr && g_steam_messages_receive != nullptr;
      }
    }
  }

  const DWORD now = GetTickCount();
  if (g_steam_networking_messages_last_failure_log_ms == 0 ||
      now - g_steam_networking_messages_last_failure_log_ms > 10000) {
    g_steam_networking_messages_last_failure_log_ms = now;
    LogWarn(
        "RevenantFixSteamControl resolve_failed reason=no_interface requestReason=%s steamApi=%p send=%p recv=%p getUser=%p findUserInterface=%p",
        reason != nullptr ? reason : "unknown",
        steam_api,
        reinterpret_cast<void*>(g_steam_messages_send),
        reinterpret_cast<void*>(g_steam_messages_receive),
        reinterpret_cast<void*>(get_user),
        reinterpret_cast<void*>(find_user_interface));
  }
  return false;
}


// 7a66e71 src/dllmain.cpp:18207
bool TrySendRevenantFixSteamPacketRaw(
    uint64_t target_steam_id,
    const void* payload,
    uint32_t payload_size,
    uint16_t type,
    const char* reason,
    bool allow_inactive_authorized_restore,
    RevenantFixSteamSendTrace* send_trace) {
  if (send_trace != nullptr) {
    *send_trace = RevenantFixSteamSendTrace{};
  }
  if (!g_config.enable_nrsc_steam_control) {
    if (send_trace != nullptr) send_trace->failure_reason = "steam_control_disabled";
    return false;
  }
  const LONG nrsc_bridge_state =
      InterlockedCompareExchange(&g_nrsc_bridge_install_state, 0, 0);
  if (nrsc_bridge_state != 2) {
    if (send_trace != nullptr) send_trace->failure_reason = "nrsc_bridge_not_ready";
    LogWarn(
        "RevenantFixSteamControl send_skipped reason=nrsc_bridge_not_ready requestReason=%s targetSteamId=%016llx type=%s bridgeState=%ld",
        reason != nullptr ? reason : "unknown",
        static_cast<unsigned long long>(target_steam_id),
        RevenantFixSteamControlPacketTypeName(type), nrsc_bridge_state);
    return false;
  }
  if (target_steam_id == 0 || target_steam_id == g_nrsc_steam_control_state.local_steam_id) {
    if (send_trace != nullptr) send_trace->failure_reason = "invalid_target";
    return false;
  }
  if (payload == nullptr || payload_size < sizeof(RevenantFixSteamControlPacket)) {
    if (send_trace != nullptr) send_trace->failure_reason = "invalid_payload";
    return false;
  }
  const auto* control_header =
      static_cast<const RevenantFixSteamControlPacket*>(payload);
  const bool peer_active = IsNrscPeerActive(target_steam_id);
  const bool preactive_probe_response = !peer_active &&
      type == kRevenantFixSteamControlPacketTypeExpeditionProbeResponse &&
      payload_size >= sizeof(RevenantFixExpeditionProbeResponsePacket) &&
      control_header->magic == kRevenantFixSteamControlPacketMagic &&
      control_header->version == kRevenantFixSteamControlPacketVersion &&
      control_header->type == type && IsLocalNrscHost() &&
      control_header->sender_steam_id ==
          g_nrsc_steam_control_state.local_steam_id &&
      control_header->target_steam_id == target_steam_id &&
      control_header->lobby_id != 0 &&
      control_header->lobby_id == GetCurrentNrscMasterLobbyId();
  const auto* full_pgd =
      type == kRevenantFixSteamControlPacketTypeFullPgdChunk &&
              payload_size >= offsetof(RevenantFixFullPgdChunkPacket, payload)
          ? static_cast<const RevenantFixFullPgdChunkPacket*>(payload)
          : nullptr;
  const bool preactive_authorized_restore = !peer_active &&
      allow_inactive_authorized_restore && full_pgd != nullptr &&
      control_header->magic == kRevenantFixSteamControlPacketMagic &&
      control_header->version == kRevenantFixSteamControlPacketVersion &&
      control_header->type == type && IsLocalNrscHost() &&
      control_header->sender_steam_id ==
          g_nrsc_steam_control_state.local_steam_id &&
      control_header->target_steam_id == target_steam_id &&
      control_header->lobby_id != 0 &&
      control_header->lobby_id == GetCurrentNrscMasterLobbyId() &&
      control_header->flags == static_cast<uint32_t>(
          BhrcFullPgdTransferPurpose::RestoreDelivery) &&
      full_pgd->purpose == static_cast<uint16_t>(
          BhrcFullPgdTransferPurpose::RestoreDelivery) &&
      full_pgd->owner_steam_id == target_steam_id &&
      full_pgd->master_lobby_id == control_header->lobby_id &&
      full_pgd->expedition_lobby_id != 0 &&
      full_pgd->expedition_lobby_id == GetSavedNrscSteamLobbyId() &&
      full_pgd->authority_commit_seq != 0 &&
      full_pgd->request_nonce != 0 && full_pgd->client_generation != 0 &&
      full_pgd->snapshot_revision != 0 && full_pgd->total_size != 0 &&
      full_pgd->total_hash32 != 0 && full_pgd->chunk_count != 0 &&
      full_pgd->chunk_index < full_pgd->chunk_count;
  if (!peer_active && !preactive_probe_response &&
      !preactive_authorized_restore) {
    if (send_trace != nullptr) send_trace->failure_reason = "nrsc_peer_inactive";
    LogWarn(
        "RevenantFixSteamControl send_skipped reason=nrsc_peer_inactive requestReason=%s targetSteamId=%016llx type=%s currentMasterA=%016llx expeditionB=%016llx",
        reason != nullptr ? reason : "unknown",
        static_cast<unsigned long long>(target_steam_id),
        RevenantFixSteamControlPacketTypeName(type),
        static_cast<unsigned long long>(GetCurrentNrscMasterLobbyId()),
        static_cast<unsigned long long>(GetSavedNrscSteamLobbyId()));
    return false;
  }
  if (preactive_probe_response) {
    Log(
        "BHRC expedition_probe_transport outcome=preactive_response_allowed targetSteamId=%016llx masterA=%016llx type=%s authority=local_nrsc_host exactTarget=1",
        static_cast<unsigned long long>(target_steam_id),
        static_cast<unsigned long long>(control_header->lobby_id),
        RevenantFixSteamControlPacketTypeName(type));
  }
  if (preactive_authorized_restore && full_pgd->chunk_index == 0) {
    Log(
        "BHRC full_pgd_transport outcome=preactive_authorized_restore_allowed target=%016llx owner=%016llx masterA=%016llx expeditionB=%016llx authoritySeq=%llu nonce=%08x clientGeneration=%u revision=%llu chunks=%u purpose=restore_delivery admissionLeaseValidated=1 peerActive=0",
        static_cast<unsigned long long>(target_steam_id),
        static_cast<unsigned long long>(full_pgd->owner_steam_id),
        static_cast<unsigned long long>(full_pgd->master_lobby_id),
        static_cast<unsigned long long>(full_pgd->expedition_lobby_id),
        static_cast<unsigned long long>(
            full_pgd->authority_commit_seq),
        full_pgd->request_nonce, full_pgd->client_generation,
        static_cast<unsigned long long>(
            full_pgd->snapshot_revision),
        static_cast<unsigned>(full_pgd->chunk_count));
  }
  if (!ResolveSteamNetworkingMessagesInterface(reason)) {
    if (send_trace != nullptr) send_trace->failure_reason = "missing_steam_networking_messages";
    LogWarn(
        "RevenantFixSteamControl send_skipped reason=missing_steam_networking_messages requestReason=%s targetSteamId=%016llx type=%s interface=%p send=%p recv=%p",
        reason != nullptr ? reason : "unknown",
        static_cast<unsigned long long>(target_steam_id),
        RevenantFixSteamControlPacketTypeName(type),
        g_steam_networking_messages,
        reinterpret_cast<void*>(g_steam_messages_send),
        reinterpret_cast<void*>(g_steam_messages_receive));
    return false;
  }

  SteamNetworkingIdentityLite identity{};
  FillSteamNetworkingIdentitySteamId(&identity, target_steam_id);
  int result = 0;
  __try {
    if (send_trace != nullptr) send_trace->steam_called = true;
    result = g_steam_messages_send(
        g_steam_networking_messages,
        &identity,
        payload,
        payload_size,
        kRevenantFixSteamSendReliable,
        kRevenantFixSteamControlChannel);
    if (send_trace != nullptr) {
      send_trace->steam_result = result;
      send_trace->steam_result_valid = true;
      send_trace->failure_reason = result == 1 ? "none" : "steam_send_result";
    }
  } __except (LogSehException("RevenantFixSteamControl SendMessageToUser", GetExceptionInformation())) {
    if (send_trace != nullptr) send_trace->failure_reason = "steam_send_exception";
    LogError(
        "RevenantFixSteamControl send_exception requestReason=%s targetSteamId=%016llx type=%s interface=%p channel=%d bytes=%u",
        reason != nullptr ? reason : "unknown",
        static_cast<unsigned long long>(target_steam_id),
        RevenantFixSteamControlPacketTypeName(type),
        g_steam_networking_messages,
        kRevenantFixSteamControlChannel,
        payload_size);
    return false;
  }

  if (type == kRevenantFixSteamControlPacketTypeTestPing ||
      type == kRevenantFixSteamControlPacketTypeTestPong) {
    const auto* packet =
        static_cast<const RevenantFixSteamControlPacket*>(payload);
    Log(
        "RevenantFixSteamControl sent requestReason=%s targetSteamId=%016llx type=%s result=%d lobbyId=%016llx senderSteamId=%016llx channel=%d bytes=%u transport=steam_networking_messages",
        reason != nullptr ? reason : "unknown",
        static_cast<unsigned long long>(target_steam_id),
        RevenantFixSteamControlPacketTypeName(type),
        result,
        static_cast<unsigned long long>(packet->lobby_id),
        static_cast<unsigned long long>(packet->sender_steam_id),
        kRevenantFixSteamControlChannel,
        payload_size);
  }
  return result == 1;
}


// 7a66e71 src/dllmain.cpp:18295
bool BroadcastBuddyRecallPacket(
    void* state,
    int buddy_slot,
    const SlotSnapshot& recalled_snapshot,
    const SlotSnapshot& authority_after,
    const char* reason) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) || IsEmptyEntity(recalled_snapshot)) {
    return false;
  }

  bool authority_known = false;
  const bool local_authority = IsLocalBuddyAuthority(state, &authority_known);
  if (!authority_known || !local_authority) {
    Log(
        "BuddyRecallGlue broadcast_skipped reason=not_local_authority source=%s authority=%d/%d slot=%d entity=%08x:%08x",
        reason != nullptr ? reason : "unknown",
        authority_known ? 1 : 0,
        local_authority ? 1 : 0,
        buddy_slot,
        recalled_snapshot.entity_high,
        recalled_snapshot.entity_low);
    return false;
  }

  const EntityId owner_entity = ReadStateOwnerEntityId(state);
  const EntityId buddy_entity{recalled_snapshot.entity_low, recalled_snapshot.entity_high};
  const uint64_t local_steam_id = g_nrsc_steam_control_state.local_steam_id;
  const uint64_t lobby_id = GetSavedNrscSteamLobbyId() != 0
      ? GetSavedNrscSteamLobbyId()
      : GetCurrentNrscMasterLobbyId();
  if (IsEmptyEntityId(owner_entity) || IsEmptyEntityId(buddy_entity) ||
      local_steam_id == 0 || lobby_id == 0) {
    LogWarn(
        "BuddyRecallGlue broadcast_skipped reason=missing_identity source=%s local=%016llx lobby=%016llx owner=%08x:%08x slot=%d entity=%08x:%08x",
        reason != nullptr ? reason : "unknown",
        static_cast<unsigned long long>(local_steam_id),
        static_cast<unsigned long long>(lobby_id),
        owner_entity.high,
        owner_entity.low,
        buddy_slot,
        buddy_entity.high,
        buddy_entity.low);
    return false;
  }

  LONG sequence_value = InterlockedIncrement(&g_buddy_recall_packet_sequence);
  if (sequence_value == 0) {
    sequence_value = InterlockedIncrement(&g_buddy_recall_packet_sequence);
  }
  RevenantFixBuddyRecallPacket packet{};
  packet.header.type = kRevenantFixSteamControlPacketTypeBuddyRecall;
  packet.header.sender_steam_id = local_steam_id;
  packet.header.lobby_id = lobby_id;
  packet.sequence = static_cast<uint32_t>(sequence_value);
  packet.owner_entity_low = owner_entity.low;
  packet.owner_entity_high = owner_entity.high;
  packet.buddy_entity_low = buddy_entity.low;
  packet.buddy_entity_high = buddy_entity.high;
  packet.buddy_slot = buddy_slot;
  packet.reason = kBuddyRecallReasonAutoDistance;
  packet.authority_state = authority_after.state;
  packet.authority_pending = authority_after.pending;
  packet.authority_flags = authority_after.flags;

  uint64_t active_peers[_countof(g_active_nrsc_peer_steam_ids)]{};
  AcquireSRWLockShared(&g_active_nrsc_peer_lock);
  std::memcpy(active_peers, g_active_nrsc_peer_steam_ids, sizeof(active_peers));
  ReleaseSRWLockShared(&g_active_nrsc_peer_lock);

  uint32_t attempted = 0;
  uint32_t sent = 0;
  for (uint64_t peer : active_peers) {
    if (peer == 0 || peer == local_steam_id) {
      continue;
    }
    ++attempted;
    packet.header.target_steam_id = peer;
    if (TrySendRevenantFixSteamPacketRaw(
            peer,
            &packet,
            static_cast<uint32_t>(sizeof(packet)),
            packet.header.type,
            "buddy_auto_distance_recall", false)) {
      ++sent;
    }
  }
  Log(
      "BuddyRecallGlue broadcast source=%s sequence=%u owner=%08x:%08x slot=%d entity=%08x:%08x authoritySlot=[%02x,%02x,%02x] attempted=%u sent=%u lobby=%016llx",
      reason != nullptr ? reason : "unknown",
      packet.sequence,
      owner_entity.high,
      owner_entity.low,
      buddy_slot,
      buddy_entity.high,
      buddy_entity.low,
      packet.authority_state,
      packet.authority_pending,
      packet.authority_flags,
      attempted,
      sent,
      static_cast<unsigned long long>(lobby_id));
  return sent != 0;
}


// 7a66e71 src/dllmain.cpp:18399
bool EnqueueBuddyRecallPacket(
    uint64_t sender_steam_id,
    const RevenantFixBuddyRecallPacket& packet) {
  const EntityId owner_entity{packet.owner_entity_low, packet.owner_entity_high};
  const EntityId buddy_entity{packet.buddy_entity_low, packet.buddy_entity_high};
  if (sender_steam_id == 0 || packet.sequence == 0 ||
      !IsValidBuddySlot(packet.buddy_slot) ||
      packet.reason != kBuddyRecallReasonAutoDistance ||
      IsEmptyEntityId(owner_entity) || IsEmptyEntityId(buddy_entity)) {
    LogWarn(
        "BuddyRecallGlue recv_rejected reason=malformed sender=%016llx sequence=%u owner=%08x:%08x slot=%d entity=%08x:%08x recallReason=%u",
        static_cast<unsigned long long>(sender_steam_id),
        packet.sequence,
        owner_entity.high,
        owner_entity.low,
        packet.buddy_slot,
        buddy_entity.high,
        buddy_entity.low,
        packet.reason);
    return true;
  }

  const uint64_t expedition_lobby = GetSavedNrscSteamLobbyId();
  const uint64_t master_lobby = GetCurrentNrscMasterLobbyId();
  if (packet.header.lobby_id != 0 && (expedition_lobby != 0 || master_lobby != 0) &&
      packet.header.lobby_id != expedition_lobby &&
      packet.header.lobby_id != master_lobby) {
    LogWarn(
        "BuddyRecallGlue recv_rejected reason=lobby_mismatch sender=%016llx sequence=%u packetLobby=%016llx expeditionB=%016llx masterA=%016llx owner=%08x:%08x slot=%d entity=%08x:%08x",
        static_cast<unsigned long long>(sender_steam_id),
        packet.sequence,
        static_cast<unsigned long long>(packet.header.lobby_id),
        static_cast<unsigned long long>(expedition_lobby),
        static_cast<unsigned long long>(master_lobby),
        owner_entity.high,
        owner_entity.low,
        packet.buddy_slot,
        buddy_entity.high,
        buddy_entity.low);
    return true;
  }

  const DWORD now = GetTickCount();
  size_t target_index = _countof(g_pending_buddy_recall_commands);
  size_t oldest_index = 0;
  DWORD oldest_age = 0;
  bool duplicate = false;
  bool replaced = false;
  AcquireSRWLockExclusive(&g_pending_buddy_recall_lock);
  for (size_t i = 0; i < _countof(g_pending_buddy_recall_commands); ++i) {
    auto& queued = g_pending_buddy_recall_commands[i];
    if (queued.active && queued.sender_steam_id == sender_steam_id &&
        queued.sequence == packet.sequence) {
      duplicate = true;
      target_index = i;
      break;
    }
    if (queued.active && now - queued.received_ms > kBuddyRecallCommandLifetimeMs) {
      queued = PendingBuddyRecallCommand{};
    }
    if (!queued.active && target_index == _countof(g_pending_buddy_recall_commands)) {
      target_index = i;
    }
    if (queued.active) {
      const DWORD age = now - queued.received_ms;
      if (age >= oldest_age) {
        oldest_age = age;
        oldest_index = i;
      }
    }
  }
  if (!duplicate) {
    if (target_index == _countof(g_pending_buddy_recall_commands)) {
      target_index = oldest_index;
      replaced = g_pending_buddy_recall_commands[target_index].active;
    }
    PendingBuddyRecallCommand& queued = g_pending_buddy_recall_commands[target_index];
    queued.active = true;
    queued.sender_steam_id = sender_steam_id;
    queued.lobby_id = packet.header.lobby_id;
    queued.sequence = packet.sequence;
    queued.owner_entity = owner_entity;
    queued.buddy_entity = buddy_entity;
    queued.buddy_slot = packet.buddy_slot;
    queued.reason = packet.reason;
    queued.authority_state = packet.authority_state;
    queued.authority_pending = packet.authority_pending;
    queued.authority_flags = packet.authority_flags;
    queued.received_ms = now;
  }
  ReleaseSRWLockExclusive(&g_pending_buddy_recall_lock);

  Log(
      "BuddyRecallGlue recv_%s sender=%016llx sequence=%u owner=%08x:%08x slot=%d entity=%08x:%08x authoritySlot=[%02x,%02x,%02x] packetLobby=%016llx queueIndex=%llu replaced=%d",
      duplicate ? "duplicate" : "queued",
      static_cast<unsigned long long>(sender_steam_id),
      packet.sequence,
      owner_entity.high,
      owner_entity.low,
      packet.buddy_slot,
      buddy_entity.high,
      buddy_entity.low,
      packet.authority_state,
      packet.authority_pending,
      packet.authority_flags,
      static_cast<unsigned long long>(packet.header.lobby_id),
      static_cast<unsigned long long>(target_index),
      replaced ? 1 : 0);
  return true;
}


// 7a66e71 src/dllmain.cpp:27921
bool TrySendRevenantFixSteamTestPacket(
    uint64_t target_steam_id,
    uint16_t type,
    uint32_t nonce,
    const char* text,
    const char* reason) {
  if (type != kRevenantFixSteamControlPacketTypeTestPing && type != kRevenantFixSteamControlPacketTypeTestPong) {
    return false;
  }
  RevenantFixSteamTestPacket packet{};
  packet.header.type = type;
  packet.header.sender_steam_id = g_nrsc_steam_control_state.local_steam_id;
  packet.header.lobby_id = GetSavedNrscSteamLobbyId();
  packet.header.target_steam_id = target_steam_id;
  packet.header.flags = IsLocalNrscHost() ? 1u : 0u;
  packet.nonce = nonce;
  std::snprintf(packet.text, sizeof(packet.text), "%s", text != nullptr ? text : "");

  const bool ok = TrySendRevenantFixSteamPacketRaw(
      target_steam_id,
      &packet,
      static_cast<uint32_t>(sizeof(packet)),
      type,
      reason, false);
  Log(
      "RevenantFixSteamControl %s_sent requestReason=%s targetSteamId=%016llx nonce=%08x text=\"%s\" ok=%d",
      RevenantFixSteamControlPacketTypeName(type),
      reason != nullptr ? reason : "unknown",
      static_cast<unsigned long long>(target_steam_id),
      nonce,
      packet.text,
      ok ? 1 : 0);
  return ok;
}


// 7a66e71 src/dllmain.cpp:27956
void MaybeSendRevenantFixSteamTestPing(uint64_t target_steam_id, const char* reason) {
  if (IsLocalNrscHost()) {
    return;
  }
  if (target_steam_id == 0 || target_steam_id == g_nrsc_steam_control_state.local_steam_id) {
    return;
  }
  RevenantFixSteamPeerTestState* state = AllocateRevenantFixSteamPeerTestState(target_steam_id);
  if (state == nullptr || state->ping_sent) {
    return;
  }

  const uint32_t nonce = GenerateRevenantFixSteamTestNonce(target_steam_id);
  char text[64]{};
  std::snprintf(
      text,
      sizeof(text),
      "client_ping nonce=%08x tick=%lu",
      nonce,
      static_cast<unsigned long>(GetTickCount()));
  state->last_nonce = nonce;
  state->sent_ms = GetTickCount();
  state->ping_sent = true;
  state->pong_received = false;

  if (!TrySendRevenantFixSteamTestPacket(
          target_steam_id,
          kRevenantFixSteamControlPacketTypeTestPing,
          nonce,
          text,
          reason)) {
    state->ping_sent = false;
  }
}


// 7a66e71 src/dllmain.cpp:30606
bool SendRevenantFixNativeEvent(
    uint64_t target,
    uint32_t nonce,
    uint32_t event_id,
    uint64_t source_record_key,
    uint64_t master_lobby_id,
    uint64_t expedition_lobby_id,
    uint64_t authority_commit_seq,
    uint32_t client_generation,
    const void* payload,
    uint32_t payload_size,
    const char* reason) {
  if (target == 0 ||
      target == g_nrsc_steam_control_state.local_steam_id ||
      g_nrsc_steam_control_state.local_steam_id == 0 ||
      nonce == 0 || source_record_key == 0 || master_lobby_id == 0 ||
      expedition_lobby_id == 0 || authority_commit_seq == 0 ||
      payload == nullptr ||
      !IsBhrcNativeEventPayloadSizeValid(event_id, payload_size)) {
    return false;
  }

  RevenantFixNativeEventPacket packet{};
  packet.header.type = kRevenantFixSteamControlPacketTypeNativeEvent;
  packet.header.sender_steam_id = g_nrsc_steam_control_state.local_steam_id;
  packet.header.lobby_id = master_lobby_id;
  packet.header.target_steam_id = target;
  packet.header.flags = kBhrcFlagHasRecordKey;
  packet.request_nonce = nonce;
  packet.event_id = event_id;
  packet.record_key = source_record_key;
  packet.master_lobby_id = master_lobby_id;
  packet.expedition_lobby_id = expedition_lobby_id;
  packet.authority_commit_seq = authority_commit_seq;
  packet.client_generation = client_generation;
  packet.payload_size = payload_size;
  packet.payload_hash32 = BhrcFnv1a32(payload, payload_size);
  packet.chunk_index = 0;
  packet.chunk_count = 1;
  std::memcpy(packet.payload, payload, payload_size);

  const bool ok = TrySendRevenantFixSteamPacketRaw(
      target,
      &packet,
      static_cast<uint32_t>(sizeof(packet)),
      packet.header.type,
      reason, false);
  return ok;
}


// 7a66e71 src/dllmain.cpp:31988
bool ProcessRevenantFixSteamControlPacket(
    uint64_t actual_steam_id,
    uint32_t channel,
    const void* payload,
    uint64_t payload_size) {
  if (channel != static_cast<uint32_t>(kRevenantFixSteamControlChannel) ||
      payload == nullptr ||
      payload_size < sizeof(RevenantFixSteamControlPacket)) {
    return false;
  }

  const auto* packet = static_cast<const RevenantFixSteamControlPacket*>(payload);
  if (packet->magic != kRevenantFixSteamControlPacketMagic || packet->version != kRevenantFixSteamControlPacketVersion) {
    return false;
  }

  const uint64_t header_steam_id = packet->sender_steam_id;
  const bool diagnostic_packet =
      packet->type == kRevenantFixSteamControlPacketTypeTestPing ||
      packet->type == kRevenantFixSteamControlPacketTypeTestPong;
  const uint64_t local_steam_id = g_nrsc_steam_control_state.local_steam_id;
  bool peer_active = IsNrscPeerActive(actual_steam_id);
  if (!peer_active && packet->type == kRevenantFixSteamControlPacketTypeFullPgdChunk &&
      payload_size >= offsetof(RevenantFixFullPgdChunkPacket, payload) &&
      actual_steam_id != 0 &&
      header_steam_id == actual_steam_id && local_steam_id != 0 &&
      packet->target_steam_id == local_steam_id && packet->lobby_id != 0 &&
      packet->lobby_id == GetCurrentNrscMasterLobbyId()) {
    // Only repair this packet's stale transport observation. The full-PGD
    // handler still enforces owner/target, A/B, authority, revision and hashes.
    bool native_active = false;
    peer_active = TryReadNrscNativePeerActive(actual_steam_id, &native_active) && native_active;
  }
  const bool preactive_probe_request = !peer_active &&
      packet->type == kRevenantFixSteamControlPacketTypeExpeditionProbeRequest &&
      payload_size >= sizeof(RevenantFixExpeditionProbeRequestPacket) &&
      actual_steam_id != 0 && header_steam_id == actual_steam_id &&
      local_steam_id != 0 && packet->target_steam_id == local_steam_id &&
      IsLocalNrscHost() && packet->lobby_id != 0 &&
      packet->lobby_id == GetCurrentNrscMasterLobbyId();
  const LONG nrsc_bridge_state =
      InterlockedCompareExchange(&g_nrsc_bridge_install_state, 0, 0);
  const char* authority_outcome = "accepted";
  if (nrsc_bridge_state != 2) {
    authority_outcome = "nrsc_bridge_not_ready";
  } else if (actual_steam_id == 0) {
    authority_outcome = "missing_actual_sender";
  } else if ((!diagnostic_packet && header_steam_id != actual_steam_id) ||
             (diagnostic_packet && header_steam_id != 0 &&
              header_steam_id != actual_steam_id)) {
    authority_outcome = "header_sender_mismatch";
  } else if (!peer_active && !preactive_probe_request) {
    authority_outcome = "nrsc_peer_inactive";
  } else if (!diagnostic_packet && local_steam_id == 0) {
    authority_outcome = "missing_local_steam_id";
  } else if (!diagnostic_packet && packet->target_steam_id != local_steam_id) {
    authority_outcome = "target_mismatch";
  }
  const bool authority_ok = std::strcmp(authority_outcome, "accepted") == 0;
  const uint64_t sender = actual_steam_id;
  if (packet->type == kRevenantFixSteamControlPacketTypeFullPgdChunk &&
      payload_size >= offsetof(RevenantFixFullPgdChunkPacket, payload)) {
    TraceBhrcPgdReceiveGate(actual_steam_id,
        *static_cast<const RevenantFixFullPgdChunkPacket*>(payload),
        peer_active, nrsc_bridge_state, authority_outcome);
  }
  if (!authority_ok) {
    LogWarn(
        "RevenantFixSteamControl recv_rejected reason=%s actualSteamId=%016llx headerSteamId=%016llx target=%016llx type=%s channel=%u",
        authority_outcome,
        static_cast<unsigned long long>(actual_steam_id),
        static_cast<unsigned long long>(header_steam_id),
        static_cast<unsigned long long>(packet->target_steam_id),
        RevenantFixSteamControlPacketTypeName(packet->type),
        channel);
    return true;
  }
  if (preactive_probe_request) {
    Log(
        "BHRC expedition_probe_transport outcome=preactive_request_allowed sender=%016llx target=%016llx masterA=%016llx type=%s authority=steam_sender_and_local_nrsc_host exactTarget=1",
        static_cast<unsigned long long>(actual_steam_id),
        static_cast<unsigned long long>(packet->target_steam_id),
        static_cast<unsigned long long>(packet->lobby_id),
        RevenantFixSteamControlPacketTypeName(packet->type));
  }

  if (actual_steam_id != 0 && actual_steam_id != g_nrsc_steam_control_state.local_steam_id) {
    RememberNrscPeerSteamId(actual_steam_id);
  }
  if (packet->type == kRevenantFixSteamControlPacketTypeTestPing ||
      packet->type == kRevenantFixSteamControlPacketTypeTestPong) {
    if (payload_size < sizeof(RevenantFixSteamTestPacket)) {
      LogWarn(
          "RevenantFixSteamControl %s_malformed sender=%016llx bytes=%llu expected=%zu",
          RevenantFixSteamControlPacketTypeName(packet->type),
          static_cast<unsigned long long>(sender),
          static_cast<unsigned long long>(payload_size),
          sizeof(RevenantFixSteamTestPacket));
      return true;
    }

    const auto* test_packet = static_cast<const RevenantFixSteamTestPacket*>(payload);
    char text[sizeof(test_packet->text) + 1]{};
    std::memcpy(text, test_packet->text, sizeof(test_packet->text));
    text[sizeof(test_packet->text)] = '\0';
    Log(
        "RevenantFixSteamControl %s_recv steamId=%016llx sender=%016llx target=%016llx nonce=%08x text=\"%s\" localHost=%d channel=%u",
        RevenantFixSteamControlPacketTypeName(packet->type),
        static_cast<unsigned long long>(actual_steam_id),
        static_cast<unsigned long long>(sender),
        static_cast<unsigned long long>(packet->target_steam_id),
        test_packet->nonce,
        text,
        IsLocalNrscHost() ? 1 : 0,
        channel);

    if (packet->type == kRevenantFixSteamControlPacketTypeTestPing) {
      const bool addressed_to_local =
          packet->target_steam_id == 0 ||
          packet->target_steam_id == g_nrsc_steam_control_state.local_steam_id;
      if ((IsLocalNrscHost() || addressed_to_local) &&
          sender != 0 &&
          sender != g_nrsc_steam_control_state.local_steam_id) {
        char response[64]{};
        std::snprintf(
            response,
            sizeof(response),
            "host_pong nonce=%08x tick=%lu",
            test_packet->nonce,
            static_cast<unsigned long>(GetTickCount()));
        TrySendRevenantFixSteamTestPacket(
            sender,
            kRevenantFixSteamControlPacketTypeTestPong,
            test_packet->nonce,
            response,
            "nrsc_register_peer_test_pong");
      } else {
        Log(
            "RevenantFixSteamControl test_ping_no_reply sender=%016llx target=%016llx localSteamId=%016llx localHost=%d",
            static_cast<unsigned long long>(sender),
            static_cast<unsigned long long>(packet->target_steam_id),
            static_cast<unsigned long long>(g_nrsc_steam_control_state.local_steam_id),
            IsLocalNrscHost() ? 1 : 0);
      }
      return true;
    }

    RevenantFixSteamPeerTestState* state = AllocateRevenantFixSteamPeerTestState(sender);
    if (state != nullptr) {
      state->last_nonce = test_packet->nonce;
      state->pong_received = true;
    }
    return true;
  }

  if (packet->type == kRevenantFixSteamControlPacketTypeExpeditionProbeRequest) {
    if (payload_size < sizeof(RevenantFixExpeditionProbeRequestPacket)) {
      LogWarn(
          "BHRC expedition_probe_request_malformed sender=%016llx bytes=%llu expected=%zu",
          static_cast<unsigned long long>(sender),
          static_cast<unsigned long long>(payload_size),
          sizeof(RevenantFixExpeditionProbeRequestPacket));
      return true;
    }
    return HandleBhrcExpeditionProbeRequest(
        sender,
        *static_cast<const RevenantFixExpeditionProbeRequestPacket*>(payload));
  }

  if (packet->type == kRevenantFixSteamControlPacketTypeExpeditionProbeResponse) {
    if (payload_size < sizeof(RevenantFixExpeditionProbeResponsePacket)) {
      LogWarn(
          "BHRC expedition_probe_response_malformed sender=%016llx bytes=%llu expected=%zu",
          static_cast<unsigned long long>(sender),
          static_cast<unsigned long long>(payload_size),
          sizeof(RevenantFixExpeditionProbeResponsePacket));
      return true;
    }
    return HandleBhrcExpeditionProbeResponse(
        sender,
        *static_cast<const RevenantFixExpeditionProbeResponsePacket*>(payload));
  }

  if (packet->type == kRevenantFixSteamControlPacketTypeBuddyRecall) {
    if (payload_size != sizeof(RevenantFixBuddyRecallPacket)) {
      LogWarn(
          "BuddyRecallGlue recv_rejected reason=malformed_size sender=%016llx bytes=%llu expected=%zu",
          static_cast<unsigned long long>(sender),
          static_cast<unsigned long long>(payload_size),
          sizeof(RevenantFixBuddyRecallPacket));
      return true;
    }
    return EnqueueBuddyRecallPacket(
        sender,
        *static_cast<const RevenantFixBuddyRecallPacket*>(payload));
  }

  if (packet->type == kRevenantFixSteamControlPacketTypeRecordRosterAnnounce) {
    if (payload_size < sizeof(RevenantFixRecordRosterAnnouncePacket)) {
      LogWarn(
          "BHRC record_roster_malformed sender=%016llx bytes=%llu expected=%zu",
          static_cast<unsigned long long>(sender),
          static_cast<unsigned long long>(payload_size),
          sizeof(RevenantFixRecordRosterAnnouncePacket));
      return true;
    }
    return HandleRecordRosterAnnounce(sender, *static_cast<const RevenantFixRecordRosterAnnouncePacket*>(payload));
  }

  if (packet->type == kRevenantFixSteamControlPacketTypeFullPgdChunk) {
    constexpr size_t kFullPgdPayloadOffset =
        offsetof(RevenantFixFullPgdChunkPacket, payload);
    const uint32_t chunk_size =
        payload_size >= kFullPgdPayloadOffset
        ? static_cast<const RevenantFixFullPgdChunkPacket*>(payload)->chunk_size
        : 0;
    if (payload_size < kFullPgdPayloadOffset || chunk_size == 0 ||
        chunk_size > kBhrcFullPgdChunkPayloadSize ||
        chunk_size > payload_size - kFullPgdPayloadOffset) {
      LogWarn(
          "BHRC full_pgd_recv outcome=rejected reason=malformed_packet_size sender=%016llx bytes=%llu expected=%zu coldStorage=0",
          static_cast<unsigned long long>(sender),
          static_cast<unsigned long long>(payload_size),
          kFullPgdPayloadOffset + chunk_size);
      return true;
    }
    return HandleRevenantFixFullPgdChunk(
        sender,
        *static_cast<const RevenantFixFullPgdChunkPacket*>(payload));
  }

  if (packet->type == kRevenantFixSteamControlPacketTypeNativeEvent) {
    if (payload_size != sizeof(RevenantFixNativeEventPacket)) {
      LogWarn(
          "BHRC native_event_slot_store outcome=rejected reason=malformed_size sender=%016llx target=0000000000000000 nonce=00000000 event=0x00 sourceRecordKey=0000000000000000 masterA=0000000000000000 expeditionB=0000000000000000 authoritySeq=0 clientGeneration=0 payloadSize=%llu payloadHash=00000000 expectedPacketSize=%zu",
          static_cast<unsigned long long>(sender),
          static_cast<unsigned long long>(payload_size),
          sizeof(RevenantFixNativeEventPacket));
      return true;
    }
    return HandleRevenantFixNativeEvent(
        sender,
        *static_cast<const RevenantFixNativeEventPacket*>(payload));
  }

  LogWarn(
      "RevenantFixSteamControl unknown_packet sender=%016llx type=%u bytes=%llu",
      static_cast<unsigned long long>(sender),
      packet->type,
      static_cast<unsigned long long>(payload_size));
  return true;
}


// 7a66e71 src/dllmain.cpp:32200
void ReleaseSteamNetworkingMessage(SteamNetworkingMessageLite* message) {
  if (message == nullptr) {
    return;
  }
  if (message->release != nullptr) {
    __try {
      message->release(message);
    } __except (LogSehException("RevenantFixSteamControl message_release", GetExceptionInformation())) {
      LogError("RevenantFixSteamControl release_exception message=%p", message);
    }
  }
}


// 7a66e71 src/dllmain.cpp:32213
void ProcessRevenantFixSteamControlReceives() {
  if (!g_config.enable_nrsc_steam_control) {
    return;
  }
  if (!ResolveSteamNetworkingMessagesInterface("receive_poll")) {
    return;
  }

  for (int batch = 0; batch < 4; ++batch) {
    SteamNetworkingMessageLite* messages[kRevenantFixSteamControlMaxMessages]{};
    int count = 0;
    __try {
      count = g_steam_messages_receive(
          g_steam_networking_messages,
          kRevenantFixSteamControlChannel,
          messages,
          kRevenantFixSteamControlMaxMessages);
    } __except (LogSehException("RevenantFixSteamControl ReceiveMessagesOnChannel", GetExceptionInformation())) {
      LogError(
          "RevenantFixSteamControl receive_exception interface=%p channel=%d",
          g_steam_networking_messages,
          kRevenantFixSteamControlChannel);
      return;
    }

    if (count <= 0) {
      return;
    }

    for (int i = 0; i < count; ++i) {
      SteamNetworkingMessageLite* message = messages[i];
      if (message == nullptr) {
        continue;
      }

      const uint64_t actual_sender = ReadSteamNetworkingIdentitySteamId(message->peer);
      if (actual_sender == 0) {
        LogWarn(
            "BHRC transport_identity_missing actualSteamId=0000000000000000 identityType=%d identitySize=%d connection=%u messageSize=%d messageNumber=%lld channel=%d",
            message->peer.type,
            message->peer.size,
            message->connection,
            message->size,
            static_cast<long long>(message->message_number),
            message->channel);
      }

      ProcessRevenantFixSteamControlPacket(
          actual_sender,
          static_cast<uint32_t>(kRevenantFixSteamControlChannel),
          message->data,
          message->size > 0 ? static_cast<uint64_t>(message->size) : 0);
      ReleaseSteamNetworkingMessage(message);
    }

    if (count < kRevenantFixSteamControlMaxMessages) {
      return;
    }
  }
}


// 7a66e71 src/dllmain.cpp:40175
void __fastcall HookNrscNativeEvent94Fanout(
    void* hook_context,
    void* team_state,
    void* arg3,
    void* arg4,
    const void* serialized_vector) {
  bool original_ok = false;
  __try {
    g_nrsc_native_event_94_fanout(
        hook_context, team_state, arg3, arg4, serialized_vector);
    original_ok = true;
  } __except (LogSehException(
      "BHRC nrsc FUN_180026250 native event 0x94 fanout",
      GetExceptionInformation())) {
    LogError(
        "BHRC hook_exception function=nrsc_FUN_180026250 event=0x94 hookContext=%p teamState=%p vector=%p",
        hook_context, team_state, serialized_vector);
  }
  if (!original_ok || !IsLocalNrscHost() || serialized_vector == nullptr) {
    return;
  }
  uint64_t begin = 0;
  uint64_t end = 0;
  if (!TryReadU64(serialized_vector, &begin) ||
      !TryReadU64(static_cast<const uint8_t*>(serialized_vector) + 8, &end) ||
      begin == 0 || end <= begin ||
      end - begin > kBhrcNativeEvent94MaxPayloadSize) {
    return;
  }
  CaptureBhrcPendingHostNativeEvent94(
      team_state,
      reinterpret_cast<const void*>(begin),
      static_cast<uint32_t>(end - begin),
      "nrsc_FUN_180026250_original_0x94");
  CaptureBhrcHostNativeEvent(
      0x94,
      ResolveBhrcHostLiveSourceRecordKey(),
      reinterpret_cast<const void*>(begin),
      static_cast<uint32_t>(end - begin),
      "nrsc_FUN_180026250_original_0x94");
}


// 7a66e71 src/dllmain.cpp:40217
void __fastcall HookNrscLobbyMemberEvent(void* context, uint64_t* event_data) {
  g_nrsc_steam_control_state.member_context = context;
  uint64_t lobby_id = 0;
  uint64_t steam_id = 0;
  uint32_t event_type = 0xffffffffu;
  if (event_data != nullptr) {
    lobby_id = ReadU64(event_data, 0);
    steam_id = ReadU64(event_data + 1, 0);
    event_type = ReadU32(event_data + 3, 0xffffffffu);
  }

  const char* tag = DescribeNrscLobbyEvent(event_type);
  CaptureNrscLobbyEventState(tag, context, lobby_id, steam_id);
  const bool lease_candidate =
      IsBhrcReturningMemberTransportLoss(event_type);
  if (lease_candidate) {
    CaptureLiveRemoteReconnectIdentities(
        ReadGameGlobalPtr(kNetModeManagerPtrRva),
        "nrsc_A_peer_loss_pre_original");
  }
  if ((event_type & 0x18u) != 0) {
    CancelBhrcReturningMemberRetention(
        steam_id, "nrsc_A_authoritative_kick_or_ban");
  }
  const BhrcReturningMemberLeaseDecision lease = lease_candidate
      ? EvaluateBhrcReturningMemberLease(
            nullptr, lobby_id, steam_id, event_type)
      : BhrcReturningMemberLeaseDecision{};
  const bool retain_returning_member = lease_candidate && lease.retain;
  if (retain_returning_member) {
    ArmBhrcReturningMemberRetention(
        lease, "nrsc!FUN_1800645a0/master_A_lobby_callback");
  }
  if (event_type == 1) {
    RememberNrscPeerSteamId(steam_id);
    SetCachedReconnectTransportEvidence(steam_id, true, "nrsc_lobby_join");
  } else if (((0x10114u >> (event_type & 0x1f)) & 1u) != 0) {
    SetNrscPeerActive(steam_id, false, tag);
    SetCachedReconnectTransportEvidence(steam_id, false, tag);
  }
  __try {
    g_nrsc_lobby_member_event(context, event_data);
  } __except (LogSehException("NRSC LobbyMemberEvent original", GetExceptionInformation())) {
    LogError(
        "NrscLobbyMemberEvent original_exception context=%p eventData=%p lobbyId=%016llx steamId=%016llx eventType=%u",
        context,
        event_data,
        static_cast<unsigned long long>(lobby_id),
        static_cast<unsigned long long>(steam_id),
        event_type);
    return;
  }

  CaptureNrscLobbyEventState(tag, context, lobby_id, steam_id);
  if (event_type == 1 && lobby_id == GetCurrentNrscMasterLobbyId()) {
    bool native_active = false;
    if (TryReadNrscNativePeerActive(steam_id, &native_active) && native_active)
      SetNrscPeerActive(steam_id, true, "nrsc_join_native_peer_readback");
  }
}


// 7a66e71 src/dllmain.cpp:40272
void __fastcall HookNrscLobbyScan(void* context, uint64_t* lobby_id) {
  const uint64_t lobby = lobby_id != nullptr ? ReadU64(lobby_id, 0) : 0;
  const BhrcExpeditionProbeAttempt ingress_probe =
      SnapshotBhrcExpeditionProbe();
  const BhrcStartupHostPreflightAttempt ingress_startup_preflight =
      SnapshotBhrcStartupHostPreflight();
  const BhrcDeferredReconnectIntent ingress_intent =
      SnapshotBhrcDeferredReconnectIntent();
  const bool reason4_join_origin = ingress_intent.serial != 0 &&
      ingress_intent.master_join_call != 0 &&
      ingress_intent.master_lobby_id == lobby &&
      (ingress_intent.stage ==
           BhrcDeferredReconnectStage::MasterJoinRequested ||
       ingress_intent.stage ==
           BhrcDeferredReconnectStage::AwaitingNativeB);
  const bool startup_join_lifecycle_active =
      IsBhrcExpeditionProbePendingStage(ingress_probe.stage) ||
      ingress_probe.stage ==
          BhrcExpeditionProbeStage::AuthorizedReturning;
  const bool startup_preflight_owns_join =
      (ingress_startup_preflight.stage ==
           BhrcStartupHostPreflightStage::Pending ||
       ingress_startup_preflight.stage ==
           BhrcStartupHostPreflightStage::RunningConfirmed ||
       ingress_startup_preflight.stage ==
           BhrcStartupHostPreflightStage::Promoted ||
       ingress_startup_preflight.stage ==
           BhrcStartupHostPreflightStage::AuthorizedLatched) &&
      BhrcStartupHostPreflightMatchesProbeIdentity(
          ingress_startup_preflight, ingress_probe);
  const bool startup_join_origin = !reason4_join_origin &&
      ingress_probe.request_nonce != 0 &&
      ingress_probe.trigger ==
          BhrcExpeditionProbeTrigger::StartupPreviousSession &&
      ingress_probe.join_call != 0 &&
      startup_join_lifecycle_active &&
      startup_preflight_owns_join &&
      ingress_probe.master_lobby_id == lobby;

  PersistedReconnectIdentity pre_scan_identity{};
  const uint64_t pre_scan_local =
      g_nrsc_steam_control_state.local_steam_id;
  const bool pre_scan_old_member_fence =
      !startup_join_origin && context != nullptr && lobby != 0 &&
      IsBhrcOldMemberRouteReady() &&
      LoadPersistedReconnectIdentity(&pre_scan_identity) &&
      pre_scan_identity.master_lobby_id == lobby &&
      pre_scan_identity.host_steam_id != 0 &&
      pre_scan_identity.record_role == kBhrcRecordRoleSelf &&
      pre_scan_identity.lobby_id != 0 &&
      pre_scan_identity.lobby_id != lobby &&
      pre_scan_identity.record_key != 0 &&
      (pre_scan_local == 0 ||
       pre_scan_local == pre_scan_identity.record_key) &&
      pre_scan_identity.generation != 0 &&
      pre_scan_identity.record_hash32 != 0;
  uint64_t pre_scan_fence_owner = pre_scan_identity.host_steam_id;
  if (pre_scan_old_member_fence) {
    // A returning host can still have itself saved as owner after migration.
    // Read only Steam's current owner for this A; NRSC still owns the actual
    // transport commit, and the fence activates only when that tuple is live.
    void* const matchmaking =
        ResolveSteamMatchmakingForProbe("friend_join_ingress_owner");
    const uint64_t vtable = matchmaking != nullptr ? ReadU64(matchmaking, 0) : 0;
    auto get_lobby_owner = vtable != 0
        ? reinterpret_cast<SteamMatchmakingGetLobbyOwnerFn>(
              ReadU64(reinterpret_cast<const void*>(vtable + 0x118), 0))
        : nullptr;
    if (get_lobby_owner != nullptr) {
      uint64_t steam_owner = 0;
      __try {
        get_lobby_owner(matchmaking, &steam_owner, lobby);
      } __except (LogSehException(
          "BHRC friend join ingress GetLobbyOwner", GetExceptionInformation())) {
        steam_owner = 0;
      }
      if (steam_owner != 0 && steam_owner != pre_scan_identity.record_key) {
        pre_scan_fence_owner = steam_owner;
      }
    }
    // FUN_1800647a0 commits A/owner/local and can synchronously expose the
    // ordinary QuickMatch ingress before it returns. Publish only the exact
    // persisted old-member scalar tuple first, so the existing firewall owns
    // that gap as soon as NRSC commits the matching live tuple.
    PublishBhrcOldMemberMatchingFence(
        BhrcOldMemberMatchingFenceKind::PendingClassification, 0,
        pre_scan_identity.master_lobby_id,
        pre_scan_identity.lobby_id,
        pre_scan_fence_owner,
        pre_scan_identity.record_key,
        pre_scan_identity.generation,
        pre_scan_identity.record_hash32, 0, 0,
        "pre_original_friend_join_await_host_classification");
  }
  CaptureNrscLobbyScanState("nrsc_lobby_scan_enter", context, lobby);

  __try {
    g_nrsc_lobby_scan(context, lobby_id);
  } __except (LogSehException("NRSC LobbyScan original", GetExceptionInformation())) {
    LogError(
        "NrscLobbyScan original_exception context=%p lobbyPtr=%p lobbyId=%016llx",
        context,
        lobby_id,
        static_cast<unsigned long long>(lobby));
    if (pre_scan_old_member_fence) {
      ClearBhrcOldMemberMatchingFenceForTransport(
          pre_scan_identity.master_lobby_id,
          pre_scan_fence_owner,
          pre_scan_identity.record_key,
          "nrsc_lobby_scan_original_exception");
    }
    return;
  }

  CaptureNrscLobbyScanState("nrsc_lobby_scan_exit", context, lobby);
  const uint64_t committed_master = GetCurrentNrscMasterLobbyId();
  const uint64_t committed_owner = g_nrsc_steam_control_state.owner_steam_id;
  const uint64_t committed_local = g_nrsc_steam_control_state.local_steam_id;
  // FUN_1800647a0 is NRSC's master-lobby LobbyEnter handler and commits A and
  // owner only after metadata validation and real peer registration.  Its
  // callback may legitimately rewrite the same A on re-entry, so pre!=post is
  // diagnostic rather than an origin gate.  The startup join_call owns the
  // whole TitleFlow admission generation, not only the first transport-only
  // LobbyEnter callback. Keep every same-A re-entry on StartupPreviousSession
  // until the original mode-1 owner installs its task and marks HandedOff.
  // The reason-4 token and every remaining external LobbyEnter are active
  // FriendJoin ingress generations.
  const bool committed_master_lobby_enter = lobby != 0 &&
      committed_master == lobby && committed_owner != 0 &&
      committed_local != 0 && committed_owner != committed_local;
  const bool committed_friend_join_generation =
      committed_master_lobby_enter && !startup_join_origin;
  const bool authority_observer_armed = committed_friend_join_generation &&
      ArmBhrcFriendJoinAuthorityObserverFromCommittedScan(
          context, committed_master, committed_owner, committed_local,
          reason4_join_origin
              ? "nrsc_lobby_scan_exit_reason4_intent"
              : "nrsc_lobby_scan_exit_external_friend_join");
  const bool pre_scan_fence_matches_committed =
      pre_scan_old_member_fence &&
      committed_master == pre_scan_identity.master_lobby_id &&
      committed_owner == pre_scan_fence_owner &&
      committed_local == pre_scan_identity.record_key;
  if (pre_scan_old_member_fence &&
      (!authority_observer_armed ||
       !pre_scan_fence_matches_committed)) {
    ClearBhrcOldMemberMatchingFenceForTransport(
        pre_scan_identity.master_lobby_id,
        pre_scan_fence_owner,
        pre_scan_identity.record_key,
        !pre_scan_fence_matches_committed
            ? "nrsc_lobby_scan_committed_tuple_mismatch"
            : "nrsc_lobby_scan_authority_observer_not_armed");
  }
  const BhrcExpeditionProbeAttempt armed_probe =
      authority_observer_armed
          ? SnapshotBhrcExpeditionProbe()
          : BhrcExpeditionProbeAttempt{};
  bool reason4_transfer_completed = false;
  if (authority_observer_armed && reason4_join_origin) {
    reason4_transfer_completed =
        MarkDeferredReconnectTransferredToAuthorityProbe(
            ingress_intent.serial, armed_probe,
            "reason4_master_join_committed_to_authority_probe");
    if (!reason4_transfer_completed) {
      // The stage mask is checked atomically with the serial. A concurrent
      // successful transfer/retirement is preserved; only the still-live
      // legacy transport is retired while the committed observer remains.
      FailDeferredReconnectIntent(
          ingress_intent.serial,
          BhrcDeferredReconnectStageMask(
              BhrcDeferredReconnectStage::MasterJoinRequested) |
              BhrcDeferredReconnectStageMask(
                  BhrcDeferredReconnectStage::AwaitingNativeB),
          "reason4_authority_transfer_race_observer_retained");
    }
  }
}


// 7a66e71 src/dllmain.cpp:40429
void __fastcall HookNrscRegisterPeer(void* steam_friends, void* peer_slot, uint64_t steam_id) {
  RememberNrscPeerSteamId(steam_id);
  __try {
    g_nrsc_register_peer(steam_friends, peer_slot, steam_id);
  } __except (LogSehException("NRSC RegisterPeer original", GetExceptionInformation())) {
    LogError(
        "NrscRegisterPeer original_exception steamFriends=%p peerSlot=%p steamId=%016llx",
        steam_friends,
        peer_slot,
        static_cast<unsigned long long>(steam_id));
    return;
  }
  uint64_t slot_steam_id = 0;
  uint8_t slot_flags = 0;
  const bool slot_read_ok =
      peer_slot != nullptr &&
      TryReadU64(static_cast<const uint8_t*>(peer_slot) + 8, &slot_steam_id) &&
      TryReadU8(static_cast<const uint8_t*>(peer_slot) + 0x128, &slot_flags);
  const bool slot_valid =
      slot_read_ok && slot_steam_id == steam_id && (slot_flags & 1) != 0;
  if (slot_valid) {
    SetNrscPeerActive(steam_id, true, "nrsc_register_peer_post");
    SetCachedReconnectTransportEvidence(
        steam_id, true, "nrsc_register_peer_post");
  } else {
    SetNrscPeerActive(steam_id, false, "nrsc_register_peer_post_invalid");
    SetCachedReconnectTransportEvidence(
        steam_id, false, "nrsc_register_peer_post_invalid");
  }
  if (slot_valid) {
    MaybeSendRevenantFixSteamTestPing(steam_id, "nrsc_register_peer_test_ping");
  }
}


// 7a66e71 src/dllmain.cpp:42255
bool VerifyBhrcNrscDependencyEntries(uintptr_t nrsc_base) {
  const BhrcNrscDependencyPreflightSpec specs[] = {
      {56, kNrscLobbyMemberEventRva, kNrscLobbyMemberEventPrologue,
       sizeof(kNrscLobbyMemberEventPrologue), "FUN_1800645a0"},
      {57, kNrscLobbyScanRva, kNrscLobbyScanPrologue,
       sizeof(kNrscLobbyScanPrologue), "FUN_1800647a0"},
      {58, kNrscRegisterPeerRva, kNrscRegisterPeerPrologue,
       sizeof(kNrscRegisterPeerPrologue), "FUN_18003d920"},
      {114, kNrscNativeEvent94FanoutRva,
       kNrscNativeEvent94FanoutPrologue,
       sizeof(kNrscNativeEvent94FanoutPrologue), "FUN_180026250"},
      {86, kNrscLeaveLobbyDetourRva, kNrscLeaveLobbyDetourPrologue,
       sizeof(kNrscLeaveLobbyDetourPrologue), "FUN_18002bc70"},
      {90, kNrscQuickMatchFinalizeRva, kNrscQuickMatchFinalizePrologue,
       sizeof(kNrscQuickMatchFinalizePrologue), "FUN_18002ad20"},
  };
  bool all_match = nrsc_base != 0;
  for (const BhrcNrscDependencyPreflightSpec& spec : specs) {
    uint8_t actual[32]{};
    char expected_text[128]{};
    char actual_text[128]{};
    const bool slot_free = spec.slot == SIZE_MAX ||
        (spec.slot < _countof(g_hooks) &&
         g_hooks[spec.slot].target == nullptr);
    const bool read_ok = nrsc_base != 0 && slot_free && spec.expected != nullptr &&
        spec.span != 0 && spec.span <= sizeof(actual) &&
        ReadBhrcHookEntryBytes(
            nrsc_base + spec.rva, actual, spec.span);
    const bool match = read_ok &&
        std::memcmp(actual, spec.expected, spec.span) == 0;
    FormatBhrcHookBytes(
        spec.expected, spec.span, expected_text, sizeof(expected_text));
    FormatBhrcHookBytes(
        actual, spec.span, actual_text, sizeof(actual_text));
    Log(
        "BHRC nrsc_dependency_preflight expectedNrscSha256=%s slot=%lld rva=%llx target=%p span=%llu slotFree=%d readOk=%d match=%d expected=\"%s\" actual=\"%s\" hook=%s",
        kBhrcExpectedNrscSha256,
        spec.slot == SIZE_MAX ? -1LL : static_cast<long long>(spec.slot),
        static_cast<unsigned long long>(spec.rva),
        reinterpret_cast<void*>(nrsc_base + spec.rva),
        static_cast<unsigned long long>(spec.span),
        slot_free ? 1 : 0, read_ok ? 1 : 0, match ? 1 : 0,
        expected_text, actual_text,
        spec.name != nullptr ? spec.name : "unknown");
    all_match = all_match && match;
  }
  return all_match;
}


// 7a66e71 src/dllmain.cpp:42304
bool InstallNrscSteamControlHooksForModule(HMODULE nrsc_module) {
  if (nrsc_module == nullptr) {
    return false;
  }

  if (InterlockedCompareExchange(&g_nrsc_bridge_install_state, 1, 0) != 0) {
    return g_nrsc_bridge_install_state == 2;
  }

  const uintptr_t nrsc_base = reinterpret_cast<uintptr_t>(nrsc_module);
  if (!VerifyBhrcNrscDependencyEntries(nrsc_base)) {
    g_nrsc_module = nullptr;
    InterlockedExchange(&g_nrsc_bridge_install_state, 3);
    LogWarn(
        "NRSC Steam control install skipped reason=dependency_preflight expectedNrscSha256=%s nrscBase=%p liveHooks=0 bridgeState=3",
        kBhrcExpectedNrscSha256, reinterpret_cast<void*>(nrsc_base));
    return false;
  }

  g_nrsc_module = nrsc_module;
  Log("NRSC Steam control installing nrscBase=%p", reinterpret_cast<void*>(nrsc_base));

  bool installed = true;
  installed = InstallInlineHook(
                  g_hooks[56],
                  nrsc_base + kNrscLobbyMemberEventRva,
                  reinterpret_cast<void*>(HookNrscLobbyMemberEvent),
                  sizeof(kNrscLobbyMemberEventPrologue),
                  reinterpret_cast<void**>(&g_nrsc_lobby_member_event),
                  "nrsc!FUN_1800645a0/LobbyMemberEvent Steam control") &&
      installed;
  installed = InstallInlineHook(
                  g_hooks[57],
                  nrsc_base + kNrscLobbyScanRva,
                  reinterpret_cast<void*>(HookNrscLobbyScan),
                  sizeof(kNrscLobbyScanPrologue),
                  reinterpret_cast<void**>(&g_nrsc_lobby_scan),
                  "nrsc!FUN_1800647a0/LobbyScan Steam control") &&
      installed;
  installed = InstallInlineHook(
                  g_hooks[58],
                  nrsc_base + kNrscRegisterPeerRva,
                  reinterpret_cast<void*>(HookNrscRegisterPeer),
                  sizeof(kNrscRegisterPeerPrologue),
                  reinterpret_cast<void**>(&g_nrsc_register_peer),
                  "nrsc!FUN_18003d920/RegisterPeer Steam control") &&
      installed;

  bool event94_installed = false;
  {
    uint8_t actual[sizeof(kNrscNativeEvent94FanoutPrologue)]{};
    char expected_text[128]{};
    char actual_text[128]{};
    const bool slot_free = g_hooks[114].target == nullptr;
    const bool read_ok = slot_free && ReadBhrcHookEntryBytes(
        nrsc_base + kNrscNativeEvent94FanoutRva,
        actual,
        sizeof(actual));
    const bool match = read_ok && std::memcmp(
        actual,
        kNrscNativeEvent94FanoutPrologue,
        sizeof(actual)) == 0;
    FormatBhrcHookBytes(
        kNrscNativeEvent94FanoutPrologue,
        sizeof(kNrscNativeEvent94FanoutPrologue),
        expected_text,
        sizeof(expected_text));
    FormatBhrcHookBytes(actual, sizeof(actual), actual_text, sizeof(actual_text));
    Log(
        "BHRC nrsc_event94_prologue expectedNrscSha256=%s slot=114 rva=%llx target=%p patchSize=%llu slotFree=%d readOk=%d match=%d expected=\"%s\" actual=\"%s\"",
        kBhrcExpectedNrscSha256,
        static_cast<unsigned long long>(kNrscNativeEvent94FanoutRva),
        reinterpret_cast<void*>(nrsc_base + kNrscNativeEvent94FanoutRva),
        static_cast<unsigned long long>(
            sizeof(kNrscNativeEvent94FanoutPrologue)),
        slot_free ? 1 : 0, read_ok ? 1 : 0, match ? 1 : 0,
        expected_text, actual_text);
    if (match && InstallInlineHook(
            g_hooks[114],
            nrsc_base + kNrscNativeEvent94FanoutRva,
            reinterpret_cast<void*>(HookNrscNativeEvent94Fanout),
            sizeof(kNrscNativeEvent94FanoutPrologue),
            reinterpret_cast<void**>(&g_nrsc_native_event_94_fanout),
            "nrsc!FUN_180026250/native event 0x94 six-player capture")) {
      event94_installed = true;
      Log(
          "BHRC nrsc_event94_install outcome=ready slot=114 originalFanoutPreserved=1 payloadMutation=0");
    } else {
      RemoveInlineHook(g_hooks[114]);
      g_nrsc_native_event_94_fanout = nullptr;
      LogWarn(
          "BHRC nrsc_event94_install outcome=skipped reason=%s",
          match ? "hook_install_failed" : "prologue_preflight");
    }
  }

  bool leave_installed = false;
  {
    uint8_t actual[sizeof(kNrscLeaveLobbyDetourPrologue)]{};
    char expected_text[128]{};
    char actual_text[128]{};
    const bool slot_free = g_hooks[86].target == nullptr;
    const bool read_ok = slot_free && ReadBhrcHookEntryBytes(
        nrsc_base + kNrscLeaveLobbyDetourRva,
        actual,
        sizeof(actual));
    const bool match = read_ok && std::memcmp(
        actual,
        kNrscLeaveLobbyDetourPrologue,
        sizeof(actual)) == 0;
    FormatBhrcHookBytes(
        kNrscLeaveLobbyDetourPrologue,
        sizeof(kNrscLeaveLobbyDetourPrologue),
        expected_text,
        sizeof(expected_text));
    FormatBhrcHookBytes(actual, sizeof(actual), actual_text, sizeof(actual_text));
    Log(
        "BHRC nrsc_leave_prologue expectedNrscSha256=%s slot=86 rva=%llx target=%p patchSize=%llu slotFree=%d readOk=%d match=%d expected=\"%s\" actual=\"%s\"",
        kBhrcExpectedNrscSha256,
        static_cast<unsigned long long>(kNrscLeaveLobbyDetourRva),
        reinterpret_cast<void*>(nrsc_base + kNrscLeaveLobbyDetourRva),
        static_cast<unsigned long long>(sizeof(kNrscLeaveLobbyDetourPrologue)),
        slot_free ? 1 : 0,
        read_ok ? 1 : 0,
        match ? 1 : 0,
        expected_text,
        actual_text);
    if (match && InstallInlineHook(
            g_hooks[86],
            nrsc_base + kNrscLeaveLobbyDetourRva,
            reinterpret_cast<void*>(HookNrscLeaveLobbyDetour),
            sizeof(kNrscLeaveLobbyDetourPrologue),
            reinterpret_cast<void**>(&g_nrsc_leave_lobby_detour),
            "nrsc!FUN_18002bc70/authorized expedition LeaveLobby gate")) {
      leave_installed = true;
      Log(
          "BHRC nrsc_leave_install outcome=ready slot=86 activeFlowChanged=active_peer_fail_closed_on_master_leave");
    } else {
      RemoveInlineHook(g_hooks[86]);
      g_nrsc_leave_lobby_detour = nullptr;
      LogWarn(
          "BHRC nrsc_leave_install outcome=skipped reason=%s activeFlowChanged=active_peer_gate_unavailable",
          match ? "hook_install_failed" : "prologue_preflight");
    }
  }

  bool quick_match_installed = false;
  {
    uint8_t actual[sizeof(kNrscQuickMatchFinalizePrologue)]{};
    char expected_text[128]{};
    char actual_text[128]{};
    const bool slot_free = g_hooks[90].target == nullptr;
    const bool read_ok = slot_free && ReadBhrcHookEntryBytes(
        nrsc_base + kNrscQuickMatchFinalizeRva,
        actual,
        sizeof(actual));
    const bool match = read_ok && std::memcmp(
        actual,
        kNrscQuickMatchFinalizePrologue,
        sizeof(actual)) == 0;
    FormatBhrcHookBytes(
        kNrscQuickMatchFinalizePrologue,
        sizeof(kNrscQuickMatchFinalizePrologue),
        expected_text,
        sizeof(expected_text));
    FormatBhrcHookBytes(actual, sizeof(actual), actual_text, sizeof(actual_text));
    Log(
        "BHRC nrsc_quickmatch_prologue expectedNrscSha256=%s slot=90 rva=%llx target=%p patchSize=%llu slotFree=%d readOk=%d match=%d expected=\"%s\" actual=\"%s\"",
        kBhrcExpectedNrscSha256,
        static_cast<unsigned long long>(kNrscQuickMatchFinalizeRva),
        reinterpret_cast<void*>(nrsc_base + kNrscQuickMatchFinalizeRva),
        static_cast<unsigned long long>(sizeof(kNrscQuickMatchFinalizePrologue)),
        slot_free ? 1 : 0,
        read_ok ? 1 : 0,
        match ? 1 : 0,
        expected_text,
        actual_text);
    if (match && InstallInlineHook(
            g_hooks[90],
            nrsc_base + kNrscQuickMatchFinalizeRva,
            reinterpret_cast<void*>(HookNrscQuickMatchFinalize),
            sizeof(kNrscQuickMatchFinalizePrologue),
            reinterpret_cast<void**>(&g_nrsc_quick_match_finalize),
            "nrsc!FUN_18002ad20/authorized QuickMatch finalizer gate")) {
      quick_match_installed = true;
      Log(
          "BHRC nrsc_quickmatch_install outcome=ready slot=90 activeFlowChanged=attempt_scoped_terminal_gate noRouteWrite=1");
    } else {
      RemoveInlineHook(g_hooks[90]);
      g_nrsc_quick_match_finalize = nullptr;
      LogWarn(
          "BHRC nrsc_quickmatch_install outcome=skipped reason=%s activeFlowChanged=finalizer_gate_unavailable",
          match ? "hook_install_failed" : "prologue_preflight");
    }
  }
  installed = installed && event94_installed && leave_installed &&
      quick_match_installed;

  if (!installed) {
    // Slots 117/118/113 require the entire NRSC dependency set. Never leave a
    // subset of peer, transport, 0x94 or finalizer detours live after a failed
    // install: a partial bridge can observe traffic but cannot preserve the
    // attempt/finalizer ownership contract.
    RemoveInlineHook(g_hooks[90]);
    RemoveInlineHook(g_hooks[86]);
    RemoveInlineHook(g_hooks[114]);
    RemoveInlineHook(g_hooks[58]);
    RemoveInlineHook(g_hooks[57]);
    RemoveInlineHook(g_hooks[56]);
    g_nrsc_quick_match_finalize = nullptr;
    g_nrsc_leave_lobby_detour = nullptr;
    g_nrsc_native_event_94_fanout = nullptr;
    g_nrsc_register_peer = nullptr;
    g_nrsc_lobby_scan = nullptr;
    g_nrsc_lobby_member_event = nullptr;
    g_nrsc_module = nullptr;
    InterlockedExchange(&g_nrsc_bridge_install_state, 3);
    LogWarn(
        "NRSC Steam control install failed and rolled back nrscBase=%p slots=90,86,114,61,60,59,58,57,56 liveHooks=0 bridgeState=3",
        reinterpret_cast<void*>(nrsc_base));
    return false;
  }
  InterlockedExchange(&g_nrsc_bridge_install_state, 2);
  Log("NRSC Steam control installed nrscBase=%p", reinterpret_cast<void*>(nrsc_base));
  return true;
}


// 7a66e71 src/dllmain.cpp:43442
DWORD WINAPI NrscSteamControlInstallThread(void*) {
  for (int attempt = 0; attempt < 100; ++attempt) {
    HMODULE nrsc = GetModuleHandleW(L"nrsc.dll");
    if (nrsc != nullptr) {
      const bool nrsc_ready = InstallNrscSteamControlHooksForModule(nrsc);
      if (nrsc_ready && !InstallBhrcActiveFriendNotificationHandoff()) {
        LogWarn(
            "BHRC active_friend_notification_install outcome=failed reason=post_nrsc_dependency_install startupProbeIndependent=1 activeFriendNotificationAvailable=0");
      }
      return 0;
    }
    Sleep(100);
  }
  Log("Skipped NRSC Steam control; nrsc.dll not loaded");
  InterlockedCompareExchange(&g_nrsc_bridge_install_state, 3, 0);
  return 0;
}


// 7a66e71 src/dllmain.cpp:43460
DWORD WINAPI RevenantFixSteamControlThread(void*) {
  DWORD last_slow_tick_ms = GetTickCount();
  for (;;) {
    Sleep(kBhrcSteamControlFastPollMs);
    if (!g_config.enable_nrsc_steam_control) {
      continue;
    }

    ProcessBhrcStartupP2PReceives();
    AdvanceBhrcStartupHostPreflight();

    const DWORD now = GetTickCount();
    if (now - last_slow_tick_ms >= kBhrcSteamControlSlowTickMs) {
      last_slow_tick_ms = now;
      ProcessRevenantFixSteamControlReceives();
      BhrcTick();
    }
  }
}


}  // namespace revenantfix::internal
