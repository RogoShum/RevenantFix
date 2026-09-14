#include "buddy_fix.hpp"

namespace revenantfix::internal {

// Original src/dllmain.cpp:4890
SlotSnapshot ReadSlot(void* state, int slot) {
  SlotSnapshot snapshot{
      kInvalidEntityPart,
      kInvalidEntityPart,
      0xff,
      0xff,
      0xff,
  };

  if (state == nullptr || slot < 0 || slot >= kBuddySlotCount) {
    return snapshot;
  }

  auto* base = static_cast<uint8_t*>(state);
  const int offset = slot * kBuddySlotSize;
  snapshot.entity_low = ReadU32(base + kSlotEntityLowOffset + offset, kInvalidEntityPart);
  snapshot.entity_high = ReadU32(base + kSlotEntityHighOffset + offset, kInvalidEntityPart);
  snapshot.state = ReadU8(base + kSlotStateOffset + offset, 0xff);
  snapshot.pending = ReadU8(base + kSlotPendingOffset + offset, 0xff);
  snapshot.flags = ReadU8(base + kSlotFlagsOffset + offset, 0xff);
  return snapshot;
}


// Original src/dllmain.cpp:4913
void LogBuddyExceptionContext(const char* tag, void* state, int slot) {
  const char* label = tag != nullptr ? tag : "<unknown>";
  if (state == nullptr) {
    LogError("%s buddy_context state=null requestedSlot=%d", label, slot);
    return;
  }

  if (slot >= 0 && slot < kBuddySlotCount) {
    const SlotSnapshot snapshot = ReadSlot(state, slot);
    LogError(
        "%s buddy_context state=%p requestedSlot=%d entity=%08x:%08x stateByte=%02x pending=%02x flags=%02x",
        label,
        state,
        slot,
        snapshot.entity_high,
        snapshot.entity_low,
        snapshot.state,
        snapshot.pending,
        snapshot.flags);
    return;
  }

  for (int i = 0; i < kBuddySlotCount; ++i) {
    const SlotSnapshot snapshot = ReadSlot(state, i);
    LogError(
        "%s buddy_context state=%p slot=%d entity=%08x:%08x stateByte=%02x pending=%02x flags=%02x",
        label,
        state,
        i,
        snapshot.entity_high,
        snapshot.entity_low,
        snapshot.state,
        snapshot.pending,
        snapshot.flags);
  }
}


// Original src/dllmain.cpp:4950
void LogSlotChange(const char* tag, int slot, const SlotSnapshot& before, const SlotSnapshot& after) {
  Log(
      "%s slot=%d entity %08x:%08x->%08x:%08x state %02x->%02x pending %02x->%02x flags %02x->%02x",
      tag,
      slot,
      before.entity_high,
      before.entity_low,
      after.entity_high,
      after.entity_low,
      before.state,
      after.state,
      before.pending,
      after.pending,
      before.flags,
      after.flags);
}


// Original src/dllmain.cpp:4977
bool IsTrackedBuddyOwnerSlot(int owner_slot) {
  return owner_slot > 0 && owner_slot < kTrackedBuddyStateCount;
}


// Original src/dllmain.cpp:4981
void ResetBuddyStateRuntimeContext(
    BuddyStateRuntimeContext* context,
    void* state,
    bool owner_known,
    uint32_t owner_id,
    int owner_slot) {
  if (context == nullptr) {
    return;
  }

  *context = BuddyStateRuntimeContext{};
  context->state = state;
  context->owner_known = owner_known;
  context->owner_id = owner_id;
  context->owner_slot = owner_slot;
}


// Original src/dllmain.cpp:4998
BuddyStateRuntimeContext* GetBuddyStateRuntime(void* state, bool create) {
  if (state == nullptr) {
    state = g_last_buddy_state;
  }

  if (state == nullptr) {
    return &g_buddy_state_contexts[kFallbackBuddyStateContextIndex];
  }

  for (int i = 0; i < kTrackedBuddyStateCount; ++i) {
    if (g_buddy_state_contexts[i].state == state) {
      return &g_buddy_state_contexts[i];
    }
  }

  if (!create) {
    return nullptr;
  }

  BuddyStateRuntimeContext* fallback =
      &g_buddy_state_contexts[kFallbackBuddyStateContextIndex];
  if (fallback->state != state) {
    ResetBuddyStateRuntimeContext(
        fallback,
        state,
        false,
        0xffffffff,
        -1);
  }
  return fallback;
}


// Original src/dllmain.cpp:5030
BuddyStateRuntimeContext* GetBuddyStateRuntimeForOwner(
    void* state,
    uint32_t owner_id,
    bool create) {
  const int owner_slot = owner_id != kInvalidEntityPart
      ? static_cast<int>(owner_id & 0xff)
      : -1;
  if (!IsTrackedBuddyOwnerSlot(owner_slot)) {
    return GetBuddyStateRuntime(state, create);
  }

  BuddyStateRuntimeContext* context = &g_buddy_state_contexts[owner_slot];
  if (context->state == state) {
    if (!context->owner_known || context->owner_id != owner_id ||
        context->owner_slot != owner_slot) {
      context->owner_known = true;
      context->owner_id = owner_id;
      context->owner_slot = owner_slot;
    }
    return context;
  }

  if (!create) {
    return nullptr;
  }

  ResetBuddyStateRuntimeContext(
      context,
      state,
      true,
      owner_id,
      owner_slot);
  return context;
}


// Original src/dllmain.cpp:5065
BuddyStateRuntimeContext* GetCurrentBuddyStateRuntime() {
  if (g_active_buddy_state_context != nullptr) {
    return g_active_buddy_state_context;
  }
  return GetBuddyStateRuntime(g_last_buddy_state, true);
}


// Original src/dllmain.cpp:5072
int GetBuddyStateRuntimeIndex(const BuddyStateRuntimeContext* context) {
  if (context == nullptr) {
    return -1;
  }

  for (int i = 0; i < kTrackedBuddyStateCount; ++i) {
    if (&g_buddy_state_contexts[i] == context) {
      return i;
    }
  }

  return -1;
}


// Original src/dllmain.cpp:5086
void MarkRevenantBuddyOwnerSeen(
    BuddyStateRuntimeContext* context,
    uint32_t owner_id,
    int owner_slot) {
  if (context == nullptr) {
    return;
  }

  if (!context->owner_known && owner_id != kInvalidEntityPart) {
    context->owner_known = true;
    context->owner_id = owner_id;
    context->owner_slot = owner_slot;
  }

  if (context->revenant_buddy_owner) {
    return;
  }

  context->revenant_buddy_owner = true;
}


// Original src/dllmain.cpp:5137
bool HasActiveBuddyMaintenanceWork(void* state) {
  BuddyStateRuntimeContext* context = GetBuddyStateRuntime(state, false);
  if (context == nullptr) {
    return false;
  }

  if (!context->revenant_buddy_owner) {
    return false;
  }

  if (context->delayed_switch_cleanup.active ||
      context->defer_switch_unsummon_this_try ||
      context->defer_switch_unsummon_slot >= 0) {
    return true;
  }

  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    const auto& runtime = context->buddy_slot_runtime[slot];
    if (runtime.custom_buddy && !IsEmptyEntityId(runtime.entity)) {
      return true;
    }

    const auto& zero_hp = context->zero_hp_fallback_regen[slot];
    if (zero_hp.active || zero_hp.monitoring) {
      return true;
    }

    const auto& recall = context->recall_regen[slot];
    if (recall.active || recall.monitoring) {
      return true;
    }

    if (context->stuck_recovery[slot].monitoring ||
        context->pending_deploy_repair[slot].monitoring ||
        context->post_recall_pulse[slot].active) {
      return true;
    }
  }

  return false;
}


// Original src/dllmain.cpp:5179
bool IsEmptyEntityId(const EntityId& entity_id) {
  return entity_id.low == kInvalidEntityPart && entity_id.high == kInvalidEntityPart;
}


// Original src/dllmain.cpp:5183
bool IsEmptyEntity(const SlotSnapshot& slot) {
  return IsEmptyEntityId(EntityId{slot.entity_low, slot.entity_high});
}


// Original src/dllmain.cpp:5187
bool SameEntity(const EntityId& left, const EntityId& right) {
  return left.low == right.low && left.high == right.high;
}


// Original src/dllmain.cpp:5191
int ComputeHpRateFromCurrent(int32_t current_hp, int32_t max_hp) {
  if (max_hp <= 0) {
    return -1;
  }

  if (current_hp <= 0) {
    return 0;
  }

  const int64_t rate = (static_cast<int64_t>(current_hp) * 100) / max_hp;
  if (rate > 100) {
    return 100;
  }

  return static_cast<int>(rate);
}


// Original src/dllmain.cpp:5208
bool SlotChanged(const SlotSnapshot& left, const SlotSnapshot& right) {
  return left.entity_low != right.entity_low || left.entity_high != right.entity_high ||
         left.state != right.state || left.pending != right.pending || left.flags != right.flags;
}


// Original src/dllmain.cpp:5213
bool IsValidBuddySlot(int buddy_slot) {
  return buddy_slot >= 0 && buddy_slot < kBuddySlotCount;
}


// Original src/dllmain.cpp:5217
bool IsCustomBuddyCreateData(int buddy_slot, int32_t chara_init, int32_t npc_param) {
  if (!IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  // Two mod styles need the custom path. Some mods replace the CharaInit row,
  // while others leave CharaInit at the vanilla negative/sentinel value and only
  // swap the BuddyParam's NpcParam. Both can bypass parts of the original
  // NecroBuddy manager, so both are tracked as custom buddies.
  return chara_init >= 0 || npc_param != kVanillaBuddyNpcParamIds[buddy_slot];
}


// Original src/dllmain.cpp:5229
bool IsCustomBuddySlot(int buddy_slot) {
  return IsValidBuddySlot(buddy_slot) && g_buddy_slot_runtime[buddy_slot].custom_buddy;
}


// Original src/dllmain.cpp:5233
bool IsCustomBuddyEntity(int buddy_slot, const EntityId& entity_id) {
  if (!IsCustomBuddySlot(buddy_slot) || IsEmptyEntityId(entity_id)) {
    return false;
  }

  return SameEntity(g_buddy_slot_runtime[buddy_slot].entity, entity_id);
}


// Original src/dllmain.cpp:5241
bool IsCustomBuddySnapshot(int buddy_slot, const SlotSnapshot& snapshot) {
  return IsCustomBuddyEntity(buddy_slot, EntityId{snapshot.entity_low, snapshot.entity_high});
}


// Original src/dllmain.cpp:5245
int SafeGetBuddyHpRate(void* state, int buddy_slot) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) || g_get_buddy_hp_rate == nullptr) {
    return -1;
  }

  __try {
    return g_get_buddy_hp_rate(state, static_cast<uint32_t>(buddy_slot));
  } __except (LogSehException("GetBuddyHpRate original", GetExceptionInformation())) {
    LogError("GetBuddyHpRate original_exception state=%p slot=%d", state, buddy_slot);
    LogBuddyExceptionContext("GetBuddyHpRate original", state, buddy_slot);
    return -2;
  }
}


// Original src/dllmain.cpp:5259
void* LookupEntityById(const EntityId& entity_id) {
  if (g_entity_lookup == nullptr || g_game_base == 0) {
    return nullptr;
  }

  const uint64_t world_chr_man =
      ReadU64(reinterpret_cast<const void*>(g_game_base + kWorldChrManPtrRva), 0);
  if (world_chr_man == 0) {
    return nullptr;
  }

  __try {
    return g_entity_lookup(reinterpret_cast<void*>(world_chr_man), &entity_id);
  } __except (LogSehException("EntityLookup original", GetExceptionInformation())) {
    LogError(
        "EntityLookup original_exception world=%p entityId=%08x:%08x",
        reinterpret_cast<void*>(world_chr_man),
        entity_id.high,
        entity_id.low);
    return nullptr;
  }
}


// Original src/dllmain.cpp:5282
bool ApplyCustomCharaBuddyPreventNearDeath(
    int buddy_slot,
    void* entity,
    const EntityId& entity_id,
    uint8_t state,
    uint8_t pending,
    bool reset_before_apply,
    const char* source) {
  if (!IsValidBuddySlot(buddy_slot) || g_apply_sp_effect == nullptr || entity == nullptr) {
    return false;
  }

  auto& runtime = g_buddy_slot_runtime[buddy_slot];
  if (!runtime.custom_buddy || !runtime.custom_chara_init ||
      !SameEntity(runtime.entity, entity_id)) {
    return false;
  }

  if (reset_before_apply) {
    runtime.prevent_near_death_applied = false;
  } else if (runtime.prevent_near_death_applied) {
    return true;
  }

  uint8_t apply_result = 0;
  __try {
    apply_result = g_apply_sp_effect(entity, kPreventNearDeathSpEffectId, 0);
  } __except (LogSehException("ApplySpEffect 102130", GetExceptionInformation())) {
    const uint32_t elapsed = g_module_update_tick - runtime.prevent_near_death_last_failure_tick;
    if (runtime.prevent_near_death_last_failure_tick == 0 || elapsed >= 300) {
      runtime.prevent_near_death_last_failure_tick = g_module_update_tick;
      LogError(
          "BuddyDirectDeath apply_exception slot=%d entity=%08x:%08x entityPtr=%p spEffect=%u source=%s",
          buddy_slot,
          entity_id.high,
          entity_id.low,
          entity,
          kPreventNearDeathSpEffectId,
          source != nullptr ? source : "unknown");
    }
    return false;
  }

  if (apply_result == 0) {
    runtime.prevent_near_death_applied = false;
    const uint32_t elapsed = g_module_update_tick - runtime.prevent_near_death_last_failure_tick;
    if (runtime.prevent_near_death_last_failure_tick == 0 || elapsed >= 300) {
      runtime.prevent_near_death_last_failure_tick = g_module_update_tick;
      LogWarn(
          "BuddyDirectDeath apply_rejected slot=%d entity=%08x:%08x entityPtr=%p spEffect=%u result=%u source=%s state=%02x pending=%02x",
          buddy_slot,
          entity_id.high,
          entity_id.low,
          entity,
          kPreventNearDeathSpEffectId,
          static_cast<unsigned int>(apply_result),
          source != nullptr ? source : "unknown",
          state,
          pending);
    }
    return false;
  }

  runtime.prevent_near_death_applied = true;
  runtime.prevent_near_death_last_failure_tick = 0;
  Log(
      "BuddyDirectDeath applied slot=%d entity=%08x:%08x entityPtr=%p spEffect=%u result=%u source=%s state=%02x pending=%02x",
      buddy_slot,
      entity_id.high,
      entity_id.low,
      entity,
      kPreventNearDeathSpEffectId,
      static_cast<unsigned int>(apply_result),
      source != nullptr ? source : "unknown",
      state,
      pending);
  return true;
}


// Original src/dllmain.cpp:5361
bool EnsureCustomCharaBuddyPreventNearDeath(
    int buddy_slot,
    const SlotSnapshot& snapshot) {
  if (!IsValidBuddySlot(buddy_slot) || snapshot.state != kSlotStateTransition ||
      snapshot.pending != 0 || IsEmptyEntity(snapshot)) {
    return false;
  }

  const EntityId entity_id{snapshot.entity_low, snapshot.entity_high};
  void* entity = LookupEntityById(entity_id);
  if (entity == nullptr) {
    auto& runtime = g_buddy_slot_runtime[buddy_slot];
    const uint32_t elapsed = g_module_update_tick - runtime.prevent_near_death_last_failure_tick;
    if (runtime.prevent_near_death_last_failure_tick == 0 || elapsed >= 300) {
      runtime.prevent_near_death_last_failure_tick = g_module_update_tick;
      LogWarn(
          "BuddyDirectDeath apply_deferred slot=%d entity=%08x:%08x reason=entity_lookup_failed source=module_retry",
          buddy_slot,
          entity_id.high,
          entity_id.low);
    }
    return false;
  }

  return ApplyCustomCharaBuddyPreventNearDeath(
      buddy_slot,
      entity,
      entity_id,
      snapshot.state,
      snapshot.pending,
      false,
      "module_retry");
}


// Original src/dllmain.cpp:5395
void* GetHpComponentFromEntity(void* entity) {
  if (entity == nullptr) {
    return nullptr;
  }

  const uint64_t hp_owner =
      ReadU64(static_cast<const uint8_t*>(entity) + kHpComponentOwnerOffset, 0);
  if (hp_owner == 0) {
    return nullptr;
  }

  const uint64_t hp_component = ReadU64(reinterpret_cast<const void*>(hp_owner), 0);
  return reinterpret_cast<void*>(hp_component);
}


// Original src/dllmain.cpp:5410
bool TryReadCustomBuddyContext(
    void* state,
    int buddy_slot,
    const SlotSnapshot& snapshot,
    int hp_rate,
    BuddyContext* context) {
  if (context == nullptr || state == nullptr || !IsValidBuddySlot(buddy_slot) || IsEmptyEntity(snapshot)) {
    return false;
  }

  const EntityId entity_id{snapshot.entity_low, snapshot.entity_high};
  if (!IsCustomBuddyEntity(buddy_slot, entity_id)) {
    return false;
  }

  void* entity = LookupEntityById(entity_id);
  void* hp_component = GetHpComponentFromEntity(entity);
  if (hp_component == nullptr) {
    return false;
  }

  const int32_t current_hp = ReadI32(static_cast<const uint8_t*>(hp_component) + kHpCurrentOffset, 0);
  const int32_t max_hp = ReadI32(static_cast<const uint8_t*>(hp_component) + kHpMaxOffset, 0);
  if (max_hp <= 0) {
    return false;
  }

  context->slot = snapshot;
  context->entity = entity_id;
  context->hp_component = hp_component;
  context->current_hp = current_hp;
  context->max_hp = max_hp;
  context->hp_rate = hp_rate;
  return true;
}


// Original src/dllmain.cpp:5446
int32_t ComputeBuddyRegenStep(int32_t max_hp, uint32_t regen_tick_count) {
  if (max_hp <= 0) {
    return 0;
  }

  const uint64_t previous_budget =
      (static_cast<uint64_t>(regen_tick_count) * static_cast<uint64_t>(max_hp)) /
      static_cast<uint64_t>(kZeroHpFallbackRegenDivisor);
  const uint64_t next_budget =
      (static_cast<uint64_t>(regen_tick_count + 1) * static_cast<uint64_t>(max_hp)) /
      static_cast<uint64_t>(kZeroHpFallbackRegenDivisor);
  int32_t step_hp = static_cast<int32_t>(next_budget - previous_budget);
  if (step_hp < 1) {
    step_hp = 1;
  }
  return step_hp;
}


// Original src/dllmain.cpp:5464
bool ApplyBuddyRegenTick(
    const char* tag,
    int buddy_slot,
    const BuddyContext& context,
    int32_t base_hp,
    uint32_t* regen_tick_count,
    int32_t* after_hp,
    int32_t* step_hp) {
  if (after_hp != nullptr) {
    *after_hp = context.current_hp;
  }
  if (step_hp != nullptr) {
    *step_hp = 0;
  }

  if (g_hp_set_current == nullptr) {
    LogError(
        "%s failed slot=%d entity=%08x:%08x hpComponent=%p currentHp=%d maxHp=%d reason=no_hp_setter",
        tag,
        buddy_slot,
        context.entity.high,
        context.entity.low,
        context.hp_component,
        context.current_hp,
        context.max_hp);
    return false;
  }

  const uint32_t current_regen_tick = regen_tick_count != nullptr ? *regen_tick_count : 0;
  const int32_t step = ComputeBuddyRegenStep(context.max_hp, current_regen_tick);
  int32_t target_hp = base_hp + step;
  if (target_hp > context.max_hp) {
    target_hp = context.max_hp;
  }
  const int32_t applied_step = target_hp - base_hp;

  __try {
    g_hp_set_current(context.hp_component, target_hp, 0, 0.0f, 1.0f, 1);
  } __except (LogSehException("HpSetCurrent original", GetExceptionInformation())) {
    LogError(
        "%s exception slot=%d entity=%08x:%08x hpComponent=%p currentHp=%d maxHp=%d targetHp=%d",
        tag,
        buddy_slot,
        context.entity.high,
        context.entity.low,
        context.hp_component,
        context.current_hp,
        context.max_hp,
        target_hp);
    return false;
  }

  if (after_hp != nullptr) {
    *after_hp = ReadI32(static_cast<const uint8_t*>(context.hp_component) + kHpCurrentOffset, 0);
  }
  if (step_hp != nullptr) {
    *step_hp = applied_step;
  }
  if (regen_tick_count != nullptr) {
    *regen_tick_count = current_regen_tick + 1;
  }
  return true;
}


// Original src/dllmain.cpp:5528
int FindBuddySlotByEntity(void* entity) {
  if (entity == nullptr || g_last_buddy_state == nullptr) {
    return -1;
  }

  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    const SlotSnapshot snapshot = ReadSlot(g_last_buddy_state, slot);
    if (IsEmptyEntity(snapshot)) {
      continue;
    }

    const EntityId entity_id{snapshot.entity_low, snapshot.entity_high};
    if (LookupEntityById(entity_id) == entity) {
      return slot;
    }
  }

  return -1;
}


// Original src/dllmain.cpp:5548
uint8_t __fastcall HookApplySpEffect(
    void* entity,
    uint32_t sp_effect_id,
    uint8_t mode) {
  const uintptr_t caller_rva = RvaOf(_ReturnAddress());
  uint8_t result = 0;
  __try {
    result = g_apply_sp_effect(entity, sp_effect_id, mode);
  } __except (LogSehException("ApplySpEffect original", GetExceptionInformation())) {
    LogError(
        "ApplySpEffect/FUN_1403c7fd0 original_exception entity=%p spEffect=%u mode=%u callerRva=%llx",
        entity,
        sp_effect_id,
        static_cast<unsigned int>(mode),
        static_cast<unsigned long long>(caller_rva));
    return 0;
  }

  __try { TraceBhrcReconnectEffectApply(entity, sp_effect_id, mode, result, caller_rva); }
  __except (EXCEPTION_EXECUTE_HANDLER) {
    LogWarn("BHRC_EFFECT_TRACE apply_probe_fault effectId=%u nativeResultUnchanged=1", sp_effect_id);
  }

  const bool native_deploy_boundary =
      caller_rva >= kBuddyDeathRecoveryUpdateRva &&
      caller_rva < kBuddyDeathRecoveryUpdateEndRva;
  if (!native_deploy_boundary || entity == nullptr || g_last_buddy_state == nullptr) {
    return result;
  }

  const int buddy_slot = FindBuddySlotByEntity(entity);
  if (!IsValidBuddySlot(buddy_slot)) {
    return result;
  }

  const SlotSnapshot snapshot = ReadSlot(g_last_buddy_state, buddy_slot);
  const EntityId entity_id{snapshot.entity_low, snapshot.entity_high};
  ApplyCustomCharaBuddyPreventNearDeath(
      buddy_slot,
      entity,
      entity_id,
      snapshot.state,
      snapshot.pending,
      true,
      "native_deploy_boundary");
  return result;
}


// Original src/dllmain.cpp:5591
void* GetWorldChrMan() {
  if (g_game_base == 0) {
    return nullptr;
  }

  const uint64_t world_chr_man =
      ReadU64(reinterpret_cast<const void*>(g_game_base + kWorldChrManPtrRva), 0);
  return reinterpret_cast<void*>(world_chr_man);
}


// Original src/dllmain.cpp:5737
void* LookupChrByGameDataEntityId(uint32_t entity_id) {
  if (entity_id == kInvalidEntityPart || g_world_chr_lookup_by_game_data_id == nullptr) {
    return nullptr;
  }

  void* world_chr_man = GetWorldChrMan();
  if (world_chr_man == nullptr) {
    return nullptr;
  }

  __try {
    return g_world_chr_lookup_by_game_data_id(world_chr_man, entity_id);
  } __except (LogSehException("WorldChrManLookupByGameDataId original", GetExceptionInformation())) {
    LogError(
        "WorldChrManLookupByGameDataId original_exception world=%p entityId=%08x",
        world_chr_man,
        entity_id);
    return nullptr;
  }
}


// Original src/dllmain.cpp:5758
void* GetRemotePlayerGameDataEntry(int remote_index) {
  if (remote_index < 0 || remote_index >= static_cast<int>(kExpandedRemotePlayerGameDataSlots)) {
    return nullptr;
  }

  auto* manager = static_cast<uint8_t*>(GetPlayerGameDataManager());
  if (manager == nullptr) {
    return nullptr;
  }

  const uint64_t entries = ReadU64(manager + 0x10, 0);
  if (entries == 0) {
    return nullptr;
  }
  return reinterpret_cast<void*>(
      entries + static_cast<uint64_t>(remote_index) * kNpcPlayerGameDataEntrySize);
}


// Original src/dllmain.cpp:5776
const char* DescribeDiagnosticCallerRva(uintptr_t caller_rva) {
  if (caller_rva == 0) {
    return "none";
  }

  // WorldChr registration callers. These labels distinguish normal entity
  // finalization from static WorldChrMan setup when diagnosing missing player
  // map markers.
  if (caller_rva >= 0x51b240 && caller_rva < 0x51b300) {
    return "WorldChrInsert/FUN_14051b240";
  }
  if (caller_rva >= 0x51c0d0 && caller_rva < 0x51c140) {
    return "WorldChrAreaDispatch/FUN_14051c0d0";
  }
  if (caller_rva >= 0x51bbc0 && caller_rva < 0x51de60) {
    return "WorldChrManInit/FUN_14051bbc0";
  }
  if (caller_rva >= 0x51ff90 && caller_rva < 0x520d30) {
    return "WorldChrPacketDispatch/FUN_14051ff90";
  }
  if (caller_rva >= 0x520d30 && caller_rva < 0x5229c0) {
    return "WorldChrUpdateA/FUN_140520d30";
  }
  if (caller_rva >= 0x5229c0 && caller_rva < 0x523930) {
    return "WorldChrUpdateB/FUN_1405229c0";
  }
  if (caller_rva >= 0x523930 && caller_rva < 0x524000) {
    return "WorldChrUpdateC/FUN_140523930";
  }
  if (caller_rva >= 0x527d80 && caller_rva < 0x528b40) {
    return "ChrWorldAttach/FUN_140527d80";
  }
  if (caller_rva >= 0x528b40 && caller_rva < 0x529200) {
    return "ChrFinalize/FUN_140528b40";
  }
  if (caller_rva >= 0x64fcb0 && caller_rva < 0x650a30) {
    return "ChrSystemRegister/FUN_14064fcb0";
  }
  if (caller_rva >= 0x67a5a0 && caller_rva < 0x67b890) {
    return "ChrSystemReregister/FUN_14067a5a0";
  }

  // Map and indicator readers. If these appear above a recovery frame, the UI
  // tick observed the recovered ChrIns after the world registration path ran.
  if (caller_rva >= 0x90ee00 && caller_rva < 0x90f2d0) {
    return "WorldMapRemoteMarkerUpdate/FUN_14090ee00";
  }
  if (caller_rva >= 0xbeb630 && caller_rva < 0xbeca00) {
    return "PlayerIndicatorRefresh/FUN_140beb630";
  }

  // Remote PlayerGameData and network/session paths. These are the likely
  // source when a marker stays at the default center because the remote entity
  // id has not been assigned yet or was briefly cleared.
  if (caller_rva >= 0x1fc260 && caller_rva < 0x1fdd80) {
    return "RemotePlayerGameDataRefresh/FUN_1401fc260";
  }
  if (caller_rva >= 0xdf1de0 && caller_rva < 0xdf2270) {
    return "NetRecvAnyPeer/FUN_140df1de0";
  }
  if (caller_rva >= 0xdf2270 && caller_rva < 0xdf2400) {
    return "NetSendCore/FUN_140df2270";
  }
  if (caller_rva >= 0xdf1130 && caller_rva < 0xdf1de0) {
    return "SessionSubmit/FUN_140df1130";
  }
  if (caller_rva >= 0xe0bc80 && caller_rva < 0xe0c200) {
    return "PlayerNetworkSessionSubmit/FUN_140e0bc80";
  }

  // Game-data event callers that can refresh remote PlayerGameData and make a
  // previously missing ChrIns lookup recover on the next map/indicator tick.
  if (caller_rva >= 0xc27870 && caller_rva < 0xc284b0) {
    return "GameDataEventUpdate/FUN_140c27870";
  }
  if (caller_rva >= 0xc284b0 && caller_rva < 0xc2e2b0) {
    return "GameDataEventLoop/FUN_140c284b0";
  }
  if (caller_rva >= 0xc2e2b0 && caller_rva < 0xc473e0) {
    return "GameDataEventDispatch/FUN_140c2e2b0";
  }
  if (caller_rva >= 0xc473e0 && caller_rva < 0xc4adf0) {
    return "GameDataMapTransition/FUN_140c473e0";
  }
  if (caller_rva >= 0xc4adf0 && caller_rva < 0xc4b200) {
    return "GameDataStep/FUN_140c4adf0";
  }
  if (caller_rva >= 0xcbb892 && caller_rva < 0xcbc000) {
    return "GameDataFallback/FUN_140cbb892";
  }

  return "unknown";
}


// Original src/dllmain.cpp:5870
EntityId ReadStateOwnerEntityId(void* state) {
  if (state == nullptr) {
    return EntityId{kInvalidEntityPart, kInvalidEntityPart};
  }
  return EntityId{
      ReadU32(state, kInvalidEntityPart),
      ReadU32(static_cast<const uint8_t*>(state) + 4, kInvalidEntityPart)};
}


// Original src/dllmain.cpp:5879
EntityId ReadChrEntityId(void* chr) {
  if (chr == nullptr) {
    return EntityId{kInvalidEntityPart, kInvalidEntityPart};
  }
  return EntityId{
      ReadU32(static_cast<const uint8_t*>(chr) + 0x78, kInvalidEntityPart),
      ReadU32(static_cast<const uint8_t*>(chr) + 0x7c, kInvalidEntityPart)};
}


// Original src/dllmain.cpp:5893
uint64_t GetWorldMapRemoteMarker(void* world_map_model, int marker_index) {
  if (world_map_model == nullptr ||
      marker_index < 0 ||
      marker_index >= static_cast<int>(kExpandedRemotePlayerMarkerCount)) {
    return 0;
  }

  const uint64_t marker_table = ReadU64(static_cast<const uint8_t*>(world_map_model) + 0xb0, 0);
  if (marker_table == 0) {
    return 0;
  }
  return ReadU64(
      reinterpret_cast<const void*>(marker_table + static_cast<uint64_t>(marker_index) * sizeof(uint64_t)),
      0);
}


// Original src/dllmain.cpp:5909
bool IsWorldMapMarkerIconResourceEmpty(uint64_t marker) {
  if (marker == 0) {
    return true;
  }

  return ReadU64(reinterpret_cast<const void*>(marker + 0x68), 0) == 0 &&
      ReadU64(reinterpret_cast<const void*>(marker + 0x70), 0) == 0 &&
      ReadU64(reinterpret_cast<const void*>(marker + 0x78), 0) == 0 &&
      ReadU64(reinterpret_cast<const void*>(marker + 0x80), 0) == 0;
}


// Original src/dllmain.cpp:5920
bool CopyWorldMapMarkerIconResource(uint64_t target_marker, uint64_t source_marker) {
  if (target_marker == 0 || source_marker == 0 ||
      IsWorldMapMarkerIconResourceEmpty(source_marker)) {
    return false;
  }

  // FUN_14090a280 stores the resolved icon resource descriptor at 0x68..0xa0.
  // Extra player markers can have an empty descriptor even after the marker
  // object exists. Copying a vanilla marker descriptor gives the UI a valid
  // resource container; the caller then refreshes id/type from the target ChrIns
  // so player 4-6 do not inherit player 1-3's class portrait.
  for (size_t offset = 0x68; offset <= 0xa0; offset += sizeof(uint64_t)) {
    if (!WriteU64(
            reinterpret_cast<void*>(target_marker + offset),
            ReadU64(reinterpret_cast<const void*>(source_marker + offset), 0))) {
      return false;
    }
  }
  return true;
}


// Original src/dllmain.cpp:5941
int MirroredPlayerAvatarSlotForMarker(int marker_index) {
  if (marker_index < 0) {
    return 0;
  }

  // World-map remote markers are 0-based and exclude the local player. The
  // vanilla remote-marker resource builder only initializes two remote
  // containers. Expanded entries must mirror one of those initialized
  // containers first, then refresh id/type from the target ChrIns below.
  return marker_index % static_cast<int>(kVanillaRemotePlayerMarkerCount);
}


// Original src/dllmain.cpp:5953
uint32_t NormalizeExpandedPlayerAvatarIconId(uint32_t icon_id) {
  // FUN_14066c6f0 returns a player portrait column id, not the final
  // MENU_MenuIcon_416xx resource id. Players can move between marker slots as
  // ownership/order changes, so fold by the returned player column rather than
  // the marker index. Keep iconType unchanged to preserve the class portrait row.
  if (icon_id >= 4 && icon_id <= 6) {
    return icon_id - 3;
  }
  return icon_id;
}


// Original src/dllmain.cpp:5964
uint32_t ReadChrIconIdForWorldMap(void* chr) {
  if (chr == nullptr || g_chr_icon_id == nullptr) {
    return kInvalidEntityPart;
  }

  uint32_t icon_id = kInvalidEntityPart;
  __try {
    uint32_t* result = g_chr_icon_id(chr, &icon_id);
    if (result != nullptr) {
      icon_id = *result;
    }
  } __except (LogSehException("ChrIconId original", GetExceptionInformation())) {
    LogError("ChrIconId original_exception chr=%p", chr);
    return kInvalidEntityPart;
  }
  return icon_id;
}


// Original src/dllmain.cpp:5982
uint8_t ReadChrIconTypeForWorldMap(void* chr) {
  if (chr == nullptr) {
    return 0xff;
  }

  const uint64_t vtable = ReadU64(chr, 0);
  const uint64_t function = vtable != 0 ? ReadU64(reinterpret_cast<const void*>(vtable + 0x178), 0) : 0;
  if (function == 0) {
    return 0xff;
  }

  using ChrIconTypeFn = uint8_t(__fastcall*)(void*);
  __try {
    return reinterpret_cast<ChrIconTypeFn>(function)(chr);
  } __except (LogSehException("ChrIconType vfunc", GetExceptionInformation())) {
    LogError("ChrIconType vfunc_exception chr=%p vtable=%016llx function=%016llx",
        chr,
        static_cast<unsigned long long>(vtable),
        static_cast<unsigned long long>(function));
    return 0xff;
  }
}


// Original src/dllmain.cpp:6005
void* ResolveWorldMapMarkerChr(int marker_index, void* input_chr) {
  if (input_chr != nullptr) {
    return input_chr;
  }

  void* player_data = GetRemotePlayerGameDataEntry(marker_index);
  const uint32_t player_data_entity =
      player_data != nullptr ? ReadU32(static_cast<const uint8_t*>(player_data) + 8, kInvalidEntityPart)
                             : kInvalidEntityPart;
  return LookupChrByGameDataEntityId(player_data_entity);
}


// Original src/dllmain.cpp:6017
void RepairWorldMapMarkerAvatarResource(
    void* world_map_model,
    int marker_index,
    void* input_chr,
    void* resolved_chr,
    uint8_t is_local) {
  if (!g_config.fix_player_map_markers || world_map_model == nullptr ||
      marker_index < 0 || marker_index >= static_cast<int>(kExpandedRemotePlayerMarkerCount)) {
    return;
  }

  void* chr = resolved_chr;
  if (chr != nullptr && input_chr == nullptr && g_world_map_remote_marker_update != nullptr) {
    __try {
      g_world_map_remote_marker_update(world_map_model, marker_index, chr, is_local);
    } __except (LogSehException("WorldMapRemoteMarkerUpdate repair", GetExceptionInformation())) {
      LogError(
          "WorldMapRemoteMarkerUpdate repair_exception model=%p markerIndex=%d chr=%p isLocal=%u",
          world_map_model,
          marker_index,
          chr,
          is_local);
      return;
    }
  }

  const uint64_t marker = GetWorldMapRemoteMarker(world_map_model, marker_index);
  if (marker == 0) {
    return;
  }

  const bool resource_was_empty = IsWorldMapMarkerIconResourceEmpty(marker);
  if (resource_was_empty && marker_index >= static_cast<int>(kVanillaRemotePlayerMarkerCount)) {
    const int mirror_marker_index = MirroredPlayerAvatarSlotForMarker(marker_index);
    CopyWorldMapMarkerIconResource(
        marker,
        GetWorldMapRemoteMarker(world_map_model, mirror_marker_index));
  }

  const int32_t old_icon_id = ReadI32(reinterpret_cast<const void*>(marker + 0x1c), -1);
  const uint8_t old_icon_type = ReadU8(reinterpret_cast<const void*>(marker + 0x60), 0xff);
  const bool resource_empty = IsWorldMapMarkerIconResourceEmpty(marker);
  if (chr == nullptr) {
    return;
  }

  const uint32_t raw_icon_id = ReadChrIconIdForWorldMap(chr);
  const uint32_t icon_id = NormalizeExpandedPlayerAvatarIconId(raw_icon_id);
  const uint8_t icon_type = ReadChrIconTypeForWorldMap(chr);
  if (icon_id == kInvalidEntityPart || icon_type == 0xff ||
      g_world_map_set_icon_id == nullptr || g_world_map_set_icon_type == nullptr) {
    return;
  }

  const bool icon_matches =
      old_icon_id == static_cast<int32_t>(icon_id) && old_icon_type == icon_type;
  if (icon_matches && !resource_empty && !resource_was_empty) {
    return;
  }

  bool forced_icon_id_refresh = false;
  bool forced_icon_type_refresh = false;
  if (old_icon_id == static_cast<int32_t>(icon_id)) {
    forced_icon_id_refresh = WriteI32(reinterpret_cast<void*>(marker + 0x1c), -1);
  }
  if (old_icon_type == icon_type || resource_empty) {
    forced_icon_type_refresh = WriteU8(reinterpret_cast<void*>(marker + 0x60), 0xff);
  }

  __try {
    g_world_map_set_icon_id(reinterpret_cast<void*>(marker), static_cast<int>(icon_id));
  } __except (LogSehException("WorldMapSetIconId repair", GetExceptionInformation())) {
    if (forced_icon_id_refresh) {
      WriteI32(reinterpret_cast<void*>(marker + 0x1c), old_icon_id);
    }
    if (forced_icon_type_refresh) {
      WriteU8(reinterpret_cast<void*>(marker + 0x60), old_icon_type);
    }
    LogError(
        "WorldMapSetIconId repair_exception markerIndex=%d marker=%016llx iconId=%08x",
        marker_index,
        static_cast<unsigned long long>(marker),
        icon_id);
    return;
  }

  __try {
    g_world_map_set_icon_type(reinterpret_cast<void*>(marker), icon_type);
  } __except (LogSehException("WorldMapSetIconType repair", GetExceptionInformation())) {
    if (forced_icon_type_refresh) {
      WriteU8(reinterpret_cast<void*>(marker + 0x60), old_icon_type);
    }
    LogError(
        "WorldMapSetIconType repair_exception markerIndex=%d marker=%016llx iconType=%u",
        marker_index,
        static_cast<unsigned long long>(marker),
        icon_type);
    return;
  }
}


// Original src/dllmain.cpp:6118
uint32_t EntityHighIndex(const EntityId& entity_id) {
  if (entity_id.high == kInvalidEntityPart) {
    return 0xffffffff;
  }
  return (entity_id.high >> 0x0b) & 0x7f;
}


// Original src/dllmain.cpp:6125
void* GetWorldOwnerGroup(void* world_chr_man) {
  if (world_chr_man == nullptr) {
    return nullptr;
  }
  return reinterpret_cast<void*>(
      ReadU64(static_cast<const uint8_t*>(world_chr_man) + 0x17530, 0));
}


// Original src/dllmain.cpp:6133
int32_t GetLocalBaseOwnerIndex(void* world_chr_man) {
  if (world_chr_man == nullptr) {
    return -1;
  }

  const uint64_t local_index_owner =
      ReadU64(static_cast<const uint8_t*>(world_chr_man) + 0x9910, 0);
  if (local_index_owner == 0) {
    return -1;
  }
  return ReadI32(reinterpret_cast<const void*>(local_index_owner + 0x120), -1);
}


// Original src/dllmain.cpp:6146
int32_t GetWorldOwnerGroupCapacity(void* world_owner_group) {
  if (world_owner_group == nullptr) {
    return -1;
  }
  return ReadI32(static_cast<const uint8_t*>(world_owner_group) + 8, -1);
}


// Original src/dllmain.cpp:6153
uint64_t ReadWorldOwnerGroupSlot(void* world_owner_group, uint32_t entity_index) {
  const int32_t capacity = GetWorldOwnerGroupCapacity(world_owner_group);
  if (world_owner_group == nullptr || entity_index == 0xffffffff ||
      static_cast<int32_t>(entity_index) < 0 ||
      static_cast<int32_t>(entity_index) >= capacity) {
    return 0;
  }

  return ReadU64(
      static_cast<const uint8_t*>(world_owner_group) + 0x10 +
          static_cast<size_t>(entity_index) * sizeof(uint64_t),
      0);
}


// Original src/dllmain.cpp:6167
void* WorldOwnerGroupSlotAddress(void* world_owner_group, uint32_t entity_index) {
  const int32_t capacity = GetWorldOwnerGroupCapacity(world_owner_group);
  if (world_owner_group == nullptr || entity_index == 0xffffffff ||
      static_cast<int32_t>(entity_index) < 0 ||
      static_cast<int32_t>(entity_index) >= capacity) {
    return nullptr;
  }

  return static_cast<uint8_t*>(world_owner_group) + 0x10 +
      static_cast<size_t>(entity_index) * sizeof(uint64_t);
}


// Original src/dllmain.cpp:6179
uint8_t SafeCallChrGate(ChrGateFn fn, void* chr, bool* ok) {
  if (ok != nullptr) {
    *ok = false;
  }
  if (fn == nullptr || chr == nullptr) {
    return 0;
  }
  __try {
    const uint8_t result = fn(chr);
    if (ok != nullptr) {
      *ok = true;
    }
    return result;
  } __except (LogSehException("ChrGate original", GetExceptionInformation())) {
    LogError("ChrGate original_exception fn=%p chr=%p", reinterpret_cast<void*>(fn), chr);
    return 0;
  }
}


// Original src/dllmain.cpp:6198
void* GetNetModeManager() {
  if (g_game_base == 0) {
    return nullptr;
  }
  return reinterpret_cast<void*>(
      ReadU64(reinterpret_cast<const void*>(g_game_base + kNetModeManagerPtrRva), 0));
}


// Original src/dllmain.cpp:6645
uint64_t __fastcall HookPartyMemberRemove(
    void* party_member_info,
    uint64_t member_id,
    uint64_t reason) {
  const uintptr_t caller_rva = RvaOf(_ReturnAddress());
  uint64_t result = 0;
  __try {
    result = g_party_member_remove(party_member_info, member_id, reason);
  } __except (LogSehException("PartyMemberRemove original", GetExceptionInformation())) {
    LogError(
        "PartyMemberRemove original_exception info=%p member=%016llx reason=%016llx callerRva=%llx",
        party_member_info,
        static_cast<unsigned long long>(member_id),
        static_cast<unsigned long long>(reason),
        static_cast<unsigned long long>(caller_rva));
    return 0;
  }
  NormalizePartyMemberInfoAfterRemove(party_member_info, caller_rva, member_id, reason, result);
  return result;
}


// Original src/dllmain.cpp:6971
uint8_t SafeCallChrVfunc120(void* chr, bool* ok) {
  if (ok != nullptr) {
    *ok = false;
  }
  if (chr == nullptr) {
    return 0;
  }

  const uint64_t vtable = ReadU64(chr, 0);
  const uint64_t fn = vtable != 0 ? ReadU64(reinterpret_cast<const void*>(vtable + 0x120), 0) : 0;
  if (fn == 0) {
    return 0;
  }

  using Vfunc120Fn = uint8_t(__fastcall*)(void*);
  __try {
    const uint8_t result = reinterpret_cast<Vfunc120Fn>(fn)(chr);
    if (ok != nullptr) {
      *ok = true;
    }
    return result;
  } __except (LogSehException("ChrVfunc120 original", GetExceptionInformation())) {
    LogError(
        "ChrVfunc120 original_exception chr=%p vtable=%016llx fn=%016llx",
        chr,
        static_cast<unsigned long long>(vtable),
        static_cast<unsigned long long>(fn));
    return 0;
  }
}


// Original src/dllmain.cpp:7002
OwnerGateSnapshot CaptureOwnerGateSnapshot(void* state) {
  OwnerGateSnapshot gate{};
  gate.owner_entity = ReadStateOwnerEntityId(state);
  gate.owner_chr = LookupEntityById(gate.owner_entity);
  gate.owner_lookup_ok = gate.owner_chr != nullptr;

  void* world_chr_man = GetWorldChrMan();
  bool is_local_call_ok = false;
  gate.is_local_owner =
      SafeCallChrGate(g_is_local_owner, gate.owner_chr, &is_local_call_ok) != 0;

  gate.owner_entity_index = EntityHighIndex(gate.owner_entity);
  if (world_chr_man != nullptr) {
    gate.world_owner_group =
        reinterpret_cast<void*>(ReadU64(static_cast<const uint8_t*>(world_chr_man) + 0x17530, 0));
  }

  if (gate.world_owner_group != nullptr) {
    gate.world_owner_group_capacity =
        ReadI32(static_cast<const uint8_t*>(gate.world_owner_group) + 8, -1);
    gate.owner_index_in_group_range =
        gate.owner_entity_index != 0xffffffff &&
        static_cast<int32_t>(gate.owner_entity_index) >= 0 &&
        static_cast<int32_t>(gate.owner_entity_index) < gate.world_owner_group_capacity;
    if (gate.owner_index_in_group_range) {
      gate.owner_group_slot = ReadU64(
          static_cast<const uint8_t*>(gate.world_owner_group) + 0x10 +
              static_cast<size_t>(gate.owner_entity_index) * sizeof(uint64_t),
          0);
    }
  }

  gate.remote_gate = SafeCallChrGate(g_remote_owner_gate, gate.owner_chr, &gate.remote_gate_call_ok) != 0;
  gate.vfunc120 = SafeCallChrVfunc120(gate.owner_chr, &gate.vfunc120_call_ok) != 0;
  gate.final_allows_buddy_update =
      gate.is_local_owner || (gate.remote_gate_call_ok && gate.remote_gate &&
                              gate.vfunc120_call_ok && !gate.vfunc120);

  return gate;
}


// Original src/dllmain.cpp:7043
bool IsLocalBuddyAuthority(void* state, bool* authority_known) {
  const EntityId owner_entity = ReadStateOwnerEntityId(state);
  void* owner_chr = LookupEntityById(owner_entity);
  bool is_local_call_ok = false;
  const bool local_authority =
      SafeCallChrGate(g_is_local_owner, owner_chr, &is_local_call_ok) != 0;
  const bool known = owner_chr != nullptr && is_local_call_ok;
  if (authority_known != nullptr) {
    *authority_known = known;
  }
  return known && local_authority;
}


// Original src/dllmain.cpp:7056
BuddySlotRuntimeState* FindTrackedCustomCharaBuddyForController(
    void* player_ins,
    int* context_index,
    int* buddy_slot) {
  if (context_index != nullptr) {
    *context_index = -1;
  }
  if (buddy_slot != nullptr) {
    *buddy_slot = -1;
  }
  if (player_ins == nullptr) {
    return nullptr;
  }

  const EntityId entity_id{
      ReadU32(static_cast<const uint8_t*>(player_ins) + 0x78, kInvalidEntityPart),
      ReadU32(static_cast<const uint8_t*>(player_ins) + 0x7c, kInvalidEntityPart)};
  if (IsEmptyEntityId(entity_id)) {
    return nullptr;
  }

  for (int context = 0; context < kTrackedBuddyStateCount; ++context) {
    for (int slot = 0; slot < kBuddySlotCount; ++slot) {
      auto& runtime = g_buddy_state_contexts[context].buddy_slot_runtime[slot];
      if (!runtime.custom_buddy || !runtime.custom_chara_init ||
          !SameEntity(runtime.entity, entity_id)) {
        continue;
      }
      if (context_index != nullptr) {
        *context_index = context;
      }
      if (buddy_slot != nullptr) {
        *buddy_slot = slot;
      }
      return &runtime;
    }
  }
  return nullptr;
}


// Original src/dllmain.cpp:7115
uint8_t __fastcall ResolvePlayerInsControllerAuthority(void* player_ins) {
  const CustomBuddyControllerAuthorityTls authority =
      g_custom_buddy_controller_authority_tls;
  g_custom_buddy_controller_authority_tls =
      CustomBuddyControllerAuthorityTls{};

  uint8_t native_result = 0;
  __try {
    native_result = g_remote_owner_gate(player_ins);
  } __except (LogSehException(
      "PlayerInsControllerAuthority original",
      GetExceptionInformation())) {
    LogError(
        "BuddyNetworkController authority original_exception playerIns=%p",
        player_ins);
    return 0;
  }

  // This resolver is reached only through the single patched CALL inside
  // FUN_140670510. Eligibility already identified the exact custom Buddy on
  // this thread, so authority consumes that one-shot identity instead of
  // rescanning runtime or remote-player slots.
  if (player_ins == nullptr || authority.player_ins != player_ins ||
      authority.buddy_state == nullptr ||
      !IsValidBuddySlot(authority.buddy_slot)) {
    return native_result;
  }

  bool authority_known = false;
  const bool local_authority =
      IsLocalBuddyAuthority(authority.buddy_state, &authority_known);
  if (!authority_known) {
    return native_result;
  }

  const uint8_t corrected_result = local_authority ? 1 : 0;
  const int log_context =
      authority.context_index >= 0 &&
          authority.context_index < kTrackedBuddyStateCount
      ? authority.context_index
      : kFallbackBuddyStateContextIndex;
  auto& log_state =
      g_custom_buddy_controller_authority_log[log_context][
          authority.buddy_slot];
  if (native_result != corrected_result &&
      (log_state.player_ins != player_ins ||
       log_state.native_result != native_result ||
       log_state.corrected_result != corrected_result)) {
    log_state.player_ins = player_ins;
    log_state.native_result = native_result;
    log_state.corrected_result = corrected_result;
    const EntityId entity{
        ReadU32(static_cast<const uint8_t*>(player_ins) + 0x78,
                kInvalidEntityPart),
        ReadU32(static_cast<const uint8_t*>(player_ins) + 0x7c,
                kInvalidEntityPart)};
    const EntityId owner = ReadStateOwnerEntityId(authority.buddy_state);
    Log(
        "BuddyNetworkController authority_resolved playerIns=%p entity=%08x:%08x state=%p owner=%08x:%08x ctx=%d slot=%d nativeLocal=%u correctedLocal=%u changed=%d caller=FUN_140670510 source=%s",
        player_ins,
        entity.high,
        entity.low,
        authority.buddy_state,
        owner.high,
        owner.low,
        authority.context_index,
        authority.buddy_slot,
        static_cast<unsigned int>(native_result),
        static_cast<unsigned int>(corrected_result),
        native_result != corrected_result ? 1 : 0,
        authority.active_create ? "active_create" : "runtime");
  }
  return corrected_result;
}


// Original src/dllmain.cpp:7192
uint8_t __fastcall HookPlayerInsControllerEligibility(void* player_ins) {
  const uintptr_t caller_rva = RvaOf(_ReturnAddress());
  if (caller_rva == kPlayerInsControllerSelectorReturnRva) {
    g_custom_buddy_controller_authority_tls =
        CustomBuddyControllerAuthorityTls{};
  }
  uint8_t original_result = 0;
  __try {
    original_result = g_player_ins_controller_eligibility(player_ins);
  } __except (LogSehException(
      "PlayerInsControllerEligibility original",
      GetExceptionInformation())) {
    LogError(
        "BuddyNetworkController eligibility original_exception playerIns=%p callerRva=%llx",
        player_ins,
        static_cast<unsigned long long>(caller_rva));
    return 0;
  }

  // FUN_14066d630 is a general PlayerIns identity predicate. Relax it only for
  // its call from FUN_140670510, the native controller selector. This leaves
  // every other subsystem's 10000..10039 player identity test unchanged.
  if (player_ins == nullptr ||
      caller_rva != kPlayerInsControllerSelectorReturnRva) {
    return original_result;
  }

  const uint64_t player_game_data =
      ReadU64(static_cast<const uint8_t*>(player_ins) + 0x5a0, 0);
  const int32_t player_game_data_id = player_game_data != 0
      ? ReadI32(reinterpret_cast<const void*>(player_game_data + 8), -1)
      : -1;
  int context_index = -1;
  int buddy_slot = -1;
  void* buddy_state = nullptr;
  const bool active_create_match =
      g_active_create_context.active &&
      g_active_create_context.custom_candidate &&
      g_active_create_context.chara_init >= 0 &&
      player_game_data_id == g_active_create_context.chara_init;
  BuddySlotRuntimeState* runtime = active_create_match
      ? nullptr
      : FindTrackedCustomCharaBuddyForController(
          player_ins,
          &context_index,
          &buddy_slot);
  if (!active_create_match && runtime == nullptr) {
    return original_result;
  }

  if (active_create_match) {
    buddy_state = g_active_create_context.state;
    buddy_slot = g_active_create_context.buddy_slot;
    context_index = g_active_create_context.owner_slot;
    if (context_index < 0 || context_index >= kTrackedBuddyStateCount) {
      context_index = kFallbackBuddyStateContextIndex;
    }
  } else if (context_index >= 0 &&
             context_index < kTrackedBuddyStateCount) {
    buddy_state = g_buddy_state_contexts[context_index].state;
  }
  if (buddy_state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return original_result;
  }

  const uint64_t local_controller =
      ReadU64(static_cast<const uint8_t*>(player_ins) + 0x5a8, 0);
  const uint64_t remote_controller =
      ReadU64(static_cast<const uint8_t*>(player_ins) + 0x5b0, 0);
  const uint64_t chr_ctrl =
      ReadU64(static_cast<const uint8_t*>(player_ins) + 0x60, 0);
  const uint64_t selected_controller = chr_ctrl != 0
      ? ReadU64(reinterpret_cast<const void*>(chr_ctrl + 0x18), 0)
      : 0;
  if (local_controller == 0 || remote_controller == 0 || chr_ctrl == 0) {
    LogWarn(
        "BuddyNetworkController native_selector_not_ready playerIns=%p playerGameData=%p gameDataId=%d ctx=%d slot=%d localController=%p remoteController=%p chrCtrl=%p selected=%p source=%s",
        player_ins,
        reinterpret_cast<void*>(player_game_data),
        player_game_data_id,
        context_index,
        buddy_slot,
        reinterpret_cast<void*>(local_controller),
        reinterpret_cast<void*>(remote_controller),
        reinterpret_cast<void*>(chr_ctrl),
        reinterpret_cast<void*>(selected_controller),
        active_create_match ? "active_create" : "runtime");
    if (original_result != 0) {
      g_custom_buddy_controller_authority_tls.player_ins = player_ins;
      g_custom_buddy_controller_authority_tls.buddy_state = buddy_state;
      g_custom_buddy_controller_authority_tls.context_index = context_index;
      g_custom_buddy_controller_authority_tls.buddy_slot = buddy_slot;
      g_custom_buddy_controller_authority_tls.active_create =
          active_create_match;
    }
    return original_result;
  }

  const int log_context = context_index >= 0 && context_index < kTrackedBuddyStateCount
      ? context_index
      : kFallbackBuddyStateContextIndex;
  const int log_slot = IsValidBuddySlot(buddy_slot) ? buddy_slot : 0;
  if (original_result == 0 &&
      g_custom_buddy_controller_eligibility_logged[log_context][log_slot] !=
          player_ins) {
    g_custom_buddy_controller_eligibility_logged[log_context][log_slot] = player_ins;
    Log(
        "BuddyNetworkController native_selector_eligibility_override playerIns=%p playerGameData=%p gameDataId=%d ctx=%d slot=%d localController=%p remoteController=%p chrCtrl=%p selectedBefore=%p caller=FUN_140670510 source=%s",
        player_ins,
        reinterpret_cast<void*>(player_game_data),
        player_game_data_id,
        context_index,
        buddy_slot,
        reinterpret_cast<void*>(local_controller),
        reinterpret_cast<void*>(remote_controller),
        reinterpret_cast<void*>(chr_ctrl),
        reinterpret_cast<void*>(selected_controller),
        active_create_match ? "active_create" : "runtime");
  }
  g_custom_buddy_controller_authority_tls.player_ins = player_ins;
  g_custom_buddy_controller_authority_tls.buddy_state = buddy_state;
  g_custom_buddy_controller_authority_tls.context_index = context_index;
  g_custom_buddy_controller_authority_tls.buddy_slot = buddy_slot;
  g_custom_buddy_controller_authority_tls.active_create =
      active_create_match;
  return 1;
}


// Original src/dllmain.cpp:7321
void SnapshotBuddySlots(void* state, SlotSnapshot slots[kBuddySlotCount]) {
  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    slots[slot] = ReadSlot(state, slot);
  }
}


// Original src/dllmain.cpp:7327
void* GameAllocate(size_t size, size_t alignment) {
  const uint64_t allocator = ReadU64(reinterpret_cast<const void*>(g_game_base + kGameAllocatorPtrRva), 0);
  if (allocator == 0) {
    LogError("BuddyEntitySlotExpand failed reason=no_allocator");
    return nullptr;
  }

  const uint64_t vtable = ReadU64(reinterpret_cast<const void*>(allocator), 0);
  const uint64_t alloc_fn = ReadU64(reinterpret_cast<const void*>(vtable + 0x50), 0);
  if (alloc_fn == 0) {
    LogError("BuddyEntitySlotExpand failed reason=no_alloc_fn allocator=%016llx vtable=%016llx",
        static_cast<unsigned long long>(allocator),
        static_cast<unsigned long long>(vtable));
    return nullptr;
  }

  return reinterpret_cast<GameAllocatorAllocFn>(alloc_fn)(
      reinterpret_cast<void*>(allocator),
      size,
      alignment);
}


// Original src/dllmain.cpp:7349
PlayerGameDataTableExpansion* FindPlayerGameDataExpansionByManager(void* manager) {
  if (manager == nullptr) {
    return nullptr;
  }

  for (auto& expansion : g_player_game_data_table_expansions) {
    if (expansion.manager == manager && expansion.expanded_table != 0) {
      return &expansion;
    }
  }
  return nullptr;
}


// Original src/dllmain.cpp:7362
PlayerGameDataTableExpansion* FindPlayerGameDataExpansionByTable(uint64_t table) {
  if (table == 0) {
    return nullptr;
  }

  for (auto& expansion : g_player_game_data_table_expansions) {
    if (expansion.expanded_table == table && expansion.expanded_table != 0) {
      return &expansion;
    }
  }
  return nullptr;
}


// Original src/dllmain.cpp:7375
PlayerGameDataTableExpansion* ReservePlayerGameDataExpansionSlot() {
  for (auto& expansion : g_player_game_data_table_expansions) {
    if (expansion.manager == nullptr) {
      return &expansion;
    }
  }
  return nullptr;
}


// Original src/dllmain.cpp:7384
bool EnsurePlayerGameDataPoolCapacity(void* manager) {
  if (manager == nullptr) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(manager);
  const uint64_t current_table = ReadU64(base + 0x28, 0);
  if (current_table == 0) {
    LogError(
        "PlayerGameDataExpand failed reason=null_table manager=%p targetCapacity=%u",
        manager,
        kExpandedNpcPlayerGameDataPoolCapacity);
    return false;
  }

  if (auto* expansion = FindPlayerGameDataExpansionByTable(current_table)) {
    if (expansion->capacity >= kExpandedNpcPlayerGameDataPoolCapacity) {
      return true;
    }
  }

  if (auto* expansion = FindPlayerGameDataExpansionByManager(manager)) {
    if (current_table != expansion->original_table) {
      LogWarn(
          "PlayerGameDataExpand stale_expansion ignored manager=%p currentTable=%016llx previousOriginal=%016llx previousExpanded=%016llx",
          manager,
          static_cast<unsigned long long>(current_table),
          static_cast<unsigned long long>(expansion->original_table),
          static_cast<unsigned long long>(expansion->expanded_table));
      *expansion = PlayerGameDataTableExpansion{};
    } else if (WriteU64(base + 0x28, expansion->expanded_table)) {
      Log(
          "PlayerGameDataExpand restored manager=%p originalTable=%016llx expandedTable=%016llx capacity=%u",
          manager,
          static_cast<unsigned long long>(expansion->original_table),
          static_cast<unsigned long long>(expansion->expanded_table),
          expansion->capacity);
      return true;
    } else {
      LogError(
          "PlayerGameDataExpand failed reason=restore_write_failed manager=%p expandedTable=%016llx",
          manager,
          static_cast<unsigned long long>(expansion->expanded_table));
      return false;
    }
  }

  PlayerGameDataTableExpansion* expansion = ReservePlayerGameDataExpansionSlot();
  if (expansion == nullptr) {
    LogError(
        "PlayerGameDataExpand failed reason=no_expansion_slots manager=%p targetCapacity=%u",
        manager,
        kExpandedNpcPlayerGameDataPoolCapacity);
    return false;
  }

  const size_t old_bytes = static_cast<size_t>(kNpcPlayerGameDataPoolCapacity) * sizeof(uint64_t);
  const size_t new_bytes =
      static_cast<size_t>(kExpandedNpcPlayerGameDataPoolCapacity) * sizeof(uint64_t);
  void* new_table = GameAllocate(new_bytes, 8);
  if (new_table == nullptr) {
    LogError(
        "PlayerGameDataExpand failed reason=alloc_failed manager=%p oldTable=%016llx oldCapacity=%u newCapacity=%u newBytes=%llu",
        manager,
        static_cast<unsigned long long>(current_table),
        kNpcPlayerGameDataPoolCapacity,
        kExpandedNpcPlayerGameDataPoolCapacity,
        static_cast<unsigned long long>(new_bytes));
    return false;
  }

  __try {
    std::memset(new_table, 0, new_bytes);
    std::memcpy(new_table, reinterpret_cast<const void*>(current_table), old_bytes);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    LogError(
        "PlayerGameDataExpand failed reason=copy_exception manager=%p oldTable=%016llx newTable=%p",
        manager,
        static_cast<unsigned long long>(current_table),
        new_table);
    return false;
  }

  if (!WriteU64(base + 0x28, reinterpret_cast<uint64_t>(new_table))) {
    LogError(
        "PlayerGameDataExpand failed reason=table_write_failed manager=%p oldTable=%016llx newTable=%p",
        manager,
        static_cast<unsigned long long>(current_table),
        new_table);
    return false;
  }

  expansion->manager = manager;
  expansion->original_table = current_table;
  expansion->expanded_table = reinterpret_cast<uint64_t>(new_table);
  expansion->capacity = kExpandedNpcPlayerGameDataPoolCapacity;

  Log(
      "PlayerGameDataExpand ok manager=%p oldCapacity=%u newCapacity=%u oldTable=%016llx newTable=%p oldBytes=%llu newBytes=%llu fixedSlotStart=%u fixedSlotCount=%u",
      manager,
      kNpcPlayerGameDataPoolCapacity,
      kExpandedNpcPlayerGameDataPoolCapacity,
      static_cast<unsigned long long>(current_table),
      new_table,
      static_cast<unsigned long long>(old_bytes),
      static_cast<unsigned long long>(new_bytes),
      kNpcPlayerGameDataFixedSlotStart,
      kNpcPlayerGameDataFixedSlotCount);
  return true;
}


// Original src/dllmain.cpp:7495
int FixedPlayerGameDataSlotForActiveCreate() {
  if (!g_active_create_context.active || !g_active_create_context.custom_candidate ||
      g_active_create_context.chara_init < 0 ||
      !IsValidBuddySlot(g_active_create_context.buddy_slot)) {
    return -1;
  }

  const int owner_slot = g_active_create_context.owner_slot;
  if (owner_slot < 1 ||
      owner_slot > static_cast<int>(kNpcPlayerGameDataFixedOwnerCount)) {
    LogWarn(
        "PlayerGameData fixed_slot skipped reason=owner_out_of_range owner=%u ownerSlot=%d buddySlot=%d charaInit=%d npcParam=%d npcThinkParam=%d supportedOwners=1..%u",
        g_active_create_context.owner_id,
        owner_slot,
        g_active_create_context.buddy_slot,
        g_active_create_context.chara_init,
        g_active_create_context.npc_param,
        g_active_create_context.npc_think_param,
        kNpcPlayerGameDataFixedOwnerCount);
    return -1;
  }

  return static_cast<int>(
      kNpcPlayerGameDataFixedSlotStart +
      static_cast<uint32_t>(owner_slot - 1) * kBuddySlotCount +
      static_cast<uint32_t>(g_active_create_context.buddy_slot));
}


// Original src/dllmain.cpp:7523
void* ConstructPlayerGameDataEntry(void* memory, int fixed_slot, const char* reason) {
  if (memory == nullptr || g_player_game_data_ctor == nullptr) {
    return nullptr;
  }

  void* entry = nullptr;
  __try {
    entry = g_player_game_data_ctor(memory, 0, 0xffffffffu, 0);
  } __except (LogSehException("PlayerGameDataCtor original", GetExceptionInformation())) {
    LogError(
        "PlayerGameData fixed_slot construct_exception reason=%s memory=%p fixedSlot=%d owner=%u buddySlot=%d charaInit=%d",
        reason != nullptr ? reason : "unknown",
        memory,
        fixed_slot,
        g_active_create_context.owner_id,
        g_active_create_context.buddy_slot,
        g_active_create_context.chara_init);
    return nullptr;
  }

  if (entry == nullptr) {
    entry = memory;
  }
  WriteI32(static_cast<uint8_t*>(entry) + 8, 10000 + fixed_slot);
  return entry;
}


// Original src/dllmain.cpp:7550
void* AllocFixedCustomPlayerGameData(void* manager, int fixed_slot) {
  if (manager == nullptr || fixed_slot < static_cast<int>(kNpcPlayerGameDataFixedSlotStart) ||
      fixed_slot >= static_cast<int>(kExpandedNpcPlayerGameDataPoolCapacity)) {
    return nullptr;
  }

  if (g_player_game_data_ctor == nullptr) {
    LogError(
        "PlayerGameData fixed_slot failed reason=ctor_missing manager=%p fixedSlot=%d owner=%u buddySlot=%d",
        manager,
        fixed_slot,
        g_active_create_context.owner_id,
        g_active_create_context.buddy_slot);
    return nullptr;
  }

  if (!EnsurePlayerGameDataPoolCapacity(manager)) {
    LogError(
        "PlayerGameData fixed_slot failed reason=expand_failed manager=%p fixedSlot=%d owner=%u buddySlot=%d charaInit=%d",
        manager,
        fixed_slot,
        g_active_create_context.owner_id,
        g_active_create_context.buddy_slot,
        g_active_create_context.chara_init);
    return nullptr;
  }

  const uint64_t table = ReadU64(static_cast<uint8_t*>(manager) + 0x28, 0);
  const uint64_t slot_addr = table + static_cast<uint64_t>(fixed_slot) * sizeof(uint64_t);
  const uint64_t existing = table != 0 ? ReadU64(reinterpret_cast<const void*>(slot_addr), 0) : 0;
  if (existing != 0) {
    void* entry = ConstructPlayerGameDataEntry(
        reinterpret_cast<void*>(existing),
        fixed_slot,
        "reuse_reset");
    if (entry == nullptr) {
      return nullptr;
    }
    if (entry != reinterpret_cast<void*>(existing)) {
      WriteU64(reinterpret_cast<void*>(slot_addr), reinterpret_cast<uint64_t>(entry));
    }
    Log(
        "PlayerGameData fixed_slot reuse_reset manager=%p table=%016llx fixedSlot=%d entry=%p entryId=%d owner=%u ownerSlot=%d buddySlot=%d entitySlot=%d charaInit=%d npcParam=%d npcThinkParam=%d",
        manager,
        static_cast<unsigned long long>(table),
        fixed_slot,
        entry,
        ReadI32(static_cast<uint8_t*>(entry) + 8, -1),
        g_active_create_context.owner_id,
        g_active_create_context.owner_slot,
        g_active_create_context.buddy_slot,
        g_active_create_context.entity_slot,
        g_active_create_context.chara_init,
        g_active_create_context.npc_param,
        g_active_create_context.npc_think_param);
    return entry;
  }

  void* memory = GameAllocate(kNpcPlayerGameDataEntrySize, kNpcPlayerGameDataEntryAlignment);
  if (memory == nullptr) {
    LogError(
        "PlayerGameData fixed_slot failed reason=entry_alloc_failed manager=%p table=%016llx fixedSlot=%d size=%llu owner=%u buddySlot=%d charaInit=%d",
        manager,
        static_cast<unsigned long long>(table),
        fixed_slot,
        static_cast<unsigned long long>(kNpcPlayerGameDataEntrySize),
        g_active_create_context.owner_id,
        g_active_create_context.buddy_slot,
        g_active_create_context.chara_init);
    return nullptr;
  }

  void* entry = ConstructPlayerGameDataEntry(memory, fixed_slot, "new_construct");
  if (entry == nullptr) {
    return nullptr;
  }
  if (!WriteU64(reinterpret_cast<void*>(slot_addr), reinterpret_cast<uint64_t>(entry))) {
    LogError(
        "PlayerGameData fixed_slot failed reason=slot_write_failed manager=%p table=%016llx fixedSlot=%d entry=%p owner=%u buddySlot=%d",
        manager,
        static_cast<unsigned long long>(table),
        fixed_slot,
        entry,
        g_active_create_context.owner_id,
        g_active_create_context.buddy_slot);
    return nullptr;
  }

  Log(
      "PlayerGameData fixed_slot new manager=%p table=%016llx fixedSlot=%d entry=%p entryId=%d owner=%u ownerSlot=%d buddySlot=%d entitySlot=%d charaInit=%d npcParam=%d npcThinkParam=%d",
      manager,
      static_cast<unsigned long long>(table),
      fixed_slot,
      entry,
      ReadI32(static_cast<uint8_t*>(entry) + 8, -1),
      g_active_create_context.owner_id,
      g_active_create_context.owner_slot,
      g_active_create_context.buddy_slot,
      g_active_create_context.entity_slot,
      g_active_create_context.chara_init,
      g_active_create_context.npc_param,
      g_active_create_context.npc_think_param);
  return entry;
}


// Original src/dllmain.cpp:7655
void* GetBuddyEntityManager() {
  const uint64_t world_chr_man =
      ReadU64(reinterpret_cast<const void*>(g_game_base + kWorldChrManPtrRva), 0);
  if (world_chr_man == 0) {
    return nullptr;
  }
  return reinterpret_cast<void*>(world_chr_man + 0x9af0);
}


// Original src/dllmain.cpp:7664
uint32_t CountNonEmptyEntitySlots(uint64_t table, uint32_t capacity) {
  uint32_t non_empty = 0;
  for (uint32_t slot = 0; slot < capacity; ++slot) {
    const uint64_t entry = ReadU64(
        reinterpret_cast<const void*>(table + static_cast<uint64_t>(slot) * kBuddyEntitySlotEntrySize),
        0);
    if (entry != 0) {
      ++non_empty;
    }
  }
  return non_empty;
}


// Original src/dllmain.cpp:7677
bool EnsureBuddyEntitySlotCapacity(void* manager) {
  if (manager == nullptr) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(manager);
  const uint32_t capacity = ReadU32(base + 0x10, 0);
  const uint64_t table = ReadU64(base + 0x18, 0);
  if (capacity >= kExpandedBuddyEntitySlotCapacity) {
    if (!g_buddy_entity_slot_capacity_logged) {
      Log(
          "BuddyEntitySlotExpand already manager=%p capacity=%u table=%016llx targetCapacity=%u",
          manager,
          capacity,
          static_cast<unsigned long long>(table),
          kExpandedBuddyEntitySlotCapacity);
      g_buddy_entity_slot_capacity_logged = true;
    }
    return true;
  }
  if (capacity == 0 || table == 0) {
    Log(
        "BuddyEntitySlotExpand skipped manager=%p capacity=%u table=%016llx targetCapacity=%u",
        manager,
        capacity,
        static_cast<unsigned long long>(table),
        kExpandedBuddyEntitySlotCapacity);
    return false;
  }
  if (capacity != kVanillaBuddyEntitySlotCapacity) {
    LogWarn(
        "BuddyEntitySlotExpand continuing reason=partial_or_external_capacity manager=%p oldCapacity=%u targetCapacity=%u table=%016llx",
        manager,
        capacity,
        kExpandedBuddyEntitySlotCapacity,
        static_cast<unsigned long long>(table));
  }

  const size_t old_bytes = static_cast<size_t>(capacity) * kBuddyEntitySlotEntrySize;
  const size_t new_bytes =
      static_cast<size_t>(kExpandedBuddyEntitySlotCapacity) * kBuddyEntitySlotEntrySize;
  void* new_table = GameAllocate(new_bytes, 8);
  if (new_table == nullptr) {
      LogError(
          "BuddyEntitySlotExpand failed manager=%p capacity=%u table=%016llx newBytes=%llu",
        manager,
        capacity,
        static_cast<unsigned long long>(table),
        static_cast<unsigned long long>(new_bytes));
    return false;
  }

  const uint32_t non_empty = CountNonEmptyEntitySlots(table, capacity);
  __try {
    std::memset(new_table, 0, new_bytes);
    std::memcpy(new_table, reinterpret_cast<const void*>(table), old_bytes);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
      LogError(
          "BuddyEntitySlotExpand failed reason=copy_exception manager=%p oldTable=%016llx newTable=%p",
        manager,
        static_cast<unsigned long long>(table),
        new_table);
    return false;
  }

  const bool wrote_table = WriteU64(base + 0x18, reinterpret_cast<uint64_t>(new_table));
  const bool wrote_capacity = WriteU32(base + 0x10, kExpandedBuddyEntitySlotCapacity);
  Log(
      "BuddyEntitySlotExpand %s manager=%p oldCapacity=%u newCapacity=%u oldTable=%016llx newTable=%p oldBytes=%llu newBytes=%llu nonEmpty=%u",
      wrote_table && wrote_capacity ? "ok" : "write_failed",
      manager,
      capacity,
      kExpandedBuddyEntitySlotCapacity,
      static_cast<unsigned long long>(table),
      new_table,
      static_cast<unsigned long long>(old_bytes),
      static_cast<unsigned long long>(new_bytes),
      non_empty);
  g_buddy_entity_slot_capacity_logged = true;
  if (non_empty != 0) {
  LogWarn("BuddyEntitySlotExpand warning nonEmpty=%u expansion happened after registrations", non_empty);
  }
  return wrote_table && wrote_capacity;
}


// Original src/dllmain.cpp:7981
void* GetPlayerGameDataManager() {
  if (g_game_base == 0) {
    return nullptr;
  }
  const uint64_t manager =
      ReadU64(reinterpret_cast<const void*>(g_game_base + kPlayerGameDataManagerGlobalPtrRva), 0);
  return reinterpret_cast<void*>(manager);
}


// Original src/dllmain.cpp:8014
bool InstallRemotePlayerGameDataLimitPatches() {
  constexpr uint8_t kCmpEaxVanillaRemoteSlotCount[3] = {
      0x83,
      0xf8,
      static_cast<uint8_t>(kVanillaRemotePlayerGameDataSlots)};
  constexpr uint8_t kCmpEaxExpandedRemoteSlotCount[3] = {
      0x83,
      0xf8,
      static_cast<uint8_t>(kExpandedRemotePlayerGameDataSlots)};
  constexpr uint8_t kCmpEaxVanillaLastRemoteSlot[3] = {
      0x83,
      0xf8,
      static_cast<uint8_t>(kVanillaRemotePlayerGameDataSlots - 1)};
  constexpr uint8_t kCmpEaxExpandedLastRemoteSlot[3] = {
      0x83,
      0xf8,
      static_cast<uint8_t>(kExpandedRemotePlayerGameDataSlots - 1)};
  constexpr uint8_t kCmpEcxVanillaLastRemoteSlot[3] = {
      0x83,
      0xf9,
      static_cast<uint8_t>(kVanillaRemotePlayerGameDataSlots - 1)};
  constexpr uint8_t kCmpEcxObservedMutatedLastRemoteSlot[3] = {
      0xff,
      0xf9,
      static_cast<uint8_t>(kVanillaRemotePlayerGameDataSlots - 1)};
  constexpr uint8_t kCmpEcxExpandedLastRemoteSlot[3] = {
      0x83,
      0xf9,
      static_cast<uint8_t>(kExpandedRemotePlayerGameDataSlots - 1)};
  constexpr uint8_t kCmpEdxVanillaRemoteSlotCount[3] = {
      0x83,
      0xfa,
      static_cast<uint8_t>(kVanillaRemotePlayerGameDataSlots)};
  constexpr uint8_t kCmpEdxExpandedRemoteSlotCount[3] = {
      0x83,
      0xfa,
      static_cast<uint8_t>(kExpandedRemotePlayerGameDataSlots)};
  constexpr uint8_t kCmpEdxVanillaLastRemoteSlot[3] = {
      0x83,
      0xfa,
      static_cast<uint8_t>(kVanillaRemotePlayerGameDataSlots - 1)};
  constexpr uint8_t kCmpEdxExpandedLastRemoteSlot[3] = {
      0x83,
      0xfa,
      static_cast<uint8_t>(kExpandedRemotePlayerGameDataSlots - 1)};
  constexpr uint8_t kCmpEdiVanillaPlayerSlotCount[3] = {
      0x83,
      0xff,
      static_cast<uint8_t>(kVanillaPlayerIconAvatarCount)};
  constexpr uint8_t kCmpEdiExpandedPlayerSlotCount[3] = {
      0x83,
      0xff,
      static_cast<uint8_t>(kExpandedPlayerIconAvatarCount)};
  constexpr uint8_t kCmpEdiVanillaLastPlayerSlot[3] = {
      0x83,
      0xff,
      static_cast<uint8_t>(kVanillaPlayerIconAvatarCount - 1)};
  constexpr uint8_t kCmpEdiExpandedLastPlayerSlot[3] = {
      0x83,
      0xff,
      static_cast<uint8_t>(kExpandedPlayerIconAvatarCount - 1)};

  bool ok = true;
  ok = InstallCodePatch(
           g_remote_player_data_write_range_patch,
           kRemotePlayerGameDataWriteRangeRva,
           kCmpEaxVanillaRemoteSlotCount,
           kCmpEaxExpandedRemoteSlotCount,
           sizeof(kCmpEaxExpandedRemoteSlotCount),
           "RemotePlayerGameDataWriteRangePatch") &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_clear_range_patch,
           kRemotePlayerGameDataClearRangeRva,
           kCmpEcxVanillaLastRemoteSlot,
           kCmpEcxExpandedLastRemoteSlot,
           sizeof(kCmpEcxExpandedLastRemoteSlot),
           "RemotePlayerGameDataClearRangePatch",
           false,
           kCmpEcxObservedMutatedLastRemoteSlot) &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_flag_clear_range_patch,
           kRemotePlayerGameDataFlagClearRangeRva,
           kCmpEcxVanillaLastRemoteSlot,
           kCmpEcxExpandedLastRemoteSlot,
           sizeof(kCmpEcxExpandedLastRemoteSlot),
           "RemotePlayerGameDataFlagClearRangePatch") &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_get_by_index_range_patch,
           kRemotePlayerGameDataGetByIndexRangeRva,
           kCmpEcxVanillaLastRemoteSlot,
           kCmpEcxExpandedLastRemoteSlot,
           sizeof(kCmpEcxExpandedLastRemoteSlot),
           "RemotePlayerGameDataGetByIndexRangePatch") &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_find_by_entity_range1_patch,
           kRemotePlayerGameDataFindByEntityRange1Rva,
           kCmpEaxVanillaLastRemoteSlot,
           kCmpEaxExpandedLastRemoteSlot,
           sizeof(kCmpEaxExpandedLastRemoteSlot),
           "RemotePlayerGameDataFindByEntityRange1Patch") &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_find_by_entity_range2_patch,
           kRemotePlayerGameDataFindByEntityRange2Rva,
           kCmpEaxVanillaRemoteSlotCount,
           kCmpEaxExpandedRemoteSlotCount,
           sizeof(kCmpEaxExpandedRemoteSlotCount),
           "RemotePlayerGameDataFindByEntityRange2Patch") &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_search_by_entity_range1_patch,
           kRemotePlayerGameDataSearchByEntityRange1Rva,
           kCmpEdxVanillaLastRemoteSlot,
           kCmpEdxExpandedLastRemoteSlot,
           sizeof(kCmpEdxExpandedLastRemoteSlot),
           "RemotePlayerGameDataSearchByEntityRange1Patch") &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_search_by_entity_range2_patch,
           kRemotePlayerGameDataSearchByEntityRange2Rva,
           kCmpEdxVanillaRemoteSlotCount,
           kCmpEdxExpandedRemoteSlotCount,
           sizeof(kCmpEdxExpandedRemoteSlotCount),
           "RemotePlayerGameDataSearchByEntityRange2Patch") &&
       ok;
  ok = InstallCodePatch(
           g_remote_player_data_by_peer_range_patch,
           kRemotePlayerGameDataByPeerRangeRva,
           kCmpEaxVanillaRemoteSlotCount,
           kCmpEaxExpandedRemoteSlotCount,
           sizeof(kCmpEaxExpandedRemoteSlotCount),
           "RemotePlayerGameDataByPeerRangePatch") &&
       ok;
  ok = InstallCodePatch(
           g_player_indicator_refresh_valid_range_patch,
           kPlayerIndicatorRefreshValidRangeRva,
           kCmpEdiVanillaLastPlayerSlot,
           kCmpEdiExpandedLastPlayerSlot,
           sizeof(kCmpEdiExpandedLastPlayerSlot),
           "PlayerIndicatorRefreshValidRangePatch") &&
       ok;
  ok = InstallCodePatch(
           g_player_indicator_refresh_loop_range_patch,
           kPlayerIndicatorRefreshLoopRangeRva,
           kCmpEdiVanillaPlayerSlotCount,
           kCmpEdiExpandedPlayerSlotCount,
           sizeof(kCmpEdiExpandedPlayerSlotCount),
           "PlayerIndicatorRefreshLoopRangePatch") &&
       ok;
  return ok;
}


// Original src/dllmain.cpp:8170
bool EnsureRemotePlayerGameDataCapacity(const char* reason) {
  void* manager = GetPlayerGameDataManager();
  if (manager == nullptr) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(manager);
  const uint64_t current_entries = ReadU64(base + 0x10, 0);
  if (current_entries == 0) {
    return false;
  }

  const uint32_t current_capacity = static_cast<uint32_t>(ReadU64(
      reinterpret_cast<const void*>(current_entries - 8),
      kVanillaRemotePlayerGameDataSlots));

  if (current_capacity >= kExpandedRemotePlayerGameDataSlots) {
    g_remote_player_game_data_expansion.manager = manager;
    g_remote_player_game_data_expansion.expanded_entries = current_entries;
    g_remote_player_game_data_expansion.capacity = current_capacity;
    const bool patched = InstallRemotePlayerGameDataLimitPatches();
    if (patched) {
      Log(
          "RemotePlayerGameDataExpand already manager=%p capacity=%u entries=%016llx reason=%s",
          manager,
          current_capacity,
          static_cast<unsigned long long>(current_entries),
          reason != nullptr ? reason : "unknown");
    }
    return patched;
  }

  if (g_remote_player_game_data_expansion.expanded_entries != 0 &&
      g_remote_player_game_data_expansion.manager == manager) {
    const bool patched = InstallRemotePlayerGameDataLimitPatches();
    return patched;
  }

  if (g_player_game_data_ctor == nullptr) {
    return false;
  }

  const size_t entry_bytes =
      static_cast<size_t>(kExpandedRemotePlayerGameDataSlots) * kNpcPlayerGameDataEntrySize;
  const size_t block_bytes = 0x10 + entry_bytes;
  auto* block = static_cast<uint8_t*>(GameAllocate(block_bytes, 0x10));
  auto* flags_a = static_cast<uint8_t*>(GameAllocate(kExpandedRemotePlayerGameDataSlots, 1));
  auto* flags_b = static_cast<uint8_t*>(GameAllocate(kExpandedRemotePlayerGameDataSlots, 1));
  if (block == nullptr || flags_a == nullptr || flags_b == nullptr) {
    LogError(
        "RemotePlayerGameDataExpand failed reason=alloc_failed manager=%p currentEntries=%016llx currentCapacity=%u targetCapacity=%u block=%p flagsA=%p flagsB=%p",
        manager,
        static_cast<unsigned long long>(current_entries),
        current_capacity,
        kExpandedRemotePlayerGameDataSlots,
        block,
        flags_a,
        flags_b);
    return false;
  }

  ZeroBytesSafe(block, block_bytes);
  ZeroBytesSafe(flags_a, kExpandedRemotePlayerGameDataSlots);
  ZeroBytesSafe(flags_b, kExpandedRemotePlayerGameDataSlots);
  WriteU64(block, reinterpret_cast<uint64_t>(block));
  WriteU64(block + 8, kExpandedRemotePlayerGameDataSlots);

  uint8_t* entries = block + 0x10;
  for (uint32_t slot = 0; slot < kExpandedRemotePlayerGameDataSlots; ++slot) {
    void* entry = entries + static_cast<size_t>(slot) * kNpcPlayerGameDataEntrySize;
    __try {
      g_player_game_data_ctor(entry, 0, 0xffffffffu, 0);
    } __except (LogSehException("RemotePlayerGameData ctor", GetExceptionInformation())) {
      LogError(
          "RemotePlayerGameDataExpand failed reason=ctor_exception manager=%p slot=%u entry=%p",
          manager,
          slot,
          entry);
      return false;
    }
  }

  const uint32_t copy_slots =
      current_capacity < kVanillaRemotePlayerGameDataSlots ? current_capacity : kVanillaRemotePlayerGameDataSlots;
  const size_t copy_bytes = static_cast<size_t>(copy_slots) * kNpcPlayerGameDataEntrySize;
  if (copy_bytes != 0 && !CopyBytesSafe(entries, reinterpret_cast<const void*>(current_entries), copy_bytes)) {
    LogError(
        "RemotePlayerGameDataExpand failed reason=copy_entries_exception manager=%p source=%016llx dest=%p copySlots=%u",
        manager,
        static_cast<unsigned long long>(current_entries),
        entries,
        copy_slots);
    return false;
  }

  const uint64_t old_flags_a = ReadU64(base + 0x18, 0);
  const uint64_t old_flags_b = ReadU64(base + 0x20, 0);
  if (old_flags_a != 0) {
    CopyBytesSafe(flags_a, reinterpret_cast<const void*>(old_flags_a), copy_slots);
  }
  if (old_flags_b != 0) {
    CopyBytesSafe(flags_b, reinterpret_cast<const void*>(old_flags_b), copy_slots);
  }

  if (!WriteU64(base + 0x10, reinterpret_cast<uint64_t>(entries)) ||
      !WriteU64(base + 0x18, reinterpret_cast<uint64_t>(flags_a)) ||
      !WriteU64(base + 0x20, reinterpret_cast<uint64_t>(flags_b))) {
    LogError(
        "RemotePlayerGameDataExpand failed reason=manager_write_failed manager=%p entries=%p flagsA=%p flagsB=%p",
        manager,
        entries,
        flags_a,
        flags_b);
    return false;
  }

  g_remote_player_game_data_expansion.manager = manager;
  g_remote_player_game_data_expansion.original_entries = current_entries;
  g_remote_player_game_data_expansion.expanded_block = reinterpret_cast<uint64_t>(block);
  g_remote_player_game_data_expansion.expanded_entries = reinterpret_cast<uint64_t>(entries);
  g_remote_player_game_data_expansion.original_flags_a = old_flags_a;
  g_remote_player_game_data_expansion.expanded_flags_a = reinterpret_cast<uint64_t>(flags_a);
  g_remote_player_game_data_expansion.original_flags_b = old_flags_b;
  g_remote_player_game_data_expansion.expanded_flags_b = reinterpret_cast<uint64_t>(flags_b);
  g_remote_player_game_data_expansion.capacity = kExpandedRemotePlayerGameDataSlots;

  const bool patched = InstallRemotePlayerGameDataLimitPatches();
  Log(
      "RemotePlayerGameDataExpand %s manager=%p oldCapacity=%u newCapacity=%u oldEntries=%016llx newEntries=%p oldFlagsA=%016llx newFlagsA=%p oldFlagsB=%016llx newFlagsB=%p reason=%s",
      patched ? "ok" : "patch_failed",
      manager,
      current_capacity,
      kExpandedRemotePlayerGameDataSlots,
      static_cast<unsigned long long>(current_entries),
      entries,
      static_cast<unsigned long long>(old_flags_a),
      flags_a,
      static_cast<unsigned long long>(old_flags_b),
      flags_b,
      reason != nullptr ? reason : "unknown");
  return patched;
}


// Original src/dllmain.cpp:8314
DWORD WINAPI RemotePlayerGameDataExpansionThread(void*) {
  constexpr int kMaxAttempts = 120;
  for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
    if (EnsureRemotePlayerGameDataCapacity("poll")) {
      return 0;
    }
    Sleep(1000);
  }

  LogWarn(
      "RemotePlayerGameDataExpand timeout targetCapacity=%u attempts=%d",
      kExpandedRemotePlayerGameDataSlots,
      kMaxAttempts);
  return 0;
}


// Original src/dllmain.cpp:8330
bool InstallPlayerMapMarkerPatches() {
  constexpr uint8_t kMovEaxExpandedRemoteCount[5] = {
      0xb8,
      static_cast<uint8_t>(kExpandedRemotePlayerMarkerCount),
      0x00,
      0x00,
      0x00};
  constexpr uint8_t kMovEaxExpandedRemoteCountForList[5] = {
      0xb8,
      static_cast<uint8_t>(kExpandedRemotePlayerMarkerCount),
      0x00,
      0x00,
      0x00};
  constexpr uint8_t kWorldMapCtorCall1[5] = {0xe8, 0x56, 0x04, 0x8f, 0xff};
  constexpr uint8_t kWorldMapCtorCall2[5] = {0xe8, 0xbe, 0x03, 0x8f, 0xff};
  constexpr uint8_t kWorldMapMarkerUpdateCall[5] = {0xe8, 0xc7, 0x24, 0x5c, 0xff};
  constexpr uint8_t kWorldMapPlayerListCountCall[5] = {0xe8, 0xc8, 0x46, 0x73, 0xff};
  constexpr uint8_t kCmpEdiVanillaLastAvatarIndex[3] = {
      0x83,
      0xff,
      static_cast<uint8_t>(kVanillaPlayerIconAvatarCount - 1)};
  constexpr uint8_t kCmpEdiExpandedLastAvatarIndex[3] = {
      0x83,
      0xff,
      static_cast<uint8_t>(kExpandedPlayerIconAvatarCount - 1)};
  constexpr uint8_t kCmpEdiVanillaAvatarCount[3] = {
      0x83,
      0xff,
      kVanillaPlayerIconAvatarCount};
  constexpr uint8_t kCmpEdiExpandedAvatarCount[3] = {
      0x83,
      0xff,
      kExpandedPlayerIconAvatarCount};

  // Do not patch FUN_1401fe4c0 globally. It is also used by matching/session
  // paths, and changing the global return value can crash those unrelated
  // systems. These local call-site replacements only expand the WorldMapViewModel
  // remote-player data vector and the per-frame marker update loop that feeds
  // FUN_14090ee00. The adjacent player-icon builder also has a hardcoded
  // three-player avatar object count. Only expand the object count: the
  // id/type copy range must stay vanilla because the source cache only contains
  // three initialized player avatar records. Extra marker resources are repaired
  // later by mirroring one of those initialized containers and then refreshing
  // icon id/type from the target player's ChrIns.
  bool ok = true;
  ok = InstallCodePatch(
           g_world_map_remote_data_count_patch1,
           kWorldMapRemotePlayerDataCountCall1Rva,
           kWorldMapCtorCall1,
           kMovEaxExpandedRemoteCount,
           sizeof(kMovEaxExpandedRemoteCount),
           "WorldMapRemotePlayerDataCountPatch1",
           true) &&
       ok;
  ok = InstallCodePatch(
           g_world_map_remote_data_count_patch2,
           kWorldMapRemotePlayerDataCountCall2Rva,
           kWorldMapCtorCall2,
           kMovEaxExpandedRemoteCount,
           sizeof(kMovEaxExpandedRemoteCount),
           "WorldMapRemotePlayerDataCountPatch2",
           true) &&
       ok;
  ok = InstallCodePatch(
           g_world_map_remote_marker_update_count_patch,
           kWorldMapRemoteMarkerUpdateCountCallRva,
           kWorldMapMarkerUpdateCall,
           kMovEaxExpandedRemoteCount,
           sizeof(kMovEaxExpandedRemoteCount),
           "WorldMapRemoteMarkerUpdateCountPatch",
           true) &&
       ok;
  ok = InstallCodePatch(
           g_world_map_player_list_count_patch,
           kWorldMapPlayerListCountCallRva,
           kWorldMapPlayerListCountCall,
           kMovEaxExpandedRemoteCountForList,
           sizeof(kMovEaxExpandedRemoteCountForList),
           "WorldMapPlayerListCountPatch",
           true) &&
       ok;
  ok = InstallCodePatch(
           g_player_icon_avatar_build_count_patch,
           kPlayerIconAvatarBuildCountRva,
           kCmpEdiVanillaAvatarCount,
           kCmpEdiExpandedAvatarCount,
           sizeof(kCmpEdiExpandedAvatarCount),
           "PlayerIconAvatarBuildCountPatch") &&
       ok;
  return ok;
}


// Original src/dllmain.cpp:8422
bool InstallBuddyReadyRangePatch() {
  auto* function = reinterpret_cast<uint8_t*>(g_game_base + kBuddyReadyRangeCheckRva);
  uint8_t* immediate = nullptr;
  const char* pattern_name = nullptr;
  bool inclusive_compare = false;

  // FUN_1404e7d60 checks whether a character entity low id is deployable.
  // Vanilla compiles as: sub low, 0x14; cmp low, 0x28; setb al.
  // Some builds compile the same test as: sub low, 0x14; cmp low, 0x27; cmovbe.
  // Keep the lower bound at 20, but raise the upper entitySlot limit to 120.
  for (size_t i = 0; i + 8 < 0x90; ++i) {
    const uint8_t sub_op = ReadU8(function + i + 1, 0);
    const uint8_t cmp_op = ReadU8(function + i + 4, 0);
    if (ReadU8(function + i, 0) == 0x83 &&
        (sub_op == 0xe8 || sub_op == 0xe9) &&
        ReadU8(function + i + 2, 0) == kBuddyReadyRangeStart &&
        ReadU8(function + i + 3, 0) == 0x83 &&
        ((sub_op == 0xe8 && cmp_op == 0xf8) || (sub_op == 0xe9 && cmp_op == 0xf9)) &&
        ReadU8(function + i + 6, 0) == 0x0f &&
        ReadU8(function + i + 7, 0) == 0x92) {
      immediate = function + i + 5;
      pattern_name = sub_op == 0xe8 ? "eax" : "ecx";
      break;
    }
  }
  if (immediate == nullptr) {
    for (size_t i = 0; i + 14 < 0xc0; ++i) {
      const uint8_t sub_op = ReadU8(function + i + 1, 0);
      const uint8_t cmp_op = ReadU8(function + i + 9, 0);
      if (ReadU8(function + i, 0) == 0x83 &&
          (sub_op == 0xe8 || sub_op == 0xe9) &&
          ReadU8(function + i + 2, 0) == kBuddyReadyRangeStart &&
          ReadU8(function + i + 3, 0) == 0xba &&
          ReadU32(function + i + 4, 0) == 1 &&
          ReadU8(function + i + 8, 0) == 0x83 &&
          ((sub_op == 0xe8 && cmp_op == 0xf8) || (sub_op == 0xe9 && cmp_op == 0xf9)) &&
          ReadU8(function + i + 11, 0) == 0x0f &&
          ReadU8(function + i + 12, 0) == 0x46) {
        immediate = function + i + 10;
        pattern_name = sub_op == 0xe8 ? "eax_cmovbe" : "ecx_cmovbe";
        inclusive_compare = true;
        break;
      }
    }
  }

  if (immediate == nullptr) {
    LogError(
        "BuddyReadyRangeExpand failed reason=pattern_not_found function=%p q00=%016llx q08=%016llx q10=%016llx q18=%016llx",
        function,
        static_cast<unsigned long long>(ReadU64(function + 0x00, 0)),
        static_cast<unsigned long long>(ReadU64(function + 0x08, 0)),
        static_cast<unsigned long long>(ReadU64(function + 0x10, 0)),
        static_cast<unsigned long long>(ReadU64(function + 0x18, 0)));
    return false;
  }

  const uint8_t current_immediate = ReadU8(immediate, 0);
  const uint8_t current_span =
      inclusive_compare ? static_cast<uint8_t>(current_immediate + 1) : current_immediate;
  const uint32_t current_limit = kBuddyReadyRangeStart + current_span;
  if (current_span >= kExpandedBuddyReadyRangeSpan) {
    Log(
        "BuddyReadyRangeExpand already function=%p patchByte=%p register=%s inclusiveCompare=%d start=%u currentImmediate=%u currentSpan=%u currentLimit=%u targetSpan=%u targetLimit=%u",
        function,
        immediate,
        pattern_name,
        inclusive_compare ? 1 : 0,
        kBuddyReadyRangeStart,
        current_immediate,
        current_span,
        current_limit,
        kExpandedBuddyReadyRangeSpan,
        kExpandedBuddyEntitySlotCapacity);
    return true;
  }

  if (current_span != kVanillaBuddyReadyRangeSpan) {
    LogWarn(
        "BuddyReadyRangeExpand skipped reason=unexpected_span function=%p patchByte=%p register=%s inclusiveCompare=%d start=%u currentImmediate=%u currentSpan=%u currentLimit=%u expectedVanillaSpan=%u targetSpan=%u",
        function,
        immediate,
        pattern_name,
        inclusive_compare ? 1 : 0,
        kBuddyReadyRangeStart,
        current_immediate,
        current_span,
        current_limit,
        kVanillaBuddyReadyRangeSpan,
        kExpandedBuddyReadyRangeSpan);
    return false;
  }

  const uint8_t replacement =
      inclusive_compare ? static_cast<uint8_t>(kExpandedBuddyReadyRangeSpan - 1)
                        : kExpandedBuddyReadyRangeSpan;
  if (!WriteCodeByte(immediate, replacement, "BuddyReadyRangeExpand")) {
    return false;
  }

  g_buddy_ready_range_patch.target = immediate;
  g_buddy_ready_range_patch.original = current_immediate;
  g_buddy_ready_range_patch.replacement = replacement;
  g_buddy_ready_range_patch.installed = true;
  g_buddy_ready_range_patch.name = "FUN_1404e7d60/BuddyReadyRangeExpand";

  Log(
      "BuddyReadyRangeExpand ok function=%p patchByte=%p register=%s inclusiveCompare=%d start=%u oldImmediate=%u oldSpan=%u oldLimit=%u newImmediate=%u newSpan=%u newLimit=%u",
      function,
      immediate,
      pattern_name,
      inclusive_compare ? 1 : 0,
      kBuddyReadyRangeStart,
      current_immediate,
      current_span,
      current_limit,
      replacement,
      kExpandedBuddyReadyRangeSpan,
      kExpandedBuddyEntitySlotCapacity);
  return true;
}


// Original src/dllmain.cpp:8559
bool ResetBuddySlotState(void* state, int buddy_slot) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(state);
  const int offset = buddy_slot * kBuddySlotSize;
  const uint8_t flags = NormalizeBuddyPersistentFlags(ReadU8(base + kSlotFlagsOffset + offset, 0));
  return WriteBuddySlotControl(state, buddy_slot, kSlotStateActive, 0, flags);
}


// Original src/dllmain.cpp:8570
bool NormalizeActiveSlotForUnsummon(void* state, int buddy_slot) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(state);
  const int offset = buddy_slot * kBuddySlotSize;
  const uint8_t flags =
      NormalizeBuddyPersistentFlags(ReadU8(base + kSlotFlagsOffset + offset, 0)) | kSlotFlagsUnsummonRequest;
  return WriteBuddySlotControl(state, buddy_slot, kSlotStateTransition, 0, flags);
}


// Original src/dllmain.cpp:8582
bool PrepareSlotForOriginalResummon(void* state, int buddy_slot) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  auto* base = static_cast<uint8_t*>(state);
  const int offset = buddy_slot * kBuddySlotSize;
  const uint8_t flags =
      NormalizeBuddyPersistentFlags(ReadU8(base + kSlotFlagsOffset + offset, 0)) | kSlotFlagsResummonContinuation;
  return WriteBuddySlotControl(state, buddy_slot, kSlotStateActive, 0, flags);
}


// Original src/dllmain.cpp:8594
bool SetZeroHpFallbackRecoveryGate(void* state, int buddy_slot, const SlotSnapshot& snapshot, bool enabled) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  const uint8_t persistent = NormalizeBuddyPersistentFlags(snapshot.flags);
  uint8_t flags = static_cast<uint8_t>(snapshot.flags | persistent);
  if (enabled) {
    flags = static_cast<uint8_t>(flags | kSlotFlagsRecoveryGate);
  } else {
    flags = static_cast<uint8_t>(flags & ~kSlotFlagsRecoveryGate);
  }

  if (!WriteSlotFlags(state, buddy_slot, flags)) {
      LogWarn(
          "ZeroHpFallbackRegen gate_write_failed slot=%d enabled=%d entity=%08x:%08x flags=%02x targetFlags=%02x",
        buddy_slot,
        enabled ? 1 : 0,
        snapshot.entity_high,
        snapshot.entity_low,
        snapshot.flags,
        flags);
    return false;
  }

  Log(
      "ZeroHpFallbackRegen %s slot=%d entity=%08x:%08x flags %02x->%02x tick=%u",
      enabled ? "gate_set" : "gate_release",
      buddy_slot,
      snapshot.entity_high,
      snapshot.entity_low,
      snapshot.flags,
      flags,
      g_module_update_tick);
  return true;
}


// Original src/dllmain.cpp:8631
void StopZeroHpFallbackRegen(int buddy_slot, const SlotSnapshot& snapshot, const char* reason) {
  if (!IsValidBuddySlot(buddy_slot)) {
    return;
  }

  auto& regen = g_zero_hp_fallback_regen[buddy_slot];
  if (!regen.active && !regen.monitoring) {
    return;
  }

  Log(
      "ZeroHpFallbackRegen stop slot=%d reason=%s active=%d monitoring=%d trackedEntity=%08x:%08x slotEntity=%08x:%08x state=%02x pending=%02x flags=%02x tick=%u",
      buddy_slot,
      reason,
      regen.active ? 1 : 0,
      regen.monitoring ? 1 : 0,
      regen.entity.high,
      regen.entity.low,
      snapshot.entity_high,
      snapshot.entity_low,
      snapshot.state,
      snapshot.pending,
      snapshot.flags,
      g_module_update_tick);

  if (regen.active && !regen.gate_released) {
    SetZeroHpFallbackRecoveryGate(g_last_buddy_state, buddy_slot, snapshot, false);
  }

  regen = ZeroHpFallbackRegenState{};
}


// Original src/dllmain.cpp:8663
void StopRecallRegen(int buddy_slot, const char* reason) {
  if (!IsValidBuddySlot(buddy_slot)) {
    return;
  }

  auto& recall = g_recall_regen[buddy_slot];
  if (!recall.active && !recall.monitoring) {
    return;
  }

  int32_t current_hp = 0;
  int32_t max_hp = 0;
  if (recall.hp_component != nullptr) {
    current_hp = ReadI32(static_cast<const uint8_t*>(recall.hp_component) + kHpCurrentOffset, 0);
    max_hp = ReadI32(static_cast<const uint8_t*>(recall.hp_component) + kHpMaxOffset, 0);
  }

  Log(
      "RecallRegen stop slot=%d reason=%s active=%d monitoring=%d entity=%08x:%08x hpComponent=%p currentHp=%d maxHp=%d tick=%u",
      buddy_slot,
      reason,
      recall.active ? 1 : 0,
      recall.monitoring ? 1 : 0,
      recall.entity.high,
      recall.entity.low,
      recall.hp_component,
      current_hp,
      max_hp,
      g_module_update_tick);

  recall = RecallRegenState{};
}


// Original src/dllmain.cpp:8696
void StopStuckRecoveryMonitor(int buddy_slot, const char* reason) {
  if (!IsValidBuddySlot(buddy_slot)) {
    return;
  }

  auto& stuck = g_stuck_recovery[buddy_slot];
  if (!stuck.monitoring) {
    return;
  }

  Log(
      "StuckRecoveryState stop slot=%d reason=%s entity=%08x:%08x firstHp=%d tick=%u",
      buddy_slot,
      reason,
      stuck.entity.high,
      stuck.entity.low,
      stuck.first_hp,
      g_module_update_tick);
  stuck = StuckRecoveryState{};
}


// Original src/dllmain.cpp:8717
bool UpdateStuckRecoveryState(void* state, int buddy_slot, const SlotSnapshot& snapshot, int hp_rate) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  auto& stuck = g_stuck_recovery[buddy_slot];
  const bool candidate =
      snapshot.state == kSlotStateStuckRemoving && snapshot.pending == 0 &&
      !IsEmptyEntity(snapshot) && IsCustomBuddySnapshot(buddy_slot, snapshot);
  if (!candidate) {
    StopStuckRecoveryMonitor(buddy_slot, "candidate_cleared");
    return false;
  }

  BuddyContext context{};
  if (!TryReadCustomBuddyContext(state, buddy_slot, snapshot, hp_rate, &context)) {
    StopStuckRecoveryMonitor(buddy_slot, "context_lost");
    return false;
  }

  if (!stuck.monitoring || !SameEntity(stuck.entity, context.entity)) {
    stuck.monitoring = true;
    stuck.entity = context.entity;
    stuck.since_tick = g_module_update_tick;
    stuck.first_hp = context.current_hp;
    Log(
        "StuckRecoveryState monitor slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x hpRate=%d currentHp=%d maxHp=%d stableTicks=%u",
        buddy_slot,
        context.entity.high,
        context.entity.low,
        snapshot.state,
        snapshot.pending,
        snapshot.flags,
        hp_rate,
        context.current_hp,
        context.max_hp,
        kBuddyRegenStableTicks);
    return false;
  }

  if (g_module_update_tick - stuck.since_tick < kBuddyRegenStableTicks) {
    return false;
  }

  uint8_t flags = NormalizeBuddyPersistentFlags(snapshot.flags);
  const bool releasable =
      hp_rate >= kZeroHpFallbackReleaseHpRate || context.current_hp >= context.max_hp;
  if (!releasable) {
    flags = static_cast<uint8_t>(flags | kSlotFlagsRecoveryGate);
  }

  StopRecallRegen(buddy_slot, releasable ? "stuck_recovery_release" : "stuck_recovery_handoff");
  StopZeroHpFallbackRegen(
      buddy_slot,
      snapshot,
      releasable ? "stuck_recovery_release" : "stuck_recovery_handoff");

  if (!WriteBuddySlotControl(state, buddy_slot, kSlotStateActive, 0, flags)) {
    LogWarn(
        "StuckRecoveryState write_failed slot=%d entity=%08x:%08x releasable=%d state=%02x pending=%02x flags=%02x targetFlags=%02x hpRate=%d currentHp=%d maxHp=%d",
        buddy_slot,
        context.entity.high,
        context.entity.low,
        releasable ? 1 : 0,
        snapshot.state,
        snapshot.pending,
        snapshot.flags,
        flags,
        hp_rate,
        context.current_hp,
        context.max_hp);
    return false;
  }

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange(
      releasable ? "StuckRecoveryState release_to_recalled" : "StuckRecoveryState handoff_to_regen",
      buddy_slot,
      snapshot,
      after);
  Log(
      "StuckRecoveryState resolved slot=%d entity=%08x:%08x releasable=%d hpRate=%d currentHp=%d maxHp=%d firstHp=%d dwellTicks=%u",
      buddy_slot,
      context.entity.high,
      context.entity.low,
      releasable ? 1 : 0,
      hp_rate,
      context.current_hp,
      context.max_hp,
      stuck.first_hp,
      g_module_update_tick - stuck.since_tick);
  stuck = StuckRecoveryState{};
  return true;
}


// Original src/dllmain.cpp:8812
void StopPostRecallPulse(int buddy_slot, const char* reason) {
  if (!IsValidBuddySlot(buddy_slot)) {
    return;
  }

  auto& pulse = g_post_recall_pulse[buddy_slot];
  if (!pulse.active) {
    return;
  }

  Log(
      "PostRecallPulse stop slot=%d reason=%s entity=%08x:%08x releaseTick=%u tick=%u",
      buddy_slot,
      reason,
      pulse.entity.high,
      pulse.entity.low,
      pulse.release_tick,
      g_module_update_tick);
  pulse = PostRecallPulseState{};
}


// Original src/dllmain.cpp:8835
bool StartPostRecallPulse(
    void* state,
    int buddy_slot,
    const SlotSnapshot& snapshot,
    int hp_rate,
    const char* reason) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) || g_unsummon_buddy == nullptr ||
      snapshot.state != kSlotStateActive || snapshot.pending != 0 ||
      IsEmptyEntity(snapshot) || hp_rate <= 0 ||
      !IsCustomBuddySnapshot(buddy_slot, snapshot)) {
    return false;
  }

  // Auto-distance recall can finish as 14 -> 00 while the custom buddy entity
  // remains visible/attackable. Re-deploying through FUN_1403897c0 is not
  // sufficient here because the original second deploy stage refuses to advance
  // a manually prepared 0a slot. Instead, briefly normalize the slot to the
  // shape expected by the native unsummon entry and let FUN_140386a20 perform
  // the entity-side detach/hide cleanup.
  if (!NormalizeActiveSlotForUnsummon(state, buddy_slot)) {
    LogWarn(
        "PostRecallManualUnsummon prepare_write_failed slot=%d reason=%s entity=%08x:%08x state=%02x pending=%02x flags=%02x hpRate=%d",
        buddy_slot,
        reason != nullptr ? reason : "<unknown>",
        snapshot.entity_high,
        snapshot.entity_low,
        snapshot.state,
        snapshot.pending,
        snapshot.flags,
        hp_rate);
    return false;
  }

  SlotSnapshot prepared = ReadSlot(state, buddy_slot);
  LogSlotChange("PostRecallManualUnsummon prepare", buddy_slot, snapshot, prepared);

  bool completed = true;
  EntityId entity{snapshot.entity_low, snapshot.entity_high};
  __try {
    g_unsummon_buddy(state, &entity);
  } __except (LogSehException(
                   "PostRecallManualUnsummon FUN_140386a20/UnsummonBuddy",
                   GetExceptionInformation())) {
    completed = false;
    LogBuddyExceptionContext(
        "PostRecallManualUnsummon FUN_140386a20/UnsummonBuddy",
        state,
        buddy_slot);
  }

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange(
      completed ? "PostRecallManualUnsummon original_result" : "PostRecallManualUnsummon original_exception",
      buddy_slot,
      prepared,
      after);

  const uint8_t restore_flags = NormalizeBuddyPersistentFlags(after.flags);
  if (!WriteBuddySlotControl(state, buddy_slot, kSlotStateActive, 0, restore_flags)) {
    LogWarn(
        "PostRecallManualUnsummon restore_write_failed slot=%d reason=%s entity=%08x:%08x afterState=%02x afterPending=%02x afterFlags=%02x hpRate=%d targetFlags=%02x",
        buddy_slot,
        reason != nullptr ? reason : "<unknown>",
        snapshot.entity_high,
        snapshot.entity_low,
        after.state,
        after.pending,
        after.flags,
        hp_rate,
        restore_flags);
    return false;
  }

  const SlotSnapshot restored = ReadSlot(state, buddy_slot);
  LogSlotChange("PostRecallManualUnsummon restore", buddy_slot, after, restored);

  if (!completed) {
    LogWarn(
        "PostRecallManualUnsummon failed slot=%d reason=%s entity=%08x:%08x beforeState=%02x preparedState=%02x afterState=%02x afterPending=%02x afterFlags=%02x restoredState=%02x restoredPending=%02x restoredFlags=%02x hpRate=%d",
        buddy_slot,
        reason != nullptr ? reason : "<unknown>",
        snapshot.entity_high,
        snapshot.entity_low,
        snapshot.state,
        prepared.state,
        after.state,
        after.pending,
        after.flags,
        restored.state,
        restored.pending,
        restored.flags,
        hp_rate);
    return false;
  }

  Log(
      "PostRecallManualUnsummon complete slot=%d reason=%s entity=%08x:%08x hpRate=%d tick=%u",
      buddy_slot,
      reason != nullptr ? reason : "<unknown>",
      snapshot.entity_high,
      snapshot.entity_low,
      hp_rate,
      g_module_update_tick);
  BroadcastBuddyRecallPacket(
      state,
      buddy_slot,
      snapshot,
      restored,
      reason);
  return true;
}


// Original src/dllmain.cpp:8947
bool ProcessPostRecallPulse(void* state, int buddy_slot, const SlotSnapshot& snapshot) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  auto& pulse = g_post_recall_pulse[buddy_slot];
  if (!pulse.active) {
    return false;
  }

  const EntityId current{snapshot.entity_low, snapshot.entity_high};
  if (!SameEntity(pulse.entity, current)) {
    StopPostRecallPulse(buddy_slot, "entity_changed");
    return false;
  }

  if (g_module_update_tick < pulse.release_tick) {
    return false;
  }

  uint8_t flags = NormalizeBuddyPersistentFlags(snapshot.flags);
  flags = static_cast<uint8_t>(flags & ~0x10);
  if (!WriteBuddySlotControl(state, buddy_slot, kSlotStateActive, 0, flags)) {
    LogWarn(
        "PostRecallPulse recall_write_failed slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x targetFlags=%02x tick=%u",
        buddy_slot,
        snapshot.entity_high,
        snapshot.entity_low,
        snapshot.state,
        snapshot.pending,
        snapshot.flags,
        flags,
        g_module_update_tick);
    return false;
  }

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange("PostRecallPulse recall_after_one_tick", buddy_slot, snapshot, after);
  pulse = PostRecallPulseState{};
  return true;
}


// Original src/dllmain.cpp:8989
bool StartRecallRegen(int buddy_slot, const EntityId& entity_id, void* hp_component, const char* reason) {
  if (!IsCustomBuddyEntity(buddy_slot, entity_id) || hp_component == nullptr) {
    return false;
  }

  if (g_zero_hp_fallback_regen[buddy_slot].active) {
    return false;
  }

  const int32_t current_hp = ReadI32(static_cast<const uint8_t*>(hp_component) + kHpCurrentOffset, 0);
  const int32_t max_hp = ReadI32(static_cast<const uint8_t*>(hp_component) + kHpMaxOffset, 0);
  if (max_hp <= 0 || current_hp <= 0 || current_hp >= max_hp) {
    return false;
  }

  auto& recall = g_recall_regen[buddy_slot];
  if (recall.active && !SameEntity(recall.entity, entity_id)) {
    StopRecallRegen(buddy_slot, "entity_changed");
  }

  recall.active = true;
  recall.monitoring = false;
  recall.entity = entity_id;
  recall.hp_component = hp_component;
  recall.stable_since_tick = 0;
  recall.last_tick = 0;
  recall.regen_tick_count = 0;
  recall.observed_hp = current_hp;

  Log(
      "RecallRegen start slot=%d reason=%s entity=%08x:%08x hpComponent=%p currentHp=%d maxHp=%d intervalTicks=%u divisor=%d",
      buddy_slot,
      reason,
      entity_id.high,
      entity_id.low,
      hp_component,
      current_hp,
      max_hp,
      kZeroHpFallbackRegenIntervalTicks,
      kZeroHpFallbackRegenDivisor);
  return true;
}


// Original src/dllmain.cpp:9032
bool IsRecallRegenRecalledSlot(const SlotSnapshot& snapshot) {
  return snapshot.state == kSlotStateActive && snapshot.pending == 0 && (snapshot.flags & 0x10) == 0 &&
         (snapshot.flags & kSlotFlagsRecoveryGate) == 0;
}


// Original src/dllmain.cpp:9037
bool IsZeroHpFallbackRecalledSlot(const SlotSnapshot& snapshot) {
  if (snapshot.state != kSlotStateActive || snapshot.pending != 0 || (snapshot.flags & 0x10) != 0) {
    return false;
  }

  return true;
}


// Original src/dllmain.cpp:9045
bool IsZeroHpFallbackSlotAllowed(const SlotSnapshot& snapshot, const ZeroHpFallbackRegenState& regen) {
  if (regen.active) {
    return !IsEmptyEntity(snapshot);
  }

  return IsZeroHpFallbackRecalledSlot(snapshot);
}


// Original src/dllmain.cpp:9053
bool UpdateRecallRegen(void* state, int buddy_slot, const SlotSnapshot& snapshot, int hp_rate) {
  if (!IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  auto& recall = g_recall_regen[buddy_slot];
  if (!recall.active && !recall.monitoring) {
    if (state == nullptr || IsEmptyEntity(snapshot) || !IsRecallRegenRecalledSlot(snapshot)) {
      return false;
    }
  }

  BuddyContext context{};
  if (!TryReadCustomBuddyContext(state, buddy_slot, snapshot, hp_rate, &context)) {
    StopRecallRegen(buddy_slot, "context_lost");
    return false;
  }

  if (g_zero_hp_fallback_regen[buddy_slot].active) {
    StopRecallRegen(buddy_slot, "zero_hp_fallback_active");
    return false;
  }

  if (!IsRecallRegenRecalledSlot(snapshot)) {
    if (recall.active || recall.monitoring) {
      Log(
          "RecallRegen stop_guard slot=%d reason=slot_not_recalled entity=%08x:%08x state=%02x pending=%02x flags=%02x",
          buddy_slot,
          snapshot.entity_high,
          snapshot.entity_low,
          snapshot.state,
          snapshot.pending,
          snapshot.flags);
      StopRecallRegen(buddy_slot, "slot_not_recalled");
    }
    return false;
  }

  if ((recall.active || recall.monitoring) && !SameEntity(recall.entity, context.entity)) {
    StopRecallRegen(buddy_slot, "slot_entity_changed");
    return false;
  }

  if (context.current_hp <= 0) {
    StopRecallRegen(buddy_slot, "zero_hp");
    return false;
  }

  if (context.current_hp >= context.max_hp) {
    StopRecallRegen(buddy_slot, "full_hp");
    return false;
  }

  if (!recall.active) {
    if (!recall.monitoring || !SameEntity(recall.entity, context.entity) ||
        recall.observed_hp != context.current_hp) {
      recall.active = false;
      recall.monitoring = true;
      recall.entity = context.entity;
      recall.hp_component = context.hp_component;
      recall.stable_since_tick = g_module_update_tick;
      recall.last_tick = 0;
      recall.regen_tick_count = 0;
      recall.observed_hp = context.current_hp;
      return false;
    }

    if (g_module_update_tick - recall.stable_since_tick < kBuddyRegenStableTicks) {
      return false;
    }

    if (!StartRecallRegen(buddy_slot, context.entity, context.hp_component, "stable_recalled")) {
      return false;
    }
  }

  const uint32_t elapsed = g_module_update_tick - recall.last_tick;
  if (recall.last_tick != 0 && elapsed < kZeroHpFallbackRegenIntervalTicks) {
    return true;
  }

  int32_t after_hp = context.current_hp;
  int32_t step_hp = 0;
  recall.last_tick = g_module_update_tick;
  if (!ApplyBuddyRegenTick(
          "RecallRegen",
          buddy_slot,
          context,
          context.current_hp,
          &recall.regen_tick_count,
          &after_hp,
          &step_hp)) {
    StopRecallRegen(buddy_slot, "tick_failed");
    return true;
  }

  Log(
      "RecallRegen tick slot=%d entity=%08x:%08x hpComponent=%p currentHp %d->%d maxHp=%d step=%d regenTick=%u tick=%u",
      buddy_slot,
      context.entity.high,
      context.entity.low,
      context.hp_component,
      context.current_hp,
      after_hp,
      context.max_hp,
      step_hp,
      recall.regen_tick_count,
      g_module_update_tick);

  if (after_hp <= 0) {
    StopRecallRegen(buddy_slot, "zero_hp_after_tick");
  } else if (after_hp >= context.max_hp) {
    StopRecallRegen(buddy_slot, "full_hp_after_tick");
  }

  return true;
}


// Original src/dllmain.cpp:9171
bool UpdateZeroHpFallbackRegen(void* state, int buddy_slot, const SlotSnapshot& snapshot, int hp_rate) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  auto& regen = g_zero_hp_fallback_regen[buddy_slot];
  BuddyContext context{};
  if (!TryReadCustomBuddyContext(state, buddy_slot, snapshot, hp_rate, &context)) {
    StopZeroHpFallbackRegen(buddy_slot, snapshot, "context_lost");
    return false;
  }

  if ((regen.active || regen.monitoring) && !SameEntity(regen.entity, context.entity)) {
    StopZeroHpFallbackRegen(buddy_slot, snapshot, "entity_changed");
    return false;
  }

  if (!IsZeroHpFallbackSlotAllowed(snapshot, regen)) {
    StopZeroHpFallbackRegen(buddy_slot, snapshot, "slot_not_recalled");
    return false;
  }

  if (context.current_hp >= context.max_hp) {
    StopZeroHpFallbackRegen(buddy_slot, snapshot, "full_hp");
    return false;
  }

  if (!regen.active) {
    const bool recovery_gate_set = (snapshot.flags & kSlotFlagsRecoveryGate) != 0;
    const bool abnormal_zero_hp = hp_rate == 0 && context.current_hp <= 0 &&
                                  !recovery_gate_set;
    const bool stalled_recovery = recovery_gate_set && context.current_hp < context.max_hp;
    if (!abnormal_zero_hp && !stalled_recovery) {
      StopZeroHpFallbackRegen(buddy_slot, snapshot, "candidate_cleared");
      return false;
    }

    if (!regen.monitoring || !SameEntity(regen.entity, context.entity) ||
        regen.observed_hp != context.current_hp) {
      regen.active = false;
      regen.monitoring = true;
      regen.gate_released = false;
      regen.healed_above_zero = false;
      regen.entity = context.entity;
      regen.stable_since_tick = g_module_update_tick;
      regen.active_since_tick = 0;
      regen.last_tick = 0;
      regen.regen_tick_count = 0;
      regen.observed_hp = context.current_hp;
      regen.max_hp = context.max_hp;
      regen.healed_budget = 0;
      return false;
    }

    if (g_module_update_tick - regen.stable_since_tick < kBuddyRegenStableTicks) {
      return false;
    }

    regen.active = true;
    regen.monitoring = false;
    regen.gate_released = false;
    regen.healed_above_zero = false;
    regen.entity = context.entity;
    regen.active_since_tick = g_module_update_tick;
    regen.last_tick = 0;
    regen.regen_tick_count = 0;
    regen.observed_hp = context.current_hp;
    regen.max_hp = context.max_hp;
    regen.healed_budget = 0;
    SetZeroHpFallbackRecoveryGate(state, buddy_slot, snapshot, true);
    Log(
        "ZeroHpFallbackRegen start slot=%d reason=%s entity=%08x:%08x hpComponent=%p state=%02x pending=%02x flags=%02x hpRate=%d currentHp=%d maxHp=%d releaseRate=%d stableTicks=%u activeLimitTicks=%u intervalTicks=%u divisor=%d",
        buddy_slot,
        stalled_recovery ? "stalled_recovery" : "abnormal_zero_hp",
        context.entity.high,
        context.entity.low,
        context.hp_component,
        snapshot.state,
        snapshot.pending,
        snapshot.flags,
        hp_rate,
        context.current_hp,
        context.max_hp,
        kZeroHpFallbackReleaseHpRate,
        kBuddyRegenStableTicks,
        kZeroHpFallbackMaxActiveTicks,
        kZeroHpFallbackRegenIntervalTicks,
        kZeroHpFallbackRegenDivisor);
  } else {
    if (regen.healed_above_zero && context.current_hp <= 0) {
      StopZeroHpFallbackRegen(buddy_slot, snapshot, "zero_hp_after_recovery");
      return true;
    }

    if (regen.active_since_tick != 0 &&
        g_module_update_tick - regen.active_since_tick >= kZeroHpFallbackMaxActiveTicks) {
      StopZeroHpFallbackRegen(buddy_slot, snapshot, "active_time_limit");
      return true;
    }
  }

  const uint32_t elapsed = g_module_update_tick - regen.last_tick;
  if (regen.last_tick != 0 && elapsed < kZeroHpFallbackRegenIntervalTicks) {
    return true;
  }

  if (regen.max_hp <= 0) {
    regen.max_hp = context.max_hp;
  }

  const int32_t base_hp = context.current_hp < 0 ? 0 : context.current_hp;
  int32_t after_hp = context.current_hp;
  int32_t step_hp = 0;
  regen.last_tick = g_module_update_tick;
  if (!ApplyBuddyRegenTick(
          "ZeroHpFallbackRegen",
          buddy_slot,
          context,
          base_hp,
          &regen.regen_tick_count,
          &after_hp,
          &step_hp)) {
    StopZeroHpFallbackRegen(buddy_slot, snapshot, "tick_failed");
    return true;
  }

  regen.healed_budget += step_hp;
  if (after_hp > 0) {
    regen.healed_above_zero = true;
  }

  const int after_hp_rate = ComputeHpRateFromCurrent(after_hp, context.max_hp);
  Log(
      "ZeroHpFallbackRegen tick slot=%d entity=%08x:%08x hpComponent=%p state=%02x pending=%02x flags=%02x hpRate=%d currentHp %d->%d maxHp=%d hpRateAfter=%d step=%d healedBudget=%d/%d regenTick=%u gateReleased=%d tick=%u",
      buddy_slot,
      context.entity.high,
      context.entity.low,
      context.hp_component,
      snapshot.state,
      snapshot.pending,
      snapshot.flags,
      hp_rate,
      context.current_hp,
      after_hp,
      context.max_hp,
      after_hp_rate,
      step_hp,
      regen.healed_budget,
      regen.max_hp,
      regen.regen_tick_count,
      regen.gate_released ? 1 : 0,
      g_module_update_tick);

  if (!regen.gate_released && after_hp_rate >= kZeroHpFallbackReleaseHpRate) {
    if (SetZeroHpFallbackRecoveryGate(state, buddy_slot, ReadSlot(state, buddy_slot), false)) {
      regen.gate_released = true;
      Log(
          "ZeroHpFallbackRegen release_threshold slot=%d entity=%08x:%08x hpRateAfter=%d currentHp=%d maxHp=%d healedBudget=%d/%d",
          buddy_slot,
          context.entity.high,
          context.entity.low,
          after_hp_rate,
          after_hp,
          context.max_hp,
          regen.healed_budget,
          regen.max_hp);
    }
  }

  if (after_hp <= 0 && regen.healed_above_zero) {
    StopZeroHpFallbackRegen(buddy_slot, ReadSlot(state, buddy_slot), "zero_hp_after_tick");
    return true;
  }

  if (after_hp >= context.max_hp) {
    StopZeroHpFallbackRegen(buddy_slot, ReadSlot(state, buddy_slot), "full_hp_after_tick");
    return true;
  }

  return true;
}


// Original src/dllmain.cpp:9353
int MapRevenantBuddyTriggerToSlot(uint32_t buddy_trigger_id) {
  switch (buddy_trigger_id) {
    case 270000:
    case 27000000:
      return 0;
    case 271000:
    case 27100000:
      return 1;
    case 272000:
    case 27200000:
      return 2;
    default:
      return -1;
  }
}


// Original src/dllmain.cpp:9369
bool IsLikelyRevenantBuddyActionId(int action_id) {
  if (MapRevenantBuddyTriggerToSlot(static_cast<uint32_t>(action_id)) >= 0) {
    return true;
  }
  return (270000 <= action_id && action_id <= 272999) ||
      (27000000 <= action_id && action_id <= 27299999);
}


// Original src/dllmain.cpp:9379
bool IsZeroHpFallbackSummonBlocked(void* state, uint32_t buddy_trigger_id) {
  const int buddy_slot = MapRevenantBuddyTriggerToSlot(buddy_trigger_id);
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  if (!IsCustomBuddySlot(buddy_slot)) {
    return false;
  }

  auto& regen = g_zero_hp_fallback_regen[buddy_slot];
  if (!regen.active || regen.gate_released) {
    return false;
  }

  const SlotSnapshot snapshot = ReadSlot(state, buddy_slot);
  if (IsEmptyEntity(snapshot)) {
    StopZeroHpFallbackRegen(buddy_slot, snapshot, "try_use_empty_slot");
    return false;
  }

  const EntityId entity_id{snapshot.entity_low, snapshot.entity_high};
  if (!SameEntity(regen.entity, entity_id)) {
    StopZeroHpFallbackRegen(buddy_slot, snapshot, "try_use_entity_changed");
    return false;
  }

  void* entity = LookupEntityById(entity_id);
  void* hp_component = GetHpComponentFromEntity(entity);
  if (hp_component == nullptr) {
    LogWarn(
        "TryUseBuddy blocked fallbackRecovery slot=%d trigger=%u entity=%08x:%08x reason=no_hp_component",
        buddy_slot,
        buddy_trigger_id,
        snapshot.entity_high,
        snapshot.entity_low);
    return true;
  }

  const int32_t current_hp = ReadI32(static_cast<const uint8_t*>(hp_component) + kHpCurrentOffset, 0);
  const int32_t max_hp = ReadI32(static_cast<const uint8_t*>(hp_component) + kHpMaxOffset, 0);
  const int current_rate = ComputeHpRateFromCurrent(current_hp, max_hp);
  if (current_rate >= kZeroHpFallbackReleaseHpRate) {
    if (SetZeroHpFallbackRecoveryGate(state, buddy_slot, snapshot, false)) {
      regen.gate_released = true;
      Log(
          "TryUseBuddy fallbackRecovery auto_release slot=%d trigger=%u entity=%08x:%08x currentHp=%d maxHp=%d hpRate=%d",
          buddy_slot,
          buddy_trigger_id,
          snapshot.entity_high,
          snapshot.entity_low,
          current_hp,
          max_hp,
          current_rate);
      return false;
    }
  }

  LogWarn(
      "TryUseBuddy blocked fallbackRecovery slot=%d trigger=%u entity=%08x:%08x state=%02x pending=%02x flags=%02x currentHp=%d maxHp=%d hpRate=%d releaseRate=%d healedBudget=%d/%d",
      buddy_slot,
      buddy_trigger_id,
      snapshot.entity_high,
      snapshot.entity_low,
      snapshot.state,
      snapshot.pending,
      snapshot.flags,
      current_hp,
      max_hp,
      current_rate,
      kZeroHpFallbackReleaseHpRate,
      regen.healed_budget,
      regen.max_hp);
  return true;
}


// Original src/dllmain.cpp:9455
bool IsStalePendingUnsummonState(const SlotSnapshot& slot) {
  return slot.state == kSlotStateActive && slot.pending != 0 && (slot.flags & 0x01) != 0;
}


// Original src/dllmain.cpp:9459
bool IsForcedBuddyState(const SlotSnapshot& slot, uint8_t transition_flag) {
  if (slot.state == kSlotStateStuckRemoved || slot.state == kSlotStateStuckRemoving) {
    return true;
  }

  if (IsStalePendingUnsummonState(slot)) {
    return true;
  }

  return slot.state == kSlotStateTransition && (slot.flags & transition_flag) != 0;
}


// Original src/dllmain.cpp:9471
bool IsForcedUnsummonState(const SlotSnapshot& slot) {
  return IsForcedBuddyState(slot, 0x01);
}


// Original src/dllmain.cpp:9475
bool IsForcedResummonState(const SlotSnapshot& slot) {
  return IsForcedBuddyState(slot, 0x02);
}


// Original src/dllmain.cpp:9479
bool IsCustomBuddyDeployedSlot(void* state, int buddy_slot, const SlotSnapshot& snapshot) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) ||
      !IsCustomBuddySnapshot(buddy_slot, snapshot) ||
      IsEmptyEntity(snapshot)) {
    return false;
  }

  if ((snapshot.flags & kSlotFlagsRecoveryGate) != 0 ||
      (snapshot.flags & 0x10) == 0) {
    return false;
  }

  return SafeGetBuddyHpRate(state, buddy_slot) > 0;
}


// Original src/dllmain.cpp:9494
int FindSwitchTargetSlotAfterRequest(void* state, int old_slot, const SlotSnapshot (&before)[kBuddySlotCount]) {
  if (state == nullptr) {
    return -1;
  }

  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    if (slot == old_slot) {
      continue;
    }

    const SlotSnapshot after = ReadSlot(state, slot);
    if (IsEmptyEntity(after) || !IsCustomBuddySnapshot(slot, after) ||
        !SlotChanged(before[slot], after)) {
      continue;
    }

    if (after.pending != 0 || after.state == kSlotStateDeploying ||
        after.state == kSlotStateTransition || (after.flags & 0x10) != 0) {
      return slot;
    }
  }

  return -1;
}


// Original src/dllmain.cpp:9519
bool HasPendingCustomDeployRequest(void* state) {
  if (state == nullptr) {
    return false;
  }

  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    const SlotSnapshot snapshot = ReadSlot(state, slot);
    if (snapshot.state == kSlotStateActive && snapshot.pending == 1 &&
        IsCustomBuddySnapshot(slot, snapshot)) {
      return true;
    }
  }

  return false;
}


// Original src/dllmain.cpp:9535
bool CallBuddyModuleUpdateOriginalPlain(void* state, const char* caller) {
  if (g_buddy_module_update == nullptr) {
    return false;
  }

  bool completed = true;
  __try {
    g_buddy_module_update(state);
  } __except (LogSehException("BuddyModuleUpdate original plain", GetExceptionInformation())) {
    completed = false;
    LogError(
        "BuddyModuleUpdate original plain exception caller=%s state=%p",
        caller != nullptr ? caller : "<unknown>",
        state);
  }

  return completed;
}


// Original src/dllmain.cpp:9554
bool TryActivateOwnerWorldGroupFromState(void* state, const char* caller) {
  if (state == nullptr || g_world_owner_group_set_entity_active == nullptr ||
      g_remote_owner_gate == nullptr) {
    return false;
  }

  const OwnerGateSnapshot before = CaptureOwnerGateSnapshot(state);
  if (!before.owner_lookup_ok || before.is_local_owner ||
      IsEmptyEntityId(before.owner_entity) ||
      before.world_owner_group == nullptr ||
      !before.owner_index_in_group_range ||
      before.owner_group_slot == 0 ||
      before.remote_gate) {
    return false;
  }

  bool active_completed = false;
  __try {
    g_world_owner_group_set_entity_active(before.world_owner_group, &before.owner_entity, 1);
    active_completed = true;
  } __except (LogSehException("WorldOwnerGroupSetEntityActive original", GetExceptionInformation())) {
    LogError(
        "WorldOwnerGroupSetEntityActive original_exception caller=%s state=%p ownerId=%08x:%08x ownerChr=%p worldGroup=%p capacity=%d groupSlot=%016llx",
        caller != nullptr ? caller : "<unknown>",
        state,
        before.owner_entity.high,
        before.owner_entity.low,
        before.owner_chr,
        before.world_owner_group,
        before.world_owner_group_capacity,
        static_cast<unsigned long long>(before.owner_group_slot));
    LogBuddyExceptionContext("WorldOwnerGroupSetEntityActive original", state, -1);
  }

  const OwnerGateSnapshot after = CaptureOwnerGateSnapshot(state);
  return active_completed && after.remote_gate;
}


// Original src/dllmain.cpp:9592
bool RunOriginalBuddyModuleUpdate(void* state, int buddy_slot, const char* reason) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) || g_buddy_module_update == nullptr) {
    return false;
  }

  const SlotSnapshot before = ReadSlot(state, buddy_slot);
  const bool has_module_update_request =
      (before.flags & 0x03) != 0 || before.pending != 0;
  if (!has_module_update_request) {
    return false;
  }

  if (before.state == kSlotStateActive && before.pending == 1 &&
      IsCustomBuddySnapshot(buddy_slot, before)) {
    TryActivateOwnerWorldGroupFromState(state, reason);
  }

  Log(
      "OriginalBuddyModuleUpdate begin reason=%s slot=%d state=%02x pending=%02x flags=%02x entity=%08x:%08x",
      reason,
      buddy_slot,
      before.state,
      before.pending,
      before.flags,
      before.entity_high,
      before.entity_low);

  const bool completed = CallBuddyModuleUpdateOriginalPlain(state, reason);

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange(completed ? "OriginalBuddyModuleUpdate end" : "OriginalBuddyModuleUpdate failed",
                buddy_slot,
                before,
                after);
  return completed;
}


// Original src/dllmain.cpp:9629
bool IsRemoteOwnerGateFallbackStage(const SlotSnapshot& snapshot) {
  return (snapshot.state == kSlotStateActive && snapshot.pending == 1) ||
      (snapshot.state == kSlotStateDeploying && snapshot.pending == 0);
}


// Original src/dllmain.cpp:9634
bool IsRemoteOwnerGateFallbackCandidate(
    void* state,
    int buddy_slot,
    const SlotSnapshot& snapshot,
    OwnerGateSnapshot* gate_out) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) ||
      g_buddy_update == nullptr || g_buddy_death_recovery_update == nullptr) {
    return false;
  }

  // These are the two vanilla deploy stages handled by FUN_1403897c0:
  // 00/pending=1 is consumed into 0a/pending=0; a later 0a/pending=0 pass
  // warps/registers the buddy and writes 0b/pending=0. The fallback only
  // retries those original sub-updates; it does not write 0a or 0b itself.
  if (!IsRemoteOwnerGateFallbackStage(snapshot) || IsEmptyEntity(snapshot) ||
      !IsCustomBuddySnapshot(buddy_slot, snapshot)) {
    return false;
  }

  OwnerGateSnapshot gate = CaptureOwnerGateSnapshot(state);
  if (!gate.owner_lookup_ok || gate.is_local_owner || gate.final_allows_buddy_update) {
    return false;
  }

  // Match the vanilla outer gate exactly: non-local owners reach
  // FUN_1403895e0/FUN_1403897c0 only when remoteGate passes and vfunc120
  // returns 0. If vfunc120 returns 1 for a tracked custom buddy, retry only
  // those two original sub-updates once for this tick.
  if (!gate.remote_gate_call_ok || !gate.remote_gate ||
      !gate.vfunc120_call_ok || !gate.vfunc120) {
    return false;
  }

  if (gate_out != nullptr) {
    *gate_out = gate;
  }
  return true;
}


// Original src/dllmain.cpp:9673
bool SetEntitySummonRequestFlag(const SlotSnapshot& snapshot, const char* tag, int buddy_slot) {
  if (IsEmptyEntity(snapshot)) {
    LogWarn("%s entity_flag_skipped_empty slot=%d", tag, buddy_slot);
    return false;
  }

  const EntityId entity_id{snapshot.entity_low, snapshot.entity_high};
  void* entity = LookupEntityById(entity_id);
  if (entity == nullptr) {
    LogWarn(
        "%s entity_flag_lookup_failed slot=%d entity=%08x:%08x",
        tag,
        buddy_slot,
        entity_id.high,
        entity_id.low);
    return false;
  }

  auto* entity_bytes = static_cast<uint8_t*>(entity);
  const uint8_t before = ReadU8(entity_bytes + 0x1f2, 0);
  const uint8_t after = static_cast<uint8_t>(before | 0x02);
  if (!WriteU8(entity_bytes + 0x1f2, after)) {
    LogWarn(
        "%s entity_flag_write_failed slot=%d entity=%08x:%08x chr=%p field1f2=%02x target=%02x",
        tag,
        buddy_slot,
        entity_id.high,
        entity_id.low,
        entity,
        before,
        after);
    return false;
  }

  Log(
      "%s entity_flag_set slot=%d entity=%08x:%08x chr=%p field1f2 %02x->%02x",
      tag,
      buddy_slot,
      entity_id.high,
      entity_id.low,
      entity,
      before,
      after);
  return true;
}


// Original src/dllmain.cpp:9719
bool ManualRemoteFallbackBuddyUpdateStage(
    void* state,
    int buddy_slot,
    const SlotSnapshot& before) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  if (before.state != kSlotStateActive || before.pending != 1) {
    return false;
  }

  SetEntitySummonRequestFlag(before, "RemoteOwnerGateFallback manual_buddy_update", buddy_slot);
  const uint8_t flags = NormalizeBuddyPersistentFlags(before.flags);
  if (!WriteBuddySlotControl(state, buddy_slot, kSlotStateDeploying, 0, flags)) {
    LogWarn(
        "RemoteOwnerGateFallback manual_buddy_update write_failed slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x targetFlags=%02x",
        buddy_slot,
        before.entity_high,
        before.entity_low,
        before.state,
        before.pending,
        before.flags,
        flags);
    return false;
  }

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange("RemoteOwnerGateFallback manual_buddy_update", buddy_slot, before, after);
  return true;
}


// Original src/dllmain.cpp:9751
bool ManualRemoteFallbackDeathRecoveryStage(
    void* state,
    int buddy_slot,
    const SlotSnapshot& before) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  if (before.state != kSlotStateDeploying) {
    return false;
  }

  uint8_t flags = NormalizeBuddyPersistentFlags(before.flags);
  flags = static_cast<uint8_t>(flags | 0x10);
  if (!WriteBuddySlotControl(state, buddy_slot, kSlotStateTransition, 0, flags)) {
    LogWarn(
        "RemoteOwnerGateFallback manual_death_recovery write_failed slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x targetFlags=%02x",
        buddy_slot,
        before.entity_high,
        before.entity_low,
        before.state,
        before.pending,
        before.flags,
        flags);
    return false;
  }

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange("RemoteOwnerGateFallback manual_death_recovery", buddy_slot, before, after);
  return true;
}


// Original src/dllmain.cpp:9783
bool RunRemoteOwnerGateFallbackUpdate(void* state) {
  if (state == nullptr || g_buddy_update == nullptr || g_buddy_death_recovery_update == nullptr) {
    return false;
  }

  SlotSnapshot before[kBuddySlotCount]{};
  SnapshotBuddySlots(state, before);

  bool has_candidate = false;
  int first_candidate_slot = -1;
  bool candidate_slots[kBuddySlotCount]{};
  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    if (IsRemoteOwnerGateFallbackCandidate(state, slot, before[slot], nullptr)) {
      has_candidate = true;
      candidate_slots[slot] = true;
      if (first_candidate_slot < 0) {
        first_candidate_slot = slot;
      }
    }
  }

  if (!has_candidate) {
    return false;
  }

  bool completed = true;
  if (g_config.remote_owner_gate_fallback_mode == kRemoteOwnerGateFallbackModeOriginal) {
    __try {
      g_buddy_update(state);
    } __except (LogSehException(
                     "RemoteOwnerGateFallback FUN_1403895e0/BuddyUpdate",
                     GetExceptionInformation())) {
      completed = false;
      LogBuddyExceptionContext(
          "RemoteOwnerGateFallback FUN_1403895e0/BuddyUpdate",
          state,
          first_candidate_slot);
    }
  } else {
    for (int slot = 0; slot < kBuddySlotCount; ++slot) {
      if (candidate_slots[slot]) {
        ManualRemoteFallbackBuddyUpdateStage(state, slot, before[slot]);
      }
    }
  }

  if (g_config.remote_owner_gate_fallback_mode == kRemoteOwnerGateFallbackModeManualAll) {
    for (int slot = 0; slot < kBuddySlotCount; ++slot) {
      if (!candidate_slots[slot]) {
        continue;
      }
      const SlotSnapshot before_manual_recovery = ReadSlot(state, slot);
      ManualRemoteFallbackDeathRecoveryStage(state, slot, before_manual_recovery);
    }
  } else {
    __try {
      g_buddy_death_recovery_update(state);
    } __except (LogSehException(
                     "RemoteOwnerGateFallback FUN_1403897c0/BuddyDeathRecoveryUpdate",
                     GetExceptionInformation())) {
      completed = false;
      LogBuddyExceptionContext(
          "RemoteOwnerGateFallback FUN_1403897c0/BuddyDeathRecoveryUpdate",
          state,
          first_candidate_slot);
    }
  }

  SlotSnapshot after[kBuddySlotCount]{};
  SnapshotBuddySlots(state, after);

  bool consumed_any = false;
  bool advanced_any = false;
  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    const bool slot_changed = SlotChanged(before[slot], after[slot]);
    consumed_any = consumed_any || (before[slot].pending != 0 && after[slot].pending == 0);
    const bool advanced_00_to_0a =
        before[slot].state == kSlotStateActive && before[slot].pending == 1 &&
        after[slot].state == kSlotStateDeploying && after[slot].pending == 0;
    const bool advanced_0a_to_0b =
        before[slot].state == kSlotStateDeploying && before[slot].pending == 0 &&
        after[slot].state == kSlotStateTransition && after[slot].pending == 0;
    advanced_any = advanced_any || advanced_00_to_0a || advanced_0a_to_0b;

    if (!candidate_slots[slot] || !slot_changed) {
      continue;
    }

    LogSlotChange(
        completed ? "RemoteOwnerGateFallback repaired" : "RemoteOwnerGateFallback failed",
        slot,
        before[slot],
        after[slot]);
  }

  return completed && (consumed_any || advanced_any);
}


// Original src/dllmain.cpp:9881
void ResetPendingDeployRepair(int buddy_slot, const char* reason) {
  if (!IsValidBuddySlot(buddy_slot)) {
    return;
  }

  auto& repair = g_pending_deploy_repair[buddy_slot];
  if (!repair.monitoring) {
    return;
  }

  Log(
      "PendingDeployRepair stop slot=%d reason=%s entity=%08x:%08x state=%02x pending=%02x flags=%02x retries=%u tick=%u",
      buddy_slot,
      reason,
      repair.entity.high,
      repair.entity.low,
      repair.state,
      repair.pending,
      repair.flags,
      repair.retry_count,
      g_module_update_tick);
  repair = PendingDeployRepairState{};
}


// Original src/dllmain.cpp:9905
bool UpdatePendingDeployRepair(void* state, int buddy_slot, const SlotSnapshot& snapshot) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot)) {
    return false;
  }

  const bool custom = !IsEmptyEntity(snapshot) && IsCustomBuddySnapshot(buddy_slot, snapshot);
  const bool stuck_pending_deploy =
      snapshot.state == kSlotStateActive && snapshot.pending == 1;
  const bool stuck_half_deploy =
      snapshot.state == kSlotStateDeploying && snapshot.pending == 0;
  const bool deployed_with_stale_pending =
      snapshot.state == kSlotStateTransition && snapshot.pending != 0;
  if (!custom || (!stuck_pending_deploy && !stuck_half_deploy && !deployed_with_stale_pending)) {
    ResetPendingDeployRepair(buddy_slot, "candidate_cleared");
    return false;
  }

  auto& repair = g_pending_deploy_repair[buddy_slot];
  const EntityId entity{snapshot.entity_low, snapshot.entity_high};
  if (!repair.monitoring || !SameEntity(repair.entity, entity) ||
      repair.state != snapshot.state || repair.pending != snapshot.pending) {
    repair.monitoring = true;
    repair.entity = entity;
    repair.state = snapshot.state;
    repair.pending = snapshot.pending;
    repair.flags = snapshot.flags;
    repair.since_tick = g_module_update_tick;
    repair.last_retry_tick = 0;
    repair.retry_count = 0;
    Log(
        "PendingDeployRepair monitor slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x delayTicks=%u",
        buddy_slot,
        entity.high,
        entity.low,
        snapshot.state,
        snapshot.pending,
        snapshot.flags,
        kPendingDeployRepairTicks);
    return false;
  }

  const uint32_t dwell_ticks = g_module_update_tick - repair.since_tick;
  if (dwell_ticks < kPendingDeployRepairTicks) {
    return false;
  }

  if (deployed_with_stale_pending) {
    if (!WriteBuddySlotControl(state, buddy_slot, kSlotStateTransition, 0, snapshot.flags)) {
      LogWarn(
          "PendingDeployRepair clear_pending_failed slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x dwellTicks=%u",
          buddy_slot,
          entity.high,
          entity.low,
          snapshot.state,
          snapshot.pending,
          snapshot.flags,
          dwell_ticks);
      return false;
    }

    const SlotSnapshot after = ReadSlot(state, buddy_slot);
    LogSlotChange("PendingDeployRepair clear_stale_pending", buddy_slot, snapshot, after);
    ResetPendingDeployRepair(buddy_slot, "cleared_stale_pending");
    return true;
  }

  if (repair.last_retry_tick != 0 &&
      g_module_update_tick - repair.last_retry_tick < kPendingDeployRepairTicks) {
    return false;
  }

  repair.last_retry_tick = g_module_update_tick;
  ++repair.retry_count;
  const SlotSnapshot before_retry = ReadSlot(state, buddy_slot);
  if (before_retry.state == kSlotStateActive && before_retry.pending == 1) {
    TryActivateOwnerWorldGroupFromState(state, "PendingDeployRepair");
  }

  bool completed = true;
  __try {
    if (g_buddy_update != nullptr) {
      g_buddy_update(state);
    }
  } __except (LogSehException(
                   "PendingDeployRepair FUN_1403895e0/BuddyUpdate",
                   GetExceptionInformation())) {
    completed = false;
    LogBuddyExceptionContext("PendingDeployRepair FUN_1403895e0/BuddyUpdate", state, buddy_slot);
  }
  __try {
    if (g_buddy_death_recovery_update != nullptr) {
      g_buddy_death_recovery_update(state);
    }
  } __except (LogSehException(
                   "PendingDeployRepair FUN_1403897c0/BuddyDeathRecoveryUpdate",
                   GetExceptionInformation())) {
    completed = false;
    LogBuddyExceptionContext(
        "PendingDeployRepair FUN_1403897c0/BuddyDeathRecoveryUpdate",
        state,
        buddy_slot);
  }

  const SlotSnapshot after_retry = ReadSlot(state, buddy_slot);
  const bool changed = SlotChanged(before_retry, after_retry);
  LogSlotChange(
      completed ? "PendingDeployRepair retry_original_updates" : "PendingDeployRepair retry_exception",
      buddy_slot,
      before_retry,
      after_retry);
  if (changed) {
    ResetPendingDeployRepair(buddy_slot, "advanced");
    return true;
  }

  LogWarn(
      "PendingDeployRepair no_progress slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x dwellTicks=%u retries=%u",
      buddy_slot,
      entity.high,
      entity.low,
      snapshot.state,
      snapshot.pending,
      snapshot.flags,
      dwell_ticks,
      repair.retry_count);
  return false;
}


// Original src/dllmain.cpp:10033
bool ForceFreshResummonFromStableActiveSlot(void* state, int buddy_slot, const char* reason) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) || g_unsummon_buddy == nullptr) {
    return false;
  }

  const SlotSnapshot before = ReadSlot(state, buddy_slot);
  if (!IsCustomBuddySnapshot(buddy_slot, before)) {
    return false;
  }

  if (before.state != kSlotStateTransition || before.pending != 0 || (before.flags & 0x02) != 0 ||
      IsEmptyEntity(before)) {
    return false;
  }

  EntityId entity{before.entity_low, before.entity_high};
  Log(
      "ForceFreshResummon begin reason=%s slot=%d entity=%08x:%08x",
      reason,
      buddy_slot,
      before.entity_high,
      before.entity_low);

  NormalizeActiveSlotForUnsummon(state, buddy_slot);
  __try {
    g_unsummon_buddy(state, &entity);
  } __except (LogSehException("ForceUnsummon FUN_140386a20/UnsummonBuddy", GetExceptionInformation())) {
    LogBuddyExceptionContext("ForceUnsummon FUN_140386a20/UnsummonBuddy", state, buddy_slot);
    return false;
  }
  StopRecallRegen(buddy_slot, "fresh_resummon");
  ResetBuddySlotState(state, buddy_slot);

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange("ForceFreshResummon end", buddy_slot, before, after);
  return true;
}


// Original src/dllmain.cpp:10071
bool ForceUnsummonActiveSlot(void* state, int buddy_slot, const char* reason, bool queue_resummon) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) || g_unsummon_buddy == nullptr) {
    return false;
  }

  const SlotSnapshot before = ReadSlot(state, buddy_slot);
  if (!IsCustomBuddySnapshot(buddy_slot, before)) {
    return false;
  }

  const bool needs_fix = queue_resummon ? IsForcedResummonState(before) : IsForcedUnsummonState(before);
  if (!needs_fix || IsEmptyEntity(before)) {
    return false;
  }

  EntityId entity{before.entity_low, before.entity_high};
  Log(
      "ForceUnsummon begin reason=%s slot=%d queueResummon=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x",
      reason,
      buddy_slot,
      queue_resummon ? 1 : 0,
      before.entity_high,
      before.entity_low,
      before.state,
      before.pending,
      before.flags);

  if (before.state == kSlotStateStuckRemoved ||
      IsStalePendingUnsummonState(before) ||
      (before.state == kSlotStateTransition && (before.flags & 0x02) != 0)) {
    NormalizeActiveSlotForUnsummon(state, buddy_slot);
  }

  __try {
    g_unsummon_buddy(state, &entity);
  } __except (LogSehException("ForceUnsummonActiveSlot FUN_140386a20/UnsummonBuddy", GetExceptionInformation())) {
    LogBuddyExceptionContext("ForceUnsummonActiveSlot FUN_140386a20/UnsummonBuddy", state, buddy_slot);
    return false;
  }
  if (queue_resummon) {
    StopRecallRegen(buddy_slot, "queue_resummon");
  } else {
    StopRecallRegen(buddy_slot, "request_unsummon_reset");
  }

  if (queue_resummon) {
    PrepareSlotForOriginalResummon(state, buddy_slot);
    if (g_buddy_update != nullptr) {
      __try {
        g_buddy_update(state);
      } __except (LogSehException(
                       "ForceUnsummon FUN_1403895e0/BuddyUpdate",
                       GetExceptionInformation())) {
        LogBuddyExceptionContext("ForceUnsummon FUN_1403895e0/BuddyUpdate", state, buddy_slot);
      }
    }
  } else {
    ResetBuddySlotState(state, buddy_slot);
  }

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange("ForceUnsummon end", buddy_slot, before, after);
  return true;
}


// Original src/dllmain.cpp:10136
bool ProcessPendingRemoteBuddyRecallCommands(void* state) {
  if (state == nullptr) {
    return false;
  }

  bool authority_known = false;
  const bool local_authority = IsLocalBuddyAuthority(state, &authority_known);
  if (!authority_known) {
    return false;
  }

  const EntityId owner_entity = ReadStateOwnerEntityId(state);
  if (IsEmptyEntityId(owner_entity)) {
    return false;
  }

  PendingBuddyRecallCommand claimed[kBuddySlotCount]{};
  size_t claimed_count = 0;
  uint32_t expired_count = 0;
  const DWORD now = GetTickCount();
  AcquireSRWLockExclusive(&g_pending_buddy_recall_lock);
  for (auto& command : g_pending_buddy_recall_commands) {
    if (!command.active) {
      continue;
    }
    if (now - command.received_ms > kBuddyRecallCommandLifetimeMs) {
      command = PendingBuddyRecallCommand{};
      ++expired_count;
      continue;
    }
    if (claimed_count >= _countof(claimed) || !SameEntity(command.owner_entity, owner_entity)) {
      continue;
    }
    claimed[claimed_count++] = command;
    command = PendingBuddyRecallCommand{};
  }
  ReleaseSRWLockExclusive(&g_pending_buddy_recall_lock);

  if (expired_count != 0) {
    LogWarn(
        "BuddyRecallGlue queue_expired count=%u owner=%08x:%08x",
        expired_count,
        owner_entity.high,
        owner_entity.low);
  }
  if (claimed_count == 0) {
    return false;
  }

  bool any_applied = false;
  for (size_t i = 0; i < claimed_count; ++i) {
    const PendingBuddyRecallCommand& command = claimed[i];
    if (local_authority) {
      LogWarn(
          "BuddyRecallGlue apply_skipped reason=local_authority sender=%016llx sequence=%u owner=%08x:%08x slot=%d entity=%08x:%08x",
          static_cast<unsigned long long>(command.sender_steam_id),
          command.sequence,
          command.owner_entity.high,
          command.owner_entity.low,
          command.buddy_slot,
          command.buddy_entity.high,
          command.buddy_entity.low);
      continue;
    }

    const int buddy_slot = command.buddy_slot;
    if (!IsValidBuddySlot(buddy_slot)) {
      continue;
    }
    const SlotSnapshot before = ReadSlot(state, buddy_slot);
    const EntityId slot_entity{before.entity_low, before.entity_high};
    if (IsEmptyEntity(before)) {
      Log(
          "BuddyRecallGlue apply_complete reason=already_recalled sender=%016llx sequence=%u owner=%08x:%08x slot=%d packetEntity=%08x:%08x",
          static_cast<unsigned long long>(command.sender_steam_id),
          command.sequence,
          command.owner_entity.high,
          command.owner_entity.low,
          buddy_slot,
          command.buddy_entity.high,
          command.buddy_entity.low);
      continue;
    }
    if (!SameEntity(slot_entity, command.buddy_entity)) {
      LogWarn(
          "BuddyRecallGlue apply_skipped reason=entity_mismatch sender=%016llx sequence=%u owner=%08x:%08x slot=%d packetEntity=%08x:%08x slotEntity=%08x:%08x state=%02x pending=%02x flags=%02x",
          static_cast<unsigned long long>(command.sender_steam_id),
          command.sequence,
          command.owner_entity.high,
          command.owner_entity.low,
          buddy_slot,
          command.buddy_entity.high,
          command.buddy_entity.low,
          slot_entity.high,
          slot_entity.low,
          before.state,
          before.pending,
          before.flags);
      continue;
    }
    if (!IsCustomBuddySnapshot(buddy_slot, before)) {
      LogWarn(
          "BuddyRecallGlue apply_skipped reason=not_tracked_custom sender=%016llx sequence=%u owner=%08x:%08x slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x",
          static_cast<unsigned long long>(command.sender_steam_id),
          command.sequence,
          command.owner_entity.high,
          command.owner_entity.low,
          buddy_slot,
          slot_entity.high,
          slot_entity.low,
          before.state,
          before.pending,
          before.flags);
      continue;
    }
    if (!NormalizeActiveSlotForUnsummon(state, buddy_slot)) {
      LogWarn(
          "BuddyRecallGlue apply_failed reason=prepare_write sender=%016llx sequence=%u owner=%08x:%08x slot=%d entity=%08x:%08x",
          static_cast<unsigned long long>(command.sender_steam_id),
          command.sequence,
          command.owner_entity.high,
          command.owner_entity.low,
          buddy_slot,
          slot_entity.high,
          slot_entity.low);
      continue;
    }
    const bool applied = ForceUnsummonActiveSlot(
        state,
        buddy_slot,
        "RemoteAuthorityRecallPacket",
        false);
    const SlotSnapshot after = ReadSlot(state, buddy_slot);
    if (applied) {
      StopStuckRecoveryMonitor(buddy_slot, "remote_authority_recall");
      ResetPendingDeployRepair(buddy_slot, "remote_authority_recall");
      StopPostRecallPulse(buddy_slot, "remote_authority_recall");
      any_applied = true;
    }
    Log(
        "BuddyRecallGlue apply_%s sender=%016llx sequence=%u owner=%08x:%08x slot=%d entity=%08x:%08x reason=%u authoritySlot=[%02x,%02x,%02x] localSlot=[%02x,%02x,%02x]->[%02x,%02x,%02x]",
        applied ? "complete" : "failed",
        static_cast<unsigned long long>(command.sender_steam_id),
        command.sequence,
        command.owner_entity.high,
        command.owner_entity.low,
        buddy_slot,
        slot_entity.high,
        slot_entity.low,
        command.reason,
        command.authority_state,
        command.authority_pending,
        command.authority_flags,
        before.state,
        before.pending,
        before.flags,
        after.state,
        after.pending,
        after.flags);
  }
  return any_applied;
}


// Original src/dllmain.cpp:10299
bool HasCustomBuddyRuntime() {
  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    if (g_buddy_slot_runtime[slot].custom_buddy &&
        !IsEmptyEntityId(g_buddy_slot_runtime[slot].entity)) {
      return true;
    }
  }

  return false;
}


// Original src/dllmain.cpp:10310
void CancelDelayedSwitchCleanup(const char* reason) {
  if (!g_delayed_switch_cleanup.active) {
    return;
  }

  Log(
      "DelayedSwitchCleanup cancel reason=%s currentSlot=%d currentEntity=%08x:%08x dueTick=%u tick=%u",
      reason,
      g_delayed_switch_cleanup.current_slot,
      g_delayed_switch_cleanup.current_entity.high,
      g_delayed_switch_cleanup.current_entity.low,
      g_delayed_switch_cleanup.due_tick,
      g_module_update_tick);
  g_delayed_switch_cleanup = DelayedSwitchCleanupState{};
}


// Original src/dllmain.cpp:10326
bool ScheduleDelayedSwitchCleanup(void* state, int current_slot, const char* reason) {
  if (state == nullptr || !IsValidBuddySlot(current_slot)) {
    return false;
  }

  const SlotSnapshot current = ReadSlot(state, current_slot);
  g_delayed_switch_cleanup.active = true;
  g_delayed_switch_cleanup.state = state;
  g_delayed_switch_cleanup.current_slot = current_slot;
  g_delayed_switch_cleanup.current_entity = EntityId{current.entity_low, current.entity_high};
  g_delayed_switch_cleanup.due_tick = g_module_update_tick + kDelayedSwitchCleanupTicks;
  g_delayed_switch_cleanup.retry_count = 0;

  Log(
      "DelayedSwitchCleanup schedule reason=%s currentSlot=%d currentEntity=%08x:%08x dueTick=%u delayTicks=%u retries=%u tick=%u",
      reason,
      current_slot,
      current.entity_high,
      current.entity_low,
      g_delayed_switch_cleanup.due_tick,
      kDelayedSwitchCleanupTicks,
      g_delayed_switch_cleanup.retry_count,
      g_module_update_tick);
  return true;
}


// Original src/dllmain.cpp:10352
bool DeferForcedResummonForDelayedCleanup(void* state, int buddy_slot, const char* reason) {
  if (state == nullptr || !IsValidBuddySlot(buddy_slot) || !g_delayed_switch_cleanup.active ||
      g_delayed_switch_cleanup.state != state || !g_defer_switch_unsummon_this_try ||
      g_defer_switch_unsummon_slot != buddy_slot) {
    return false;
  }

  const SlotSnapshot before = ReadSlot(state, buddy_slot);
  if (!IsCustomBuddySnapshot(buddy_slot, before) || !IsForcedResummonState(before) ||
      IsEmptyEntity(before)) {
    return false;
  }

  Log(
      "DelayedSwitchCleanup defer_unsummon reason=%s slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x dueTick=%u",
      reason,
      buddy_slot,
      before.entity_high,
      before.entity_low,
      before.state,
      before.pending,
      before.flags,
      g_delayed_switch_cleanup.due_tick);

  StopRecallRegen(buddy_slot, "delayed_switch_defer");
  PrepareSlotForOriginalResummon(state, buddy_slot);
  if (g_buddy_update != nullptr) {
    __try {
      g_buddy_update(state);
    } __except (LogSehException(
                     "DelayedSwitchCleanup defer FUN_1403895e0/BuddyUpdate",
                     GetExceptionInformation())) {
      LogBuddyExceptionContext(
          "DelayedSwitchCleanup defer FUN_1403895e0/BuddyUpdate",
          state,
          buddy_slot);
    }
  }

  const SlotSnapshot after = ReadSlot(state, buddy_slot);
  LogSlotChange("DelayedSwitchCleanup defer_end", buddy_slot, before, after);
  return true;
}


// Original src/dllmain.cpp:10396
void ProcessDelayedSwitchCleanup(void* state) {
  if (!g_delayed_switch_cleanup.active || g_module_update_tick < g_delayed_switch_cleanup.due_tick) {
    return;
  }

  DelayedSwitchCleanupState job = g_delayed_switch_cleanup;
  g_delayed_switch_cleanup = DelayedSwitchCleanupState{};

  void* cleanup_state = job.state != nullptr ? job.state : state;
  if (cleanup_state == nullptr || !IsValidBuddySlot(job.current_slot) || g_unsummon_buddy == nullptr) {
    Log(
        "DelayedSwitchCleanup skip reason=invalid_state currentSlot=%d dueTick=%u tick=%u",
        job.current_slot,
        job.due_tick,
        g_module_update_tick);
    return;
  }

  const SlotSnapshot current_snapshot = ReadSlot(cleanup_state, job.current_slot);
  EntityId keep_entity = job.current_entity;
  if (!IsEmptyEntity(current_snapshot)) {
    keep_entity = EntityId{current_snapshot.entity_low, current_snapshot.entity_high};
  }

  Log(
      "DelayedSwitchCleanup run currentSlot=%d keepEntity=%08x:%08x dueTick=%u tick=%u",
      job.current_slot,
      keep_entity.high,
      keep_entity.low,
      job.due_tick,
      g_module_update_tick);

  if (IsEmptyEntityId(keep_entity)) {
    Log("DelayedSwitchCleanup skip reason=empty_keep_entity");
    return;
  }

  bool retry_needed = false;
  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    const SlotSnapshot snapshot = ReadSlot(cleanup_state, slot);
    if (IsEmptyEntity(snapshot) || !IsCustomBuddySnapshot(slot, snapshot)) {
      continue;
    }

    EntityId cleanup_entity{snapshot.entity_low, snapshot.entity_high};
    if (SameEntity(cleanup_entity, keep_entity)) {
      continue;
    }

    const int hp_rate = SafeGetBuddyHpRate(cleanup_state, slot);
    if ((snapshot.flags & kSlotFlagsRecoveryGate) != 0 ||
        g_zero_hp_fallback_regen[slot].active ||
        hp_rate == 0) {
      Log(
          "DelayedSwitchCleanup skip slot=%d reason=recovery_or_zero_hp entity=%08x:%08x state=%02x pending=%02x flags=%02x hpRate=%d",
          slot,
          cleanup_entity.high,
          cleanup_entity.low,
          snapshot.state,
          snapshot.pending,
          snapshot.flags,
          hp_rate);
      continue;
    }

    void* entity = LookupEntityById(cleanup_entity);
    if (entity == nullptr) {
      Log(
          "DelayedSwitchCleanup skip slot=%d reason=entity_missing entity=%08x:%08x",
          slot,
          cleanup_entity.high,
          cleanup_entity.low);
      continue;
    }

    Log(
        "DelayedSwitchCleanup unsummon slot=%d entity=%08x:%08x keepEntity=%08x:%08x state=%02x pending=%02x flags=%02x hpRate=%d",
        slot,
        cleanup_entity.high,
        cleanup_entity.low,
        keep_entity.high,
        keep_entity.low,
        snapshot.state,
        snapshot.pending,
        snapshot.flags,
        hp_rate);

    __try {
      g_unsummon_buddy(cleanup_state, &cleanup_entity);
    } __except (LogSehException(
                     "DelayedSwitchCleanup FUN_140386a20/UnsummonBuddy",
                     GetExceptionInformation())) {
      LogError(
          "DelayedSwitchCleanup unsummon_exception slot=%d entity=%08x:%08x",
          slot,
          cleanup_entity.high,
          cleanup_entity.low);
      LogBuddyExceptionContext(
          "DelayedSwitchCleanup FUN_140386a20/UnsummonBuddy",
          cleanup_state,
          slot);
      continue;
    }

    StopRecallRegen(slot, "delayed_switch_cleanup");
    StopZeroHpFallbackRegen(slot, snapshot, "delayed_switch_cleanup");
    const SlotSnapshot after = ReadSlot(cleanup_state, slot);
    if (SameEntity(EntityId{after.entity_low, after.entity_high}, cleanup_entity)) {
      ResetBuddySlotState(cleanup_state, slot);
      LogSlotChange("DelayedSwitchCleanup reset_slot", slot, after, ReadSlot(cleanup_state, slot));
    }

    const SlotSnapshot verify = ReadSlot(cleanup_state, slot);
    const EntityId verify_entity{verify.entity_low, verify.entity_high};
    const bool still_visible =
        SameEntity(verify_entity, cleanup_entity) &&
        (verify.pending != 0 || verify.state == kSlotStateDeploying ||
         verify.state == kSlotStateTransition || (verify.flags & 0x10) != 0);
    retry_needed = retry_needed || still_visible;
  }

  if (retry_needed && job.retry_count < kDelayedSwitchCleanupMaxRetries) {
    g_delayed_switch_cleanup.active = true;
    g_delayed_switch_cleanup.state = cleanup_state;
    g_delayed_switch_cleanup.current_slot = job.current_slot;
    g_delayed_switch_cleanup.current_entity = keep_entity;
    g_delayed_switch_cleanup.due_tick = g_module_update_tick + kDelayedSwitchCleanupTicks;
    g_delayed_switch_cleanup.retry_count = job.retry_count + 1;
    Log(
        "DelayedSwitchCleanup reschedule currentSlot=%d keepEntity=%08x:%08x dueTick=%u delayTicks=%u retry=%u/%u tick=%u",
        job.current_slot,
        keep_entity.high,
        keep_entity.low,
        g_delayed_switch_cleanup.due_tick,
        kDelayedSwitchCleanupTicks,
        g_delayed_switch_cleanup.retry_count,
        kDelayedSwitchCleanupMaxRetries,
        g_module_update_tick);
  }
}


// Original src/dllmain.cpp:10537
void __fastcall HookBuddyModuleUpdate(void* state) {
  // Per-frame NecroBuddy state update. This hook does not decide whether a
  // summon starts; it is the timing driver for custom-buddy maintenance:
  // delayed switch cleanup, abnormal zero-HP fallback regen, and recall regen.
  // Vanilla buddies are left to the original update unless a slot has been
  // marked custom by HookCreateBuddy.
  if (state != nullptr) {
    g_last_buddy_state = state;
  }
  ++g_module_update_tick;

  if (!HasActiveBuddyMaintenanceWork(state)) {
    CallBuddyModuleUpdateOriginalPlain(state, "BuddyModuleUpdate fast_path");
    return;
  }

  ScopedBuddyStateRuntime scoped_runtime(state);
  bool authority_known = false;
  const bool local_authority = IsLocalBuddyAuthority(state, &authority_known);

  SlotSnapshot before[kBuddySlotCount]{};
  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    before[slot] = ReadSlot(state, slot);
  }

  if (HasPendingCustomDeployRequest(state)) {
    TryActivateOwnerWorldGroupFromState(state, "BuddyModuleUpdatePreOriginal");
  }

  CallBuddyModuleUpdateOriginalPlain(state, "BuddyModuleUpdate");
  RunRemoteOwnerGateFallbackUpdate(state);
  ProcessPendingRemoteBuddyRecallCommands(state);

  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    const SlotSnapshot after = ReadSlot(state, slot);
    const int hp_after = SafeGetBuddyHpRate(state, slot);

    EnsureCustomCharaBuddyPreventNearDeath(slot, after);

    if (ProcessPostRecallPulse(state, slot, after)) {
      continue;
    }

    if (before[slot].state == kSlotStateStuckRemoving &&
        after.state == kSlotStateActive && after.pending == 0 && hp_after > 0) {
      if (StartPostRecallPulse(state, slot, after, hp_after, "original_14_to_00")) {
        continue;
      }
    }

    const bool pending_repair_handled =
        UpdatePendingDeployRepair(state, slot, after);
    if (pending_repair_handled) {
      continue;
    }

    const bool stuck_recovery_handled =
        UpdateStuckRecoveryState(state, slot, after, hp_after);
    if (stuck_recovery_handled) {
      const SlotSnapshot after_stuck_recovery = ReadSlot(state, slot);
      if (after.state == kSlotStateStuckRemoving &&
          after_stuck_recovery.state == kSlotStateActive &&
          after_stuck_recovery.pending == 0 && hp_after > 0) {
        StartPostRecallPulse(state, slot, after_stuck_recovery, hp_after, "stuck_recovery_14_to_00");
      }
      continue;
    }

    const bool may_write_buddy_hp = authority_known && local_authority;
    const bool zero_hp_fallback_handled =
        may_write_buddy_hp &&
        UpdateZeroHpFallbackRegen(state, slot, after, hp_after);
    const bool recall_regen_handled =
        may_write_buddy_hp && !zero_hp_fallback_handled &&
        UpdateRecallRegen(state, slot, after, hp_after);
    if (zero_hp_fallback_handled || recall_regen_handled) {
      continue;
    }
  }

  ProcessDelayedSwitchCleanup(state);
}


// Original src/dllmain.cpp:10620
uint64_t CallTryUseBuddyOriginalSeh(void* state, uint32_t buddy_trigger_id, int buddy_slot) {
  if (g_try_use_buddy == nullptr) {
    return 0;
  }

  uint64_t result = 0;
  __try {
    result = g_try_use_buddy(state, buddy_trigger_id);
  } __except (LogSehException("TryUseBuddy original", GetExceptionInformation())) {
    LogError(
        "TryUseBuddy original_exception state=%p trigger=%u buddySlot=%d",
        state,
        buddy_trigger_id,
        buddy_slot);
    LogBuddyExceptionContext("TryUseBuddy original", state, buddy_slot);
  }
  return result;
}


// Original src/dllmain.cpp:10639
uint32_t* CallCreateBuddyOriginalSeh(
    void* state,
    uint32_t* out_entity,
    uint32_t owner_id,
    int owner_slot,
    int buddy_slot,
    int entity_slot,
    const void* buddy_create_data,
    char flag,
    int32_t chara_init,
    int32_t npc_param,
    int32_t npc_think_param,
    bool custom_candidate) {
  uint32_t* result = nullptr;
  __try {
    result = g_create_buddy(state, out_entity, owner_id, buddy_slot, buddy_create_data, flag);
  } __except (LogSehException("CreateBuddy original", GetExceptionInformation())) {
    LogError(
        "CreateBuddy original_exception state=%p out=%p owner=%u ownerSlot=%d buddySlot=%d entitySlot=%d charaInit=%d npcParam=%d npcThinkParam=%d flag=%d customCandidate=%d",
        state,
        out_entity,
        owner_id,
        owner_slot,
        buddy_slot,
        entity_slot,
        chara_init,
        npc_param,
        npc_think_param,
        static_cast<int>(flag),
        custom_candidate ? 1 : 0);
    LogBuddyExceptionContext("CreateBuddy original", state, buddy_slot);
  }
  return result;
}


// Original src/dllmain.cpp:10674
bool CallChangeBuddyStateOriginalSeh(void* state, int buddy_slot) {
  __try {
    g_change_buddy_state(state, buddy_slot);
  } __except (LogSehException("ChangeBuddyState original", GetExceptionInformation())) {
    LogError(
        "ChangeBuddyState original_exception state=%p slot=%d",
        state,
        buddy_slot);
    LogBuddyExceptionContext("ChangeBuddyState original", state, buddy_slot);
    return false;
  }
  return true;
}


// Original src/dllmain.cpp:10688
bool CallRequestUnsummonOriginalSeh(void* state, int buddy_slot) {
  __try {
    g_request_unsummon(state, buddy_slot);
  } __except (LogSehException("RequestUnsummon original", GetExceptionInformation())) {
    LogError(
        "RequestUnsummon original_exception state=%p slot=%d",
        state,
        buddy_slot);
    LogBuddyExceptionContext("RequestUnsummon original", state, buddy_slot);
    return false;
  }
  return true;
}


// Original src/dllmain.cpp:10702
uint64_t __fastcall HookTryUseBuddy(void* state, uint32_t buddy_trigger_id) {
  // High-level use gate for Revenant buddy trigger SpEffects. If this returns 0
  // the animation may have played, but ChangeBuddyState/CreateBuddy will not be
  // reached. The hook logs that decision point, blocks custom buddies that are
  // still in fallback recovery, and schedules delayed cleanup when switching to
  // a different buddy slot.
  const int buddy_slot = MapRevenantBuddyTriggerToSlot(buddy_trigger_id);
  if (!IsValidBuddySlot(buddy_slot)) {
    return CallTryUseBuddyOriginalSeh(state, buddy_trigger_id, buddy_slot);
  }

  if (state != nullptr) {
    g_last_buddy_state = state;
  }
  ScopedBuddyStateRuntime scoped_runtime(state);
  BuddyStateRuntimeContext* runtime_context = GetCurrentBuddyStateRuntime();
  const int context_index = GetBuddyStateRuntimeIndex(runtime_context);
  MarkRevenantBuddyOwnerSeen(
      runtime_context,
      runtime_context != nullptr ? runtime_context->owner_id : kInvalidEntityPart,
      runtime_context != nullptr ? runtime_context->owner_slot : -1);

  const SlotSnapshot target_before = ReadSlot(state, buddy_slot);

  if (IsZeroHpFallbackSummonBlocked(state, buddy_trigger_id)) {
    Log(
        "TryUseBuddy post state=%p trigger=%u result=0 reason=fallback_recovery_locked",
        state,
        buddy_trigger_id);
    return 0;
  }

  Log(
      "TryUseBuddy pre state=%p ctx=%d knownOwner=%d owner=%u ownerSlot=%d trigger=%u buddySlot=%d likelyRevenant=%d entity=%08x:%08x stateByte=%02x pending=%02x flags=%02x",
      state,
      context_index,
      runtime_context != nullptr && runtime_context->owner_known ? 1 : 0,
      runtime_context != nullptr ? runtime_context->owner_id : 0xffffffff,
      runtime_context != nullptr ? runtime_context->owner_slot : -1,
      buddy_trigger_id,
      buddy_slot,
      IsLikelyRevenantBuddyActionId(static_cast<int>(buddy_trigger_id)) ? 1 : 0,
      target_before.entity_high,
      target_before.entity_low,
      target_before.state,
      target_before.pending,
      target_before.flags);
  const bool target_already_deployed =
      IsValidBuddySlot(buddy_slot) &&
      IsCustomBuddyDeployedSlot(state, buddy_slot, target_before);
  const bool scheduled_cleanup =
      IsValidBuddySlot(buddy_slot) && !target_already_deployed && HasCustomBuddyRuntime() &&
      ScheduleDelayedSwitchCleanup(state, buddy_slot, "try_use_pre");

  g_defer_switch_unsummon_this_try = scheduled_cleanup;
  g_defer_switch_unsummon_slot = scheduled_cleanup ? buddy_slot : -1;
  const uint64_t result = CallTryUseBuddyOriginalSeh(state, buddy_trigger_id, buddy_slot);
  g_defer_switch_unsummon_this_try = false;
  g_defer_switch_unsummon_slot = -1;

  if (result != 0 && IsValidBuddySlot(buddy_slot)) {
    StopRecallRegen(buddy_slot, "try_use_success");
    if (!target_already_deployed && (scheduled_cleanup || HasCustomBuddyRuntime())) {
      ScheduleDelayedSwitchCleanup(state, buddy_slot, "try_use_post");
    }
  } else if (scheduled_cleanup) {
    CancelDelayedSwitchCleanup("try_use_failed");
  }

  const SlotSnapshot target_after =
      IsValidBuddySlot(buddy_slot) ? ReadSlot(state, buddy_slot) : SlotSnapshot{};
  Log(
      "TryUseBuddy post state=%p ctx=%d knownOwner=%d owner=%u ownerSlot=%d trigger=%u buddySlot=%d likelyRevenant=%d result=%llu entity=%08x:%08x stateByte=%02x pending=%02x flags=%02x",
      state,
      context_index,
      runtime_context != nullptr && runtime_context->owner_known ? 1 : 0,
      runtime_context != nullptr ? runtime_context->owner_id : 0xffffffff,
      runtime_context != nullptr ? runtime_context->owner_slot : -1,
      buddy_trigger_id,
      buddy_slot,
      IsLikelyRevenantBuddyActionId(static_cast<int>(buddy_trigger_id)) ? 1 : 0,
      static_cast<unsigned long long>(result),
      target_after.entity_high,
      target_after.entity_low,
      target_after.state,
      target_after.pending,
      target_after.flags);

  if (target_already_deployed) {
    Log(
        "TryUseBuddy same_slot_immediate_resummon slot=%d entity=%08x:%08x state=%02x pending=%02x flags=%02x",
        buddy_slot,
        target_before.entity_high,
        target_before.entity_low,
        target_before.state,
        target_before.pending,
        target_before.flags);
  }

  return result;
}


// Original src/dllmain.cpp:10804
uint32_t* __fastcall HookCreateBuddy(
    void* state,
    uint32_t* out_entity,
    uint32_t owner_id,
    int buddy_slot,
    const void* buddy_create_data,
    char flag) {
  // CreateBuddy is the factory path reached before a buddy is actually stored in
  // the per-slot state. It receives the owner id, the 0..2 Revenant buddy slot,
  // and the BuddyParam-derived create data: NpcParam/NpcThinkParam plus the
  // CharaInit selector. The hook lets the original factory run first, then
  // records the produced entity and HP component if the data indicates a custom
  // buddy. Later fixes use that runtime record to remove/regen only entities the
  // vanilla NecroBuddy manager did not fully manage.
  EnsureBuddyEntitySlotCapacity(GetBuddyEntityManager());

  if (state != nullptr) {
    g_last_buddy_state = state;
  }
  const int owner_slot = static_cast<int>(owner_id & 0xff);
  ScopedBuddyStateRuntime scoped_runtime(state, owner_id, true);
  BuddyStateRuntimeContext* runtime_context = GetCurrentBuddyStateRuntime();
  const int context_index = GetBuddyStateRuntimeIndex(runtime_context);
  const uint64_t buddy_param_pair = ReadU64(buddy_create_data, 0);
  const int32_t npc_param = static_cast<int32_t>(buddy_param_pair & 0xffffffffu);
  const int32_t npc_think_param = static_cast<int32_t>((buddy_param_pair >> 32) & 0xffffffffu);
  const int32_t chara_init =
      static_cast<int32_t>(ReadU32(static_cast<const uint8_t*>(buddy_create_data) + 8, 0xffffffff));
  const int entity_slot = static_cast<int>(owner_id) < 0 ? -1 : (owner_slot + 2) * 10 + buddy_slot;
  const bool custom_candidate = IsCustomBuddyCreateData(buddy_slot, chara_init, npc_param);
  if (runtime_context != nullptr) {
    runtime_context->owner_known = true;
    runtime_context->owner_id = owner_id;
    runtime_context->owner_slot = owner_slot;
    MarkRevenantBuddyOwnerSeen(
        runtime_context,
        owner_id,
        owner_slot);
  }

  const ActiveCreateBuddyContext previous_create_context = g_active_create_context;
  g_active_create_context = ActiveCreateBuddyContext{
      true,
      state,
      owner_id,
      owner_slot,
      buddy_slot,
      entity_slot,
      chara_init,
      npc_param,
      npc_think_param,
      custom_candidate,
  };
  uint32_t* result = CallCreateBuddyOriginalSeh(
      state,
      out_entity,
      owner_id,
      owner_slot,
      buddy_slot,
      entity_slot,
      buddy_create_data,
      flag,
      chara_init,
      npc_param,
      npc_think_param,
      custom_candidate);
  g_active_create_context = previous_create_context;

  const uint32_t out_low = out_entity != nullptr ? ReadU32(out_entity, kInvalidEntityPart) : kInvalidEntityPart;
  const uint32_t out_high =
      out_entity != nullptr ? ReadU32(out_entity + 1, kInvalidEntityPart) : kInvalidEntityPart;
  const EntityId created_entity{out_low, out_high};
  void* buddy_entity_manager = GetBuddyEntityManager();
  const bool owned_create_path = owner_id != kInvalidEntityPart;
  const uint32_t entity_capacity = buddy_entity_manager != nullptr
      ? ReadU32(static_cast<const uint8_t*>(buddy_entity_manager) + 0x10, 0xffffffff)
      : 0xffffffff;
  const uint64_t entity_table = buddy_entity_manager != nullptr
      ? ReadU64(static_cast<const uint8_t*>(buddy_entity_manager) + 0x18, 0)
      : 0;
  const bool entity_slot_in_range =
      owned_create_path && entity_table != 0 && entity_slot >= 0 &&
      static_cast<uint32_t>(entity_slot) < entity_capacity;
  const uint64_t entity_slot_value = entity_slot_in_range
      ? ReadU64(reinterpret_cast<const void*>(
                    entity_table + static_cast<uint64_t>(entity_slot) * kBuddyEntitySlotEntrySize),
                0)
      : 0;
  Log(
      "CreateBuddy result state=%p ctx=%d result=%p outEntity=%08x:%08x owner=%u ownerSlot=%d buddySlot=%d entitySlot=%d charaInit=%d npcParam=%d npcThinkParam=%d flag=%d customCandidate=%d",
      state,
      context_index,
      result,
      out_high,
      out_low,
      owner_id,
      owner_slot,
      buddy_slot,
      entity_slot,
      chara_init,
      npc_param,
      npc_think_param,
      static_cast<int>(flag),
      custom_candidate ? 1 : 0);
  const char* create_failure_reason = nullptr;
  if (state == nullptr) {
    create_failure_reason = "null_state";
  } else if (buddy_create_data == nullptr) {
    create_failure_reason = "null_create_data";
  } else if (out_entity == nullptr) {
    create_failure_reason = "null_out_entity";
  } else if (!IsValidBuddySlot(buddy_slot)) {
    create_failure_reason = "invalid_buddy_slot";
  } else if (IsEmptyEntityId(created_entity)) {
    if (owned_create_path && buddy_entity_manager == nullptr) {
      create_failure_reason = "null_buddy_entity_manager";
    } else if (owned_create_path && !entity_slot_in_range) {
      create_failure_reason = "entity_slot_out_of_range";
    } else if (owned_create_path && entity_slot_value != 0) {
      create_failure_reason = "entity_slot_occupied_or_stale";
    } else if (chara_init >= 0) {
      create_failure_reason = "custom_chara_init_create_failed";
    } else if (custom_candidate) {
      create_failure_reason = "custom_npc_param_create_failed";
    } else {
      create_failure_reason = owned_create_path ? "owned_entity_create_failed" : "free_entity_create_failed";
    }
  }
  if (create_failure_reason != nullptr) {
    LogError(
        "CreateBuddy failed reason=%s state=%p ctx=%d result=%p outEntity=%08x:%08x owner=%u ownerSlot=%d buddySlot=%d entitySlot=%d ownedPath=%d manager=%p capacity=%u inRange=%d slotValue=%016llx charaInit=%d npcParam=%d npcThinkParam=%d flag=%d customCandidate=%d",
        create_failure_reason,
        state,
        context_index,
        result,
        out_high,
        out_low,
        owner_id,
        owner_slot,
        buddy_slot,
        entity_slot,
        owned_create_path ? 1 : 0,
        buddy_entity_manager,
        entity_capacity,
        entity_slot_in_range ? 1 : 0,
        static_cast<unsigned long long>(entity_slot_value),
        chara_init,
        npc_param,
        npc_think_param,
        static_cast<int>(flag),
        custom_candidate ? 1 : 0);
  }
  void* created_entity_ptr = !IsEmptyEntityId(created_entity)
      ? LookupEntityById(created_entity)
      : nullptr;

  if (IsValidBuddySlot(buddy_slot)) {
    StopRecallRegen(buddy_slot, "new_create");
    StopZeroHpFallbackRegen(buddy_slot, ReadSlot(state, buddy_slot), "new_create");

    auto& runtime = g_buddy_slot_runtime[buddy_slot];
    const bool custom_buddy = custom_candidate;
    if (custom_buddy && !IsEmptyEntityId(created_entity)) {
      void* entity = created_entity_ptr;
      void* hp_component = GetHpComponentFromEntity(entity);
      const bool custom_chara_init = chara_init >= 0;
      const bool custom_npc_param = npc_param != kVanillaBuddyNpcParamIds[buddy_slot];
      runtime.custom_buddy = true;
      runtime.custom_chara_init = custom_chara_init;
      runtime.custom_npc_param = custom_npc_param;
      runtime.prevent_near_death_applied = false;
      runtime.prevent_near_death_last_failure_tick = 0;
      runtime.chara_init = chara_init;
      runtime.npc_param = npc_param;
      runtime.npc_think_param = npc_think_param;
      runtime.entity = created_entity;
      runtime.hp_component = hp_component;
      Log(
          "BuddyRuntime custom slot=%d sourceSlot=%d entity=%08x:%08x customCharaInit=%d customNpcParam=%d charaInit=%d npcParam=%d npcThinkParam=%d hpComponent=%p reason=create_buddy",
          buddy_slot,
          buddy_slot,
          created_entity.high,
          created_entity.low,
          custom_chara_init ? 1 : 0,
          custom_npc_param ? 1 : 0,
          chara_init,
          npc_param,
          npc_think_param,
          hp_component);
      if (entity == nullptr || hp_component == nullptr) {
        LogWarn(
            "BuddyRuntime custom incomplete slot=%d entity=%08x:%08x entityPtr=%p hpComponent=%p reason=%s charaInit=%d npcParam=%d npcThinkParam=%d",
            buddy_slot,
            created_entity.high,
            created_entity.low,
            entity,
            hp_component,
            entity == nullptr ? "entity_lookup_failed" : "hp_component_missing",
            chara_init,
            npc_param,
            npc_think_param);
      }
    } else {
      if (runtime.custom_buddy) {
        Log(
            "BuddyRuntime clear slot=%d previousEntity=%08x:%08x previousCharaInit=%d previousNpcParam=%d previousNpcThinkParam=%d newCharaInit=%d newNpcParam=%d newNpcThinkParam=%d reason=create_buddy",
            buddy_slot,
            runtime.entity.high,
            runtime.entity.low,
            runtime.chara_init,
            runtime.npc_param,
            runtime.npc_think_param,
            chara_init,
            npc_param,
            npc_think_param);
      }
      runtime = BuddySlotRuntimeState{};
    }
  }

  return result;
}


// Original src/dllmain.cpp:11027
void* __fastcall HookCreateEntityForOwner(void* manager, void* create_data, uint8_t owner_id, int buddy_slot) {
  // This lower-level entity allocator maps owner/buddy slot to the shared
  // CSChrSet.NecroBuddy table: entitySlot = (owner + 2) * 10 + buddySlot. It is
  // useful for six-player debugging because owners 4..6 land in higher table
  // slots; if SeamlessCoop has not expanded that table, creation can fail before
  // the buddy reaches normal state management.
  if (manager == nullptr) {
    LogError(
        "CreateEntityForOwner failed_pre reason=null_manager owner=%u buddySlot=%d createData=%p",
        static_cast<unsigned int>(owner_id),
        buddy_slot,
        create_data);
    void* null_manager_result = nullptr;
    __try {
      null_manager_result = g_create_entity_for_owner(manager, create_data, owner_id, buddy_slot);
    } __except (LogSehException("CreateEntityForOwner original null_manager", GetExceptionInformation())) {
      LogError(
          "CreateEntityForOwner original_exception reason=null_manager owner=%u buddySlot=%d createData=%p",
          static_cast<unsigned int>(owner_id),
          buddy_slot,
          create_data);
      return nullptr;
    }
    return null_manager_result;
  }

  EnsureBuddyEntitySlotCapacity(manager);

  const int entity_slot = (static_cast<int>(owner_id) + 2) * 10 + buddy_slot;
  const uint32_t capacity = ReadU32(static_cast<uint8_t*>(manager) + 0x10, 0xffffffff);
  const uint64_t table = ReadU64(static_cast<uint8_t*>(manager) + 0x18, 0);
  const bool entity_slot_in_range =
      table != 0 && entity_slot >= 0 && static_cast<uint32_t>(entity_slot) < capacity;
  const uint64_t existing = entity_slot_in_range
      ? ReadU64(reinterpret_cast<const void*>(table + static_cast<uint64_t>(entity_slot) * 0x10), 0)
      : 0;
  if (create_data == nullptr) {
    LogError(
        "CreateEntityForOwner failed_pre reason=null_create_data manager=%p owner=%u buddySlot=%d entitySlot=%d",
        manager,
        static_cast<unsigned int>(owner_id),
        buddy_slot,
        entity_slot);
  } else if (!entity_slot_in_range) {
    LogError(
        "CreateEntityForOwner failed_pre reason=entity_slot_out_of_range manager=%p owner=%u buddySlot=%d entitySlot=%d capacity=%u table=%016llx",
        manager,
        static_cast<unsigned int>(owner_id),
        buddy_slot,
        entity_slot,
        capacity,
        static_cast<unsigned long long>(table));
  } else if (existing != 0) {
    LogWarn(
        "CreateEntityForOwner precondition reason=entity_slot_occupied manager=%p owner=%u buddySlot=%d entitySlot=%d existing=%016llx",
        manager,
        static_cast<unsigned int>(owner_id),
        buddy_slot,
        entity_slot,
        static_cast<unsigned long long>(existing));
  }

  void* result = nullptr;
  __try {
    result = g_create_entity_for_owner(manager, create_data, owner_id, buddy_slot);
  } __except (LogSehException("CreateEntityForOwner original", GetExceptionInformation())) {
    LogError(
        "CreateEntityForOwner original_exception manager=%p owner=%u buddySlot=%d entitySlot=%d capacity=%u inRange=%d table=%016llx existing=%016llx createData=%p",
        manager,
        static_cast<unsigned int>(owner_id),
        buddy_slot,
        entity_slot,
        capacity,
        entity_slot_in_range ? 1 : 0,
        static_cast<unsigned long long>(table),
        static_cast<unsigned long long>(existing),
        create_data);
  }

  const uint32_t post_capacity = ReadU32(static_cast<uint8_t*>(manager) + 0x10, 0xffffffff);
  const uint64_t post_table = ReadU64(static_cast<uint8_t*>(manager) + 0x18, 0);
  const bool post_entity_slot_in_range =
      post_table != 0 && entity_slot >= 0 && static_cast<uint32_t>(entity_slot) < post_capacity;
  const uint64_t after = post_entity_slot_in_range
      ? ReadU64(reinterpret_cast<const void*>(post_table + static_cast<uint64_t>(entity_slot) * 0x10), 0)
      : 0;
  if (result == nullptr) {
    const char* reason = "create_returned_null";
    if (create_data == nullptr) {
      reason = "null_create_data";
    } else if (!post_entity_slot_in_range) {
      reason = "entity_slot_out_of_range";
    } else if (existing != 0) {
      reason = "entity_slot_occupied";
    } else if (after == 0) {
      reason = "entity_factory_failed";
    } else {
      reason = "result_null_but_slot_changed";
    }
    LogError(
        "CreateEntityForOwner failed_post reason=%s manager=%p owner=%u buddySlot=%d entitySlot=%d capacity=%u inRange=%d existing=%016llx after=%016llx createData=%p",
        reason,
        manager,
        static_cast<unsigned int>(owner_id),
        buddy_slot,
        entity_slot,
        post_capacity,
        post_entity_slot_in_range ? 1 : 0,
        static_cast<unsigned long long>(existing),
        static_cast<unsigned long long>(after),
        create_data);
  } else if (post_entity_slot_in_range && after == 0) {
    LogWarn(
        "CreateEntityForOwner inconsistent_post reason=result_nonnull_but_slot_empty result=%p manager=%p owner=%u buddySlot=%d entitySlot=%d capacity=%u existing=%016llx createData=%p",
        result,
        manager,
        static_cast<unsigned int>(owner_id),
        buddy_slot,
        entity_slot,
        post_capacity,
        static_cast<unsigned long long>(existing),
        create_data);
  }
  return result;
}


// Original src/dllmain.cpp:11153
void* __fastcall HookPlayerGameDataAlloc(void* manager) {
  const int fixed_slot = FixedPlayerGameDataSlotForActiveCreate();

  if (fixed_slot >= 0) {
    void* result = AllocFixedCustomPlayerGameData(manager, fixed_slot);
    if (result != nullptr) {
      return result;
    }
  }

  void* result = nullptr;
  __try {
    result = g_player_game_data_alloc(manager);
  } __except (LogSehException("PlayerGameDataAlloc original", GetExceptionInformation())) {
    LogError(
        "PlayerGameDataAlloc original_exception manager=%p activeCreate=%d fixedSlot=%d owner=%u ownerSlot=%d buddySlot=%d entitySlot=%d charaInit=%d npcParam=%d npcThinkParam=%d custom=%d",
        manager,
        g_active_create_context.active ? 1 : 0,
        fixed_slot,
        g_active_create_context.owner_id,
        g_active_create_context.owner_slot,
        g_active_create_context.buddy_slot,
        g_active_create_context.entity_slot,
        g_active_create_context.chara_init,
        g_active_create_context.npc_param,
        g_active_create_context.npc_think_param,
        g_active_create_context.custom_candidate ? 1 : 0);
    return nullptr;
  }

  return result;
}


// Original src/dllmain.cpp:11186
void TryRegisterBaseOwnerWorldGroup(
    void* world_chr_man,
    uint64_t entity_index64,
    void* source,
    void* aux,
    const char* caller) {
  if (world_chr_man == nullptr || source == nullptr || aux != nullptr ||
      g_world_owner_group_register == nullptr || entity_index64 > 0x84) {
    return;
  }

  const uint32_t entity_index = static_cast<uint32_t>(entity_index64);
  const int32_t local_base_index = GetLocalBaseOwnerIndex(world_chr_man);
  if (local_base_index < 0 || entity_index != static_cast<uint32_t>(local_base_index)) {
    return;
  }

  void* world_owner_group = GetWorldOwnerGroup(world_chr_man);
  const int32_t capacity = GetWorldOwnerGroupCapacity(world_owner_group);
  if (world_owner_group == nullptr || capacity <= 0 ||
      entity_index >= static_cast<uint32_t>(capacity)) {
    LogWarn(
        "WorldChrRegister/FUN_14051de60 base_group_skip caller=%s world=%p source=%p index=%u localBase=%d group=%p capacity=%d reason=group_missing_or_index_out_of_range",
        caller != nullptr ? caller : "<unknown>",
        world_chr_man,
        source,
        entity_index,
        local_base_index,
        world_owner_group,
        capacity);
    return;
  }

  const uint64_t before_slot = ReadWorldOwnerGroupSlot(world_owner_group, entity_index);
  const EntityId source_entity = ReadChrEntityId(source);
  const uint32_t source_entity_index = EntityHighIndex(source_entity);
  const bool source_entity_valid =
      source_entity.low != kInvalidEntityPart &&
      source_entity.high != kInvalidEntityPart &&
      source_entity_index == entity_index;
  bool gate_before_ok = false;
  const uint8_t gate_before = source_entity_valid
      ? SafeCallChrGate(g_remote_owner_gate, source, &gate_before_ok)
      : 0;
  bool completed = true;
  __try {
    g_world_owner_group_register(world_owner_group, static_cast<int>(entity_index), source);
  } __except (LogSehException("WorldOwnerGroupRegister original", GetExceptionInformation())) {
    completed = false;
  }

  const uint64_t after_slot = ReadWorldOwnerGroupSlot(world_owner_group, entity_index);
  if (!completed) {
    LogError(
        "WorldChrRegister/FUN_14051de60 base_group_register_exception caller=%s world=%p source=%p index=%u localBase=%d group=%p capacity=%d slotBefore=%016llx slotAfter=%016llx",
        caller != nullptr ? caller : "<unknown>",
        world_chr_man,
        source,
        entity_index,
        local_base_index,
        world_owner_group,
        capacity,
        static_cast<unsigned long long>(before_slot),
        static_cast<unsigned long long>(after_slot));
    return;
  }

  bool active_completed = false;
  bool active_exception = false;
  if (source_entity_valid && g_world_owner_group_set_entity_active != nullptr) {
    __try {
      g_world_owner_group_set_entity_active(world_owner_group, &source_entity, 1);
      active_completed = true;
    } __except (LogSehException("WorldOwnerGroupSetEntityActive base_register original", GetExceptionInformation())) {
      active_exception = true;
    }
  }

  bool gate_after_ok = false;
  const uint8_t gate_after = source_entity_valid
      ? SafeCallChrGate(g_remote_owner_gate, source, &gate_after_ok)
      : 0;

  if (before_slot == 0 || after_slot == 0 || before_slot != after_slot) {
    Log(
        "WorldChrRegister/FUN_14051de60 base_group_registered caller=%s world=%p source=%p index=%u localBase=%d sourceEntity=%08x:%08x sourceIndex=%u group=%p capacity=%d slot %016llx->%016llx activeSet=%d activeException=%d gateBefore=%d/%d gateAfter=%d/%d",
        caller != nullptr ? caller : "<unknown>",
        world_chr_man,
        source,
        entity_index,
        local_base_index,
        source_entity.low,
        source_entity.high,
        source_entity_index,
        world_owner_group,
        capacity,
        static_cast<unsigned long long>(before_slot),
        static_cast<unsigned long long>(after_slot),
        active_completed ? 1 : 0,
        active_exception ? 1 : 0,
        gate_before ? 1 : 0,
        gate_before_ok ? 1 : 0,
        gate_after ? 1 : 0,
        gate_after_ok ? 1 : 0);
  } else if (source_entity_valid && (!gate_before || !gate_after || active_exception)) {
    LogWarn(
        "WorldChrRegister/FUN_14051de60 base_group_active_bit caller=%s world=%p source=%p index=%u sourceEntity=%08x:%08x sourceIndex=%u group=%p capacity=%d slot=%016llx activeSet=%d activeException=%d gateBefore=%d/%d gateAfter=%d/%d",
        caller != nullptr ? caller : "<unknown>",
        world_chr_man,
        source,
        entity_index,
        source_entity.low,
        source_entity.high,
        source_entity_index,
        world_owner_group,
        capacity,
        static_cast<unsigned long long>(after_slot),
        active_completed ? 1 : 0,
        active_exception ? 1 : 0,
        gate_before ? 1 : 0,
        gate_before_ok ? 1 : 0,
        gate_after ? 1 : 0,
        gate_after_ok ? 1 : 0);
  } else if (!source_entity_valid) {
    LogWarn(
        "WorldChrRegister/FUN_14051de60 base_group_active_skip caller=%s world=%p source=%p index=%u localBase=%d sourceEntity=%08x:%08x sourceIndex=%u group=%p capacity=%d slot=%016llx reason=source_entity_index_mismatch_or_invalid",
        caller != nullptr ? caller : "<unknown>",
        world_chr_man,
        source,
        entity_index,
        local_base_index,
        source_entity.low,
        source_entity.high,
        source_entity_index,
        world_owner_group,
        capacity,
        static_cast<unsigned long long>(after_slot));
  }
}


// Original src/dllmain.cpp:11326
void __fastcall HookWorldChrRegister(
    void* world_chr_man,
    uint64_t entity_index,
    void* source,
    void* aux) {
  const uintptr_t caller_rva = RvaOf(_ReturnAddress());

  __try {
    g_world_chr_register(world_chr_man, entity_index, source, aux);
  } __except (LogSehException("WorldChrRegister original", GetExceptionInformation())) {
    LogError(
        "WorldChrRegister/FUN_14051de60 original_exception world=%p index=%llu source=%p aux=%p caller=%s callerRva=%llx",
        world_chr_man,
        static_cast<unsigned long long>(entity_index),
        source,
        aux,
        DescribeDiagnosticCallerRva(caller_rva),
        static_cast<unsigned long long>(caller_rva));
    return;
  }

  ObserveBhrcExpeditionWorldRegistration(
      world_chr_man, entity_index, source, aux);


  TryRegisterBaseOwnerWorldGroup(
      world_chr_man,
      entity_index,
      source,
      aux,
      "WorldChrRegister");
}


// Original src/dllmain.cpp:11358
void __fastcall HookChangeBuddyState(void* state, int buddy_slot) {
  // The original function owns the native transition and network side effects.
  // RevenantFix only fills the custom-entity maintenance gaps after it returns.
  if (state != nullptr) {
    g_last_buddy_state = state;
  }

  if (IsValidBuddySlot(buddy_slot)) {
    BuddyStateRuntimeContext* runtime_context =
        GetBuddyStateRuntime(state, true);
    MarkRevenantBuddyOwnerSeen(
        runtime_context,
        runtime_context != nullptr
            ? runtime_context->owner_id
            : kInvalidEntityPart,
        runtime_context != nullptr ? runtime_context->owner_slot : -1);
  }

  if (!HasActiveBuddyMaintenanceWork(state)) {
    CallChangeBuddyStateOriginalSeh(state, buddy_slot);
    return;
  }

  ScopedBuddyStateRuntime scoped_runtime(state);
  if (!CallChangeBuddyStateOriginalSeh(state, buddy_slot)) {
    return;
  }

  RunOriginalBuddyModuleUpdate(state, buddy_slot, "ChangeBuddyState");
  const bool remote_fallback_consumed =
      RunRemoteOwnerGateFallbackUpdate(state);
  if (remote_fallback_consumed) {
    Log(
        "ChangeBuddyState repair=remote_fallback_consumed slot=%d",
        buddy_slot);
  } else if (!DeferForcedResummonForDelayedCleanup(
                 state, buddy_slot, "ChangeBuddyState") &&
             !ForceUnsummonActiveSlot(
                 state, buddy_slot, "ChangeBuddyState", true)) {
    ForceFreshResummonFromStableActiveSlot(
        state, buddy_slot, "ChangeBuddyStateFallback");
  }
}


// Original src/dllmain.cpp:11401
void __fastcall HookRequestUnsummon(void* state, int buddy_slot) {
  // Keep the original request first; the custom cleanup only fills the gap
  // where a replaced Buddy entity remains after the native request.
  if (state != nullptr) {
    g_last_buddy_state = state;
  }

  if (IsValidBuddySlot(buddy_slot)) {
    BuddyStateRuntimeContext* runtime_context =
        GetBuddyStateRuntime(state, true);
    MarkRevenantBuddyOwnerSeen(
        runtime_context,
        runtime_context != nullptr
            ? runtime_context->owner_id
            : kInvalidEntityPart,
        runtime_context != nullptr ? runtime_context->owner_slot : -1);
  }

  if (!HasActiveBuddyMaintenanceWork(state)) {
    CallRequestUnsummonOriginalSeh(state, buddy_slot);
    return;
  }

  ScopedBuddyStateRuntime scoped_runtime(state);
  SlotSnapshot before[kBuddySlotCount]{};
  for (int slot = 0; slot < kBuddySlotCount; ++slot) {
    before[slot] = ReadSlot(state, slot);
  }

  if (!CallRequestUnsummonOriginalSeh(state, buddy_slot)) {
    return;
  }

  RunOriginalBuddyModuleUpdate(state, buddy_slot, "RequestUnsummon");
  RunRemoteOwnerGateFallbackUpdate(state);

  const int switch_target_slot =
      FindSwitchTargetSlotAfterRequest(state, buddy_slot, before);
  if (switch_target_slot >= 0 &&
      ScheduleDelayedSwitchCleanup(
          state, switch_target_slot, "request_unsummon_switch")) {
    Log(
        "DelayedSwitchCleanup repair=defer_request_unsummon oldSlot=%d currentSlot=%d",
        buddy_slot,
        switch_target_slot);
    StopRecallRegen(buddy_slot, "delayed_request_unsummon");
    return;
  }

  ForceUnsummonActiveSlot(state, buddy_slot, "RequestUnsummon", false);
}


// Original src/dllmain.cpp:11452
void __fastcall HookWorldMapRemoteMarkerUpdate(
    void* world_map_model,
    int marker_index,
    void* chr,
    uint8_t is_local) {
  __try {
    g_world_map_remote_marker_update(world_map_model, marker_index, chr, is_local);
  } __except (LogSehException("WorldMapRemoteMarkerUpdate original", GetExceptionInformation())) {
    LogError(
        "WorldMapRemoteMarkerUpdate original_exception model=%p markerIndex=%d chr=%p isLocal=%u",
        world_map_model,
        marker_index,
        chr,
        is_local);
    return;
  }

  void* resolved_chr = g_config.fix_player_map_markers
      ? ResolveWorldMapMarkerChr(marker_index, chr)
      : chr;
  RepairWorldMapMarkerAvatarResource(
      world_map_model,
      marker_index,
      chr,
      resolved_chr,
      is_local);
}


// Original src/dllmain.cpp:45797
bool RemovePlayerInsControllerAuthorityCallPatch() {
  if (g_player_ins_controller_authority_relay == nullptr &&
      !g_player_ins_controller_authority_call_patch.installed) {
    return true;
  }

  WriteCodeBytes(
      reinterpret_cast<void*>(
          g_game_base + kPlayerInsControllerAuthorityCallRva),
      kPlayerInsControllerAuthorityCall,
      sizeof(kPlayerInsControllerAuthorityCall),
      "PlayerInsControllerAuthorityCallRemove");
  uint8_t restored[sizeof(kPlayerInsControllerAuthorityCall)]{};
  bool restored_ok = ReadCodeBytes(
      reinterpret_cast<void*>(
          g_game_base + kPlayerInsControllerAuthorityCallRva),
      restored,
      sizeof(restored)) &&
      std::memcmp(
          restored,
          kPlayerInsControllerAuthorityCall,
          sizeof(restored)) == 0 &&
      FlushInstructionCache(
          GetCurrentProcess(),
          reinterpret_cast<void*>(
              g_game_base + kPlayerInsControllerAuthorityCallRva),
          sizeof(kPlayerInsControllerAuthorityCall)) != 0;
  if (!restored_ok) {
    bool relay_fallback_ok = false;
    if (g_player_ins_controller_authority_relay != nullptr &&
        g_remote_owner_gate != nullptr) {
      constexpr size_t kRelaySize = 14;
      DWORD old_protection = 0;
      if (VirtualProtect(
              g_player_ins_controller_authority_relay,
              kRelaySize,
              PAGE_EXECUTE_READWRITE,
              &old_protection)) {
        WriteAbsoluteJump(
            static_cast<uint8_t*>(
                g_player_ins_controller_authority_relay),
            reinterpret_cast<void*>(g_remote_owner_gate));
        const bool flushed = FlushInstructionCache(
            GetCurrentProcess(),
            g_player_ins_controller_authority_relay,
            kRelaySize) != 0;
        DWORD ignored = 0;
        const bool protected_rx = VirtualProtect(
            g_player_ins_controller_authority_relay,
            kRelaySize,
            PAGE_EXECUTE_READ,
            &ignored) != 0;
        relay_fallback_ok = flushed && protected_rx;
      }
    }
    LogError(
        "BuddyNetworkController authority_call_remove failed reason=callsite_not_restored relay_retained=1 relayFallbackNative=%d",
        relay_fallback_ok ? 1 : 0);
    return false;
  }

  g_player_ins_controller_authority_call_patch = CodePatch{};
  if (g_player_ins_controller_authority_relay != nullptr) {
    VirtualFree(
        g_player_ins_controller_authority_relay,
        0,
        MEM_RELEASE);
  }
  g_player_ins_controller_authority_relay = nullptr;
  return true;
}


// Original src/dllmain.cpp:45869
bool InstallPlayerInsControllerAuthorityCallPatch() {
  uint8_t context[sizeof(kPlayerInsControllerAuthorityContext)]{};
  uint8_t native_prologue[sizeof(kRemoteOwnerGatePrologue)]{};
  const bool context_ok = ReadCodeBytes(
      reinterpret_cast<void*>(
          g_game_base + kPlayerInsControllerAuthorityContextRva),
      context,
      sizeof(context)) &&
      std::memcmp(
          context,
          kPlayerInsControllerAuthorityContext,
          sizeof(context)) == 0;
  const bool native_prologue_ok = ReadCodeBytes(
      reinterpret_cast<void*>(g_game_base + kRemoteOwnerGateRva),
      native_prologue,
      sizeof(native_prologue)) &&
      std::memcmp(
          native_prologue,
          kRemoteOwnerGatePrologue,
          sizeof(native_prologue)) == 0;

  int32_t original_relative = 0;
  std::memcpy(
      &original_relative,
      kPlayerInsControllerAuthorityCall + 1,
      sizeof(original_relative));
  const uintptr_t decoded_target = static_cast<uintptr_t>(
      static_cast<int64_t>(
          g_game_base + kPlayerInsControllerAuthorityReturnRva) +
      original_relative);
  const bool target_ok =
      decoded_target == g_game_base + kRemoteOwnerGateRva;
  if (!context_ok || !native_prologue_ok || !target_ok ||
      g_remote_owner_gate != reinterpret_cast<ChrGateFn>(
          g_game_base + kRemoteOwnerGateRva)) {
    LogError(
        "BuddyNetworkController authority_call_install failed reason=preflight context=%d nativePrologue=%d decodedTarget=%p expectedTarget=%p directNative=%p",
        context_ok ? 1 : 0,
        native_prologue_ok ? 1 : 0,
        reinterpret_cast<void*>(decoded_target),
        reinterpret_cast<void*>(g_game_base + kRemoteOwnerGateRva),
        reinterpret_cast<void*>(g_remote_owner_gate));
    return false;
  }

  constexpr size_t kRelaySize = 14;
  auto* relay = static_cast<uint8_t*>(AllocateRelayNear(
      g_game_base + kPlayerInsControllerAuthorityCallRva,
      kRelaySize));
  if (relay == nullptr) {
    LogError(
        "BuddyNetworkController authority_call_install failed reason=near_relay_allocation");
    return false;
  }
  WriteAbsoluteJump(
      relay,
      reinterpret_cast<void*>(ResolvePlayerInsControllerAuthority));
  DWORD old_protection = 0;
  if (!VirtualProtect(
          relay,
          kRelaySize,
          PAGE_EXECUTE_READ,
          &old_protection)) {
    LogError(
        "BuddyNetworkController authority_call_install failed reason=relay_protect error=%lu",
        GetLastError());
    VirtualFree(relay, 0, MEM_RELEASE);
    return false;
  }
  if (!FlushInstructionCache(
          GetCurrentProcess(),
          relay,
          kRelaySize)) {
    LogError(
        "BuddyNetworkController authority_call_install failed reason=relay_flush error=%lu",
        GetLastError());
    VirtualFree(relay, 0, MEM_RELEASE);
    return false;
  }

  uint8_t replacement[sizeof(kPlayerInsControllerAuthorityCall)]{};
  if (!BuildRelativeCall(
          g_game_base + kPlayerInsControllerAuthorityCallRva,
          reinterpret_cast<uintptr_t>(relay),
          replacement)) {
    VirtualFree(relay, 0, MEM_RELEASE);
    return false;
  }
  if (!InstallCodePatch(
          g_player_ins_controller_authority_call_patch,
          kPlayerInsControllerAuthorityCallRva,
          kPlayerInsControllerAuthorityCall,
          replacement,
          sizeof(replacement),
          "PlayerInsControllerAuthorityCallPatch")) {
    WriteCodeBytes(
        reinterpret_cast<void*>(
            g_game_base + kPlayerInsControllerAuthorityCallRva),
        kPlayerInsControllerAuthorityCall,
        sizeof(kPlayerInsControllerAuthorityCall),
        "PlayerInsControllerAuthorityCallRollback");
    uint8_t restored_bytes[sizeof(replacement)]{};
    const bool restored = ReadCodeBytes(
        reinterpret_cast<void*>(
            g_game_base + kPlayerInsControllerAuthorityCallRva),
        restored_bytes,
        sizeof(restored_bytes)) &&
        std::memcmp(
            restored_bytes,
            kPlayerInsControllerAuthorityCall,
            sizeof(restored_bytes)) == 0 &&
        FlushInstructionCache(
            GetCurrentProcess(),
            reinterpret_cast<void*>(
                g_game_base + kPlayerInsControllerAuthorityCallRva),
            sizeof(kPlayerInsControllerAuthorityCall)) != 0;
    if (restored) {
      VirtualFree(relay, 0, MEM_RELEASE);
    } else {
      g_player_ins_controller_authority_relay = relay;
      LogError(
          "BuddyNetworkController authority_call_install failed reason=patch_write rollback=failed relay_retained=1");
    }
    return false;
  }

  g_player_ins_controller_authority_relay = relay;
  Log(
      "BuddyNetworkController authority_call_install ok callRva=%llx nativeTargetRva=%llx relay=%p scope=FUN_140670510_only",
      static_cast<unsigned long long>(
          kPlayerInsControllerAuthorityCallRva),
      static_cast<unsigned long long>(kRemoteOwnerGateRva),
      relay);
  return true;
}


// Original src/dllmain.cpp:46005
bool InstallBuddyControllerHooks() {
  if (!InstallPlayerInsControllerAuthorityCallPatch()) {
    LogError(
        "BuddyNetworkController install failed reason=authority_call");
    return false;
  }

  uint8_t eligibility_prologue[
      sizeof(kPlayerInsControllerEligibilityPrologue)]{};
  const bool eligibility_verified = ReadCodeBytes(
      reinterpret_cast<void*>(
          g_game_base + kPlayerInsControllerEligibilityRva),
      eligibility_prologue,
      sizeof(eligibility_prologue)) &&
      std::memcmp(
          eligibility_prologue,
          kPlayerInsControllerEligibilityPrologue,
          sizeof(eligibility_prologue)) == 0 &&
      g_hooks[kPlayerInsControllerEligibilityHookSlot].target == nullptr;
  if (!eligibility_verified ||
      !InstallInlineHook(
          g_hooks[kPlayerInsControllerEligibilityHookSlot],
          g_game_base + kPlayerInsControllerEligibilityRva,
          reinterpret_cast<void*>(HookPlayerInsControllerEligibility),
          sizeof(kPlayerInsControllerEligibilityPrologue),
          reinterpret_cast<void**>(
              &g_player_ins_controller_eligibility),
          "FUN_14066d630/PlayerIns native controller selector eligibility")) {
    RemoveInlineHook(g_hooks[kPlayerInsControllerEligibilityHookSlot]);
    g_player_ins_controller_eligibility = nullptr;
    RemovePlayerInsControllerAuthorityCallPatch();
    LogError(
        "BuddyNetworkController install failed reason=eligibility verified=%d rollback=authority_call",
        eligibility_verified ? 1 : 0);
    return false;
  }

  Log(
      "BuddyNetworkController install ok eligibilitySlot=%llu eligibilityRva=%llx authorityCallRva=%llx scope=custom_chara_buddy_only",
      static_cast<unsigned long long>(
          kPlayerInsControllerEligibilityHookSlot),
      static_cast<unsigned long long>(
          kPlayerInsControllerEligibilityRva),
      static_cast<unsigned long long>(
          kPlayerInsControllerAuthorityCallRva));
  return true;
}


}  // namespace revenantfix::internal
