#pragma once

// Internal compatibility surface for the first behavior-preserving physical
// split. It keeps the original ABI types, constants, state objects and forward
// declarations in their source order while function bodies live in the six
// owning .cpp files. This header is private to RevenantFix, not a public SDK.

#include <windows.h>

#include <cstddef>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <intrin.h>

namespace revenantfix::internal {


inline constexpr uintptr_t kUnsummonBuddyRva = 0x386a20;    // FUN_140386a20
inline constexpr uintptr_t kTryUseBuddyRva = 0x3860d0;      // FUN_1403860d0
inline constexpr uintptr_t kGetBuddyHpRateRva = 0x3863e0;   // FUN_1403863e0

// Naming note: the animation/action layers use different names for the same
// buddy transition. The TAE export calls InvokeHeroSummon[1005](slot, ...);
// c0000.hks then reaches W_Event60505 -> Event60505_onUpdate()/onDeactivate(),
// both of which call act(ChangeBuddyState). The executable also contains
// "SummonBuddy" strings, but those are playlog/network names rather than the
// native hook name used here.
inline constexpr uintptr_t kChangeBuddyStateRva = 0x386d20; // Real target of FUN_140460d00

// Native desummon entry paired with ChangeBuddyState. In the TAE export the
// animation-frame event name is InvokeHeroDesummon[1006](slot, 0, 0, 0).
inline constexpr uintptr_t kRequestUnsummonRva = 0x3874d0;  // Real target of FUN_140460d10
inline constexpr uintptr_t kBuddyModuleUpdateRva = 0x3874f0; // FUN_1403874f0
inline constexpr uintptr_t kBuddyUpdateRva = 0x3895e0;      // FUN_1403895e0
inline constexpr uintptr_t kBuddyDeathRecoveryUpdateRva = 0x3897c0; // FUN_1403897c0
inline constexpr uintptr_t kBuddyDeathRecoveryUpdateEndRva = 0x389fe0; // next function: FUN_140389fe0
inline constexpr uintptr_t kIsLocalOwnerRva = 0x3d4180; // FUN_1403d4180, owner ChrIns == local ChrIns
inline constexpr uintptr_t kRemoteOwnerGateRva = 0x3d3fe0; // FUN_1403d3fe0, non-local owner relevancy/authority gate
inline constexpr uint8_t kRemoteOwnerGatePrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x57, 0x48, 0x83,
    0xec, 0x30, 0x48, 0x8b, 0xf9, 0x33, 0xdb};
inline constexpr uintptr_t kDefaultPlayerGameDataRva = 0x20c8c0; // FUN_14020c8c0, returns DAT_143c078f8
inline constexpr uintptr_t kCreateBuddyRva = 0x389000;      // FUN_140389000
inline constexpr uintptr_t kHpSetCurrentRva = 0x426b00;     // FUN_140426b00
inline constexpr uintptr_t kApplySpEffectRva = 0x3c7fd0;    // FUN_1403c7fd0
inline constexpr uintptr_t kPlayerGameDataAllocRva = 0x1fe590; // FUN_1401fe590, allocates NPC Player GameData for positive CharaInit
inline constexpr uintptr_t kPlayerGameDataCtorRva = 0x2087c0; // FUN_1402087c0, CS::PlayerGameData constructor
inline constexpr uintptr_t kPlayerInsControllerEligibilityRva = 0x66d630; // FUN_14066d630, PlayerIns vtable+0x130 controller eligibility
inline constexpr uintptr_t kPlayerInsControllerSelectorReturnRva = 0x670522; // FUN_140670510 immediately after vtable+0x130 call
inline constexpr uintptr_t kPlayerInsControllerAuthorityCallRva = 0x67053d; // FUN_140670510 direct CALL to FUN_1403d3fe0
inline constexpr uintptr_t kPlayerInsControllerAuthorityReturnRva = 0x670542;
inline constexpr size_t kPlayerInsControllerEligibilityHookSlot = 19;
inline constexpr uint8_t kPlayerInsControllerAuthorityCall[] = {
    0xe8, 0x9e, 0x3a, 0xd6, 0xff};
inline constexpr uintptr_t kPlayerInsControllerAuthorityContextRva = 0x67053a;
inline constexpr uint8_t kPlayerInsControllerAuthorityContext[] = {
    0x48, 0x8b, 0xcb, 0xe8, 0x9e, 0x3a, 0xd6, 0xff,
    0x84, 0xc0, 0x74, 0x15};
inline constexpr uint8_t kPlayerInsControllerEligibilityPrologue[] = {
    0x48, 0x83, 0xec, 0x28, 0x48, 0x8b, 0x81,
    0xa0, 0x05, 0x00, 0x00, 0x48, 0x85, 0xc0};
inline constexpr uintptr_t kPlayerGameDataManagerGlobalPtrRva = 0x3c078d0; // DAT_143c078d0, owns local and remote PlayerGameData
inline constexpr uintptr_t kCurrentMapFamilyReadRva = 0x1fe1d0; // FUN_1401fe1d0, copies DAT_143c078d0+0x164
inline constexpr uintptr_t kMapFamilyObjectPtrRva = 0x3c123f8; // DAT_143c123f8, local map object whose +0x2c holds the raw map id
inline constexpr uintptr_t kMapFamilyDecodeRva = 0x632cd0; // FUN_140632cd0, decodes the raw +0x2c map id into a map family
inline constexpr uintptr_t kRemotePlayerGameDataWriteRangeRva = 0x1fc3e1; // FUN_1401fc260, remote peer -> remote PlayerGameData slot
inline constexpr uintptr_t kRemotePlayerGameDataClearRangeRva = 0x1fdd84; // FUN_1401fdd80, clears remote PlayerGameData slot
inline constexpr uintptr_t kRemotePlayerGameDataFlagClearRangeRva = 0x1fde00; // FUN_1401fde00, clears remote PlayerGameData visibility flag
inline constexpr uintptr_t kRemotePlayerGameDataGetByIndexRangeRva = 0x1fe530; // FUN_1401fe530, remote PlayerGameData by index
inline constexpr uintptr_t kRemotePlayerGameDataFindByEntityRange1Rva = 0x1fe560; // FUN_1401fe550 first range check
inline constexpr uintptr_t kRemotePlayerGameDataFindByEntityRange2Rva = 0x1fe57d; // FUN_1401fe550 loop bound
inline constexpr uintptr_t kRemotePlayerGameDataSearchByEntityRange1Rva = 0x1fe7d0; // FUN_1401fe7b0 first range check
inline constexpr uintptr_t kRemotePlayerGameDataSearchByEntityRange2Rva = 0x1fe7ec; // FUN_1401fe7b0 loop bound
inline constexpr uintptr_t kRemotePlayerGameDataByPeerRangeRva = 0x1fe93c; // FUN_1401fe830 peer -> remote PlayerGameData
inline constexpr uintptr_t kPlayerIndicatorRefreshValidRangeRva = 0xbeb76a; // FUN_140beb630 remote player entity-id refresh valid slot range
inline constexpr uintptr_t kPlayerIndicatorRefreshLoopRangeRva = 0xbeb7d6; // FUN_140beb630 remote player entity-id refresh loop count
inline constexpr uintptr_t kWorldMapRemoteMarkerUpdateRva = 0x90ee00; // FUN_14090ee00, writes one world-map remote player marker
inline constexpr uintptr_t kWorldChrManLookupByGameDataIdRva = 0x5197e0; // FUN_1405197e0, WorldChrMan lookup by PlayerGameData entity id
inline constexpr uintptr_t kChrIconIdRva = 0x66c6f0; // FUN_14066c6f0, ChrIns -> icon/avatar id
inline constexpr uintptr_t kWorldMapSetIconTypeRva = 0x90a280; // FUN_14090a280, fills WorldMapPlayerData icon resource block
inline constexpr uintptr_t kWorldMapSetIconIdRva = 0x90a2d0; // FUN_14090a2d0, writes WorldMapPlayerData icon id
inline constexpr uintptr_t kWorldChrRegisterRva = 0x51de60; // FUN_14051de60, WorldChrMan entity/index registration
inline constexpr uintptr_t kWorldOwnerGroupRegisterRva = 0x4eaa70; // FUN_1404eaa70, creates/updates WorldChrMan+0x17530 group slots
inline constexpr uintptr_t kWorldOwnerGroupSetEntityActiveRva = 0x4eafe0; // FUN_1404eafe0, writes WorldOwnerGroup entity active bit
inline constexpr uintptr_t kCreateEntityForOwnerRva = 0x4abdf0; // FUN_1404abdf0
inline constexpr uintptr_t kBuddyReadyRangeCheckRva = 0x4e7d60; // FUN_1404e7d60, low entity id deployable range gate
inline constexpr uintptr_t kEntityLookupRva = 0x519140;     // FUN_140519140
inline constexpr uintptr_t kWorldChrAuthorityRva = 0x519ef0; // FUN_140519ef0, true when this peer owns the entity's native state
inline constexpr uintptr_t kWorldChrStatusMarkDirtyRva = 0x4eb5d0; // FUN_1404eb5d0, queues native 0x70 HP/life status publication
inline constexpr uintptr_t kPlayerInsVtableRva = 0x2c307f8; // UNK_142c307f8, PlayerIns vtable
inline constexpr uintptr_t kWorldChrManPtrRva = 0x3c0f0a8;  // DAT_143c0f0a8
inline constexpr uintptr_t kGameManPtrRva = 0x3c13258;  // DAT_143c13258, owns PartyMemberInfo at +0xdc8
inline constexpr uintptr_t kNetModeManagerPtrRva = 0x3c21ec8; // DAT_143c21ec8
inline constexpr uintptr_t kNetSendCoreRva = 0xdf2270; // FUN_140df2270, low-level send wrapper used by host/peer broadcasts
inline constexpr uintptr_t kNativeEventReadAnyRecordRva = 0xdf1f20; // FUN_140df1f20, polls one native event across genuine session records
inline constexpr uintptr_t kNativeEventPairReadRva = 0x22d75a0; // FUN_1422d75a0, pair[0] event read called directly by FUN_140dfe070
inline constexpr uintptr_t kPartyMemberRemoveRva = 0xaedd00; // FUN_140aedd00, CS::PartyMemberInfo remove/leave and compact
inline constexpr uintptr_t kWorldMapRemotePlayerDataCountCall1Rva = 0x90e065; // FUN_14090dc80, WorldMapViewModel remote data create count
inline constexpr uintptr_t kWorldMapRemotePlayerDataCountCall2Rva = 0x90e0fd; // FUN_14090dc80, WorldMapViewModel remote data loop count
inline constexpr uintptr_t kWorldMapRemoteMarkerUpdateCountCallRva = 0xc3bff4; // FUN_140c3bb30, calls FUN_14090ee00 for each remote player marker
inline constexpr uintptr_t kWorldMapPlayerListCountCallRva = 0xac9df3; // FUN_140ac9de0, copies WorldMapPlayerData into UI marker list
inline constexpr uintptr_t kPlayerIconAvatarBuildCountRva = 0xa5b635; // FUN_140a5bxxx, player icon build loop cmp edi, 3
inline constexpr uintptr_t kJoinFailedMessageBoxTextRva = 0xb4c870; // FUN_140b4c870, builds MessageBoxParamDialog text id 506060/0x7b8cc
inline constexpr uintptr_t kTeamSessionDialogRva = 0xe00a20; // FUN_140e00a20, maps team-session join failure reason to Dialog text
inline constexpr uintptr_t kTeamSessionCleanupRva = 0xe00440; // FUN_140e00440, reason dialog followed by destructive session cleanup
inline constexpr uintptr_t kSystemMessageTextByIdRva = 0x7b0a20; // FUN_1407b0a20, builds System Message text by id
inline constexpr uintptr_t kDialogueTextByIdRva = 0x7ab100; // FUN_1407ab100, builds Dialogue text by id
inline constexpr uintptr_t kDialogShowTextRva = 0x86a7a0; // FUN_14086a7a0, submits/shows a Dialog text object
inline constexpr uintptr_t kSessionRecordFindForPgdRva = 0xdf0630; // FUN_140df0630, session 0xf0 record by PlayerGameData
inline constexpr uintptr_t kSessionRecordKeyToPlayerIndexRva = 0xdf0470; // FUN_140df0470, session record key -> player index
inline constexpr uintptr_t kPlayerStatusCompleteRva = 0x1feab0; // FUN_1401feab0, player reconnect status byte == 0x1f
inline constexpr uintptr_t kSessionEventBroadcastRva = 0xdf24d0; // FUN_140df24d0, broadcasts one event to all session records
inline constexpr uintptr_t kSessionEventBroadcastToTargetsRva = 0xdf2380; // FUN_140df2380, broadcasts one event to selected session records
inline constexpr uintptr_t kSessionRecordEventReadRva = 0xdf6da0; // FUN_140df6da0, reads one event payload from a session record
inline constexpr uintptr_t kPlayerStatusSerializePayloadRva = 0x20bde0; // FUN_14020bde0, PlayerGameData -> 0x1f8 status payload
inline constexpr uintptr_t kPlayerStatusPayloadValidateRva = 0x20c350; // FUN_14020c350, validates 0x1f8 status payload
inline constexpr uintptr_t kPlayerGameDataFullSerializeRva = 0x20d210; // FUN_14020d210, complete PlayerGameData -> native writer
inline constexpr uintptr_t kPlayerGameDataFullDeserializeRva = 0x20c540; // FUN_14020c540, native reader -> complete PlayerGameData
inline constexpr uintptr_t kPlayerGameDataCodecVersionRva = 0x215440; // func_0x000140215440, current PlayerGameData codec version
inline constexpr uintptr_t kPlayerGameDataPostLoadRefreshRva = 0x20ce50; // func_0x00014020ce50, first native post-load refresh
inline constexpr uintptr_t kPlayerGameDataPostLoadRebuildRva = 0x209ee0; // func_0x000140209ee0, second native post-load rebuild
inline constexpr uintptr_t kPlayerGameDataRuneAdjustRva = 0x2098c0; // FUN_1402098c0, adjusts held Rune and native totals
inline constexpr uintptr_t kPlayerGameDataLevelSetRva = 0x209e20; // FUN_140209e20, sets level and rebuilds derived stats
inline constexpr uintptr_t kNativeWriteStreamInitRva = 0x217b130; // FUN_14217b130
inline constexpr uintptr_t kNativeWriteStreamRemainingRva = 0x217b410; // func_0x00014217b410, capacity - bytes written
inline constexpr uintptr_t kNativeWriteStreamCapacityRva = 0x217b4d0; // func_0x00014217b4d0, total capacity
inline constexpr uintptr_t kNativeWriteStreamDestroyRva = 0x217b440; // FUN_14217b440
inline constexpr uintptr_t kNativeReadStreamInitRva = 0x217c640; // func_0x00014217c640
inline constexpr uintptr_t kNativeReadStreamDestroyRva = 0x217c8c0; // func_0x00014217c8c0
inline constexpr uintptr_t kPlayerGameDataIdRva = 0x20bb20; // FUN_14020bb20, PGD+0x98 field (-1..23); WorldChr lookup instead takes PGD+8
inline constexpr uintptr_t kNativePeerEventType1Rva = 0x2590d20; // FUN_142590d20, original peer-event type-1 create/find path
inline constexpr uintptr_t kNativeOperationSubmitRva = 0x2585a70; // FUN_142585a70, selects a native manager and owns operation payload
inline constexpr uintptr_t kNativeManagerTeardownRva = 0x2590830; // FUN_142590830, native manager teardown/conditional LeaveLobby
inline constexpr uintptr_t kNativeLobbyMemberEventQueueRva = 0x25906d0; // FUN_1425906d0, queues native Steam lobby callbacks before peer teardown
inline constexpr uintptr_t kNativeJoinLobbyBeginRva = 0x25921e0; // FUN_1425921e0, writes manager+0x870 and calls Steam JoinLobby
inline constexpr uintptr_t kNativeJoinLobbyResultCallbackRva = 0x2592270; // FUN_142592270, polls the Steam JoinLobby async result
inline constexpr uintptr_t kNativeJoinLobbyLeaveRva = 0x2592480; // FUN_142592480, native LeaveLobby(manager+0x870) fail-clean
// NOTE: channel 0x49 is sent via FUN_140df2380 (already hooked as
// HookSessionEventBroadcastToTargets). Do NOT install a second hook on that RVA.
// Decision fix is at FUN_140b66fb0 / FUN_140b4f560: returning friends must pass
// the matching accept-table check so dde430 natively emits type=0 (not rewrite).
// FUN_140b66fb0 is a 14-byte thunk into FUN_140b4f560 (real body).
inline constexpr uintptr_t kNativeMatchingMemberAcceptCheckRva = 0xb4f560;
inline constexpr uintptr_t kQuickMatchGuestWaitSessionRva = 0xb539f0; // FUN_140b539f0, guest route-code short-circuit boundary
inline constexpr uintptr_t kQuickMatchGuestReadyWaitRva = 0xb545a0; // FUN_140b545a0, guest native descriptor producer gate
inline constexpr uintptr_t kNativeExpeditionDescriptorReadRva = 0xdf0560; // FUN_140df0560, reads B from the live native manager
inline constexpr uintptr_t kNativePersistentDescriptorCommitRva = 0x2022d0; // FUN_1402022d0, commits the owning previous-session descriptor
inline constexpr uintptr_t kNativeDescriptorDeepCopyRva = 0x1fbf30; // FUN_1401fbf30, original descriptor value/deep-copy implementation
inline constexpr uintptr_t kNativePreviousSessionDescriptorReadyRva = 0x2021e0; // FUN_1402021e0, marks the native previous-session descriptor eligible
inline constexpr uintptr_t kQuickMatchDescriptorClearReturnRva = 0xb5def9; // FUN_140b5da90 immediately after its default descriptor commit
inline constexpr uintptr_t kGlobalDescriptorClearReturnRva = 0xc325af; // FUN_140c322e0 immediately after its default descriptor commit
inline constexpr uintptr_t kNativeBState2OwnerRva = 0x9ed090; // FUN_1409ed090, original state-2 task producer for FUN_1409ea5f0
inline constexpr uintptr_t kNativeBTeamSessionStatePtrRva = 0x3c21fc8; // DAT_143c21fc8, native B business state/record owner
inline constexpr uintptr_t kNativeBTeamSessionRecordIngressRva = 0xdffdb0; // FUN_140dffdb0, installs one live 0xd8 result into the TeamSession 0xf0 business vector
inline constexpr uintptr_t kNativeBTeamSessionRecordIngressQueuedType1CallerRva = 0xdfee30; // FUN_140dfe740 queued type-1 live-record consumer, immediately after FUN_140dffdb0
inline constexpr uintptr_t kNativePreviousSessionOwnerRva = 0xb51070; // FUN_140b51070, original previous-session saved-descriptor owner
inline constexpr uintptr_t kNativeQuickMatchFacadeCancelRva = 0xb67240; // FUN_140b67240, public persistent-facade cancel
inline constexpr uintptr_t kNativePreviousSessionLatchSetRva = 0x6b0570; // FUN_1406b0570, original direct-world/previous-session latch setter
inline constexpr uintptr_t kNativePreviousSessionAdapterRva = 0x8903d0; // FUN_1408903d0, zero-argument saved-descriptor high-level adapter
inline constexpr uintptr_t kNativeServiceEnableRva = 0xdecc10; // FUN_140decc10, original unified enable owner for all three service wrappers
inline constexpr uintptr_t kNativeServiceSignInCallbackRva = 0xf18d30; // FUN_140f18d30, original local service-config and three-wrapper activation callback
inline constexpr uintptr_t kNativeServiceReadinessRva = 0xdec9c0; // FUN_140dec9c0, original three-wrapper readiness aggregate (0 == ready)
inline constexpr uintptr_t kNativeBackendStatusBitTestRva = 0x257da80; // FUN_14257da80, backend object status-bit predicate used by FUN_140dec9c0
inline constexpr uintptr_t kNativePreviousSessionFactoryRva = 0xafc2f0; // FUN_140afc2f0, original mode-1 previous-session task factory
inline constexpr uintptr_t kNativePreviousSessionFactoryOwnerReturnRva = 0xb51663; // FUN_140b51070 immediately after its mode-1 factory call
 // FUN_140b1a3e0, mode-1 task outer state gate and submit invoke
 // vtable for the mode-1 task produced by FUN_140afc2f0
 // vtable 0x142c71aa8 +0x10
 // FUN_140df0f70, native mode-1 session request submit
 // FUN_140df6e70, first native submit eligibility gate
 // FUN_140df0f70 call to FUN_140df6e70
 // FUN_140b1a3e0 immediately after its FUN_140df0f70 call
 // FUN_1425911e0, manager +0xe0 low-task owner
 // FUN_1427bc640, alternate descriptor/resource construction
 // FUN_1427b7050, alternate provider type-1 queue insertion
 // FUN_141d7a667, alternate provider type-1 consumer
 // FUN_1427c2dc0, four-slot callback registry
 // FUN_14258e320, original asynchronous success terminal
 // FUN_14258e390, original asynchronous failure terminal
 // FUN_142595aa0, inserts the native operation into its manager queue
 // FUN_142595b30, runs the queued operation state machine
 // FUN_14258f5b0, first busy/cancel/start branch
 // FUN_140ff2cf5, dispatches manager vfunc+0xe0 low-task creation
 // FUN_14258f510, waits for manager+0x6a0 success/failure
 // FUN_142592270, consumes the original Steam lobby-call result
inline constexpr uintptr_t kNativeMasterLobbyLeaveStepRva = 0x2592480; // FUN_142592480, queued LeaveLobby(manager+0x870) step
inline constexpr uintptr_t kNativeFirstEntryMasterLeaveCallbackRva = 0x258fca0; // FUN_14258fca0, operation callback which can advance into FUN_142592480
inline constexpr uintptr_t kNativeOperationRetiredCallbackRva = 0x258d1dc; // operation+0x08 after FUN_14258fca0 schedules retirement
 // manager vfunc+0x130, stores first operation error and schedules the error callback
 // vtable 0x14326a448 +0x130
inline constexpr uintptr_t kNativeMultiplaySessionIdReadRva = 0xafe120; // FUN_140afe120, reads online-manager inner +0x78
inline constexpr uintptr_t kNativeMultiplaySessionIdCommitRva = 0xaff1e0; // FUN_140aff1e0, commits online-manager inner +0x78
inline constexpr uintptr_t kNativeTitleOnlineModeSelectorRva = 0x7b3bb0; // selector 0x39 thunk -> FUN_140f8c330
inline constexpr uintptr_t kNativeTitleOnlineModeSelectorSlotRva = 0x39aa258; // FUN_1407b3d00 selector 0x39 jump-table slot
inline constexpr uintptr_t kNativeTitleOnlineModeContextInitReturnRva = 0x886ac7; // FUN_1408865a0 immediately after selector 0x39
inline constexpr uintptr_t kNativeTitleMasterOnlineGateRva = 0x890630; // FUN_140890630, master previous-session online fact -> result 2/3
inline constexpr uintptr_t kNativeTitleMasterOnlineGateVtableSlotRva = 0x2c45e98; // vtable 0x142c45e88 +0x10
inline constexpr uintptr_t kNativeTitleParentOnlineGateRva = 0x890b30; // FUN_140890b30, parent 401824/previous-session child online fact -> result 2/3
inline constexpr uintptr_t kNativeTitleParentOnlineGateVtableSlotRva = 0x2c45ed0; // vtable 0x142c45ec0 +0x10
inline constexpr uintptr_t kNativeTitleDescriptorGateRva = 0x88f800; // FUN_14088f800, original descriptor-present MenuJob gate
inline constexpr uintptr_t kNativeTitleDescriptorGateVtableSlotRva = 0x2c46058; // descriptor gate vptr +0x10 -> invoke
inline constexpr uintptr_t kNativeTitleExistenceProviderInvokeRva = 0x88df50; // FUN_14088df50, lazy CheckSessionExistenceJob owner
inline constexpr uintptr_t kNativeTitleExistenceProviderInvokeVtableSlotRva = 0x2c45720; // vtable 0x142c45710 +0x10
inline constexpr uintptr_t kNativeTitleJob78ExistenceProviderInvokeRva = 0x88dde0; // FUN_14088dde0, job-0x78 CheckSessionExistenceJob owner
inline constexpr uintptr_t kNativeTitleJob78ExistenceProviderInvokeVtableSlotRva = 0x2c45740; // vtable 0x142c45730 +0x10
inline constexpr uintptr_t kNativeTitleCheckSessionSubmitRva = 0xb1ad10; // FUN_140b1ad10, official CheckSessionExistence provider submit
inline constexpr size_t kNativeTitleCheckSessionSubmitHookSlot = 124;
inline constexpr size_t kNativeTitlePlayerChrResultReadHookSlot = 125;
inline constexpr uintptr_t kNativeTitleSearchStartRva = 0x88f5d0; // FUN_14088f5d0, original CSMultiplayCheckPenaltyJob creator
inline constexpr uintptr_t kNativeTitleSearchStartVtableSlotRva = 0x2c460c8; // search-start vptr +0x10 -> invoke
inline constexpr uintptr_t kNativeTitleCheckPenaltyTaskVtableRva = 0x2c74de8; // CSMultiplayCheckPenaltyJob::vftable
inline constexpr uintptr_t kNativeTitleCheckPenaltySubmitRva = 0xb1c4d0; // CSMultiplayCheckPenaltyJob::execute, FNMultiplay vfunc+0x20
inline constexpr uintptr_t kNativeTitleCheckPenaltySubmitVtableSlotRva = 0x2c74e00; // CSMultiplayCheckPenaltyJob vtable +0x18
inline constexpr uintptr_t kNativeTitleCheckPenaltyPollRva = 0xb1c480; // CSMultiplayCheckPenaltyJob native result poll/callback
inline constexpr uintptr_t kNativeTitleCheckPenaltyPollVtableSlotRva = 0x2c74e08; // CSMultiplayCheckPenaltyJob vtable +0x20
inline constexpr uintptr_t kNativeTitleCheckPenaltyResultCopyRva = 0xb19e20; // FNMultiplay internal result copy
inline constexpr uintptr_t kNativeTitleCheckPenaltyResultCopyVtableSlotRva = 0x2c74e10; // CSMultiplayCheckPenaltyJob vtable +0x28
// FUN_145afaf0c, the only real ResponseCheckPenaltyMultiplayParams applier.
// The vanilla poll FUN_140b1c480 calls it (via thunk_FUN_145afaf0c) as
// thunk(*(task+0x78), response); it writes owner+0x68/+0x69 and, when
// response.duration_seconds > 0, stamps the packed deadline through
// FUN_140b1e200(owner, seconds). The previously used 0xb1da60 has no
// decompiled body and no reference anywhere in nightreign.exe.c.
inline constexpr uintptr_t kNativeTitleCheckPenaltyResultApplyRva = 0x5afaf0c;
inline constexpr uintptr_t kNativeTitlePositiveConfirmCallbackRva = 0x8909e0; // 401822 positive callback; writes the same TitleFlowContext +0x16c
inline constexpr uintptr_t kNativeTitlePositiveConfirmCallbackVtableSlotRva = 0x2c46170;
 // wrapper that enters FUN_140aa86b0
inline constexpr uintptr_t kNativeTitlePostConfirmDispatcherInvokeRva = 0xaae6b0; // wrapper that enters the state dispatcher
inline constexpr uintptr_t kNativeTitlePostConfirmDispatcherInvokeVtableSlotRva = 0x2c6c738;
inline constexpr uintptr_t kNativeTitleReconnectGraphGateRva = 0x886220; // func_0x000140886220, context+0x1bc == 0
inline constexpr uintptr_t kNativeTitleReconnectGraphGateCallerReturnRva = 0xab0bf2; // FUN_140ab0ba0 immediately after the graph-gate call
inline constexpr uintptr_t kNativeTitleReconnectTaskResultGateRva = 0x88fa80; // FUN_14088fa80, same context+0x1bc as TaskResult {2,0}/{3,0}
inline constexpr uintptr_t kNativeTitleReconnectTaskResultGateVtableSlotRva = 0x2c45840; // UNK_142c45830 +0x10, built only by thunk_FUN_146ae9d42
 // no-capture callback vtable 0x142c46240 +0x10
inline constexpr uintptr_t kNativeTitleJob78BuilderRva = 0x893f40;
inline constexpr uintptr_t kNativeTitlePlayerChrLoginFactoryInvokeRva = 0x890690; // FUN_140890690 -> FUN_140afb060, delayed job 0x5a factory
inline constexpr uintptr_t kNativeTitlePlayerChrLoginFactoryInvokeVtableSlotRva = 0x2c46448; // vtable 0x142c46438 +0x10
inline constexpr uintptr_t kNativeTitlePlayerChrLoginTaskVtableRva = 0x2c71718;
inline constexpr uintptr_t kNativeTitlePlayerChrLoginCompletionRva = 0xb431e0;
inline constexpr uintptr_t kNativeTitlePlayerChrLoginCompletionVtableSlotRva = 0x2c71738; // CSPlayerChrLoginUpdateJob vtable +0x20
inline constexpr uintptr_t kNativeTitlePlayerChrLoginResultCopyVtableSlotRva = 0x2c71740; // CSPlayerChrLoginUpdateJob vtable +0x28
inline constexpr uintptr_t kNativeTitleChaosRankTaskVtableRva = 0x2c71798;
inline constexpr uintptr_t kNativeTitleChaosRankResultCopyVtableSlotRva = 0x2c717c0; // CSChaosMatchRankInfoDownloadJob vtable +0x28
inline constexpr uintptr_t kNativeTitleChaosRateTaskVtableRva = 0x2c71818;
inline constexpr uintptr_t kNativeTitleChaosRateResultCopyVtableSlotRva = 0x2c71840; // CSChaosMatchRateDownloadJob vtable +0x28
inline constexpr uintptr_t kNativeTitlePlayerChrResultReadRva = 0x20978b0; // FUN_1420978b0, typed scalar result reader used by FUN_140b431e0
inline constexpr size_t kNativeTitleJob78BuilderHookSlot = 111;
inline constexpr size_t kNativeTitleReconnectGraphGateHookSlot = 123;
inline constexpr uintptr_t kNativeMainGameUpdateRva = 0xc3a900; // FUN_140c3a900, original main game-thread update dispatcher
inline constexpr uintptr_t kQuickMatchNotificationQueueProducerRva = 0xaff200; // FUN_140aff200, copies ResponseGetNotificationMessageParams into the original manager queues
inline constexpr uintptr_t kQuickMatchNotificationQueueProducerBodyRva = 0x77a4db; // current 1.3.3.0 thunk target, preserves RCX manager / RDX response DTO
inline constexpr uintptr_t kQuickMatchUiAttemptStartRva = 0xb67190; // FUN_140b67190, commits the UI-owned persistent QuickMatch context
inline constexpr uintptr_t kQuickMatchResultAttemptStartRva = 0xb671d0; // FUN_140b671d0, commits the candidate/result-owned QuickMatch context
inline constexpr uintptr_t kQuickMatchAlternateAttemptStartRva = 0xb66d30; // FUN_140b66d30, alternate state-3/state-7 QuickMatch completion
inline constexpr uintptr_t kNativeQuickMatchContextFinalizeRva = 0xb45530; // FUN_140b45530, cancels the active controller and maps the existing disposition to a native route
inline constexpr uintptr_t kNativeSteamLobbyManagerVtableRva = 0x326a448; // FUN_142590430 manager; vtable+0x18 reads manager+0x870
inline constexpr uintptr_t kNativeOperationExplicitPayloadCallerReturnRva = 0x258506e;
inline constexpr uintptr_t kNativePeerEventType1CallerReturnRva = 0x25927b7;
inline constexpr uint8_t kNativeTitleExistenceProviderInvokePrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x6c, 0x24, 0x20, 0x4c, 0x89,
    0x44, 0x24, 0x18, 0x56, 0x57, 0x41, 0x56, 0x48, 0x83, 0xec, 0x30};
inline constexpr uint8_t kNativeTitleCheckSessionSubmitPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b,
    0x79, 0x08, 0x48, 0x8b, 0xda};
inline constexpr uint8_t kNativeTitleJob78BuilderPrologue[] = {
    0x48, 0x89, 0x54, 0x24, 0x10, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41,
    0x55, 0x41, 0x56};
inline constexpr uint8_t kNativeTitlePlayerChrResultReadPrologue[] = {
    0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xda,
    0x48, 0x8b, 0xd1, 0x48, 0x8d, 0x4c, 0x24, 0x40};
inline constexpr uint8_t kNativeTitleReconnectGraphGatePrologue[] = {
    0x80, 0xb9, 0xbc, 0x01, 0x00, 0x00, 0x00, 0x48, 0x8b, 0xc2, 0x74, 0x04,
    0xc6, 0x02, 0x00, 0xc3, 0xc6, 0x02, 0x01, 0xc3};
inline constexpr uint8_t kNativePeerEventType1Prologue[] = {
    0x40, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56,
    0x41, 0x57, 0x48, 0x8d, 0xac, 0x24, 0xf8, 0xfe, 0xff, 0xff};
inline constexpr uint8_t kNativeSessionRecordEventReadPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x6c, 0x24, 0x10, 0x48, 0x89,
    0x74, 0x24, 0x18};
inline constexpr uint8_t kNativeEventReadAnyRecordPrologue[] = {
    0x48, 0x89, 0x6c, 0x24, 0x18, 0x56, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83,
    0xec, 0x40};
inline constexpr uint8_t kNativeEventPairReadPrologue[] = {
    0x48, 0x8b, 0x09, 0x48, 0x85, 0xc9, 0x75, 0x03, 0x33, 0xc0, 0xc3, 0xe9,
    0x20, 0x05, 0x00, 0x00};
inline constexpr uint8_t kTeamSessionCleanupPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x6c, 0x24, 0x18, 0x48, 0x89,
    0x74, 0x24, 0x20};
inline constexpr uint8_t kNativeOperationSubmitPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x6c, 0x24, 0x10, 0x48, 0x89,
    0x74, 0x24, 0x18};
inline constexpr uint8_t kNativeManagerTeardownPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x18, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b,
    0xd9, 0x48, 0x8b, 0x89, 0xb0, 0x08, 0x00, 0x00};
inline constexpr uint8_t kNativeLobbyMemberEventQueuePrologue[] = {
    0x40, 0x53, 0x41, 0x56, 0x48, 0x83, 0xec, 0x08, 0x4c, 0x8b, 0x59, 0x08,
    0x48, 0x8b, 0xda};
inline constexpr uint8_t kNativeJoinLobbyBeginPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x83,
    0x79, 0x20, 0x00};
// FUN_142592480: 16-byte window ending after full `lea rcx,[rip+disp32]`.
// Last byte of disp32 is image-local and rewritten by the trampoline relocator.
inline constexpr uint8_t kNativeJoinLobbyLeavePrologue[] = {
    0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xd9, 0x48, 0x8d, 0x0d,
    0xd8, 0xa8, 0x54, 0x01};
// FUN_140b4f560 (body of FUN_140b66fb0): 15-byte window through third mov.
inline constexpr uint8_t kNativeMatchingMemberAcceptCheckPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89,
    0x7c, 0x24, 0x18};
inline constexpr uint8_t kQuickMatchGuestWaitSessionPrologue[] = {
    0x48, 0x8b, 0xc4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x70, 0x18, 0x48,
    0x89, 0x78, 0x20};
inline constexpr uint8_t kQuickMatchGuestReadyWaitPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x55, 0x57,
    0x41, 0x54};
inline constexpr uint8_t kNativePersistentDescriptorCommitPrologue[] = {
    0x48, 0x8b, 0xd1, 0x48, 0x8b, 0x0d, 0xf6, 0x55, 0xa0, 0x03, 0x48,
    0x8b, 0x89, 0x50, 0x01, 0x00, 0x00, 0xe9, 0x4a, 0x9c, 0xff, 0xff};
inline constexpr uint8_t kNativePreviousSessionDescriptorReadyPrologue[] = {
    0x48, 0x83, 0xec, 0x28, 0xe8, 0xa7, 0x9c, 0xd8, 0x00, 0x84, 0xc0, 0x75,
    0x0e, 0x48, 0x8b, 0x05};
inline constexpr uint8_t kNativeExpeditionDescriptorReadPrologue[] = {
    0x40, 0x55, 0x57, 0x41, 0x56, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xe9,
    0xc7, 0x44, 0x24, 0x50, 0x00, 0x00, 0x00, 0x00};
inline constexpr uint8_t kNativeMainGameUpdatePrologue[] = {
    0x48, 0x8b, 0xc4, 0x48, 0x89, 0x58, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54,
    0x41, 0x55, 0x41, 0x56, 0x41, 0x57};
inline constexpr uint8_t kQuickMatchUiAttemptStartPrologue[] = {
    0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xd9, 0xe8, 0x22, 0x0b,
    0x00, 0x00, 0x84, 0xc0};
inline constexpr uint8_t kQuickMatchResultAttemptStartPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x57, 0x48, 0x83, 0xec, 0x20, 0x49, 0x8b,
    0xf8, 0x48, 0x8b, 0xd9};
inline constexpr uint8_t kQuickMatchAlternateAttemptStartPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x57, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b,
    0xfa, 0x48, 0x8b, 0xd9};
inline constexpr uint8_t kQuickMatchNotificationQueueProducerThunk[] = {
    0xe9, 0xd6, 0xb2, 0xc7, 0xff};
inline constexpr uint8_t kQuickMatchNotificationQueueProducerBodyPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x20, 0x48, 0x89, 0x4c, 0x24, 0x08, 0x55, 0x56,
    0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x48, 0x83, 0xec,
    0x30, 0x4c, 0x8b, 0xfa};
inline constexpr uint8_t kNativeQuickMatchContextFinalizePrologue[] = {
    0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xd9};
inline constexpr uint8_t kNativeBState2OwnerPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x08, 0x55, 0x56, 0x57, 0x41, 0x56, 0x41, 0x57};
inline constexpr uint8_t kNativeBTeamSessionRecordIngressPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55,
    0x41, 0x56};
inline constexpr uint8_t kNativePreviousSessionOwnerPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x18, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55,
    0x41, 0x56};
inline constexpr uint8_t kNativeQuickMatchFacadeCancelPrologue[] = {
    0x48, 0x8b, 0x01, 0x48, 0x85, 0xc0, 0x74, 0x10, 0x48, 0xc7, 0x41, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x48, 0x8b, 0xc8, 0xe9, 0x98, 0xbc, 0xfe, 0xff};
inline constexpr uint8_t kNativePreviousSessionLatchSetPrologue[] = {
    0x48, 0x8b, 0x05, 0xe1, 0x2c, 0x56, 0x03, 0x88, 0x88, 0x96, 0x0c, 0x00,
    0x00, 0xc3};
inline constexpr uint8_t kNativePreviousSessionAdapterPrologue[] = {
    0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0x1d, 0xcb, 0x42, 0x37,
    0x03};
inline constexpr uint8_t kNativeServiceEnablePrologue[] = {
    0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x0f, 0xb6,
    0xd9, 0x48, 0x8b, 0x0d, 0x30, 0x52, 0xe3, 0x02};
inline constexpr uint8_t kNativeBackendStatusBitTestPrologue[] = {
    0x48, 0x8b, 0x01, 0x8b, 0x88, 0x70, 0x0c, 0x00,
    0x00, 0x23, 0xca, 0x3b, 0xca, 0x0f, 0x94, 0xc0};
inline constexpr uint8_t kNativePreviousSessionFactoryPrologue[] = {
    0x48, 0x89, 0x5c, 0x24, 0x18, 0x48, 0x89, 0x54,
    0x24, 0x10, 0x55, 0x56, 0x57, 0x41, 0x56};
inline constexpr uint8_t kNativeMultiplaySessionIdReadPrologue[] = {
    0x48, 0x8b, 0x41, 0x10, 0x48, 0x8b, 0x40, 0x78, 0xc3};
inline constexpr uint8_t kNativeMultiplaySessionIdCommitPrologue[] = {
    0x48, 0x8b, 0x41, 0x10, 0x48, 0x89, 0x50, 0x78, 0xc3};
inline constexpr char kBhrcRoundOneSourceAnchor[] = "5fd18cc+previous-session-owner-boundary";
inline constexpr char kBhrcExpectedGameVersion[] = "1.3.3.0";
inline constexpr char kBhrcExpectedGameSha256[] =
    "9545E8732919BB6A63904CB73223641B3C4F9A67869BE5722612ACCA413E8B3A";
inline constexpr uintptr_t kPartySessionGlobalPtrRva = 0x3c046a8; // DAT_143c046a8
inline constexpr uintptr_t kOnlineSessionGlobalPtrRva = 0x3c17248; // DAT_143c17248
inline constexpr uintptr_t kMapGaitemMarkAcquiredRva = 0x57d400; // FUN_14057d400, host reward acquire validation
inline constexpr uintptr_t kRewardSpawnMapNodeRva = 0x574b50; // FUN_140574b50, creates/syncs a map reward from gaitem data
inline constexpr uintptr_t kRewardEntryAvailableRva = 0x572c10; // FUN_140572c10, reward gaitem-list availability gate
inline constexpr uintptr_t kRewardDiscardRequestRva = 0x575810; // FUN_140575810, reward UI direct discard entry
inline constexpr uintptr_t kRewardLocalDiscardApplyBodyRva = 0x57cf39; // FUN_14057cf30+0x9, after null guard
inline constexpr uintptr_t kRewardFindByIdRva = 0x582680; // FUN_140582680, map reward node lookup by id
inline constexpr uintptr_t kGameAllocatorPtrRva = 0x3c2d9c0; // DAT_143c2d9c0

inline constexpr uintptr_t kNrscLobbyMemberEventRva = 0x645b0; // FUN_1800645a0 actual entry, Steam lobby member join/leave
inline constexpr uintptr_t kNrscLobbyScanRva = 0x647b0; // FUN_1800647a0 actual entry, scan lobby and register peers
inline constexpr uintptr_t kNrscRegisterPeerRva = 0x3d920; // FUN_18003d920, fills one NRSC peer slot
inline constexpr uintptr_t kNrscLeaveLobbyDetourRva = 0x2bc70; // FUN_18002bc70, clears tracked master peers before original LeaveLobby
inline constexpr uintptr_t kNrscQuickMatchFinalizeRva = 0x2ad20; // FUN_18002ad20, forces disposition 2 then calls native finalizer
inline constexpr uintptr_t kNrscNativeEvent94FanoutRva = 0x26250; // FUN_180026250, six-player native event 0x94 fanout
inline constexpr uint8_t kNrscLobbyMemberEventPrologue[] = {
    0xf3, 0x0f, 0x1e, 0xfa, 0x55, 0x56, 0x57, 0x48, 0x81, 0xec, 0xc0, 0x00,
    0x00, 0x00};
inline constexpr uint8_t kNrscLobbyScanPrologue[] = {
    0xf3, 0x0f, 0x1e, 0xfa, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54,
    0x56, 0x57};
inline constexpr uint8_t kNrscRegisterPeerPrologue[] = {
    0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x56, 0x57, 0x53, 0x48, 0x81,
    0xec, 0x90, 0x00, 0x00, 0x00};
inline constexpr uint8_t kNrscLeaveLobbyDetourPrologue[] = {
    0xf3, 0x0f, 0x1e, 0xfa, 0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x56};
inline constexpr uint8_t kNrscQuickMatchFinalizePrologue[] = {
    0xf3, 0x0f, 0x1e, 0xfa, 0x48, 0x8b, 0x41, 0x58, 0x83, 0x7a, 0x10, 0x02,
};
inline constexpr uint8_t kNrscNativeEvent94FanoutPrologue[] = {
    0xf3, 0x0f, 0x1e, 0xfa, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54,
};
inline constexpr char kBhrcExpectedNrscSha256[] =
    "243EEC929A97B71E1E2E3B4215778B89C37D629436B8DD5403E830593D3CE24E";

inline constexpr int kBuddySlotCount = 3;
inline constexpr int32_t kVanillaBuddyNpcParamIds[kBuddySlotCount] = {137030000, 143400000, 149610000};
inline constexpr int kBuddySlotSize = 0x60;
inline constexpr uint32_t kBuddyReadyRangeStart = 0x14;
inline constexpr uint32_t kVanillaBuddyEntitySlotCapacity = 0x3c;
inline constexpr uint32_t kExpandedBuddyEntitySlotCapacity = 0x78;
inline constexpr uint32_t kVanillaRemotePlayerMarkerCount = 2;
inline constexpr uint32_t kExpandedRemotePlayerMarkerCount = 5;
inline constexpr uint32_t kVanillaRemotePlayerGameDataSlots = 2;
inline constexpr uint32_t kExpandedRemotePlayerGameDataSlots = 5;
inline constexpr uint8_t kVanillaPlayerIconAvatarCount = 3;
inline constexpr uint8_t kExpandedPlayerIconAvatarCount = 6;
inline constexpr uint32_t kNpcPlayerGameDataPoolCapacity = 0x28;
inline constexpr uint32_t kNpcPlayerGameDataFixedSlotStart = kNpcPlayerGameDataPoolCapacity;
inline constexpr uint32_t kNpcPlayerGameDataFixedOwnerCount = 6;
inline constexpr uint32_t kNpcPlayerGameDataFixedSlotCount =
    kNpcPlayerGameDataFixedOwnerCount * kBuddySlotCount;
inline constexpr uint32_t kExpandedNpcPlayerGameDataPoolCapacity =
    kNpcPlayerGameDataPoolCapacity + kNpcPlayerGameDataFixedSlotCount;
inline constexpr size_t kNetPeerRecordSize = 0xf0;
inline constexpr size_t kNetPeerEventRecordSize = 0xd8;
inline constexpr uint32_t kRevenantFixSteamControlPacketMagic = 0x43524842; // "BHRC"
inline constexpr uint16_t kRevenantFixSteamControlPacketVersion = 2;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeTestPing = 1;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeTestPong = 2;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeNativeEvent = 7;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeRecordRosterAnnounce = 11;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeExpeditionProbeRequest = 14;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeExpeditionProbeResponse = 15;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeBuddyRecall = 16;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeFullPgdChunk = 17;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeStartupHostPreflightRequest = 18;
inline constexpr uint16_t kRevenantFixSteamControlPacketTypeStartupHostPreflightResponse = 19;
inline constexpr uint32_t kBhrcFlagLocalHost = 1u << 0;
inline constexpr uint32_t kBhrcFlagHasRecordKey = 1u << 3;
inline constexpr uint32_t kBhrcFlagAuthorityObserverOnly = 1u << 9;
inline constexpr uint32_t kBhrcFlagAdmissionReserved = 1u << 10;
inline constexpr uint32_t kBhrcActiveRecordSlots = 6;
// NOTE: kBhrcInactiveRecordSlots feeds kBhrcRecordRosterSlots which sizes the
// on-wire RevenantFixRecordRosterAnnouncePacket. Bumping it (e.g. to 6) changes
// the packet size and breaks cross-end protocol compat (static_assert at the
// packet). To grow local inactive capacity to 6, the wire roster slot count
// must be decoupled from the local cache capacity first. Kept at 3 for now.
inline constexpr uint32_t kBhrcInactiveRecordSlots = 3;
inline constexpr uint32_t kBhrcRecordRosterSlots = kBhrcActiveRecordSlots + kBhrcInactiveRecordSlots;
inline constexpr uint32_t kBhrcStartupHostCandidateCapacity = kBhrcRecordRosterSlots;
inline constexpr uint16_t kBhrcRecordStateActive = 1;
inline constexpr uint16_t kBhrcRecordStateInactive = 2;
inline constexpr uint32_t kBhrcPlayerIdentitySnapshotSchemaVersion = 3;
inline constexpr uint32_t kBhrcRecordRoleUnknown = 0;
inline constexpr uint32_t kBhrcRecordRoleSelf = 1;
inline constexpr uint32_t kBhrcRecordRoleRemote = 2;
inline constexpr uint32_t kBhrcIdentityValidSteamId = 1u << 0;
inline constexpr uint32_t kBhrcIdentityValidRecordKey = 1u << 1;
inline constexpr uint32_t kBhrcIdentityValidPlayerIndex = 1u << 2;
inline constexpr uint32_t kBhrcIdentityValidRecordD4Id = 1u << 3;
inline constexpr uint32_t kBhrcIdentityValidPlayerGameDataId = 1u << 4;
inline constexpr uint32_t kBhrcIdentityValidPartyMemberHandle = 1u << 5;
inline constexpr uint32_t kBhrcIdentityValidPartyMemberSlot = 1u << 6;
inline constexpr uint32_t kBhrcIdentityEvidenceTransportActive = 1u << 0;
inline constexpr uint32_t kBhrcIdentityEvidenceNativeLiveRecord = 1u << 1;
inline constexpr uint32_t kBhrcIdentityEvidencePlayerGameData = 1u << 2;
inline constexpr uint32_t kBhrcIdentityEvidenceRecordD4MatchesPgd = 1u << 3;
inline constexpr uint32_t kBhrcIdentityEvidenceWorldChr = 1u << 4;
inline constexpr uint32_t kBhrcIdentityEvidencePartyMember = 1u << 5;
inline constexpr uint32_t kBhrcIdentityEvidenceNativeBAuthority = 1u << 6;
inline constexpr uint32_t kBhrcPlayerStatusPayloadSize = 0x1f8;
inline constexpr uint32_t kBhrcFullPgdMaxPayloadSize = 0x100000;
inline constexpr uint32_t kBhrcFullPgdMinPayloadSize = 0x1b0;
// Maximum application payload per Steam reliable message. Steam's complete
// message limit is 512 KiB; leave room for the RevenantFix control fields.
// This is a ceiling, not a fixed send size.
inline constexpr uint32_t kBhrcFullPgdChunkPayloadSize = 0x7f000;
inline constexpr uint32_t kBhrcFullPgdMaxChunks =
    (kBhrcFullPgdMaxPayloadSize + kBhrcFullPgdChunkPayloadSize - 1) /
    kBhrcFullPgdChunkPayloadSize;
inline constexpr uint32_t kBhrcFullPgdHotSnapshotCapacity = 6;
inline constexpr uint32_t kBhrcFullPgdInboundAssemblyCapacity = 6;
inline constexpr uint32_t kBhrcFullPgdPinnedRestoreCapacity = 12;
inline constexpr DWORD kBhrcFullPgdCaptureIntervalMs = 1000;
inline constexpr ULONGLONG kBhrcFullPgdPublishRetryMs = 5000;
inline constexpr ULONGLONG kBhrcFullPgdRestoreTargetStableMs = 1000;
inline constexpr ULONGLONG kBhrcFullPgdAssemblyLifetimeMs = 30000;
inline constexpr ULONGLONG kBhrcFullPgdRestoreRetryMs = 2000;
inline constexpr ULONGLONG kBhrcFullPgdRestoreWindowMs = 60000;
inline constexpr uint32_t kBhrcNativeEventMaxPayloadSize = 0x400;
inline constexpr uint32_t kBhrcNativeEvent94MaxPayloadSize = 0x400;
inline constexpr uintptr_t kBhrcNativeEvent94PairReadCallerReturnRva = 0xdfe0fd;
inline constexpr uint32_t kBhrcNativeEvent1dPayloadSize = 0x20;
inline constexpr uint32_t kBhrcNativeEvent67PayloadSize = 0x10;
inline constexpr uint32_t kBhrcNativeEventMask94 = 1u << 0;
inline constexpr uint32_t kBhrcNativeEventMask1d = 1u << 1;
inline constexpr uint32_t kBhrcNativeEventMask0c = 1u << 2;
inline constexpr uint32_t kBhrcNativeEventMask67 = 1u << 3;
inline constexpr uint32_t kBhrcHostAdmissionCapacity = 6;
inline constexpr uint32_t kBhrcInboundNativeEventSlotCapacity = 24;
inline constexpr uint32_t kBhrcNativeGenerationPayloadCapacity = 3;
inline constexpr ULONGLONG kBhrcFriendNativeAdmissionLifetimeMs = 90000;
inline constexpr ULONGLONG kBhrcStartupNativeAdmissionLifetimeMs = 300000;
inline constexpr ULONGLONG kBhrcNativeEventSlotLifetimeMs = 300000;
inline constexpr ULONGLONG kBhrcPendingNativeEventLifetimeMs = 120000;
inline constexpr ULONGLONG kBhrcDescriptorPersistRetryIntervalMs = 1000;
inline constexpr ULONGLONG kBhrcDescriptorPersistLifetimeMs = 15000;
inline constexpr uint32_t kBhrcDescriptorPersistMaxAttempts = 15;
inline constexpr uint32_t kBhrcNativeDescriptorSidecarSchemaVersion = 1;
inline constexpr size_t kBhrcNativeDescriptorFixedSize = 0x40;
inline constexpr size_t kBhrcNativeDescriptorObjectSize = 0x58;
inline constexpr size_t kBhrcNativeDescriptorMaxPayloadSize = 0x100;
inline constexpr size_t kBhrcNativeDescriptorMultiplaySessionIdOffset = 0x28;
inline constexpr DWORD kBhrcIdentityCaptureIntervalMs = 5000;
inline constexpr ULONGLONG kBhrcNativeAuthorityCandidateTimeoutMs = 10000;
inline constexpr DWORD kBhrcRecordRosterAnnounceIntervalMs = 10000;
inline constexpr DWORD kBhrcDeferredIntentLogIntervalMs = 1000;
inline constexpr DWORD kBhrcMasterJoinTimeoutMs = 15000;
inline constexpr DWORD kBhrcNativeBOwnerWaitTimeoutMs = 30000;
inline constexpr DWORD kBhrcExpeditionProbeTimeoutMs = 20000;
inline constexpr DWORD kBhrcExpeditionProbeStartingRequeryMs = 1500;
inline constexpr DWORD kBhrcStartupHostResponseTimeoutMs = 15000;
inline constexpr DWORD kBhrcStartupHostPreflightRetryMs = 500;
inline constexpr ULONGLONG kBhrcMigratedMasterLeaveLeaseMs = 30000;
inline constexpr DWORD kBhrcSteamControlFastPollMs = 100;
inline constexpr DWORD kBhrcSteamControlSlowTickMs = 1000;
inline constexpr DWORD kBhrcExpeditionMapStableMs = 1000;
inline constexpr int kRevenantFixSteamControlChannel = 77;
inline constexpr int kRevenantFixSteamControlMaxMessages = 64;
inline constexpr int kRevenantFixSteamSendReliable = 8; // k_nSteamNetworkingSend_Reliable
inline constexpr int kRevenantFixStartupP2pChannel = 78;
inline constexpr int kRevenantFixStartupP2pSendReliable = 2; // k_EP2PSendReliable
inline constexpr int kSteamP2PSessionRequestCallbackId = 1202;
inline constexpr uint32_t kRevenantFixStartupP2pMaxPacketSize = 0x1000;
inline constexpr int kSteamNetworkingIdentityTypeSteamID = 16;
inline constexpr size_t kNpcPlayerGameDataEntrySize = 0x8a0;
inline constexpr size_t kNpcPlayerGameDataEntryAlignment = 8;
inline constexpr uint8_t kVanillaBuddyReadyRangeSpan =
    static_cast<uint8_t>(kVanillaBuddyEntitySlotCapacity - kBuddyReadyRangeStart);
inline constexpr uint8_t kExpandedBuddyReadyRangeSpan =
    static_cast<uint8_t>(kExpandedBuddyEntitySlotCapacity - kBuddyReadyRangeStart);
inline constexpr size_t kBuddyEntitySlotEntrySize = 0x10;
inline constexpr int kSlotEntityLowOffset = 0x10;
inline constexpr int kSlotEntityHighOffset = 0x14;
inline constexpr int kSlotStateOffset = 0x18;
inline constexpr int kSlotPendingOffset = 0x24;
inline constexpr int kSlotFlagsOffset = 0x64;
inline constexpr int kHpComponentOwnerOffset = 0x1b8;
inline constexpr int kHpCurrentOffset = 0x140;
inline constexpr int kHpMaxOffset = 0x144;

inline constexpr uint32_t kInvalidEntityPart = 0xffffffff;
inline constexpr uint8_t kSlotStateActive = 0x00;
inline constexpr uint8_t kSlotStateDeploying = 0x0a;
inline constexpr uint8_t kSlotStateTransition = 0x0b;
inline constexpr uint8_t kSlotStateStuckRemoving = 0x14;
inline constexpr uint8_t kSlotStateStuckRemoved = 0x16;
inline constexpr uint32_t kPreventNearDeathSpEffectId = 102130;
inline constexpr uint8_t kSlotFlagsUnsummonRequest = 0x11;
inline constexpr uint8_t kSlotFlagsResummonContinuation = 0x04;
inline constexpr uint8_t kSlotFlagsPersistentDefault = 0xe0;

inline constexpr uint8_t kSlotFlagsPersistentMask = 0xe0;
inline constexpr uint8_t kSlotFlagsRecoveryGate = 0x08;
inline constexpr int kRemoteOwnerGateFallbackModeOriginal = 0;
inline constexpr int kRemoteOwnerGateFallbackModeManualAll = 2;
inline constexpr uint32_t kZeroHpFallbackRegenIntervalTicks = 60;
inline constexpr uint32_t kBuddyRegenStableTicks = 5 * 60;
inline constexpr uint32_t kDelayedSwitchCleanupTicks = 30;
inline constexpr uint32_t kDelayedSwitchCleanupMaxRetries = 3;
inline constexpr uint32_t kPendingDeployRepairTicks = 60;
inline constexpr ULONGLONG kLogFlushIntervalMs = 1000;
inline constexpr DWORD kBuddyRecallCommandLifetimeMs = 10000;
inline constexpr uint8_t kBuddyRecallReasonAutoDistance = 1;
inline constexpr size_t kPendingBuddyRecallCommandCount = 16;
inline constexpr int32_t kZeroHpFallbackRegenFullSeconds = 155;
inline constexpr uint32_t kZeroHpFallbackMaxActiveTicks = (kZeroHpFallbackRegenFullSeconds + 5) * 60;
inline constexpr int32_t kZeroHpFallbackRegenDivisor = kZeroHpFallbackRegenFullSeconds;
inline constexpr int32_t kZeroHpFallbackReleaseHpRate = 50;
inline constexpr int kPartyMemberCountOffset = 0x14;
inline constexpr int kPartyMemberNativeCount1cOffset = 0x1c;
inline constexpr int kPartyMemberNativeCount20Offset = 0x20;
inline constexpr int kPartyMemberSlotBaseOffset = 0x28;
inline constexpr int kPartyMemberSlotStride = 0x30;
inline constexpr int kObservedPartyMemberSlots = 6;
// PlayerGameData is owned by the game after FUN_1401fe590 returns. Earlier
// diagnostic builds tried to release tracked custom entries when our per-owner
// context was reset. That can invalidate data still referenced by the ChrIns
// creation/update path, so this hook is intentionally read-only.
// Runtime contexts are keyed by the game's local buddy owner id, not by the
// transient state pointer. Index 0 is a fallback for unknown owner callers;
// indices 1..6 map directly to owner 1..6. A new state for the same owner
// replaces the old context, which prevents previous runs from leaking delayed
// cleanup/regen state into later runs.
inline constexpr int kTrackedBuddyStateCount = 8;
inline constexpr int kFallbackBuddyStateContextIndex = 0;

struct EntityId {
  uint32_t low;
  uint32_t high;
};

struct SlotSnapshot {
  uint32_t entity_low;
  uint32_t entity_high;
  uint8_t state;
  uint8_t pending;
  uint8_t flags;
};

struct BuddyContext {
  SlotSnapshot slot{};
  EntityId entity{kInvalidEntityPart, kInvalidEntityPart};
  void* hp_component = nullptr;
  int32_t current_hp = 0;
  int32_t max_hp = 0;
  int hp_rate = -1;
};

using UnsummonBuddyFn = void(__fastcall*)(
    void* state,
    const EntityId* entity_id);

using ChangeBuddyStateFn = void(__fastcall*)(
    void* state,
    int buddy_slot);

using TryUseBuddyFn = uint64_t(__fastcall*)(
    void* state,
    uint32_t buddy_trigger_id);

using GetBuddyHpRateFn = int(__fastcall*)(
    void* state,
    uint32_t buddy_slot);

using BuddyUpdateFn = void(__fastcall*)(
    void* state);

using ChrGateFn = uint8_t(__fastcall*)(
    void* chr);

using DefaultPlayerGameDataFn = void*(__fastcall*)();

using NetSendCoreFn = void(__fastcall*)(
    void* net_manager,
    void* peer,
    uint32_t msg_id,
    const void* packet,
    int size);

using NativeEventReadAnyRecordFn = int(__fastcall*)(
    void* session_mgr,
    uint32_t event_id,
    void* out_payload,
    uint32_t payload_size);

using NativeEventPairReadFn = int(__fastcall*)(
    void* record_pair,
    uint8_t event_id,
    void* out_payload,
    uint32_t payload_size,
    uint8_t* out_flags);

using PartyMemberRemoveFn = uint64_t(__fastcall*)(
    void* party_member_info,
    uint64_t member_id,
    uint64_t reason);

using JoinFailedMessageBoxTextFn = void(__fastcall*)(
    int64_t* param);

using TeamSessionCleanupFn = void(__fastcall*)(
    void* state,
    uint8_t reason);

using TextByIdFn = void*(__fastcall*)(
    void* out_text,
    uint32_t text_id);

using DialogShowTextFn = void*(__fastcall*)(
    void* out_dialog,
    void* text);

using SessionRecordFindForPgdFn = void*(__fastcall*)(
    void* session_mgr,
    void* player_game_data);

using SessionRecordKeyToPlayerIndexFn = int32_t(__fastcall*)(
    void* session_mgr,
    const uint64_t* record_key);

using SessionEventBroadcastFn = void(__fastcall*)(
    void* session_mgr,
    uint32_t event_type,
    const void* payload,
    uint32_t payload_size);

using SessionEventBroadcastToTargetsFn = uint64_t(__fastcall*)(
    void* session_mgr,
    void* target_list,
    uint32_t event_type,
    const void* payload,
    uint64_t payload_size);

using SessionRecordEventReadFn = int(__fastcall*)(
    void* session_mgr,
    const void* record,
    uint32_t event_type,
    void* out_payload,
    uint32_t payload_size,
    uint8_t pending,
    uint8_t removing);

using PlayerStatusSerializePayloadFn = void(__fastcall*)(
    void* player_game_data,
    void* out_payload);

using PlayerStatusPayloadValidateFn = uint8_t(__fastcall*)(
    const void* payload);

using PlayerGameDataFullSerializeFn = uint8_t(__fastcall*)(
    void* player_game_data,
    void* writer);

using PlayerGameDataFullDeserializeFn = uint8_t(__fastcall*)(
    void* player_game_data,
    void* reader,
    uint32_t codec_version);

using PlayerGameDataCodecVersionFn = uint32_t(__fastcall*)();
using PlayerGameDataPostLoadFn = void(__fastcall*)(void* player_game_data);
using PlayerGameDataRuneAdjustFn = int32_t(__fastcall*)(
    void* player_game_data,
    int32_t delta);
using PlayerGameDataLevelSetFn = void(__fastcall*)(
    void* player_game_data,
    uint32_t level);
using NativeWriteStreamInitFn = void(__fastcall*)(
    void* stream,
    void* buffer,
    uint64_t capacity);
using NativeWriteStreamMetricFn = uint64_t(__fastcall*)(void* stream);
using NativeWriteStreamDestroyFn = void(__fastcall*)(void* stream);
using NativeReadStreamInitFn = void(__fastcall*)(
    void* stream,
    const void* buffer,
    uint64_t size);
using NativeReadStreamDestroyFn = void(__fastcall*)(void* stream);

using NativePeerEventType1Fn = void(__fastcall*)(
    void* manager,
    const uint64_t* peer_key,
    const uint64_t* comparison_key,
    uint8_t role_flag);

using NativeOperationSubmitFn = int(__fastcall*)(
    void* registry,
    uint64_t selector,
    const void* payload,
    uint32_t payload_size,
    uint32_t timeout);

using NativeManagerTeardownFn = void(__fastcall*)(
    void* manager);

using NativeLobbyMemberEventQueueFn = uint64_t(__fastcall*)(
    void* queue,
    const void* event_data);



using NativeJoinLobbyBeginFn = void(__fastcall*)(
    void* manager);

using NativeJoinLobbyLeaveFn = void(__fastcall*)(
    void* manager);

// FUN_140b4f560 / FUN_140b66fb0(matching, peer_key_ptr, arg2, arg3) → bool accept.
using NativeMatchingMemberAcceptCheckFn = uint8_t(__fastcall*)(
    void* matching_facade,
    void* peer_key_ptr,
    uint32_t arg2,
    uint32_t arg3);

using QuickMatchGuestStateFn = void(__fastcall*)(
    void* controller);

using NativeExpeditionDescriptorReadFn = void(__fastcall*)(
    void* team_session,
    void* output_bytes);

using NativeBState2OwnerFn = void(__fastcall*)(
    void* activity_root);

using NativeBTeamSessionRecordIngressFn = void(__fastcall*)(
    void* team_state,
    const void* live_record);

using NativePreviousSessionOwnerFn = void(__fastcall*)(
    void* owner,
    const void* saved_descriptor);

using NativeQuickMatchFacadeCancelFn = void(__fastcall*)(void* facade);
using NativePreviousSessionLatchSetFn = void(__fastcall*)(uint8_t enabled);
using NativePreviousSessionAdapterFn = void(__fastcall*)();
using NativeServiceEnableFn = void(__fastcall*)(uint8_t enabled);
using NativeServiceSignInCallbackFn = uint8_t(__fastcall*)(uint32_t result);
using NativeServiceReadinessFn = uint8_t(__fastcall*)();
using NativeBackendStatusBitTestFn = uint8_t(__fastcall*)(
    void* backend_status,
    uint32_t mask);
using NativePreviousSessionFactoryFn = void*(__fastcall*)(
    void* online_manager,
    void* out_task,
    const uint32_t* descriptor_fields,
    const void* descriptor_string,
    uint8_t mode);
using NativeMultiplaySessionIdReadFn = uint64_t(__fastcall*)(
    void* online_manager);
using NativeMultiplaySessionIdCommitFn = void(__fastcall*)(
    void* online_manager,
    uint64_t multiplay_session_id);
using NativeQuickMatchOwnerStateFn = uint32_t(__fastcall*)(void* owner);

using NativeTitleOnlineModeSelectorFn = uint64_t(__fastcall*)();
using NativeTitleMenuResultFn = uint64_t*(__fastcall*)(
    void* functor,
    uint64_t* out_result);
using NativeTitleExistenceProviderInvokeFn = uint64_t*(__fastcall*)(
    void* functor,
    uint64_t* out_result,
    void* frame_time);
using NativeTitleCheckSessionSubmitFn = uint64_t*(__fastcall*)(
    void* callback_functor,
    uint64_t* out_result);
using NativeTitleSearchStartFn = void(__fastcall*)(void* functor);
using NativeTitleCheckPenaltySubmitFn = void(__fastcall*)(
    void* native_task,
    void* scheduler_context);
using NativeTitleCheckPenaltyPollFn = uint64_t(__fastcall*)(
    void* native_task);
using NativeTitleCheckPenaltyResultCopyFn = uint64_t*(__fastcall*)(
    void* native_task,
    uint64_t* out_internal_result,
    const uint64_t* source_internal_result);
using NativeTitleCheckPenaltyResultApplyFn = void(__fastcall*)(
    void* title_manager,
    const void* response);
using NativeTitlePostConfirmActionFn = void(__fastcall*)(void* functor);
using NativeTitlePostConfirmGraphInvokeFn = void*(__fastcall*)(
    void* functor,
    void* out_task,
    void* arg3,
    void* arg4);
using NativeTitleJob78BuilderFn = void*(__fastcall*)(
    void* title_context_pair,
    void* out_task);
using NativeTitlePlayerChrLoginFactoryInvokeFn = void*(__fastcall*)(
    void* functor,
    void* out_task);
using NativeTitleOnlineTaskResultCopyFn = uint64_t*(__fastcall*)(
    void* native_task,
    uint64_t* out_internal_result,
    const uint64_t* source_internal_result);
using NativeTitlePlayerChrLoginCompletionFn = void(__fastcall*)(
    void* native_task);
using NativeTitlePlayerChrResultReadFn = int(__fastcall*)(
    void* result_storage,
    uint32_t* out_value);
using NativeTitleReconnectGraphGateFn = uint8_t*(__fastcall*)(
    void* title_context,
    uint8_t* out_enabled);
using NativeTitleReconnectTaskResultGateFn = uint64_t*(__fastcall*)(
    void* callback_functor,
    uint64_t* out_result);
using NativeMainGameUpdateFn = void(__fastcall*)(
    void* game,
    void* frame);

using NativeDescriptorDeepCopyFn = void(__fastcall*)(
    void* destination,
    const void* source);

using NativePreviousSessionDescriptorReadyFn = void(__fastcall*)();

using QuickMatchNotificationQueueProducerFn = void(__fastcall*)(
    void* online_manager,
    const void* response_dto);

using QuickMatchUiAttemptStartFn = void(__fastcall*)(
    void* quick_match_manager,
    void* native_descriptor);

using QuickMatchResultAttemptStartFn = void(__fastcall*)(
    void* quick_match_manager,
    void* native_descriptor,
    void* native_result);

using QuickMatchAlternateAttemptStartFn = void(__fastcall*)(
    void* quick_match_manager,
    void* native_result);

using NativeQuickMatchContextFinalizeFn = void(__fastcall*)(
    void* quick_match_context);

using PlayerGameDataByIndexFn = void*(__fastcall*)(
    int32_t player_index);

using PlayerGameDataIdFn = int32_t*(__fastcall*)(
    void* player_game_data,
    int32_t* out_game_data_id);

using NrscLobbyMemberEventFn = void(__fastcall*)(
    void* context,
    uint64_t* event_data);

using NrscLobbyScanFn = void(__fastcall*)(
    void* context,
    uint64_t* lobby_id);

using NrscRegisterPeerFn = void(__fastcall*)(
    void* steam_friends,
    void* peer_slot,
    uint64_t steam_id);

using NrscLeaveLobbyDetourFn = void(__fastcall*)(
    void* hook_context,
    void* steam_matchmaking,
    uint64_t lobby_id);

using NrscQuickMatchFinalizeFn = void(__fastcall*)(
    void* hook_context,
    void* quick_match_context);

using NrscNativeEvent94FanoutFn = void(__fastcall*)(
    void* hook_context,
    void* team_state,
    void* arg3,
    void* arg4,
    const void* serialized_vector);

using SteamApiGetHSteamUserFn = int (*)();
using SteamInternalFindOrCreateUserInterfaceFn = void* (*)(int hsteam_user, const char* version);
using SteamApiNetworkingMessagesFn = void* (*)();
using SteamApiNetworkingFn = void* (*)();
using SteamMatchmakingJoinLobbyFn = uint64_t(__fastcall*)(
    void* self,
    uint64_t lobby_id);
using SteamMatchmakingInviteUserToLobbyFn = bool(__fastcall*)(
    void* self,
    uint64_t lobby_id,
    uint64_t invitee_steam_id);
using SteamMatchmakingSetLobbyJoinableFn = bool(__fastcall*)(
    void* self,
    uint64_t lobby_id,
    bool joinable);
using SteamMatchmakingGetLobbyOwnerFn = void(__fastcall*)(
    void* self,
    uint64_t* owner_steam_id,
    uint64_t lobby_id);
using SteamMatchmakingSetLobbyOwnerFn = bool(__fastcall*)(
    void* self,
    uint64_t lobby_id,
    uint64_t new_owner_steam_id);

#pragma pack(push, 8)
struct SteamNetworkingIdentityLite {
  int type = 0;
  int size = 0;
  uint8_t data[128]{};
};

void HandleBhrcStartupP2PSessionRequest(uint64_t remote_steam_id);

class SteamCallbackBaseLite {
 public:
  SteamCallbackBaseLite() = default;
  virtual void Run(void* parameter) = 0;
  virtual void Run(void* parameter, bool io_failure, uint64_t api_call) = 0;
  virtual int GetCallbackSizeBytes() = 0;

 protected:
  uint8_t callback_flags_ = 0;
  uint8_t callback_padding_[3]{};
  int callback_id_ = 0;
};

struct SteamP2PSessionRequestLite {
  uint64_t remote_steam_id = 0;
};

class BhrcStartupP2PSessionRequestCallback final
    : public SteamCallbackBaseLite {
 public:
  void PrepareForRegistration(int callback_id) {
    callback_flags_ = 0;
    callback_id_ = callback_id;
  }

  void Run(void* parameter) override {
    if (parameter != nullptr) {
      HandleBhrcStartupP2PSessionRequest(
          static_cast<const SteamP2PSessionRequestLite*>(parameter)
              ->remote_steam_id);
    }
  }

  void Run(void* parameter, bool, uint64_t) override {
    Run(parameter);
  }

  int GetCallbackSizeBytes() override {
    return static_cast<int>(sizeof(SteamP2PSessionRequestLite));
  }
};

struct SteamNetworkingMessageLite {
  void* data = nullptr;
  int size = 0;
  uint32_t connection = 0;
  SteamNetworkingIdentityLite peer{};
  int64_t connection_user_data = 0;
  int64_t time_received = 0;
  int64_t message_number = 0;
  void(__cdecl* free_data)(SteamNetworkingMessageLite* message) = nullptr;
  void(__cdecl* release)(SteamNetworkingMessageLite* message) = nullptr;
  int channel = 0;
  int flags = 0;
  int64_t user_data = 0;
  uint16_t lane = 0;
  uint16_t pad = 0;
};
#pragma pack(pop)

using SteamNetworkingMessagesSendMessageToUserFn = int(__cdecl*)(
    void* self,
    const SteamNetworkingIdentityLite* identity_remote,
    const void* data,
    uint32_t size,
    int send_flags,
    int remote_channel);
using SteamNetworkingMessagesReceiveMessagesOnChannelFn = int(__cdecl*)(
    void* self,
    int local_channel,
    SteamNetworkingMessageLite** out_messages,
    int max_messages);
using SteamNetworkingSendP2PPacketFn = bool(__cdecl*)(
    void* self,
    uint64_t remote_steam_id,
    const void* data,
    uint32_t size,
    int send_type,
    int channel);
using SteamNetworkingIsP2PPacketAvailableFn = bool(__cdecl*)(
    void* self,
    uint32_t* message_size,
    int channel);
using SteamNetworkingReadP2PPacketFn = bool(__cdecl*)(
    void* self,
    void* destination,
    uint32_t destination_size,
    uint32_t* message_size,
    uint64_t* remote_steam_id,
    int channel);
using SteamNetworkingAcceptP2PSessionWithUserFn = bool(__cdecl*)(
    void* self,
    uint64_t remote_steam_id);
using SteamApiRegisterCallbackFn = void(__cdecl*)(
    void* callback,
    int callback_id);
using MapGaitemMarkAcquiredFn = uint8_t(__fastcall*)(
    void* map_item_man,
    uint32_t map_gaitem_id,
    uint8_t acquired);

using RewardSpawnMapNodeFn =
    void(__fastcall*)(void* map_item_man, void* one_entry_gaitem_list, uint8_t variant, int16_t flags);

using RewardEntryAvailableFn = uint64_t(__fastcall*)(
    void* gaitem_list);

using RewardDiscardRequestFn = void(__fastcall*)(
    void* map_item_man,
    uint32_t reward_id);

using RewardLocalDiscardApplyFn = void(__fastcall*)(
    void* map_item_man,
    void* reward);

using RewardFindByIdFn = void*(__fastcall*)(
    void* map_item_man,
    uint32_t reward_id);

using CreateBuddyFn = uint32_t*(__fastcall*)(
    void* state,
    uint32_t* out_entity,
    uint32_t owner_id,
    int buddy_slot,
    const void* buddy_create_data,
    char flag);

using PlayerInsControllerEligibilityFn = uint8_t(__fastcall*)(
    void* player_ins);

using CreateEntityForOwnerFn = void*(__fastcall*)(
    void* manager,
    void* create_data,
    uint8_t owner_id,
    int buddy_slot);

using HpSetCurrentFn = void(__fastcall*)(
    void* hp_component,
    int32_t current_hp,
    uint8_t arg3,
    float arg4,
    float arg5,
    uint8_t arg6);

using ApplySpEffectFn = uint8_t(__fastcall*)(
    void* entity,
    uint32_t sp_effect_id,
    uint8_t mode);

using WorldChrRegisterFn = void(__fastcall*)(
    void* world_chr_man,
    uint64_t entity_index,
    void* source,
    void* aux);

using WorldOwnerGroupRegisterFn = void(__fastcall*)(
    void* world_owner_group,
    int entity_index,
    void* source);

using WorldOwnerGroupSetEntityActiveFn = void(__fastcall*)(
    void* world_owner_group,
    const void* entity_id,
    uint8_t active);

using EntityLookupFn = void*(__fastcall*)(
    void* world_chr_man,
    const EntityId* entity_id);

using WorldChrAuthorityFn = uint8_t(__fastcall*)(
    void* world_chr_man,
    EntityId* entity_id);

using WorldChrStatusMarkDirtyFn = void(__fastcall*)(
    void* status_manager,
    EntityId* entity_id);

using WorldChrManLookupByGameDataIdFn = void*(__fastcall*)(
    void* world_chr_man,
    uint32_t entity_id);

using CurrentMapFamilyReadFn = uint32_t*(__fastcall*)(
    uint32_t* out_map_family);

using WorldMapRemoteMarkerUpdateFn = void(__fastcall*)(
    void* world_map_model,
    int marker_index,
    void* chr,
    uint8_t is_local);

using ChrIconIdFn = uint32_t*(__fastcall*)(
    void* chr,
    uint32_t* out_icon_id);

using WorldMapSetIconIdFn = void(__fastcall*)(
    void* marker,
    int icon_id);

using WorldMapSetIconTypeFn = void(__fastcall*)(
    void* marker,
    uint8_t icon_type);

using PlayerGameDataAllocFn = void*(__fastcall*)(
    void* player_game_data_manager);

using PlayerGameDataCtorFn = void*(__fastcall*)(
    void* memory,
    uint8_t is_player,
    uint32_t unk_id,
    uint64_t reserved);

using GameAllocatorAllocFn = void*(__fastcall*)(
    void* allocator,
    size_t size,
    size_t alignment);

struct InlineHook {
  void* target = nullptr;
  void* detour = nullptr;
  void* trampoline = nullptr;
  size_t patch_size = 0;
  uint8_t original[32]{};
};

struct BytePatch {
  void* target = nullptr;
  uint8_t original = 0;
  uint8_t replacement = 0;
  bool installed = false;
  const char* name = nullptr;
};

struct CodePatch {
  void* target = nullptr;
  uint8_t original[16]{};
  uint8_t replacement[16]{};
  size_t size = 0;
  bool installed = false;
  const char* name = nullptr;
};

inline HMODULE g_module = nullptr;
inline uintptr_t g_game_base = 0;
inline HANDLE g_log = INVALID_HANDLE_VALUE;
inline CRITICAL_SECTION g_log_lock{};
inline bool g_log_lock_initialized = false;
inline wchar_t g_dll_dir[MAX_PATH]{};

inline UnsummonBuddyFn g_unsummon_buddy = nullptr;
inline TryUseBuddyFn g_try_use_buddy = nullptr;
inline GetBuddyHpRateFn g_get_buddy_hp_rate = nullptr;
inline ChangeBuddyStateFn g_change_buddy_state = nullptr;
inline ChangeBuddyStateFn g_request_unsummon = nullptr;
inline BuddyUpdateFn g_buddy_module_update = nullptr;
inline BuddyUpdateFn g_buddy_update = nullptr;
inline ChrGateFn g_is_local_owner = nullptr;
inline ChrGateFn g_remote_owner_gate = nullptr;
inline DefaultPlayerGameDataFn g_default_player_game_data = nullptr;
inline MapGaitemMarkAcquiredFn g_map_gaitem_mark_acquired = nullptr;
inline RewardSpawnMapNodeFn g_reward_spawn_map_node = nullptr;
inline RewardEntryAvailableFn g_reward_entry_available = nullptr;
inline RewardDiscardRequestFn g_reward_discard_request = nullptr;
inline RewardLocalDiscardApplyFn g_reward_local_discard_apply = nullptr;
inline RewardFindByIdFn g_reward_find_by_id = nullptr;
inline BuddyUpdateFn g_buddy_death_recovery_update = nullptr;
inline CreateBuddyFn g_create_buddy = nullptr;
inline PlayerInsControllerEligibilityFn g_player_ins_controller_eligibility = nullptr;
inline CreateEntityForOwnerFn g_create_entity_for_owner = nullptr;
inline HpSetCurrentFn g_hp_set_current = nullptr;
inline ApplySpEffectFn g_apply_sp_effect = nullptr;
inline PlayerGameDataAllocFn g_player_game_data_alloc = nullptr;
inline PlayerGameDataCtorFn g_player_game_data_ctor = nullptr;
inline WorldChrRegisterFn g_world_chr_register = nullptr;
inline WorldOwnerGroupRegisterFn g_world_owner_group_register = nullptr;
inline WorldOwnerGroupSetEntityActiveFn g_world_owner_group_set_entity_active = nullptr;
inline EntityLookupFn g_entity_lookup = nullptr;
inline WorldChrAuthorityFn g_world_chr_authority = nullptr;
inline WorldChrStatusMarkDirtyFn g_world_chr_status_mark_dirty = nullptr;
inline WorldChrManLookupByGameDataIdFn g_world_chr_lookup_by_game_data_id = nullptr;
inline CurrentMapFamilyReadFn g_current_map_family_read = nullptr;
using NativeMapFamilyDecodeFn = uint32_t*(__fastcall*)(
    void* map_object, uint32_t* out);
inline NativeMapFamilyDecodeFn g_native_map_family_decode = nullptr;
inline WorldMapRemoteMarkerUpdateFn g_world_map_remote_marker_update = nullptr;
inline ChrIconIdFn g_chr_icon_id = nullptr;
inline WorldMapSetIconIdFn g_world_map_set_icon_id = nullptr;
inline WorldMapSetIconTypeFn g_world_map_set_icon_type = nullptr;
inline NetSendCoreFn g_net_send_core = nullptr;
inline NativeEventReadAnyRecordFn g_native_event_read_any_record = nullptr;
inline NativeEventPairReadFn g_native_event_pair_read = nullptr;
inline PartyMemberRemoveFn g_party_member_remove = nullptr;
inline JoinFailedMessageBoxTextFn g_join_failed_message_box_text = nullptr;
inline TeamSessionCleanupFn g_team_session_cleanup = nullptr;
inline SessionEventBroadcastFn g_session_event_broadcast = nullptr;
inline SessionEventBroadcastToTargetsFn g_session_event_broadcast_to_targets = nullptr;
inline SessionRecordEventReadFn g_session_record_event_read = nullptr;
inline PlayerStatusSerializePayloadFn g_player_status_serialize_payload = nullptr;
inline PlayerStatusPayloadValidateFn g_player_status_payload_validate = nullptr;
inline PlayerGameDataFullSerializeFn g_player_game_data_full_serialize = nullptr;
inline PlayerGameDataFullDeserializeFn g_player_game_data_full_deserialize = nullptr;
inline PlayerGameDataCodecVersionFn g_player_game_data_codec_version = nullptr;
inline PlayerGameDataPostLoadFn g_player_game_data_post_load_refresh = nullptr;
inline PlayerGameDataPostLoadFn g_player_game_data_post_load_rebuild = nullptr;
inline PlayerGameDataRuneAdjustFn g_player_game_data_rune_adjust = nullptr;
inline PlayerGameDataLevelSetFn g_player_game_data_level_set = nullptr;
inline NativeWriteStreamInitFn g_native_write_stream_init = nullptr;
inline NativeWriteStreamMetricFn g_native_write_stream_remaining = nullptr;
inline NativeWriteStreamMetricFn g_native_write_stream_capacity = nullptr;
inline NativeWriteStreamDestroyFn g_native_write_stream_destroy = nullptr;
inline NativeReadStreamInitFn g_native_read_stream_init = nullptr;
inline NativeReadStreamDestroyFn g_native_read_stream_destroy = nullptr;
inline NativePeerEventType1Fn g_native_peer_event_type1 = nullptr;
inline NativeOperationSubmitFn g_native_operation_submit = nullptr;
inline NativeManagerTeardownFn g_native_manager_teardown = nullptr;
inline NativeLobbyMemberEventQueueFn g_native_lobby_member_event_queue = nullptr;
inline NativeJoinLobbyBeginFn g_native_join_lobby_begin = nullptr;
inline NativeJoinLobbyLeaveFn g_native_join_lobby_leave = nullptr;
inline NativeMatchingMemberAcceptCheckFn g_native_matching_member_accept_check = nullptr;
inline QuickMatchGuestStateFn g_quick_match_guest_wait_session = nullptr;
inline QuickMatchGuestStateFn g_quick_match_guest_ready_wait = nullptr;
inline NativeExpeditionDescriptorReadFn g_native_expedition_descriptor_read = nullptr;
inline NativeBState2OwnerFn g_native_b_state2_owner = nullptr;
inline NativeBTeamSessionRecordIngressFn g_native_b_team_session_record_ingress = nullptr;
inline NativePreviousSessionOwnerFn g_native_previous_session_owner = nullptr;
inline NativeQuickMatchFacadeCancelFn g_native_quick_match_facade_cancel = nullptr;
inline NativePreviousSessionLatchSetFn g_native_previous_session_latch_set = nullptr;
inline NativePreviousSessionAdapterFn g_native_previous_session_adapter = nullptr;
inline NativeServiceEnableFn g_native_service_enable = nullptr;
inline NativeServiceSignInCallbackFn g_native_service_sign_in_callback = nullptr;
inline NativeServiceReadinessFn g_native_service_readiness = nullptr;
inline NativeBackendStatusBitTestFn g_native_backend_status_bit_test = nullptr;
inline NativePreviousSessionFactoryFn g_native_previous_session_factory = nullptr;
inline NativeMultiplaySessionIdReadFn g_native_multiplay_session_id_read = nullptr;
inline NativeMultiplaySessionIdCommitFn g_native_multiplay_session_id_commit = nullptr;
inline NativeTitleOnlineModeSelectorFn
    g_native_title_online_mode_selector = nullptr;
inline NativeTitleMenuResultFn g_native_title_master_online_gate = nullptr;
inline NativeTitleMenuResultFn g_native_title_parent_online_gate = nullptr;
inline NativeTitleMenuResultFn g_native_title_descriptor_gate = nullptr;
inline NativeTitleExistenceProviderInvokeFn
    g_native_title_existence_provider_invoke = nullptr;
inline NativeTitleExistenceProviderInvokeFn
    g_native_title_job78_existence_provider_invoke = nullptr;
inline NativeTitleCheckSessionSubmitFn g_native_title_check_session_submit = nullptr;
inline NativeTitleSearchStartFn g_native_title_search_start = nullptr;
inline NativeTitleCheckPenaltySubmitFn
    g_native_title_check_penalty_submit = nullptr;
inline NativeTitleCheckPenaltyPollFn g_native_title_check_penalty_poll = nullptr;
inline NativeTitleCheckPenaltyResultCopyFn
    g_native_title_check_penalty_result_copy = nullptr;
inline NativeTitleCheckPenaltyResultApplyFn
    g_native_title_check_penalty_result_apply = nullptr;
inline NativeTitlePostConfirmActionFn
    g_native_title_positive_confirm_callback = nullptr;
inline NativeTitlePostConfirmGraphInvokeFn
    g_native_title_post_confirm_dispatcher_invoke = nullptr;
inline NativeTitleJob78BuilderFn g_native_title_job78_builder = nullptr;
inline NativeTitlePlayerChrLoginFactoryInvokeFn
    g_native_title_player_chr_login_factory_invoke = nullptr;
inline NativeTitleOnlineTaskResultCopyFn
    g_native_title_online_task_result_copy = nullptr;
inline NativeTitlePlayerChrLoginCompletionFn
    g_native_title_player_chr_login_completion = nullptr;
inline NativeTitlePlayerChrResultReadFn
    g_native_title_player_chr_result_read = nullptr;
inline NativeTitleReconnectGraphGateFn
    g_native_title_reconnect_graph_gate = nullptr;
inline NativeTitleReconnectTaskResultGateFn
    g_native_title_reconnect_task_result_gate = nullptr;
inline NativeMainGameUpdateFn g_native_main_game_update = nullptr;
inline NativeDescriptorDeepCopyFn g_native_descriptor_deep_copy = nullptr;
inline NativePreviousSessionDescriptorReadyFn
    g_native_previous_session_descriptor_ready = nullptr;
inline QuickMatchNotificationQueueProducerFn
    g_quick_match_notification_queue_producer = nullptr;
inline QuickMatchUiAttemptStartFn g_quick_match_ui_attempt_start = nullptr;
inline QuickMatchResultAttemptStartFn g_quick_match_result_attempt_start = nullptr;
inline QuickMatchAlternateAttemptStartFn
    g_quick_match_alternate_attempt_start = nullptr;
inline NativeQuickMatchContextFinalizeFn
    g_native_quick_match_context_finalize_direct = nullptr;
inline NrscLobbyMemberEventFn g_nrsc_lobby_member_event = nullptr;
inline NrscLobbyScanFn g_nrsc_lobby_scan = nullptr;
inline NrscRegisterPeerFn g_nrsc_register_peer = nullptr;
inline NrscLeaveLobbyDetourFn g_nrsc_leave_lobby_detour = nullptr;
inline NrscQuickMatchFinalizeFn g_nrsc_quick_match_finalize = nullptr;
inline NrscNativeEvent94FanoutFn g_nrsc_native_event_94_fanout = nullptr;
inline HMODULE g_nrsc_module = nullptr;
inline volatile LONG g_nrsc_bridge_install_state = 0;
inline volatile LONG g_nrsc_control_thread_started = 0;
inline volatile LONG g_bhrc_returning_member_lease_hook_ready = 0;
inline volatile LONG g_bhrc_startup_title_provider_nonce = 0;
// Slots 157 and 166 own the automatic reconnect metadata fix.
inline InlineHook g_hooks[167]{};
inline BytePatch g_buddy_ready_range_patch{};
inline CodePatch g_world_map_remote_data_count_patch1{};
inline CodePatch g_world_map_remote_data_count_patch2{};
inline CodePatch g_world_map_remote_marker_update_count_patch{};
inline CodePatch g_world_map_player_list_count_patch{};
inline CodePatch g_player_icon_avatar_build_count_patch{};
inline CodePatch g_remote_player_data_write_range_patch{};
inline CodePatch g_remote_player_data_clear_range_patch{};
inline CodePatch g_remote_player_data_flag_clear_range_patch{};
inline CodePatch g_remote_player_data_get_by_index_range_patch{};
inline CodePatch g_remote_player_data_find_by_entity_range1_patch{};
inline CodePatch g_remote_player_data_find_by_entity_range2_patch{};
inline CodePatch g_remote_player_data_search_by_entity_range1_patch{};
inline CodePatch g_remote_player_data_search_by_entity_range2_patch{};
inline CodePatch g_remote_player_data_by_peer_range_patch{};
inline CodePatch g_player_indicator_refresh_valid_range_patch{};
inline CodePatch g_player_indicator_refresh_loop_range_patch{};
inline CodePatch g_player_ins_controller_authority_call_patch{};
inline CodePatch g_native_title_online_mode_selector_patch{};
inline CodePatch g_native_title_master_online_gate_vtable_patch{};
inline CodePatch g_native_title_parent_online_gate_vtable_patch{};
inline CodePatch g_native_title_descriptor_gate_vtable_patch{};
inline CodePatch g_native_title_existence_provider_invoke_vtable_patch{};
inline CodePatch g_native_title_job78_existence_provider_invoke_vtable_patch{};
inline CodePatch g_native_title_search_start_vtable_patch{};
inline CodePatch g_native_title_check_penalty_submit_vtable_patch{};
inline CodePatch g_native_title_check_penalty_poll_vtable_patch{};
inline CodePatch g_native_title_check_penalty_result_copy_vtable_patch{};
inline CodePatch g_native_title_positive_confirm_callback_vtable_patch{};
inline CodePatch g_native_title_post_confirm_dispatcher_invoke_vtable_patch{};
inline CodePatch g_native_title_reconnect_task_result_gate_vtable_patch{};
inline CodePatch g_native_title_player_chr_login_factory_invoke_vtable_patch{};
inline CodePatch g_native_title_player_chr_login_completion_vtable_patch{};
inline CodePatch g_native_title_player_chr_login_result_copy_vtable_patch{};
inline CodePatch g_native_title_chaos_rank_result_copy_vtable_patch{};
inline CodePatch g_native_title_chaos_rate_result_copy_vtable_patch{};
inline volatile LONG g_bhrc_title_post_confirm_trace_nonce = 0;
inline volatile LONG g_bhrc_title_post_confirm_trace_epoch = 0;
inline volatile LONG g_bhrc_title_post_confirm_trace_bits = 0;
alignas(8) inline volatile LONG64 g_bhrc_title_post_confirm_trace_context = 0;
inline thread_local uint32_t g_bhrc_title_post_confirm_dispatcher_depth = 0;
inline void* g_player_ins_controller_authority_relay = nullptr;


// Exact dynamic scope of the original FUN_142592480 invocation: the
// authoritative callback boundary for its LeaveLobby argument.
struct BhrcNativeJoinLobbyLeaveTls {
  uint32_t depth = 0;
  void* manager = nullptr;
  uint64_t owned_lobby = 0;
  uint32_t manager_id = 0;
  uint32_t native_state = 0xffffffffu;
};

inline thread_local BhrcNativeJoinLobbyLeaveTls
    g_bhrc_native_join_lobby_leave_tls{};


inline void* g_last_buddy_state = nullptr;

struct ActiveCreateBuddyContext {
  bool active = false;
  void* state = nullptr;
  uint32_t owner_id = 0xffffffff;
  int owner_slot = -1;
  int buddy_slot = -1;
  int entity_slot = -1;
  int32_t chara_init = -1;
  int32_t npc_param = -1;
  int32_t npc_think_param = -1;
  bool custom_candidate = false;
};

inline thread_local ActiveCreateBuddyContext g_active_create_context{};

struct ZeroHpFallbackRegenState {
  bool active = false;
  bool monitoring = false;
  bool gate_released = false;
  bool healed_above_zero = false;
  EntityId entity{kInvalidEntityPart, kInvalidEntityPart};
  uint32_t stable_since_tick = 0;
  uint32_t active_since_tick = 0;
  uint32_t last_tick = 0;
  uint32_t regen_tick_count = 0;
  int32_t observed_hp = 0;
  int32_t max_hp = 0;
  int32_t healed_budget = 0;
};

struct StuckRecoveryState {
  bool monitoring = false;
  EntityId entity{kInvalidEntityPart, kInvalidEntityPart};
  uint32_t since_tick = 0;
  int32_t first_hp = 0;
};

struct PendingDeployRepairState {
  bool monitoring = false;
  EntityId entity{kInvalidEntityPart, kInvalidEntityPart};
  uint8_t state = 0xff;
  uint8_t pending = 0xff;
  uint8_t flags = 0xff;
  uint32_t since_tick = 0;
  uint32_t last_retry_tick = 0;
  uint32_t retry_count = 0;
};

struct PostRecallPulseState {
  bool active = false;
  EntityId entity{kInvalidEntityPart, kInvalidEntityPart};
  uint32_t release_tick = 0;
};

struct BuddySlotRuntimeState {
  bool custom_buddy = false;
  bool custom_chara_init = false;
  bool custom_npc_param = false;
  bool prevent_near_death_applied = false;
  uint32_t prevent_near_death_last_failure_tick = 0;
  int32_t chara_init = -1;
  int32_t npc_param = -1;
  int32_t npc_think_param = -1;
  EntityId entity{kInvalidEntityPart, kInvalidEntityPart};
  void* hp_component = nullptr;
};

struct PendingBuddyRecallCommand {
  bool active = false;
  uint64_t sender_steam_id = 0;
  uint64_t lobby_id = 0;
  uint32_t sequence = 0;
  EntityId owner_entity{kInvalidEntityPart, kInvalidEntityPart};
  EntityId buddy_entity{kInvalidEntityPart, kInvalidEntityPart};
  int32_t buddy_slot = -1;
  uint8_t reason = 0;
  uint8_t authority_state = 0xff;
  uint8_t authority_pending = 0xff;
  uint8_t authority_flags = 0xff;
  DWORD received_ms = 0;
};

struct RecallRegenState {
  bool active = false;
  bool monitoring = false;
  EntityId entity{kInvalidEntityPart, kInvalidEntityPart};
  void* hp_component = nullptr;
  uint32_t stable_since_tick = 0;
  uint32_t last_tick = 0;
  uint32_t regen_tick_count = 0;
  int32_t observed_hp = 0;
};

struct DelayedSwitchCleanupState {
  bool active = false;
  void* state = nullptr;
  int current_slot = -1;
  EntityId current_entity{kInvalidEntityPart, kInvalidEntityPart};
  uint32_t due_tick = 0;
  uint32_t retry_count = 0;
};

struct OwnerGateSnapshot {
  EntityId owner_entity{kInvalidEntityPart, kInvalidEntityPart};
  void* owner_chr = nullptr;
  bool owner_lookup_ok = false;
  bool is_local_owner = false;
  bool remote_gate_call_ok = false;
  bool remote_gate = false;
  bool vfunc120_call_ok = false;
  bool vfunc120 = false;
  bool final_allows_buddy_update = false;
  uint32_t owner_entity_index = 0xffffffff;
  void* world_owner_group = nullptr;
  int32_t world_owner_group_capacity = -1;
  bool owner_index_in_group_range = false;
  uint64_t owner_group_slot = 0;
};

struct HookConfig {
  bool fix_reward_acquire = true;
  bool fix_reward_discard = true;
  bool fix_player_map_markers = true;
  bool fix_player_list_compaction = true;
  bool enable_nrsc_steam_control = true;
  bool enable_nrsc_matched_players_ui_suppress = true;
  int remote_owner_gate_fallback_mode = kRemoteOwnerGateFallbackModeOriginal;
  // When old-member handoff is active (WaitingOwnerIdle / FailClosedPending /
  // NativeOwnerSubmitted), hold NRSC LeaveLobby for the authorized expedition B
  // only. Master A leave is never held. Disable if it freezes session teardown.
  bool hold_authorized_expedition_b_leave = true;
  // Keep high-frequency reconnect/transport diagnostics out of normal logs.
  bool show_detailed_logs = false;
};

struct LocalRewardDiscardRequest {
  uint32_t reward_id = 0xffffffff;
  ULONGLONG tick = 0;
};

inline constexpr size_t kLocalRewardDiscardRequestSlots = 16;
inline constexpr ULONGLONG kLocalRewardDiscardRequestTtlMs = 30000;

struct NrscSteamControlRuntimeState {
  void* scan_context = nullptr;
  void* member_context = nullptr;
  void* steam_matchmaking = nullptr;
  uint64_t current_master_lobby_id = 0;
  uint64_t last_master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t last_observed_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t last_owner_steam_id = 0;
  uint64_t local_steam_id = 0;
  DWORD last_seen_ms = 0;
};

enum class BhrcExpeditionProbeTrigger : uint16_t {
  None = 0,
  StartupPreviousSession = 1,
  FriendJoin = 2,
};

enum class BhrcExpeditionHostPhase : uint16_t {
  Unknown = 0,
  NoExpedition = 1,
  WaitingHall = 2,
  StartingOrLoading = 3,
  RunningExpedition = 4,
  EndingUnavailable = 5,
};

enum class BhrcExpeditionMembership : uint16_t {
  Unknown = 0,
  ReturningSameB = 1,
  OrdinaryLobby = 3,
  Reject = 4,
};

enum class BhrcExpeditionProbeStage : uint16_t {
  Idle = 0,
  Armed = 1,
  JoinRequested = 2,
  AwaitingPeer = 3,
  RequestSent = 4,
  StartingRequery = 5,
  AuthorizedReturning = 6,
  OrdinaryLobby = 8,
  Rejected = 9,
  TimedOut = 10,
  HandedOff = 11,
  Completed = 12,
};

enum class BhrcStartupHostPreflightStage : uint16_t {
  Idle = 0,
  Pending = 1,
  RunningConfirmed = 2,
  NotRunning = 3,
  TimedOut = 4,
  Promoted = 5,
  AuthorizedLatched = 6,
  HandedOff = 7,
};

enum class BhrcFullPgdTransferPurpose : uint16_t {
  OwnerPublish = 1,
  AuthorityRelay = 2,
  RestoreDelivery = 3,
};

#pragma pack(push, 1)
struct RevenantFixSteamControlPacket {
  uint32_t magic = kRevenantFixSteamControlPacketMagic;
  uint16_t version = kRevenantFixSteamControlPacketVersion;
  uint16_t type = 0;
  uint64_t sender_steam_id = 0;
  uint64_t lobby_id = 0;
  uint64_t target_steam_id = 0;
  uint32_t flags = 0;
  uint32_t reserved = 0;
};

struct RevenantFixSteamTestPacket {
  RevenantFixSteamControlPacket header;
  uint32_t nonce = 0;
  char text[64]{};
};

struct RevenantFixExpeditionProbeRequestPacket {
  RevenantFixSteamControlPacket header;
  uint32_t request_nonce = 0;
  uint16_t trigger = 0;
  uint16_t reserved = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t record_key = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
};

struct RevenantFixExpeditionProbeResponsePacket {
  RevenantFixSteamControlPacket header;
  uint32_t request_nonce = 0;
  uint16_t trigger = 0;
  uint16_t host_phase = 0;
  uint16_t membership = 0;
  uint16_t reject_reason = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t manager_id = 0;
  uint16_t member_count = 0;
  uint16_t capacity = 0;
  uint32_t map_family = 0xffffffffu;
  uint32_t detail_flags = 0;
};

// Startup-only reachability/phase preflight.  This request is deliberately
// separate from ExpeditionProbeRequest: it is sent before joining master A,
// creates no host admission, and carries no native payload.
struct RevenantFixStartupHostPreflightRequestPacket {
  RevenantFixSteamControlPacket header;
  uint32_t request_nonce = 0;
  uint32_t reserved = 0;
  uint64_t master_lobby_id = 0;
  uint64_t persisted_expedition_lobby_id = 0;
  uint64_t record_key = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
};

struct RevenantFixStartupHostPreflightResponsePacket {
  RevenantFixSteamControlPacket header;
  uint32_t request_nonce = 0;
  uint16_t host_phase = 0;
  uint16_t reserved = 0;
  uint64_t requested_master_lobby_id = 0;
  uint64_t requested_expedition_lobby_id = 0;
  uint64_t current_master_lobby_id = 0;
  uint64_t current_expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint64_t multiplay_session_id = 0;
  uint32_t manager_id = 0;
  uint32_t detail_flags = 0;
};

struct RevenantFixBuddyRecallPacket {
  RevenantFixSteamControlPacket header;
  uint32_t sequence = 0;
  uint32_t owner_entity_low = kInvalidEntityPart;
  uint32_t owner_entity_high = kInvalidEntityPart;
  uint32_t buddy_entity_low = kInvalidEntityPart;
  uint32_t buddy_entity_high = kInvalidEntityPart;
  int32_t buddy_slot = -1;
  uint8_t reason = 0;
  uint8_t authority_state = 0xff;
  uint8_t authority_pending = 0xff;
  uint8_t authority_flags = 0xff;
};

struct RevenantFixNativeEventPacket {
  RevenantFixSteamControlPacket header;
  uint32_t request_nonce = 0;
  uint32_t event_id = 0;
  uint64_t record_key = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t client_generation = 0;
  uint32_t payload_hash32 = 0;
  uint32_t payload_size = 0;
  uint16_t chunk_index = 0;
  uint16_t chunk_count = 0;
  uint8_t payload[kBhrcNativeEventMaxPayloadSize]{};
};

// The payload is an opaque byte range emitted by FUN_14020d210. RevenantFix
// transports and generation-binds it, but never interprets or hand-writes PGD
// fields. The receiver feeds a complete, verified range to FUN_14020c540 once.
struct RevenantFixFullPgdChunkPacket {
  RevenantFixSteamControlPacket header;
  uint64_t owner_steam_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t codec_version = 0;
  uint64_t snapshot_revision = 0;
  uint32_t total_size = 0;
  uint32_t total_hash32 = 0;
  uint16_t chunk_index = 0;
  uint16_t chunk_count = 0;
  uint32_t chunk_size = 0;
  uint32_t chunk_hash32 = 0;
  uint16_t purpose = 0;
  uint16_t reserved = 0;
  uint8_t payload[kBhrcFullPgdChunkPayloadSize]{};
};

struct RevenantFixRecordRosterEntry {
  uint64_t steam_id = 0;
  uint64_t record_key = 0;
  uint32_t record_hash32 = 0;
  uint32_t identity_hash32 = 0;
  uint32_t status_payload_hash32 = 0;
  uint32_t status_payload_size = 0;
  uint32_t generation = 0;
  int32_t player_index = -1;
  int32_t game_data_id = -1;
  int32_t record_d4_id = -1;
  uint16_t state = 0;
  uint16_t record_role = 0;
};

struct RevenantFixRecordRosterAnnouncePacket {
  RevenantFixSteamControlPacket header;
  uint64_t owner_steam_id = 0;
  uint32_t generation = 0;
  uint16_t entry_count = 0;
  uint16_t reserved = 0;
  RevenantFixRecordRosterEntry entries[kBhrcRecordRosterSlots]{};
};

#pragma pack(pop)
static_assert(sizeof(RevenantFixSteamControlPacket) == 40, "RevenantFix Steam control packet size changed");
static_assert(sizeof(RevenantFixSteamTestPacket) == 108, "RevenantFix Steam test packet size changed");
static_assert(sizeof(RevenantFixExpeditionProbeRequestPacket) == 80, "RevenantFix expedition probe request packet size changed");
static_assert(sizeof(RevenantFixExpeditionProbeResponsePacket) == 92, "RevenantFix expedition probe response packet size changed");
static_assert(sizeof(RevenantFixStartupHostPreflightRequestPacket) == 80, "RevenantFix startup host preflight request packet size changed");
static_assert(sizeof(RevenantFixStartupHostPreflightResponsePacket) == 104, "RevenantFix startup host preflight response packet size changed");
static_assert(sizeof(RevenantFixBuddyRecallPacket) == 68, "RevenantFix buddy recall packet size changed");
static_assert(sizeof(RevenantFixNativeEventPacket) == 1120, "RevenantFix native event packet size changed");
static_assert(sizeof(RevenantFixFullPgdChunkPacket) < 512 * 1024,
              "RevenantFix full PGD chunk packet exceeds Steam message limit");
static_assert(sizeof(RevenantFixRecordRosterEntry) == 52, "RevenantFix record roster entry size changed");
static_assert(sizeof(RevenantFixRecordRosterAnnouncePacket) == 524, "RevenantFix record roster announce packet size changed");

// Native ResponseGetNotificationMessageParams ABI used by FUN_140aff200.
// The function deep-copies all three vectors before returning.  RevenantFix
// supplies only the second, 0x48-byte QuickMatch notification vector; the
// candidate bytes themselves are the untouched host-produced event 0x94.
struct BhrcNativeVectorView {
  const void* begin = nullptr;
  const void* end = nullptr;
  const void* capacity = nullptr;
};

struct BhrcNativeSmallString {
  char storage[16]{};
  uint64_t size = 0;
  uint64_t capacity = 15;
};

struct BhrcQuickMatchNotificationDtoEntry {
  int32_t type = 0;
  uint32_t pad04 = 0;
  int32_t opaque_id = 0;
  uint32_t pad0c = 0;
  BhrcNativeSmallString tag{};
  BhrcNativeVectorView candidate{};
};

struct BhrcQuickMatchResponseDto {
  BhrcNativeVectorView first{};
  BhrcNativeVectorView notifications{};
  BhrcNativeVectorView third{};
};

static_assert(sizeof(BhrcNativeVectorView) == 0x18,
              "native vector view ABI changed");
static_assert(sizeof(BhrcNativeSmallString) == 0x20,
              "native small string ABI changed");
static_assert(offsetof(BhrcQuickMatchNotificationDtoEntry, opaque_id) == 0x08,
              "QuickMatch notification id offset changed");
static_assert(offsetof(BhrcQuickMatchNotificationDtoEntry, tag) == 0x10,
              "QuickMatch notification string offset changed");
static_assert(offsetof(BhrcQuickMatchNotificationDtoEntry, candidate) == 0x30,
              "QuickMatch notification candidate offset changed");
static_assert(sizeof(BhrcQuickMatchNotificationDtoEntry) == 0x48,
              "QuickMatch notification entry ABI changed");
static_assert(sizeof(BhrcQuickMatchResponseDto) == 0x48,
              "QuickMatch response DTO ABI changed");

enum class BhrcRejectReason : uint16_t {
  None = 0,
  NotHost = 1,
  MissingLobby = 2,
  LobbyMismatch = 3,
  MissingSteamId = 4,
  MissingRecordKey = 5,
  NoSessionManager = 6,
  SessionNotInExpedition = 7,
  RecordNotFound = 8,
  PlayerIndexMismatch = 9,
  PlayerGameDataMissing = 10,
  RestoreContextMissing = 11,
  NativeRestoreException = 12,
  WorldChrMissing = 13,
  PartyMemberMissing = 14,
  ClientRecordPrereqMissing = 15,
  Timeout = 17,
  GameDataIdMismatch = 18,
  CachedRecordOnly = 19,
};

enum class BhrcPhase : uint16_t {
  Idle = 0,
  RequestingTransport = 1,
  PeerDiscovered = 2,
  Done = 3,
  Failed = 4,
};

struct BhrcRuntimeState {
  BhrcPhase phase = BhrcPhase::Idle;
  BhrcRejectReason last_error = BhrcRejectReason::None;
  uint32_t request_nonce = 0;
  uint32_t generation = 0;
  DWORD phase_started_ms = 0;
  uint64_t peer_steam_id = 0;
  uint64_t lobby_id = 0; // expedition lobby B
  uint64_t transport_lobby_id = 0; // NRSC master lobby A
  uint64_t record_key = 0;
  int32_t player_index = -1;
  int32_t game_data_id = -1;
  int32_t party_slot = -1;
  uint32_t record_hash32 = 0;
  bool suppressed_team_dialog_valid = false;
  bool suppressed_failure_prompt_shown = false;
  void* suppressed_team_dialog_state = nullptr;
  uint8_t suppressed_team_dialog_reason = 0;
  DWORD suppressed_team_dialog_ms = 0;
};

enum class BhrcDeferredReconnectStage : uint8_t {
  Idle = 0,
  ArmedReason4,
  CleanupExited,
  MasterJoinRequested,
  AwaitingNativeB,
  AuthorityProbeTransferred,
  Failed,
};

struct BhrcDeferredReconnectIntent {
  BhrcDeferredReconnectStage stage = BhrcDeferredReconnectStage::Idle;
  uint64_t serial = 0;
  DWORD armed_ms = 0;
  DWORD cleanup_exited_ms = 0;
  DWORD last_log_ms = 0;
  DWORD master_join_requested_ms = 0;
  DWORD native_b_wait_started_ms = 0;
  uint32_t master_join_attempts = 0;
  uint64_t master_join_call = 0;
  DWORD cleanup_thread_id = 0;
  void* cleanup_state_cookie = nullptr;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  uint32_t authority_probe_nonce = 0;
  uint32_t authority_client_generation = 0;
  uint32_t authority_identity_hash32 = 0;
};

struct BhrcReason4CleanupTls {
  uint32_t depth = 0;
  uint64_t intent_serial = 0;
};

struct BhrcPersistentSessionDescriptorSnapshot {
  void* game_data_manager = nullptr;
  void* descriptor = nullptr;
  uint64_t descriptor_begin = 0;
  uint64_t descriptor_end = 0;
  uint64_t descriptor_capacity = 0;
  uint64_t descriptor_size = UINT64_MAX;
  uint64_t expedition_lobby_id = 0;
  uint8_t descriptor_flags[3]{0xff, 0xff, 0xff};
  bool descriptor_bounds_valid = false;
  bool expedition_lobby_valid = false;
  bool read_fault = false;
};

struct BhrcNativeManagerRegistryLookup {
  void* nr_manager = nullptr;
  void* provider = nullptr;
  uint64_t provider_vtable = 0;
  uint32_t provider_count = 0xffffffffu;
  void* manager = nullptr;
  bool read_fault = false;
};

enum class BhrcTitleProviderScopePhase : uint8_t {
  None = 0,
  StartupPreConfirm = 1,
  PostConfirmJob78 = 2,
};

struct BhrcStartupTitleProviderScopeTls {
  uint32_t depth = 0;
  BhrcTitleProviderScopePhase phase = BhrcTitleProviderScopePhase::None;
  uint32_t preflight_nonce = 0;
  uint32_t formal_probe_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t manager_id = 0;
  uint32_t descriptor_hash32 = 0;
  bool result_committed = false;
};

struct BhrcStartupTitleCheckPenaltyBridge {
  void* native_task = nullptr;
  void* title_manager = nullptr;
  // CSMultiplayCheckPenaltyJob +0x80 is the scalar multiplay session id the
  // native ctor FUN_140b1ce80 copied by value, never a descriptor pointer.
  uint64_t multiplay_session_id = 0;
  uint32_t preflight_nonce = 0;
  uint32_t formal_probe_nonce = 0;
  uint64_t host_steam_id = 0;
  bool authorized = false;
};

struct BhrcNativeCheckPenaltyResponse {
  uint8_t previous_session_available = 0;
  uint8_t secondary_flag = 0;
  uint8_t reserved[6]{};
  int64_t duration_seconds = 0;
};

static_assert(
    sizeof(BhrcNativeCheckPenaltyResponse) == 0x10,
    "native check-penalty response ABI changed");

struct BhrcTitlePlayerChrLoginCompletionBridge {
  void* native_task = nullptr;
  uint32_t preflight_nonce = 0;
  uint32_t formal_probe_nonce = 0;
  uint64_t host_steam_id = 0;
  uint32_t local_character_value = 0;
  bool authorized = false;
};

struct BhrcTitlePlayerChrResultReadTls {
  uint32_t depth = 0;
  void* expected_storage = nullptr;
  uint32_t local_character_value = 0;
  bool read_succeeded = false;
  bool substituted = false;
};

struct BhrcNativeDescriptorPersistenceState {
  bool pending = false;
  bool identity_specified = false;
  uint64_t request_generation = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  int32_t player_index = -1;
  int32_t game_data_id = -1;
  int32_t party_slot = -1;
  uint32_t record_role = kBhrcRecordRoleUnknown;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint64_t last_persisted_master_lobby_id = 0;
  uint64_t last_persisted_lobby_id = 0;
  uint64_t last_persisted_host_steam_id = 0;
  uint64_t last_persisted_record_key = 0;
  int32_t last_persisted_player_index = -1;
  int32_t last_persisted_game_data_id = -1;
  int32_t last_persisted_party_slot = -1;
  uint32_t last_persisted_record_role = kBhrcRecordRoleUnknown;
  uint32_t last_persisted_generation = 0;
  uint32_t last_persisted_identity_hash32 = 0;
  ULONGLONG queued_tick_ms = 0;
  ULONGLONG last_attempt_tick_ms = 0;
  uint32_t attempt_count = 0;
  uint32_t restore_attempted_nonce = 0;
  bool restore_succeeded = false;
};

// Exact value representation used by the game's own descriptor serializer:
// the first 0x40 bytes are persisted verbatim, followed by the bounded byte
// vector whose native object stores begin/end/capacity at +0x40/+0x48/+0x50.
// This contains no record, provider, result, PlayerGameData or native pointer.
struct BhrcNativeDescriptorSidecar {
  bool valid = false;
  uint32_t schema_version = 0;
  uint32_t payload_size = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint8_t descriptor_flags[3]{};
  uint8_t fixed_bytes[kBhrcNativeDescriptorFixedSize]{};
  uint8_t payload[kBhrcNativeDescriptorMaxPayloadSize]{};
  uint32_t snapshot_hash32 = 0;
};

enum class BhrcOldMemberHandoffStage : uint8_t {
  Idle = 0,
  AwaitingActiveAuthority,
  WaitingOwnerIdle,
  FailClosedPending,
  NativeOwnerSubmitted,
  Failed,
};

struct BhrcExistingWorldTeardownSnapshot {
  void* session_manager = nullptr;
  void* team_state = nullptr;
  uint32_t session_state = UINT32_MAX;
  uint32_t session_records = UINT32_MAX;
  uint32_t team_records = UINT32_MAX;
  uint64_t team_handle = UINT64_MAX;
  bool session_state_admits_mode1 = false;
  bool retired = false;
};

struct BhrcOldMemberHandoffState {
  BhrcOldMemberHandoffStage stage = BhrcOldMemberHandoffStage::Idle;
  uint32_t observer_nonce = 0;
  uint32_t active_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint32_t manager_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  uint64_t authority_commit_seq = 0;
  ULONGLONG held_tick_ms = 0;
  ULONGLONG stage_tick_ms = 0;
  ULONGLONG last_log_tick_ms = 0;
  bool cancel_called = false;
  bool latch_set = false;
};

// This is a synchronous, thread-local authorization envelope around the
// original FUN_1408903d0 -> FUN_140b51070 -> FUN_140afc2f0 call stack.  It
// carries scalar authority only; no native manager/provider/result pointer is
// retained after the call returns.
struct BhrcOldMemberBackendGateTls {
  bool active = false;
  bool startup_title_submission = false;
  void* online_manager = nullptr;
  void* expected_owner = nullptr;
  const void* expected_descriptor = nullptr;
  uint32_t observer_nonce = 0;
  uint32_t active_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint32_t manager_id = 0;
  uint32_t descriptor_hash32 = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t call_count = 0;
  uint32_t caller_mask = 0;
  uint32_t original_ready_mask = 0;
  uint32_t bridged_mask = 0;
  uint32_t rejected_call_count = 0;
  uint32_t factory_call_count = 0;
  uint32_t factory_exact_call_count = 0;
  uintptr_t factory_caller_rva = 0;
  uint32_t factory_manager_state_before = UINT32_MAX;
  uint32_t factory_manager_state_after = UINT32_MAX;
  bool factory_mode1 = false;
  bool factory_returned = false;
  bool factory_output_nonnull = false;
  bool factory_manager_state_bridged = false;
  bool factory_manager_state_restored = false;
};

enum class BhrcOldMemberMatchingFenceKind : uint8_t {
  None = 0,
  PendingClassification,
  AuthorizedReturning,
  FailClosed,
};

// Network-admission ownership only.  This fence deliberately contains no
// native object pointer: it closes the asynchronous gap between a committed
// NRSC FriendJoin and the host's classification, then remains sticky for an
// authorized/fail-closed old-member generation even if its probe nonce or peer
// transport is rearmed.  Only an exact host-confirmed OrdinaryLobby response,
// a different live A/host/local tuple, or verified world completion releases
// it.
struct BhrcOldMemberMatchingFence {
  bool valid = false;
  BhrcOldMemberMatchingFenceKind kind =
      BhrcOldMemberMatchingFenceKind::None;
  uint32_t source_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint32_t manager_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  uint64_t authority_commit_seq = 0;
  ULONGLONG published_tick_ms = 0;
};

struct BhrcPersistentQuickMatchFacadeSnapshot {
  void* singleton = nullptr;
  void* facade = nullptr;
  void* owner = nullptr;
  void* active_alias = nullptr;
  void* native_task = nullptr;
  uint64_t owner_vtable = 0;
  uint64_t state_query = 0;
  uint32_t owner_state = 0xffffffffu;
  bool facade_pointer_read = false;
  bool owner_pointer_read = false;
  bool active_alias_read = false;
  bool owner_state_valid = false;
  bool read_fault = false;
};

// The native manager reuses one persistent controller/context allocation for
// multiple attempts.  Pointer equality is therefore insufficient: every
// successful manager+0x10 activation receives a monotonically increasing
// scalar serial which is the only value retained by the network attempt.
enum class BhrcQuickMatchAttemptOrigin : uint8_t {
  None = 0,
  Ui = 1,
  Result = 2,
};

struct BhrcQuickMatchAttemptEpoch {
  bool active = false;
  bool running_expedition_authorized = false;
  BhrcQuickMatchAttemptOrigin origin = BhrcQuickMatchAttemptOrigin::None;
  uint64_t serial = 0;
  ULONGLONG started_tick_ms = 0;
  void* manager = nullptr;
  void* controller = nullptr;
  void* context = nullptr;
};

// FUN_1800647a0 validates the incoming lobby before committing it to NRSC
// state+0x120/+0x128 at function exit.  Keep that source-verified identity
// only long enough to bridge the callback window before the state commit.
// It is scoped to one native UI attempt serial and never writes NRSC state.
struct BhrcPendingFriendLobbyTarget {
  bool valid = false;
  uint64_t attempt_serial = 0;
  uint64_t master_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t local_steam_id = 0;
  ULONGLONG observed_tick_ms = 0;
  void* scan_context = nullptr;
};

struct BhrcQuickMatchBinding {
  bool valid = false;
  void* manager = nullptr;
  void* controller = nullptr;
  void* context = nullptr;
  uint32_t context_type = 0;
  uint32_t completion_disposition = 0xffffffffu;
  uint8_t route_code = 0xff;
  uint8_t controller_busy = 0xff;
};

struct BhrcQuickMatchFinalizerGate {
  bool held = false;
  bool running_authorized = false;
  uint64_t attempt_serial = 0;
  ULONGLONG held_tick_ms = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint64_t master_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t local_steam_id = 0;
  uint64_t record_key = 0;
  void* hook_context = nullptr;
  void* native_manager = nullptr;
  void* native_controller = nullptr;
  void* native_context = nullptr;
};

struct BhrcNativeExpeditionAuthorityState {
  bool valid = false;
  uint64_t commit_seq = 0;
  ULONGLONG committed_tick_ms = 0;
  uint64_t lobby_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t local_steam_id = 0;
  void* native_manager = nullptr;
  uint32_t manager_id = 0;
};

struct BhrcMigratedMasterLeaveLease {
  bool valid = false;
  bool consumed = false;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t previous_owner_steam_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t authority_commit_seq = 0;
  void* native_b_manager = nullptr;
  uint32_t manager_id = 0;
  ULONGLONG armed_tick_ms = 0;
};

struct BhrcExpeditionProbeAttempt {
  BhrcExpeditionProbeStage stage = BhrcExpeditionProbeStage::Idle;
  BhrcExpeditionProbeTrigger trigger = BhrcExpeditionProbeTrigger::None;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  DWORD armed_ms = 0;
  DWORD last_action_ms = 0;
  DWORD last_log_ms = 0;
  uint32_t send_count = 0;
  uint32_t native_event_sent_mask = 0;
  uint32_t native_event_consumed_mask = 0;
  bool native_owner_ready = false;
  // Starts at the verified native previous-session owner, not packet receipt.
  // Nonce-only transport retries retain this clock and its terminal outcome.
  uint64_t full_pgd_restore_started_tick_ms = 0;
  bool full_pgd_restore_expired = false;
  bool full_pgd_restore_finished = false;
  bool restoring_player_metadata = false;
  bool native_attempt_running_authorized = false;
  bool native_attempt_transport_lost = false;
  bool master_transport_preserved = false;
  // FUN_140dfe740 aggregates one membership result per remote source record.
  // The host's formal admission authorizes the attempt, while this bounded
  // set preserves the original per-source one-shot consumption semantics.
  bool authority_observer_only = false;
  bool previous_session_owner_only = false;
  uint64_t quick_match_attempt_serial = 0;
  uint64_t join_call = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  BhrcExpeditionHostPhase host_phase = BhrcExpeditionHostPhase::Unknown;
  BhrcExpeditionMembership membership = BhrcExpeditionMembership::Unknown;
  uint16_t reject_reason = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t manager_id = 0;
  uint32_t map_family = 0xffffffffu;
};

struct BhrcStartupHostPreflightAttempt {
  BhrcStartupHostPreflightStage stage =
      BhrcStartupHostPreflightStage::Idle;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  DWORD armed_ms = 0;
  DWORD native_wait_started_ms = 0;
  DWORD last_send_ms = 0;
  uint32_t send_count = 0;
  uint64_t master_lobby_id = 0;
  uint64_t persisted_expedition_lobby_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t record_key = 0;
  uint32_t host_candidate_count = 0;
  uint64_t host_candidates[kBhrcStartupHostCandidateCapacity]{};
  BhrcExpeditionHostPhase host_phase =
      BhrcExpeditionHostPhase::Unknown;
  uint64_t response_expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint64_t multiplay_session_id = 0;
  uint32_t manager_id = 0;
};

struct BhrcHostAdmissionAttempt {
  bool valid = false;
  uint64_t reservation_serial = 0;
  uint64_t admitted_tick_ms = 0;
  uint64_t expires_tick_ms = 0;
  uint64_t steam_id = 0;
  uint64_t record_key = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t manager_id = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
  uint32_t sent_event_mask = 0;
  uint32_t consumed_event_mask = 0;
  uint64_t last_full_pgd_send_tick_ms = 0;
  uint32_t full_pgd_codec_version = 0;
  uint64_t full_pgd_snapshot_revision = 0;
  uint32_t full_pgd_payload_size = 0;
  uint32_t full_pgd_snapshot_hash32 = 0;
  bool full_pgd_restore_sent = false;
  BhrcExpeditionProbeTrigger trigger = BhrcExpeditionProbeTrigger::None;
  BhrcExpeditionMembership membership = BhrcExpeditionMembership::Unknown;
};

// A host response may only advertise a running-expedition admission after a
// bounded scalar slot has been reserved for that exact request.  Keeping the
// previous scalar value lets a failed Steam send roll the reservation back
// without destroying an already-authorized retry.
struct BhrcHostAdmissionReservation {
  bool valid = false;
  bool replaced = false;
  uint32_t slot_index = 0;
  BhrcHostAdmissionAttempt previous{};
  BhrcHostAdmissionAttempt admitted{};
};

struct BhrcInboundNativeEventSlot {
  bool valid = false;
  bool consumed = false;
  uint64_t reservation_id = 0;
  uint64_t received_tick_ms = 0;
  uint64_t sender_steam_id = 0;
  uint64_t target_steam_id = 0;
  uint64_t source_record_key = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t event_id = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  uint8_t payload[kBhrcNativeEventMaxPayloadSize]{};
};

struct BhrcNativeGenerationPayload {
  bool valid = false;
  uint64_t captured_tick_ms = 0;
  uint64_t source_record_key = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t event_id = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  uint8_t payload[kBhrcNativeEventMaxPayloadSize]{};
};

// NRSC broadcasts the original 0x94 candidate before RevenantFix can commit the
// strict native B authority tuple. Keep only that original serialized event
// long enough to bind it to the subsequently committed A/B generation. This
// is transport state, never a provider, result, record, or player-data cache.
struct BhrcPendingNativeEvent94Payload {
  bool valid = false;
  uint64_t captured_tick_ms = 0;
  uint64_t candidate_seq = 0;
  uint64_t master_lobby_id = 0;
  uint64_t observed_expedition_lobby_id = 0;
  uint64_t local_steam_id = 0;
  void* team_state = nullptr;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  uint8_t payload[kBhrcNativeEventMaxPayloadSize]{};
};

// Every admitted guest receives the original B 0x94 through an authenticated
// transport slot before the native state-7 reader necessarily consumes it.
// Retain those exact bytes only for a possible Steam-owner migration of the
// same live A/B generation. This is transport payload state, never a native
// provider, result, record, or player-data object.
struct BhrcMigratableNativeEvent94Payload {
  bool valid = false;
  bool transport_authenticated = false;
  uint64_t captured_tick_ms = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t local_steam_id = 0;
  uint64_t sender_steam_id = 0;
  uint64_t source_record_key = 0;
  uint64_t packet_authority_commit_seq = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  uint8_t payload[kBhrcNativeEventMaxPayloadSize]{};
};

// The host emits the one-shot native 0x1d ready snapshot from the same
// original owner which first exposes the B authority candidate. The strict B
// commit follows asynchronously, so retain only the exact serialized bytes
// and the already-observed native manager ownership until that commit lands.
struct BhrcPendingNativeEvent1dPayload {
  bool valid = false;
  uint64_t captured_tick_ms = 0;
  uint64_t owner_evidence_seq = 0;
  uint64_t master_lobby_id = 0;
  uint64_t observed_expedition_lobby_id = 0;
  uint64_t local_steam_id = 0;
  uint64_t source_record_key = 0;
  void* session_mgr = nullptr;
  void* native_manager = nullptr;
  uint32_t manager_id = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  uint8_t payload[kBhrcNativeEventMaxPayloadSize]{};
};

// The host can naturally broadcast the one-shot 0x67 WorldChr snapshot before
// the strict B authority tuple is committed. Preserve only those original
// bytes and their already-observed native owner identity until the matching
// authority commit; the original 0x67 consumer still owns every world write.
struct BhrcPendingNativeEvent67Payload {
  bool valid = false;
  uint64_t captured_tick_ms = 0;
  uint64_t owner_evidence_seq = 0;
  uint64_t master_lobby_id = 0;
  uint64_t observed_expedition_lobby_id = 0;
  uint64_t local_steam_id = 0;
  uint64_t source_record_key = 0;
  void* session_mgr = nullptr;
  void* native_manager = nullptr;
  uint32_t manager_id = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  uint8_t payload[kBhrcNativeEventMaxPayloadSize]{};
};

struct BhrcExpeditionWorldPhaseState {
  uint64_t registration_seq = 0;
  ULONGLONG last_registration_tick_ms = 0;
  void* last_registered_world_chr_man = nullptr;
  uint64_t authority_commit_seq = 0;
  uint64_t commit_registration_seq = 0;
  void* commit_world_chr_man = nullptr;
  uint64_t local_base_registration_seq = 0;
  uint64_t commit_local_base_registration_seq = 0;
  ULONGLONG last_local_base_registration_tick_ms = 0;
  void* last_local_base_world_chr_man = nullptr;
  uint64_t last_local_base_authority_seq = 0;
  uint32_t commit_map_family = 0xffffffffu;
  uint32_t stable_map_family = 0xffffffffu;
  ULONGLONG stable_map_since_ms = 0;
  void* observed_world_chr_man = nullptr;
  void* local_player_game_data = nullptr;
  void* local_world_chr = nullptr;
  struct {
    bool valid = false;
    uint64_t local_steam_id = 0;
    uint64_t host_steam_id = 0;
    uint64_t master_lobby_id = 0;
    uint64_t expedition_lobby_id = 0;
    uint64_t record_key = 0;
    uint64_t authority_commit_seq = 0;
    uint64_t baseline_local_base_registration_seq = 0;
    uint32_t request_nonce = 0;
    uint32_t client_generation = 0;
    uint32_t identity_hash32 = 0;
    uint32_t source_authority_manager_id = 0;
    ULONGLONG owner_started_tick_ms = 0;
  } pending_returning_owner;
};

struct BhrcExpeditionHostPhaseSnapshot {
  BhrcExpeditionHostPhase phase = BhrcExpeditionHostPhase::Unknown;
  BhrcNativeExpeditionAuthorityState authority{};
  uint64_t registration_seq = 0;
  uint64_t commit_registration_seq = 0;
  void* world_chr_man = nullptr;
  void* local_player_game_data = nullptr;
  void* local_world_chr = nullptr;
  uint32_t commit_map_family = 0xffffffffu;
  uint32_t current_map_family = 0xffffffffu;
  bool authority_live = false;
  bool post_commit_registration = false;
  bool world_stable = false;
  bool map_changed = false;
  bool map_stable = false;
};

struct BhrcNativeExpeditionAuthorityCandidate {
  bool valid = false;
  bool authorized_returning_producer = false;
  uint64_t candidate_seq = 0;
  ULONGLONG observed_tick_ms = 0;
  uintptr_t caller_rva = 0;
  bool guest_producer = false;
  bool host_producer = false;
  void* team_session = nullptr;
  void* native_manager = nullptr;
  uint32_t manager_id = 0;
  uint32_t source_authority_manager_id = 0;
  uint64_t lobby_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t authority_commit_seq = 0;
  uint64_t record_key = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t identity_hash32 = 0;
};

struct BhrcNativeExpeditionOwnerEvidence {
  bool submit_valid = false;
  bool join_valid = false;
  bool d1_valid = false;
  uint64_t evidence_seq = 0;
  ULONGLONG observed_tick_ms = 0;
  void* native_manager = nullptr;
  uint32_t manager_id = 0;
  uint32_t operation_token = 0;
  uint64_t lobby_id = 0;
};

struct BhrcCommittedExpeditionDescriptorSnapshot {
  void* game_data_manager = nullptr;
  void* descriptor = nullptr;
  uint64_t vector_begin = 0;
  uint64_t vector_end = 0;
  uint64_t vector_capacity = 0;
  uint64_t lobby_id = 0;
  uint8_t flags[3]{};
  bool valid = false;
};

struct PersistedReconnectIdentity {
  uint64_t record_key = 0;
  int32_t player_index = -1;
  int32_t game_data_id = -1;
  int32_t party_slot = -1;
  uint32_t record_hash32 = 0;
  uint32_t generation = 0;
  uint64_t host_steam_id = 0;
  uint64_t lobby_id = 0;
  uint64_t master_lobby_id = 0;
  uint32_t record_role = kBhrcRecordRoleUnknown;
};

struct SessionRecordBounds {
  const uint8_t* begin = nullptr;
  const uint8_t* end = nullptr;
  uint32_t count = 0;
  uint64_t bytes = 0;
};

enum class BhrcIdentityMembershipState : uint8_t {
  Unknown = 0,
  NativeLive = 1,
  Inactive = 2,
};

// Stable scalar evidence for one player identity. This deliberately contains
// no PlayerGameData, PartyMemberInfo, WorldChr, provider, wrapper, or result
// pointers: all of those are process-local native objects and must be rebuilt
// and owned by the original game/NRSC path.
struct BhrcPlayerIdentitySnapshot {
  uint32_t schema_version = kBhrcPlayerIdentitySnapshotSchemaVersion;
  uint32_t valid_mask = 0;
  uint32_t evidence_mask = 0;
  uint32_t snapshot_revision = 0;
  uint32_t expedition_generation = 0;
  uint32_t record_role = kBhrcRecordRoleUnknown;
  BhrcIdentityMembershipState membership_state = BhrcIdentityMembershipState::Unknown;
  uint8_t reserved0[3]{};
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t steam_id = 0;
  uint64_t record_key = 0;
  int32_t player_index = -1;
  int32_t record_d4_id = -1;
  // Legacy transport field from 20bb20 / PGD+98, not the native PGD+8 ID.
  int32_t player_game_data_id = -1;
  int32_t party_member_slot = -1;
  uint64_t party_member_handle = 0;
  uint32_t party_member_hash32 = 0;
  uint32_t identity_hash32 = 0;
  uint32_t status_payload_hash32 = 0;
  uint32_t status_payload_size = 0;
  DWORD observed_ms = 0;
};

struct BhrcDistributedCacheAuthorityReceipt {
  uint64_t authority_commit_seq = 0;
  uint64_t owner_steam_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint32_t last_roster_generation = 0;
};

// Full PlayerGameData is deliberately a separate, process-lifetime hot store.
// It is not part of active[6]/inactive[3], never owns native pointers and is
// never serialized to INI. Those record caches remain identity maps only.
struct BhrcFullPgdHotSnapshot {
  bool valid = false;
  bool publish_pending = false;
  uint64_t updated_tick_ms = 0;
  uint64_t last_publish_tick_ms = 0;
  uint64_t owner_steam_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t codec_version = 0;
  uint64_t snapshot_revision = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  uint8_t payload[kBhrcFullPgdMaxPayloadSize]{};
};

struct BhrcFullPgdInboundAssembly {
  bool valid = false;
  uint64_t received_tick_ms = 0;
  uint64_t sender_steam_id = 0;
  uint64_t target_steam_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t codec_version = 0;
  uint64_t snapshot_revision = 0;
  uint32_t total_size = 0;
  uint32_t total_hash32 = 0;
  uint16_t chunk_count = 0;
  uint16_t purpose = 0;
  uint64_t received_chunk_mask = 0;
  uint8_t payload[kBhrcFullPgdMaxPayloadSize]{};
};

struct BhrcFullPgdRestoreTicket {
  bool valid = false;
  bool applying = false;
  bool applied = false;
  bool final_applied = false;
  bool failed = false;
  bool cancel_pending = false;
  uint64_t received_tick_ms = 0;
  uint64_t owner_steam_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  // manager ids are process-local.  The probe carries the host's authority
  // manager id, while native_manager/local_manager_id belong to this client.
  uint32_t source_authority_manager_id = 0;
  uint32_t local_manager_id = 0;
  void* native_manager = nullptr;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  uint32_t codec_version = 0;
  uint64_t snapshot_revision = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
  void* ready_player_game_data = nullptr;
  int32_t ready_record_d4_id = -1;
  uint64_t ready_since_tick_ms = 0;
  void* applied_player_game_data = nullptr;
  uint64_t applied_tick_ms = 0;
  void* final_player_game_data = nullptr;
  uint64_t final_applied_tick_ms = 0;
};

enum class BhrcFullPgdRestoreGateStage : uint8_t {
  Idle = 0,
  AwaitingSeed = 1,
  SeedApplied = 2,
  FinalApplied = 3,
  Completed = 4,
};

struct BhrcFullPgdRestoreGate {
  bool valid = false;
  BhrcFullPgdRestoreGateStage stage =
      BhrcFullPgdRestoreGateStage::Idle;
  uint64_t local_steam_id = 0;
  uint64_t host_steam_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t source_authority_manager_id = 0;
  uint32_t local_manager_id = 0;
  void* native_manager = nullptr;
  void* final_world_chr_man = nullptr;
  void* final_player_game_data = nullptr;
  uint64_t final_local_base_registration_seq = 0;
  uint64_t updated_tick_ms = 0;
};

struct BhrcFullPgdAuthorityReceipt {
  bool valid = false;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t authority_commit_seq = 0;
};

struct BhrcFullPgdSnapshotKey {
  uint64_t owner_steam_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t codec_version = 0;
  uint64_t snapshot_revision = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
};

struct BhrcFullPgdPinnedRestore {
  bool valid = false;
  uint64_t reservation_serial = 0;
  uint64_t steam_id = 0;
  uint32_t request_nonce = 0;
  uint32_t client_generation = 0;
  BhrcFullPgdSnapshotKey key{};
  uint8_t* payload = nullptr;
};

struct BhrcSteamRecordCacheEntry {
  bool valid = false;
  bool inactive = false;
  uint64_t steam_id = 0;
  uint64_t lobby_id = 0;
  uint64_t record_key = 0;
  int32_t player_index = -1;
  int32_t game_data_id = -1;
  int32_t party_slot = -1;
  uint32_t record_hash32 = 0;
  uint32_t event_record_hash32 = 0;
  uint32_t native_record_hash32 = 0;
  uint32_t status_payload_hash32 = 0;
  uint32_t generation = 0;
  DWORD last_seen_ms = 0;
  DWORD inactive_since_ms = 0;
  bool status_payload_metadata_valid = false;
  // party_slot is the current packet-layout field for record+0xd4 and must
  // never be treated as the physical PartyMemberInfo ordinal.
  // identity.party_member_slot is the real NRSC-expanded 0..5 slot when
  // native evidence can resolve it.
  BhrcPlayerIdentitySnapshot identity{};
};

struct RevenantFixSteamPeerTestState {
  uint64_t steam_id = 0;
  uint32_t last_nonce = 0;
  DWORD sent_ms = 0;
  bool ping_sent = false;
  bool pong_received = false;
};

struct BuddyStateRuntimeContext {
  void* state = nullptr;
  bool owner_known = false;
  uint32_t owner_id = 0xffffffff;
  int owner_slot = -1;
  bool revenant_buddy_owner = false;
  ZeroHpFallbackRegenState zero_hp_fallback_regen[kBuddySlotCount]{};
  StuckRecoveryState stuck_recovery[kBuddySlotCount]{};
  PendingDeployRepairState pending_deploy_repair[kBuddySlotCount]{};
  PostRecallPulseState post_recall_pulse[kBuddySlotCount]{};
  BuddySlotRuntimeState buddy_slot_runtime[kBuddySlotCount]{};
  RecallRegenState recall_regen[kBuddySlotCount]{};
  DelayedSwitchCleanupState delayed_switch_cleanup{};
  bool defer_switch_unsummon_this_try = false;
  int defer_switch_unsummon_slot = -1;
};

inline HookConfig g_config{};
inline LocalRewardDiscardRequest g_local_reward_discard_requests[kLocalRewardDiscardRequestSlots]{};
inline uint32_t g_local_reward_discard_request_cursor = 0;
inline thread_local uint32_t g_reward_discard_request_depth = 0;
inline thread_local uint32_t g_reward_local_discard_apply_count = 0;
inline NrscSteamControlRuntimeState g_nrsc_steam_control_state{};
inline RevenantFixSteamPeerTestState g_revenantfix_steam_test_states[8]{};
inline BhrcRuntimeState g_bhrc{};
inline BhrcDeferredReconnectIntent g_bhrc_deferred_reconnect{};
inline SRWLOCK g_bhrc_deferred_reconnect_lock = SRWLOCK_INIT;
alignas(8) inline volatile LONG64 g_bhrc_deferred_reconnect_serial = 0;
inline thread_local BhrcReason4CleanupTls g_bhrc_reason4_cleanup_tls{};
inline thread_local BhrcStartupTitleProviderScopeTls
    g_bhrc_startup_title_provider_scope_tls{};
inline thread_local uint32_t g_bhrc_old_member_adapter_submission_depth = 0;
inline thread_local void* g_bhrc_old_member_expected_owner = nullptr;
inline thread_local const void* g_bhrc_old_member_expected_descriptor = nullptr;
inline thread_local bool g_bhrc_old_member_owner_original_returned = false;
inline thread_local BhrcOldMemberBackendGateTls g_bhrc_old_member_backend_gate_tls{};
inline SRWLOCK g_bhrc_quick_match_attempt_lock = SRWLOCK_INIT;
// Serial publication, candidate consumption and finalizer completion share
// this lifecycle lock.  The original attempt-start hooks take it exclusively;
// a state-3 consume holds it shared; a terminal native call takes it
// exclusively.  This closes the persistent-context ABA window without
// copying or mutating a native object.
inline SRWLOCK g_bhrc_quick_match_lifecycle_lock = SRWLOCK_INIT;
inline BhrcQuickMatchAttemptEpoch g_bhrc_quick_match_attempt{};
alignas(8) inline volatile LONG64 g_bhrc_quick_match_attempt_counter = 0;
inline volatile LONG g_bhrc_active_friend_handoff_ready = 0;
// Successful installation is only armed. Ready is published by the next
// clean original QuickMatch UI or candidate/result attempt entry, so a call
// already executing before its entry was patched can never be adopted halfway
// through its generation.
inline volatile LONG g_bhrc_active_friend_handoff_armed = 0;
// Installation publishes three cooperating native hooks as one logical
// adapter.  A QuickMatch start that enters after the first entry is patched
// but before the adapter is published must leave the hooks in pass-through
// mode for this process; otherwise its later result could be mistaken for a
// generation observed by RevenantFix.
inline volatile LONG g_bhrc_active_friend_handoff_installing = 0;
inline volatile LONG g_bhrc_active_friend_handoff_install_activity = 0;
inline SRWLOCK g_bhrc_pending_friend_lobby_lock = SRWLOCK_INIT;
inline BhrcPendingFriendLobbyTarget g_bhrc_pending_friend_lobby{};
inline SRWLOCK g_bhrc_quick_match_finalizer_gate_lock = SRWLOCK_INIT;
inline BhrcQuickMatchFinalizerGate g_bhrc_quick_match_finalizer_gate{};
inline SRWLOCK g_bhrc_native_expedition_authority_lock = SRWLOCK_INIT;
inline BhrcNativeExpeditionAuthorityState g_bhrc_native_expedition_authority{};
inline BhrcNativeExpeditionAuthorityCandidate g_bhrc_native_expedition_authority_candidate{};
inline SRWLOCK g_bhrc_migrated_master_leave_lease_lock = SRWLOCK_INIT;
inline BhrcMigratedMasterLeaveLease g_bhrc_migrated_master_leave_lease{};
inline BhrcNativeExpeditionOwnerEvidence g_bhrc_native_expedition_owner_evidence{};
alignas(8) inline volatile LONG64 g_bhrc_native_expedition_authority_counter = 0;
inline volatile LONG g_bhrc_functional_reconnect_hooks_ready = 0;
inline SRWLOCK g_bhrc_expedition_probe_lock = SRWLOCK_INIT;
inline BhrcExpeditionProbeAttempt g_bhrc_expedition_probe{};
inline SRWLOCK g_bhrc_startup_host_preflight_lock = SRWLOCK_INIT;
inline BhrcStartupHostPreflightAttempt g_bhrc_startup_host_preflight{};
inline SRWLOCK g_bhrc_startup_title_check_penalty_lock = SRWLOCK_INIT;
inline BhrcStartupTitleCheckPenaltyBridge
    g_bhrc_startup_title_check_penalty{};
inline SRWLOCK g_bhrc_title_player_chr_login_lock = SRWLOCK_INIT;
inline BhrcTitlePlayerChrLoginCompletionBridge
    g_bhrc_title_player_chr_login{};
alignas(8) inline volatile LONG64
    g_bhrc_title_player_chr_login_task = 0;
inline thread_local BhrcTitlePlayerChrResultReadTls
    g_bhrc_title_player_chr_result_read_tls{};
inline SRWLOCK g_bhrc_host_admission_lock = SRWLOCK_INIT;
inline BhrcHostAdmissionAttempt
    g_bhrc_host_admission_attempts[kBhrcHostAdmissionCapacity]{};
inline uint64_t g_bhrc_host_admission_reservation_counter = 0;
inline SRWLOCK g_bhrc_inbound_native_event_lock = SRWLOCK_INIT;
inline BhrcInboundNativeEventSlot
    g_bhrc_inbound_native_event_slots[kBhrcInboundNativeEventSlotCapacity]{};
inline SRWLOCK g_bhrc_native_generation_payload_lock = SRWLOCK_INIT;
inline BhrcNativeGenerationPayload
    g_bhrc_native_generation_payloads[kBhrcNativeGenerationPayloadCapacity]{};
inline SRWLOCK g_bhrc_pending_native_event_94_lock = SRWLOCK_INIT;
inline BhrcPendingNativeEvent94Payload g_bhrc_pending_native_event_94{};
inline SRWLOCK g_bhrc_migratable_native_event_94_lock = SRWLOCK_INIT;
inline BhrcMigratableNativeEvent94Payload g_bhrc_migratable_native_event_94{};
inline SRWLOCK g_bhrc_pending_native_event_1d_lock = SRWLOCK_INIT;
inline BhrcPendingNativeEvent1dPayload g_bhrc_pending_native_event_1d{};
inline SRWLOCK g_bhrc_pending_native_event_67_lock = SRWLOCK_INIT;
inline BhrcPendingNativeEvent67Payload g_bhrc_pending_native_event_67{};
inline SRWLOCK g_bhrc_expedition_world_phase_lock = SRWLOCK_INIT;
inline BhrcExpeditionWorldPhaseState g_bhrc_expedition_world_phase{};
inline SRWLOCK g_bhrc_native_descriptor_persistence_lock = SRWLOCK_INIT;
inline BhrcNativeDescriptorPersistenceState g_bhrc_native_descriptor_persistence{};
inline SRWLOCK g_bhrc_state_file_lock = SRWLOCK_INIT;
inline SRWLOCK g_bhrc_old_member_handoff_lock = SRWLOCK_INIT;
inline BhrcOldMemberHandoffState g_bhrc_old_member_handoff{};
inline volatile LONG g_bhrc_old_member_handoff_ready = 0;
inline volatile LONG g_bhrc_startup_service_callback_nonce = 0;
// Published only after the NRSC ingress, every competing QuickMatch edge and
// the main-thread previous-session scheduler are installed as one complete
// route. The adapter preflight alone is not permission to arm an attempt.
inline volatile LONG g_bhrc_old_member_route_ready = 0;
inline SRWLOCK g_bhrc_old_member_matching_fence_lock = SRWLOCK_INIT;
inline BhrcOldMemberMatchingFence g_bhrc_old_member_matching_fence{};
inline SRWLOCK g_bhrc_native_descriptor_lifecycle_lock = SRWLOCK_INIT;
inline bool g_bhrc_native_descriptor_lifecycle_initialized = false;
inline BhrcPersistentSessionDescriptorSnapshot
    g_bhrc_native_descriptor_lifecycle_last{};
inline volatile LONG g_bhrc_native_descriptor_retention_hook_ready = 0;
inline BhrcSteamRecordCacheEntry g_bhrc_active_record_cache[kBhrcActiveRecordSlots]{};
inline BhrcSteamRecordCacheEntry g_bhrc_inactive_record_cache[kBhrcInactiveRecordSlots]{};
struct BhrcTemporaryFullPgdProfilerState {
  LARGE_INTEGER frequency{};
  uint64_t total_ticks = 0;
  uint32_t sample_count = 0;
};
inline BhrcTemporaryFullPgdProfilerState g_bhrc_temporary_full_pgd_profiler{};
inline SRWLOCK g_bhrc_full_pgd_hot_lock = SRWLOCK_INIT;
inline BhrcFullPgdHotSnapshot
    g_bhrc_full_pgd_hot_snapshots[kBhrcFullPgdHotSnapshotCapacity]{};
inline SRWLOCK g_bhrc_full_pgd_assembly_lock = SRWLOCK_INIT;
inline BhrcFullPgdInboundAssembly
    g_bhrc_full_pgd_inbound_assemblies[kBhrcFullPgdInboundAssemblyCapacity]{};
inline SRWLOCK g_bhrc_full_pgd_restore_lock = SRWLOCK_INIT;
inline BhrcFullPgdRestoreTicket g_bhrc_full_pgd_restore_ticket{};
alignas(16) inline uint8_t
    g_bhrc_full_pgd_restore_payload[kBhrcFullPgdMaxPayloadSize]{};
inline BhrcFullPgdRestoreGate g_bhrc_full_pgd_restore_gate{};
inline SRWLOCK g_bhrc_full_pgd_pinned_restore_lock = SRWLOCK_INIT;
inline BhrcFullPgdPinnedRestore
    g_bhrc_full_pgd_pinned_restores[kBhrcFullPgdPinnedRestoreCapacity]{};
inline SRWLOCK g_bhrc_full_pgd_authority_receipt_lock = SRWLOCK_INIT;
inline BhrcFullPgdAuthorityReceipt g_bhrc_full_pgd_authority_receipt{};
alignas(16) inline uint8_t g_bhrc_full_pgd_writer_stream[0x100]{};
alignas(16) inline uint8_t g_bhrc_full_pgd_reader_stream[0x100]{};
alignas(16) inline uint8_t g_bhrc_full_pgd_capture_scratch[kBhrcFullPgdMaxPayloadSize]{};
inline DWORD g_bhrc_full_pgd_last_capture_ms = 0;
inline INIT_ONCE g_bhrc_record_cache_lock_once = INIT_ONCE_STATIC_INIT;
inline CRITICAL_SECTION g_bhrc_record_cache_lock{};
inline SRWLOCK g_bhrc_distributed_cache_authority_lock = SRWLOCK_INIT;
inline BhrcDistributedCacheAuthorityReceipt g_bhrc_distributed_cache_authority_receipt{};
inline uint32_t g_bhrc_record_cache_generation = 0;
inline DWORD g_last_record_roster_announce_ms = 0;
inline DWORD g_last_record_roster_skip_log_ms = 0;

BOOL CALLBACK InitializeBhrcRecordCacheLock(
    PINIT_ONCE, PVOID, PVOID*);


class BhrcRecordCacheGuard {
 public:
  BhrcRecordCacheGuard() {
    if (InitOnceExecuteOnce(
            &g_bhrc_record_cache_lock_once,
            InitializeBhrcRecordCacheLock,
            nullptr,
            nullptr)) {
      EnterCriticalSection(&g_bhrc_record_cache_lock);
      locked_ = true;
    }
  }

  ~BhrcRecordCacheGuard() {
    if (locked_) {
      LeaveCriticalSection(&g_bhrc_record_cache_lock);
    }
  }

  BhrcRecordCacheGuard(const BhrcRecordCacheGuard&) = delete;
  BhrcRecordCacheGuard& operator=(const BhrcRecordCacheGuard&) = delete;

 private:
  bool locked_ = false;
};
inline DWORD g_last_reconnect_identity_capture_ms = 0;
inline void* g_steam_networking_messages = nullptr;
inline SteamNetworkingMessagesSendMessageToUserFn g_steam_messages_send = nullptr;
inline SteamNetworkingMessagesReceiveMessagesOnChannelFn g_steam_messages_receive = nullptr;
inline bool g_steam_networking_messages_resolved_logged = false;
inline DWORD g_steam_networking_messages_last_failure_log_ms = 0;
inline void* g_bhrc_startup_p2p = nullptr;
inline SteamNetworkingSendP2PPacketFn g_bhrc_startup_p2p_send = nullptr;
inline SteamNetworkingIsP2PPacketAvailableFn
    g_bhrc_startup_p2p_is_available = nullptr;
inline SteamNetworkingReadP2PPacketFn g_bhrc_startup_p2p_read = nullptr;
inline SteamNetworkingAcceptP2PSessionWithUserFn
    g_bhrc_startup_p2p_accept = nullptr;
inline SteamApiRegisterCallbackFn g_bhrc_steam_api_register_callback = nullptr;
inline BhrcStartupP2PSessionRequestCallback
    g_bhrc_startup_p2p_session_request_callback{};
inline volatile LONG g_bhrc_startup_p2p_callback_state = 0;
inline bool g_bhrc_startup_p2p_resolved_logged = false;
inline DWORD g_bhrc_startup_p2p_last_failure_log_ms = 0;
inline uint64_t g_known_nrsc_peer_steam_ids[8]{};
inline uint32_t g_known_nrsc_peer_cursor = 0;
inline uint64_t g_active_nrsc_peer_steam_ids[8]{};
inline SRWLOCK g_active_nrsc_peer_lock = SRWLOCK_INIT;
inline PendingBuddyRecallCommand g_pending_buddy_recall_commands[kPendingBuddyRecallCommandCount]{};
inline SRWLOCK g_pending_buddy_recall_lock = SRWLOCK_INIT;
inline volatile LONG g_buddy_recall_packet_sequence = 0;

inline BuddyStateRuntimeContext g_buddy_state_contexts[kTrackedBuddyStateCount]{};
inline thread_local BuddyStateRuntimeContext* g_active_buddy_state_context = nullptr;
inline uint32_t g_module_update_tick = 0;
inline bool g_buddy_entity_slot_capacity_logged = false;

struct PlayerGameDataTableExpansion {
  void* manager = nullptr;
  uint64_t original_table = 0;
  uint64_t expanded_table = 0;
  uint32_t capacity = 0;
};

inline PlayerGameDataTableExpansion g_player_game_data_table_expansions[8]{};

struct RemotePlayerGameDataExpansion {
  void* manager = nullptr;
  uint64_t original_entries = 0;
  uint64_t expanded_block = 0;
  uint64_t expanded_entries = 0;
  uint64_t original_flags_a = 0;
  uint64_t expanded_flags_a = 0;
  uint64_t original_flags_b = 0;
  uint64_t expanded_flags_b = 0;
  uint32_t capacity = 0;
};

inline RemotePlayerGameDataExpansion g_remote_player_game_data_expansion{};

void Log(const char* format, ...);
void LogWarn(const char* format, ...);
struct BhrcNativeJoinOwnershipSnapshot;
void* GetPlayerGameDataManager();
void ObserveBhrcExpeditionWorldRegistration(
    void* world_chr_man,
    uint64_t entity_index,
    void* source,
    void* aux);
void MarkBhrcExpeditionProbeHandedOff(const char* reason);
void AdvanceBhrcExpeditionProbe();
BhrcExpeditionProbeAttempt SnapshotBhrcExpeditionProbe();
BhrcQuickMatchBinding ReadBhrcQuickMatchBinding(
    void* expected_manager = nullptr);
uint32_t ReadBhrcQuickMatchControllerStep(void* controller);
const char* BhrcQuickMatchStepHandlerName(uint32_t step);
bool IsBhrcActiveFriendHandoffReady();
BhrcQuickMatchAttemptEpoch SnapshotBhrcQuickMatchAttemptEpoch();
BhrcOldMemberHandoffState SnapshotBhrcOldMemberHandoff();
void RebindBhrcOldMemberHandoffNonce(
    const BhrcExpeditionProbeAttempt& before,
    const BhrcExpeditionProbeAttempt& after,
    const char* reason);
const char* BhrcOldMemberHandoffStageName(
    BhrcOldMemberHandoffStage stage);
bool BhrcProbeForbidsGenericState7Event94(
    const BhrcExpeditionProbeAttempt& probe);
void BindBhrcFriendProbeToCurrentQuickMatchAttempt(const char* reason);
bool BhrcQuickMatchEpochMatchesBinding(
    const BhrcQuickMatchAttemptEpoch& epoch,
    const BhrcQuickMatchBinding& binding);
bool BhrcQuickMatchEpochMatchesPersistentIdentity(
    const BhrcQuickMatchAttemptEpoch& epoch,
    void* expected_manager);
NativeQuickMatchContextFinalizeFn ResolveBhrcSavedNativeQuickMatchFinalizer(
    void* hook_context);
void ClearBhrcInboundSlotsForProbeAttempt(
    const BhrcExpeditionProbeAttempt& probe);
bool BhrcExpeditionProbeOwnsReconnect();
bool IsBhrcDeferredIntentExactForProbe(
    const BhrcDeferredReconnectIntent& intent,
    const BhrcExpeditionProbeAttempt& probe);
bool RetireBhrcDeferredAuthorityTransfer(
    const BhrcExpeditionProbeAttempt& probe,
    bool completed,
    const char* reason);
void RebindBhrcDeferredAuthorityTransfer(
    const BhrcExpeditionProbeAttempt& before,
    const BhrcExpeditionProbeAttempt& after,
    const char* reason);
bool IsBhrcAuthorizedClientProbe(const BhrcExpeditionProbeAttempt& probe);
BhrcExpeditionHostPhaseSnapshot EvaluateBhrcExpeditionHostPhase();
BhrcNativeExpeditionAuthorityState SnapshotNativeExpeditionAuthority();
BhrcNativeExpeditionOwnerEvidence SnapshotNativeExpeditionOwnerEvidence();
void* ReadGameGlobalPtr(uintptr_t rva);
bool HasBhrcReturningMemberRetentionCredential(
    uint64_t steam_id,
    uint64_t record_key,
    const BhrcNativeExpeditionAuthorityState& authority);
void EnsureBhrcReturningMemberRetentionForAdmission(
    uint64_t steam_id,
    uint64_t record_key,
    const BhrcNativeExpeditionAuthorityState& authority);
bool ReserveBhrcHostAdmissionAttempt(
    uint64_t sender,
    const RevenantFixExpeditionProbeRequestPacket& packet,
    const BhrcExpeditionHostPhaseSnapshot& phase,
    BhrcExpeditionMembership membership,
    BhrcHostAdmissionReservation* reservation);
bool CommitBhrcHostAdmissionAttempt(
    const BhrcHostAdmissionReservation& reservation,
    const BhrcExpeditionHostPhaseSnapshot& phase);
void RollbackBhrcHostAdmissionAttempt(
    const BhrcHostAdmissionReservation& reservation,
    const char* reason);
void SendCachedBhrcGenerationPayloadsToPeer(uint64_t steam_id);
BhrcNativeJoinOwnershipSnapshot ReadBhrcNativeJoinOwnershipSnapshot(void* manager);
BhrcNativeManagerRegistryLookup ReadBhrcNativeManagerRegistry(
    void* team_session,
    uint32_t manager_id);
BhrcCommittedExpeditionDescriptorSnapshot ReadCommittedNativeExpeditionDescriptor();
BhrcPersistentSessionDescriptorSnapshot ReadBhrcPersistentSessionDescriptorSnapshot(
    const void* descriptor_override);
BhrcPersistentQuickMatchFacadeSnapshot
ReadBhrcPersistentQuickMatchFacadeSnapshot();
void ObserveBhrcPersistentSessionDescriptorLifecycle(const char* edge);
uint32_t ReadLocalMapObjectFamilyRaw();
uint32_t ReadLocalMapObjectFamily();
void QueueBhrcNativeDescriptorPersistence(
    const BhrcPersistentSessionDescriptorSnapshot& descriptor,
    const char* source);
void QueueBhrcNativeDescriptorPersistenceForIdentity(
    const PersistedReconnectIdentity& identity,
    const char* source);
void ProcessBhrcNativeDescriptorPersistence();
bool LoadBhrcNativeDescriptorSidecar(BhrcNativeDescriptorSidecar* out);
bool PersistBhrcNativeDescriptorSidecar(
    BhrcNativeDescriptorSidecar sidecar,
    const PersistedReconnectIdentity& identity,
    const char* source);
uint32_t ComputeBhrcNativeDescriptorSidecarHash(
    const BhrcNativeDescriptorSidecar& sidecar);
bool CaptureBhrcNativeDescriptorSidecarValue(
    const BhrcPersistentSessionDescriptorSnapshot& descriptor,
    BhrcNativeDescriptorSidecar* out);
void RetireBhrcStartupTitleProviderGraph(
    uint32_t expected_nonce,
    const char* reason);
bool InstallBhrcOldMemberHandoffAdapter();
bool TryRestoreBhrcAuthorizedNativeDescriptor();
bool TryBeginBhrcAuthorizedOldMemberHandoff();
void ProcessBhrcAuthorizedOldMemberHandoff();
bool CommitBhrcNativeDescriptorOriginal(const void* source);
void __fastcall HookNativePersistentDescriptorCommit(const void* source);
bool InstallBhrcNativeDescriptorRetentionHook();
const char* DescribeDiagnosticCallerRva(uintptr_t caller_rva);
int LogSehException(const char* where, EXCEPTION_POINTERS* info);
uint32_t BhrcFnv1a32(const void* data, size_t size);
bool IsLocalNrscHost();
bool IsBhrcOldMemberRouteReady();
uint8_t __fastcall HookNativeMatchingMemberAcceptCheck(
    void* matching_facade,
    void* peer_key_ptr,
    uint32_t arg2,
    uint32_t arg3);
uint64_t ResolveBhrcHostLiveSourceRecordKey();
uint64_t ResolveBhrcLocalLiveSourceRecordKey(const char* reason);
void CaptureBhrcHostNativeEvent(
    uint32_t event_id,
    uint64_t source_record_key,
    const void* payload,
    uint32_t payload_size,
    const char* reason);
void CaptureBhrcPendingHostNativeEvent94(
    void* team_state,
    const void* payload,
    uint32_t payload_size,
    const char* reason);
void PromoteBhrcPendingHostNativeEvent94();
void CaptureBhrcMigratableNativeEvent94(
    const void* payload,
    uint32_t payload_size,
    const char* reason);
bool PromoteBhrcMigratableNativeEvent94ForAuthority(
    const BhrcNativeExpeditionAuthorityState& authority);
void CaptureBhrcPendingHostNativeEvent1d(
    void* session_mgr,
    const void* payload,
    uint32_t payload_size,
    const char* reason);
void PromoteBhrcPendingHostNativeEvent1d();
void CaptureBhrcPendingHostNativeEvent67(
    void* session_mgr,
    const void* payload,
    uint32_t payload_size,
    const char* reason);
void PromoteBhrcPendingHostNativeEvent67();
bool ConsumeBhrcInboundNativeEvent(
    uint32_t event_id,
    uint64_t source_record_key,
    void* out_payload,
    uint32_t payload_capacity,
    uint32_t* out_payload_size,
    const char* reason);
bool IsBhrcReturningSameBEvent1dAuthorityValid(
    const BhrcExpeditionProbeAttempt& probe,
    uint64_t source_record_key);
bool TryReserveBhrcReturningSameBEvent1d(
    void* session_mgr,
    uint64_t source_record_key,
    void* out_payload,
    uint32_t payload_capacity,
    uint32_t* out_payload_size);
uint64_t ValidateBhrcTeamSessionEventRecord(const void* record);
void ClearBhrcNativeAdmissionForPeer(uint64_t steam_id, const char* reason);
void ClearBhrcNativeAdmissionState(const char* reason);
void AdvanceBhrcNativeAdmissionTransport();
void MarkBhrcFullPgdSnapshotsForPublish(const char* reason);
void ClearBhrcFullPgdTransportForPeer(uint64_t steam_id, const char* reason);
void ClearBhrcFullPgdAdmissionTransport(const char* reason);
void AdvanceBhrcFullPgdTransport();
void CaptureBhrcLocalFullPgdMainThread();
bool StoreBhrcFullPgdHotSnapshot(
    uint64_t owner_steam_id,
    uint64_t master_lobby_id,
    uint64_t expedition_lobby_id,
    uint64_t authority_commit_seq,
    uint32_t codec_version,
    uint64_t requested_revision,
    const void* payload,
    uint32_t payload_size,
    uint32_t payload_hash32,
    bool publish_pending,
    const char* reason,
    BhrcFullPgdSnapshotKey* out_key,
    bool authoritative_replace = false);
bool TryPublishBhrcAuthorizedReturningAuthorityCandidate();
void* ResolveBhrcAuthorizedFullPgdRestoreTarget(
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcNativeExpeditionAuthorityState& local_authority);
void TryApplyBhrcFullPgdRestoreMainThread(bool post_native_update);
void ReconcileBhrcFullPgdAuthorityGeneration(
    const BhrcNativeExpeditionAuthorityState& previous,
    const BhrcNativeExpeditionAuthorityState& committed);
bool HasExactBhrcFullPgdHotSnapshot(
    uint64_t owner_steam_id,
    const BhrcNativeExpeditionAuthorityState& authority);
bool CopyBhrcFullPgdHotSnapshot(
    const BhrcFullPgdSnapshotKey& requested,
    BhrcFullPgdSnapshotKey* out_key,
    uint8_t** out_payload);
bool SendBhrcFullPgdPayload(
    uint64_t target_steam_id,
    const BhrcFullPgdSnapshotKey& key,
    const uint8_t* payload,
    BhrcFullPgdTransferPurpose purpose,
    uint32_t request_nonce,
    uint32_t client_generation,
    const char* reason,
    const BhrcHostAdmissionAttempt* admission_guard = nullptr);
bool SendBhrcFullPgdSnapshot(
    uint64_t target_steam_id,
    const BhrcFullPgdSnapshotKey& requested,
    BhrcFullPgdTransferPurpose purpose,
    uint32_t request_nonce,
    uint32_t client_generation,
    const char* reason,
    BhrcFullPgdSnapshotKey* out_sent_key);
void AdvanceBhrcFullPgdSnapshotPublication();
bool HandleRevenantFixFullPgdChunk(
    uint64_t sender,
    const RevenantFixFullPgdChunkPacket& packet);
void CompleteBhrcClientAdmissionIfWorldReady();
bool GetSessionRecordBounds(void* session_mgr, SessionRecordBounds* out);

uintptr_t RvaOf(const void* address);


uint32_t ReadU32(const void* ptr, uint32_t fallback = 0);


uint64_t ReadU64(const void* ptr, uint64_t fallback = 0);


uint16_t ReadU16(const void* ptr, uint16_t fallback = 0);


int32_t ReadI32(const void* ptr, int32_t fallback = 0);


uint8_t ReadU8(const void* ptr, uint8_t fallback = 0);


bool TryReadU64(const void* ptr, uint64_t* out);


bool TryReadU32(const void* ptr, uint32_t* out);


bool TryCompareExchangeU32(
    void* ptr,
    uint32_t desired,
    uint32_t expected,
    uint32_t* observed);


bool TryReadU8(const void* ptr, uint8_t* out);


bool WriteU8(void* ptr, uint8_t value);


bool WriteU16(void* ptr, uint16_t value);


bool WriteI32(void* ptr, int32_t value);


bool WriteU32(void* ptr, uint32_t value);


bool WriteU64(void* ptr, uint64_t value);


void FormatPartyMemberSlots(void* party_member_info, char* slot_text, size_t slot_text_size);


void* GetGlobalPartyMemberInfo();


bool ReadPartyMemberSlot(void* party_member_info, int index, uint8_t* out_slot);


bool WritePartyMemberSlot(void* party_member_info, int index, const uint8_t* slot_data);


bool PartyMemberSlotHasPlayer(const uint8_t* slot_data);


void NormalizePartyMemberInfoAfterRemove(
    void* party_member_info,
    uintptr_t caller_rva,
    uint64_t member_id,
    uint64_t reason,
    uint64_t result);


uint8_t NormalizeBuddyPersistentFlags(uint8_t current_flags);


bool WriteSlotFlags(void* state, int buddy_slot, uint8_t flags);


bool WriteBuddySlotControl(void* state, int buddy_slot, uint8_t state_byte, uint8_t pending, uint8_t flags);


void LogRaw(const char* text, bool flush_immediately = false);


enum class LogLevel {
  Info,
  Warn,
  Error,
};

const char* LogLevelName(LogLevel level);


void VLog(LogLevel level, const char* format, va_list args);


void Log(const char* format, ...);


void LogWarn(const char* format, ...);


void LogError(const char* format, ...);


const char* SehExceptionName(DWORD code);


const char* AccessViolationOperationName(ULONG_PTR operation);


int LogSehException(const char* where, EXCEPTION_POINTERS* info);


void OpenLog();


void BuildDllSidecarPath(const wchar_t* file_name, wchar_t* out_path, size_t out_count);


void EnsureDefaultConfigFile(const wchar_t* config_path);


void EnsureIniKeyDefault(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    const wchar_t* value);


void EnsureConfigDefaults(const wchar_t* config_path);


void LoadConfig();


SlotSnapshot ReadSlot(void* state, int slot);


void LogBuddyExceptionContext(const char* tag, void* state, int slot);


void LogSlotChange(const char* tag, int slot, const SlotSnapshot& before, const SlotSnapshot& after);


BuddyStateRuntimeContext* GetCurrentBuddyStateRuntime();
int GetBuddyStateRuntimeIndex(const BuddyStateRuntimeContext* context);
bool IsLocalBuddyAuthority(void* state, bool* authority_known);
bool BroadcastBuddyRecallPacket(
    void* state,
    int buddy_slot,
    const SlotSnapshot& recalled_snapshot,
    const SlotSnapshot& authority_after,
    const char* reason);

bool IsTrackedBuddyOwnerSlot(int owner_slot);


void ResetBuddyStateRuntimeContext(
    BuddyStateRuntimeContext* context,
    void* state,
    bool owner_known,
    uint32_t owner_id,
    int owner_slot);


BuddyStateRuntimeContext* GetBuddyStateRuntime(void* state, bool create);


BuddyStateRuntimeContext* GetBuddyStateRuntimeForOwner(
    void* state,
    uint32_t owner_id,
    bool create);


BuddyStateRuntimeContext* GetCurrentBuddyStateRuntime();


int GetBuddyStateRuntimeIndex(const BuddyStateRuntimeContext* context);


void MarkRevenantBuddyOwnerSeen(
    BuddyStateRuntimeContext* context,
    uint32_t owner_id,
    int owner_slot);


struct ScopedBuddyStateRuntime {
  BuddyStateRuntimeContext* previous = nullptr;

  explicit ScopedBuddyStateRuntime(void* state, bool create = true) {
    previous = g_active_buddy_state_context;
    g_active_buddy_state_context = GetBuddyStateRuntime(state, create);
  }

  ScopedBuddyStateRuntime(void* state, uint32_t owner_id, bool create) {
    previous = g_active_buddy_state_context;
    g_active_buddy_state_context = GetBuddyStateRuntimeForOwner(state, owner_id, create);
  }

  ~ScopedBuddyStateRuntime() {
    g_active_buddy_state_context = previous;
  }
};

#define g_zero_hp_fallback_regen (GetCurrentBuddyStateRuntime()->zero_hp_fallback_regen)
#define g_stuck_recovery (GetCurrentBuddyStateRuntime()->stuck_recovery)
#define g_pending_deploy_repair (GetCurrentBuddyStateRuntime()->pending_deploy_repair)
#define g_post_recall_pulse (GetCurrentBuddyStateRuntime()->post_recall_pulse)
#define g_buddy_slot_runtime (GetCurrentBuddyStateRuntime()->buddy_slot_runtime)
#define g_recall_regen (GetCurrentBuddyStateRuntime()->recall_regen)
#define g_delayed_switch_cleanup (GetCurrentBuddyStateRuntime()->delayed_switch_cleanup)
#define g_defer_switch_unsummon_this_try (GetCurrentBuddyStateRuntime()->defer_switch_unsummon_this_try)
#define g_defer_switch_unsummon_slot (GetCurrentBuddyStateRuntime()->defer_switch_unsummon_slot)

bool IsEmptyEntityId(const EntityId& entity_id);

bool HasActiveBuddyMaintenanceWork(void* state);


bool IsEmptyEntityId(const EntityId& entity_id);


bool IsEmptyEntity(const SlotSnapshot& slot);


bool SameEntity(const EntityId& left, const EntityId& right);


int ComputeHpRateFromCurrent(int32_t current_hp, int32_t max_hp);


bool SlotChanged(const SlotSnapshot& left, const SlotSnapshot& right);


bool IsValidBuddySlot(int buddy_slot);


bool IsCustomBuddyCreateData(int buddy_slot, int32_t chara_init, int32_t npc_param);


bool IsCustomBuddySlot(int buddy_slot);


bool IsCustomBuddyEntity(int buddy_slot, const EntityId& entity_id);


bool IsCustomBuddySnapshot(int buddy_slot, const SlotSnapshot& snapshot);


int SafeGetBuddyHpRate(void* state, int buddy_slot);


void* LookupEntityById(const EntityId& entity_id);


bool ApplyCustomCharaBuddyPreventNearDeath(
    int buddy_slot,
    void* entity,
    const EntityId& entity_id,
    uint8_t state,
    uint8_t pending,
    bool reset_before_apply,
    const char* source);


bool EnsureCustomCharaBuddyPreventNearDeath(
    int buddy_slot,
    const SlotSnapshot& snapshot);


void* GetHpComponentFromEntity(void* entity);


bool TryReadCustomBuddyContext(
    void* state,
    int buddy_slot,
    const SlotSnapshot& snapshot,
    int hp_rate,
    BuddyContext* context);


int32_t ComputeBuddyRegenStep(int32_t max_hp, uint32_t regen_tick_count);


bool ApplyBuddyRegenTick(
    const char* tag,
    int buddy_slot,
    const BuddyContext& context,
    int32_t base_hp,
    uint32_t* regen_tick_count,
    int32_t* after_hp,
    int32_t* step_hp);


int FindBuddySlotByEntity(void* entity);


uint8_t __fastcall HookApplySpEffect(
    void* entity,
    uint32_t sp_effect_id,
    uint8_t mode);


void* GetWorldChrMan();


void RepublishAuthoritativePlayerStatusAfterNativeRegistration(
    void* peer,
    const void* packet,
    int size,
    uintptr_t caller_rva);


void* LookupChrByGameDataEntityId(uint32_t entity_id);


void* GetRemotePlayerGameDataEntry(int remote_index);


const char* DescribeDiagnosticCallerRva(uintptr_t caller_rva);


EntityId ReadStateOwnerEntityId(void* state);


EntityId ReadChrEntityId(void* chr);


uint32_t EntityHighIndex(const EntityId& entity_id);
void* GetWorldOwnerGroup(void* world_chr_man);
uint64_t ReadWorldOwnerGroupSlot(void* world_owner_group, uint32_t entity_index);
void* WorldOwnerGroupSlotAddress(void* world_owner_group, uint32_t entity_index);

uint64_t GetWorldMapRemoteMarker(void* world_map_model, int marker_index);


bool IsWorldMapMarkerIconResourceEmpty(uint64_t marker);


bool CopyWorldMapMarkerIconResource(uint64_t target_marker, uint64_t source_marker);


int MirroredPlayerAvatarSlotForMarker(int marker_index);


uint32_t NormalizeExpandedPlayerAvatarIconId(uint32_t icon_id);


uint32_t ReadChrIconIdForWorldMap(void* chr);


uint8_t ReadChrIconTypeForWorldMap(void* chr);


void* ResolveWorldMapMarkerChr(int marker_index, void* input_chr);


void RepairWorldMapMarkerAvatarResource(
    void* world_map_model,
    int marker_index,
    void* input_chr,
    void* resolved_chr,
    uint8_t is_local);


uint32_t EntityHighIndex(const EntityId& entity_id);


void* GetWorldOwnerGroup(void* world_chr_man);


int32_t GetLocalBaseOwnerIndex(void* world_chr_man);


int32_t GetWorldOwnerGroupCapacity(void* world_owner_group);


uint64_t ReadWorldOwnerGroupSlot(void* world_owner_group, uint32_t entity_index);


void* WorldOwnerGroupSlotAddress(void* world_owner_group, uint32_t entity_index);


uint8_t SafeCallChrGate(ChrGateFn fn, void* chr, bool* ok);


void* GetNetModeManager();


int32_t GetNetModeState(void* net_mode_manager);


bool IsRewardAcquireAuthority(int32_t net_mode_state);


bool HasRecentLocalRewardDiscardRequest(uint32_t reward_id);

void* SafeFindRewardById(void* map_item_man, uint32_t reward_id);


uint64_t SafeRewardEntryAvailable(void* gaitem_list);


uint32_t RewardNetworkHandleFlag(void* reward);


uint32_t FormatRewardDiscardQueue(void* map_item_man, char* out, size_t out_count);


void LogRewardDiscardRequestState(
    const char* phase,
    void* map_item_man,
    uint32_t reward_id,
    void* net_mode_manager,
    int32_t net_mode_state);


uint8_t __fastcall HookMapGaitemMarkAcquired(
    void* map_item_man,
    uint32_t map_gaitem_id,
    uint8_t acquired);


void __fastcall HookNetSendCore(
    void* net_manager,
    void* peer,
    uint32_t msg_id,
    const void* packet,
  int size);


int __fastcall HookNativeEventPairRead(
    void* record_pair,
    uint8_t event_id,
    void* out_payload,
    uint32_t payload_size,
    uint8_t* out_flags);


int __fastcall HookNativeEventReadAnyRecord(
    void* session_mgr,
    uint32_t event_id,
    void* out_payload,
    uint32_t payload_size);


uint64_t __fastcall HookPartyMemberRemove(
    void* party_member_info,
    uint64_t member_id,
    uint64_t reason);


void RememberLocalRewardDiscardRequest(uint32_t reward_id);


bool HasRecentLocalRewardDiscardRequest(uint32_t reward_id);


void ForgetLocalRewardDiscardRequest(uint32_t reward_id);


bool SpawnDiscardedRewardAsMapNode(void* map_item_man, void* reward, uint32_t reward_id);


void DisableOriginalDiscardRewardNode(void* reward, uint32_t reward_id, const char* phase);


void TryClientRewardDiscardFallback(
    void* map_item_man,
    uint32_t reward_id,
    uint32_t local_apply_count_before,
    int32_t net_mode_state);


void __fastcall HookRewardDiscardRequest(void* map_item_man, uint32_t reward_id);


void __fastcall HookRewardLocalDiscardApply(void* map_item_man, void* reward);


uint8_t SafeCallChrVfunc120(void* chr, bool* ok);


OwnerGateSnapshot CaptureOwnerGateSnapshot(void* state);


bool IsLocalBuddyAuthority(void* state, bool* authority_known = nullptr);


BuddySlotRuntimeState* FindTrackedCustomCharaBuddyForController(
    void* player_ins,
    int* context_index,
    int* buddy_slot);


struct CustomBuddyControllerAuthorityLogState {
  void* player_ins = nullptr;
  uint8_t native_result = 0xff;
  uint8_t corrected_result = 0xff;
};

struct CustomBuddyControllerAuthorityTls {
  void* player_ins = nullptr;
  void* buddy_state = nullptr;
  int context_index = -1;
  int buddy_slot = -1;
  bool active_create = false;
};

inline CustomBuddyControllerAuthorityLogState
    g_custom_buddy_controller_authority_log[kTrackedBuddyStateCount][kBuddySlotCount]{};
inline thread_local CustomBuddyControllerAuthorityTls
    g_custom_buddy_controller_authority_tls{};

uint8_t __fastcall ResolvePlayerInsControllerAuthority(void* player_ins);


inline void* g_custom_buddy_controller_eligibility_logged[kTrackedBuddyStateCount][kBuddySlotCount]{};

uint8_t __fastcall HookPlayerInsControllerEligibility(void* player_ins);


int CountPartyMemberPlayers(void* party_member_info);
void SnapshotBuddySlots(void* state, SlotSnapshot slots[kBuddySlotCount]);


void* GameAllocate(size_t size, size_t alignment);


PlayerGameDataTableExpansion* FindPlayerGameDataExpansionByManager(void* manager);


PlayerGameDataTableExpansion* FindPlayerGameDataExpansionByTable(uint64_t table);


PlayerGameDataTableExpansion* ReservePlayerGameDataExpansionSlot();


bool EnsurePlayerGameDataPoolCapacity(void* manager);


int FixedPlayerGameDataSlotForActiveCreate();


void* ConstructPlayerGameDataEntry(void* memory, int fixed_slot, const char* reason);


void* AllocFixedCustomPlayerGameData(void* manager, int fixed_slot);


void* GetBuddyEntityManager();


uint32_t CountNonEmptyEntitySlots(uint64_t table, uint32_t capacity);


bool EnsureBuddyEntitySlotCapacity(void* manager);


bool WriteCodeByte(void* target, uint8_t value, const char* name);


bool WriteCodeBytes(void* target, const uint8_t* bytes, size_t size, const char* name);


bool ReadCodeBytes(void* target, uint8_t* out, size_t size);


void FormatBytes(const uint8_t* bytes, size_t size, char* out, size_t out_size);


bool InstallCodePatch(
    CodePatch& patch,
    uintptr_t rva,
    const uint8_t* expected,
    const uint8_t* replacement,
    size_t size,
    const char* name,
    bool allow_call_overwrite = false,
    const uint8_t* alternate_expected = nullptr);


void RemoveCodePatch(CodePatch& patch);


void* GetPlayerGameDataManager();


bool CopyBytesSafe(void* dst, const void* src, size_t size);


bool ZeroBytesSafe(void* dst, size_t size);


bool InstallRemotePlayerGameDataLimitPatches();


bool EnsureRemotePlayerGameDataCapacity(const char* reason);


DWORD WINAPI RemotePlayerGameDataExpansionThread(void*);


bool InstallPlayerMapMarkerPatches();


bool InstallBuddyReadyRangePatch();


void RemoveBytePatch(BytePatch& patch);


bool ResetBuddySlotState(void* state, int buddy_slot);


bool NormalizeActiveSlotForUnsummon(void* state, int buddy_slot);


bool PrepareSlotForOriginalResummon(void* state, int buddy_slot);


bool SetZeroHpFallbackRecoveryGate(void* state, int buddy_slot, const SlotSnapshot& snapshot, bool enabled);


void StopZeroHpFallbackRegen(int buddy_slot, const SlotSnapshot& snapshot, const char* reason);


void StopRecallRegen(int buddy_slot, const char* reason);


void StopStuckRecoveryMonitor(int buddy_slot, const char* reason);


bool UpdateStuckRecoveryState(void* state, int buddy_slot, const SlotSnapshot& snapshot, int hp_rate);


void StopPostRecallPulse(int buddy_slot, const char* reason);


bool SetEntitySummonRequestFlag(const SlotSnapshot& snapshot, const char* tag, int buddy_slot);

bool StartPostRecallPulse(
    void* state,
    int buddy_slot,
    const SlotSnapshot& snapshot,
    int hp_rate,
    const char* reason);


bool ProcessPostRecallPulse(void* state, int buddy_slot, const SlotSnapshot& snapshot);


bool StartRecallRegen(int buddy_slot, const EntityId& entity_id, void* hp_component, const char* reason);


bool IsRecallRegenRecalledSlot(const SlotSnapshot& snapshot);


bool IsZeroHpFallbackRecalledSlot(const SlotSnapshot& snapshot);


bool IsZeroHpFallbackSlotAllowed(const SlotSnapshot& snapshot, const ZeroHpFallbackRegenState& regen);


bool UpdateRecallRegen(void* state, int buddy_slot, const SlotSnapshot& snapshot, int hp_rate);


bool UpdateZeroHpFallbackRegen(void* state, int buddy_slot, const SlotSnapshot& snapshot, int hp_rate);


int MapRevenantBuddyTriggerToSlot(uint32_t buddy_trigger_id);


bool IsLikelyRevenantBuddyActionId(int action_id);


uint64_t __fastcall HookTryUseBuddy(void* state, uint32_t buddy_trigger_id);

bool IsZeroHpFallbackSummonBlocked(void* state, uint32_t buddy_trigger_id);


bool IsStalePendingUnsummonState(const SlotSnapshot& slot);


bool IsForcedBuddyState(const SlotSnapshot& slot, uint8_t transition_flag);


bool IsForcedUnsummonState(const SlotSnapshot& slot);


bool IsForcedResummonState(const SlotSnapshot& slot);


bool IsCustomBuddyDeployedSlot(void* state, int buddy_slot, const SlotSnapshot& snapshot);


int FindSwitchTargetSlotAfterRequest(void* state, int old_slot, const SlotSnapshot (&before)[kBuddySlotCount]);


bool HasPendingCustomDeployRequest(void* state);


bool CallBuddyModuleUpdateOriginalPlain(void* state, const char* caller);


bool TryActivateOwnerWorldGroupFromState(void* state, const char* caller);


bool RunOriginalBuddyModuleUpdate(void* state, int buddy_slot, const char* reason);


bool IsRemoteOwnerGateFallbackStage(const SlotSnapshot& snapshot);


bool IsRemoteOwnerGateFallbackCandidate(
    void* state,
    int buddy_slot,
    const SlotSnapshot& snapshot,
    OwnerGateSnapshot* gate_out);


bool SetEntitySummonRequestFlag(const SlotSnapshot& snapshot, const char* tag, int buddy_slot);


bool ManualRemoteFallbackBuddyUpdateStage(
    void* state,
    int buddy_slot,
    const SlotSnapshot& before);


bool ManualRemoteFallbackDeathRecoveryStage(
    void* state,
    int buddy_slot,
    const SlotSnapshot& before);


bool RunRemoteOwnerGateFallbackUpdate(void* state);


void ResetPendingDeployRepair(int buddy_slot, const char* reason);


bool UpdatePendingDeployRepair(void* state, int buddy_slot, const SlotSnapshot& snapshot);


bool ForceFreshResummonFromStableActiveSlot(void* state, int buddy_slot, const char* reason);


bool ForceUnsummonActiveSlot(void* state, int buddy_slot, const char* reason, bool queue_resummon);


bool ProcessPendingRemoteBuddyRecallCommands(void* state);


bool HasCustomBuddyRuntime();


void CancelDelayedSwitchCleanup(const char* reason);


bool ScheduleDelayedSwitchCleanup(void* state, int current_slot, const char* reason);


bool DeferForcedResummonForDelayedCleanup(void* state, int buddy_slot, const char* reason);


void ProcessDelayedSwitchCleanup(void* state);


void __fastcall HookBuddyModuleUpdate(void* state);


uint64_t CallTryUseBuddyOriginalSeh(void* state, uint32_t buddy_trigger_id, int buddy_slot);


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
    bool custom_candidate);


bool CallChangeBuddyStateOriginalSeh(void* state, int buddy_slot);


bool CallRequestUnsummonOriginalSeh(void* state, int buddy_slot);


uint64_t __fastcall HookTryUseBuddy(void* state, uint32_t buddy_trigger_id);


uint32_t* __fastcall HookCreateBuddy(
    void* state,
    uint32_t* out_entity,
    uint32_t owner_id,
    int buddy_slot,
    const void* buddy_create_data,
    char flag);


void* __fastcall HookCreateEntityForOwner(void* manager, void* create_data, uint8_t owner_id, int buddy_slot);


void* __fastcall HookPlayerGameDataAlloc(void* manager);


void TryRegisterBaseOwnerWorldGroup(
    void* world_chr_man,
    uint64_t entity_index64,
    void* source,
    void* aux,
    const char* caller);


void __fastcall HookWorldChrRegister(
    void* world_chr_man,
    uint64_t entity_index,
    void* source,
    void* aux);


void __fastcall HookChangeBuddyState(void* state, int buddy_slot);

void __fastcall HookRequestUnsummon(void* state, int buddy_slot);

void __fastcall HookWorldMapRemoteMarkerUpdate(
    void* world_map_model,
    int marker_index,
    void* chr,
    uint8_t is_local);


const char* DescribeNrscLobbyEvent(uint32_t event_type);


bool IsLocalNrscHost();
const char* BhrcPhaseName(BhrcPhase phase);
struct RevenantFixSteamSendTrace {
  const char* failure_reason = "not_attempted";
  bool steam_called = false;
  bool steam_result_valid = false;
  int steam_result = 0;
};

bool TrySendRevenantFixSteamPacketRaw(
    uint64_t target_steam_id,
    const void* payload,
    uint32_t payload_size,
    uint16_t type,
    const char* reason,
    bool allow_inactive_authorized_restore,
    RevenantFixSteamSendTrace* send_trace = nullptr);
uint64_t GetSavedNrscSteamLobbyId();
uint64_t GetSavedNrscMasterLobbyId();
uint64_t GetCurrentNrscMasterLobbyId();
void TryCommitPendingNativeExpeditionAuthority();

int CountPartyMemberPlayers(void* party_member_info);


int CountRemotePlayerGameDataEntities();


void RememberNrscPeerSteamId(uint64_t steam_id);


bool IsNrscPeerActive(uint64_t steam_id);
bool TryReadNrscNativePeerActive(uint64_t steam_id, bool* active);


void SetNrscPeerActive(uint64_t steam_id, bool active, const char* reason);


void ClearNrscActivePeers(const char* reason);

void TraceBhrcPgdPeerChange(uint64_t steam_id, bool active, bool changed,
    const char* reason, const void* caller);
void TraceBhrcPgdReceiveGate(uint64_t actual_steam_id,
    const RevenantFixFullPgdChunkPacket& packet, bool peer_active,
    LONG bridge_state, const char* outcome);


uint64_t ParseIniHexU64(const wchar_t* text, uint64_t fallback);


uint64_t ReadIniU64(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    uint64_t fallback);


void WriteIniU64Hex(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    uint64_t value);


int BhrcHexNibble(wchar_t value);


bool WriteIniHexBytes(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    const uint8_t* bytes,
    size_t size);


bool ReadIniHexBytes(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    uint8_t* out,
    size_t size);


// PlayerGameData bytes are never persisted or hydrated here. The 0x1f8 payload
// is metadata-only; complete native PGD snapshots live only in the
// generation-bound process hot store and disappear when all peers exit.

void WriteIniI32(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    int32_t value);


int32_t ReadIniI32(
    const wchar_t* config_path,
    const wchar_t* section,
    const wchar_t* key,
    int32_t fallback);


bool BuildReconnectStatePath(wchar_t* path, DWORD path_count);


class BhrcStateFileGuard {
 public:
  explicit BhrcStateFileGuard(bool exclusive) : exclusive_(exclusive) {
    if (exclusive_) {
      AcquireSRWLockExclusive(&g_bhrc_state_file_lock);
    } else {
      AcquireSRWLockShared(&g_bhrc_state_file_lock);
    }
  }

  ~BhrcStateFileGuard() {
    if (exclusive_) {
      ReleaseSRWLockExclusive(&g_bhrc_state_file_lock);
    } else {
      ReleaseSRWLockShared(&g_bhrc_state_file_lock);
    }
  }

  BhrcStateFileGuard(const BhrcStateFileGuard&) = delete;
  BhrcStateFileGuard& operator=(const BhrcStateFileGuard&) = delete;

 private:
  bool exclusive_ = false;
};

template <typename Fn>
Fn ResolveNative(uintptr_t rva) {
  if (g_game_base == 0 || rva == 0) {
    return nullptr;
  }
  return reinterpret_cast<Fn>(g_game_base + rva);
}


void* ReadGameGlobalPtr(uintptr_t rva);


const uint8_t* FindSessionRecordByKey(void* session_mgr, uint64_t record_key);
uint32_t CountSessionRecords(void* session_mgr);
uint32_t ReadBhrcTeamRecordCount(void* state);

uint8_t ReadBhrcPlayerStatusByte(int32_t player_index);


bool CallBhrcPlayerStatusComplete(int32_t player_index, bool* out_call_ok);


int32_t FindPlayerIndexForPlayerGameData(void* player_game_data);


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
    const char* reason);

const char* BhrcPhaseName(BhrcPhase phase);
void BhrcSetPhase(BhrcPhase phase, const char* reason);

void PersistCachedReconnectRecord(const BhrcSteamRecordCacheEntry& entry, const char* reason);
void BuildReconnectRecordSection(uint64_t steam_id, wchar_t* section, size_t section_count);

BhrcSteamRecordCacheEntry* FindCachedReconnectRecordByRecordKeyForStatus(uint64_t record_key);


const uint8_t* FindSessionRecordByPlayerIndexForStatus(void* session_mgr, int32_t player_index);


bool StoreBhrcStatusPayloadMetadataInCache(
    BhrcSteamRecordCacheEntry* entry,
    const void* payload,
    uint32_t payload_size,
    uint32_t payload_hash,
    const char* reason);


// Thread-local bridge for compact-status metadata: FUN_14020dcb0 apply is
// intentionally unhooked (trampoline breaks official status apply). Read
// (FUN_140ddad10) and validate (FUN_14020c350) run on the same thread
// back-to-back (callerRva ddae2b->ddae82), so retain the record key only long
// enough to associate size/hash metadata. No 0x1f8 bytes are cached.
struct BhrcTlsLast0cRead {
  uint64_t record_key = 0;
  const void* payload = nullptr;
  DWORD tick_ms = 0;
};
inline thread_local BhrcTlsLast0cRead g_bhrc_tls_last_0c_read{};

void RememberBhrcTlsLast0cRead(uint64_t record_key, const void* payload);


void ClearBhrcTlsLast0cRead();


bool CaptureBhrcStatusPayloadForRecordKey(uint64_t record_key, const void* payload, const char* reason);


bool CaptureBhrcStatusPayloadForPlayerIndex(int32_t player_index, const void* payload, const char* reason);


void __fastcall HookSessionEventBroadcast(
    void* session_mgr,
    uint32_t event_type,
    const void* payload,
    uint32_t payload_size);


uint64_t __fastcall HookSessionEventBroadcastToTargets(
    void* session_mgr,
    void* target_list,
    uint32_t event_type,
    const void* payload,
    uint64_t payload_size);


int __fastcall HookSessionRecordEventRead(
    void* session_mgr,
    const void* record,
    uint32_t event_type,
    void* out_payload,
    uint32_t payload_size,
    uint8_t pending,
    uint8_t removing);


void __fastcall HookPlayerStatusSerializePayload(void* player_game_data, void* out_payload);


uint8_t __fastcall HookPlayerStatusPayloadValidate(const void* payload);


bool __fastcall HookPlayerStatusComplete(int32_t player_index);


uint32_t BhrcFnv1a32(const void* data, size_t size);


const char* BhrcFullPgdPurposeName(BhrcFullPgdTransferPurpose purpose);


bool IsBhrcFullPgdPayloadShapeValid(uint32_t size, uint16_t chunk_count);


void PublishBhrcFullPgdAuthorityReceipt(
    uint64_t master_lobby_id,
    uint64_t expedition_lobby_id,
    uint64_t owner_steam_id,
    uint64_t authority_commit_seq);


bool SnapshotBhrcFullPgdAuthorityReceipt(
    const BhrcNativeExpeditionAuthorityState& local_authority,
    uint64_t* out_authority_commit_seq);


bool BhrcFullPgdRestoreTicketMatchesProbe(
    const BhrcFullPgdRestoreTicket& ticket,
    const BhrcExpeditionProbeAttempt& probe);


bool SameBhrcFullPgdRestoreTicketIdentity(
    const BhrcFullPgdRestoreTicket& lhs,
    const BhrcFullPgdRestoreTicket& rhs);


bool BhrcFullPgdRestoreGateMatchesProbe(
    const BhrcFullPgdRestoreGate& gate,
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcNativeExpeditionAuthorityState& authority);


void ArmBhrcFullPgdRestoreGate(
    const BhrcExpeditionProbeAttempt& probe);


void ArmBhrcReturningOwnerWorldGeneration(
    const BhrcExpeditionProbeAttempt& probe,
    const char* reason);


void ClearBhrcFullPgdRestoreGate(const char* reason);


void MarkBhrcFullPgdRestoreGateStage(
    const BhrcExpeditionProbeAttempt& probe,
    BhrcFullPgdRestoreGateStage stage);


bool IsBhrcFullPgdCaptureFrozen(
    const BhrcNativeExpeditionAuthorityState& authority,
    uint64_t local_steam_id,
    uint64_t host_steam_id);


void* ResolveBhrcFinalFullPgdRestoreTarget(
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcNativeExpeditionAuthorityState& authority);


bool IsBhrcFullPgdRestoreAppliedForProbe(
    const BhrcExpeditionProbeAttempt& probe);


bool IsBhrcFullPgdFinalRestoreAppliedForProbe(
    const BhrcExpeditionProbeAttempt& probe);


void MarkBhrcFullPgdRestoreFailed(
    const BhrcFullPgdRestoreTicket& expected,
    const char* reason);


void TryApplyBhrcFullPgdRestoreMainThread(bool post_native_update);


void ClearBhrcFullPgdTransportForPeer(uint64_t steam_id, const char* reason);


void ClearBhrcFullPgdAdmissionTransport(const char* reason);


void ReconcileBhrcFullPgdAuthorityGeneration(
    const BhrcNativeExpeditionAuthorityState& previous,
    const BhrcNativeExpeditionAuthorityState& committed);


bool BhrcFullPgdAdmissionAttemptIdentityMatches(
    const BhrcHostAdmissionAttempt& lhs,
    const BhrcHostAdmissionAttempt& rhs);


bool BhrcFullPgdPinnedRestoreMatchesAttempt(
    const BhrcFullPgdPinnedRestore& pinned,
    const BhrcHostAdmissionAttempt& attempt,
    bool require_reservation_serial);


void ResetBhrcFullPgdPinnedRestoreLocked(
    BhrcFullPgdPinnedRestore* pinned);


void ClearBhrcFullPgdPinnedRestoreForSerial(uint64_t reservation_serial);


void ClearBhrcFullPgdPinnedRestoresForPeer(uint64_t steam_id);


void ClearAllBhrcFullPgdPinnedRestores();


bool CopyBhrcFullPgdPinnedRestoreForAttempt(
    const BhrcHostAdmissionAttempt& attempt,
    BhrcFullPgdSnapshotKey* out_key,
    uint8_t** out_payload);


bool PinBhrcFullPgdRestoreSnapshotForAttempt(
    const BhrcHostAdmissionAttempt& attempt,
    BhrcFullPgdSnapshotKey* out_requested);


bool SendBhrcFullPgdRestoreToAttempt(
    const BhrcHostAdmissionAttempt& attempt,
    const char* reason);


void AdvanceBhrcFullPgdHostRestoreDeliveries();


void AdvanceBhrcFullPgdTransport();


BhrcFullPgdHotSnapshot* FindBhrcFullPgdHotSnapshotLocked(
    uint64_t owner_steam_id,
    uint64_t master_lobby_id,
    uint64_t expedition_lobby_id,
    uint64_t authority_commit_seq);


bool StoreBhrcFullPgdHotSnapshot(
    uint64_t owner_steam_id,
    uint64_t master_lobby_id,
    uint64_t expedition_lobby_id,
    uint64_t authority_commit_seq,
    uint32_t codec_version,
    uint64_t requested_revision,
    const void* payload,
    uint32_t payload_size,
    uint32_t payload_hash32,
    bool publish_pending,
    const char* reason,
    BhrcFullPgdSnapshotKey* out_key,
    bool authoritative_replace);


bool HasExactBhrcFullPgdHotSnapshot(
    uint64_t owner_steam_id,
    const BhrcNativeExpeditionAuthorityState& authority);


bool CopyBhrcFullPgdHotSnapshot(
    const BhrcFullPgdSnapshotKey& requested,
    BhrcFullPgdSnapshotKey* out_key,
    uint8_t** out_payload);


bool SerializeBhrcLocalFullPgd(
    void* player_game_data,
    uint32_t* out_codec_version,
    uint32_t* out_payload_size,
    uint32_t* out_payload_hash32);


void CaptureBhrcLocalFullPgdMainThread();


bool SendBhrcFullPgdPayload(
    uint64_t target_steam_id,
    const BhrcFullPgdSnapshotKey& key,
    const uint8_t* payload,
    BhrcFullPgdTransferPurpose purpose,
    uint32_t request_nonce,
    uint32_t client_generation,
    const char* reason,
    const BhrcHostAdmissionAttempt* admission_guard);


bool SendBhrcFullPgdSnapshot(
    uint64_t target_steam_id,
    const BhrcFullPgdSnapshotKey& requested,
    BhrcFullPgdTransferPurpose purpose,
    uint32_t request_nonce,
    uint32_t client_generation,
    const char* reason,
    BhrcFullPgdSnapshotKey* out_sent_key);


void MarkBhrcFullPgdSnapshotsForPublish(const char* reason);


void MarkBhrcFullPgdSnapshotPublished(
    const BhrcFullPgdSnapshotKey& key);


void AdvanceBhrcFullPgdSnapshotPublication();


bool BhrcFullPgdPacketMatchesAssembly(
    const BhrcFullPgdInboundAssembly& assembly,
    uint64_t sender,
    const RevenantFixFullPgdChunkPacket& packet);


bool ArmBhrcFullPgdRestoreTicket(
    uint64_t sender,
    const RevenantFixFullPgdChunkPacket& packet,
    const BhrcExpeditionProbeAttempt& probe,
    const uint8_t* payload);


bool HandleRevenantFixFullPgdChunk(
    uint64_t sender,
    const RevenantFixFullPgdChunkPacket& packet);


const char* BhrcRejectReasonName(BhrcRejectReason reason);


uint32_t TeamSessionDialogTextIdForReason(uint8_t reason);
const char* TeamSessionDialogReasonName(uint8_t reason);

const char* BhrcPhaseName(BhrcPhase phase);


uint32_t CountKnownNrscPeers();


const char* BhrcIdentityMembershipStateName(
    BhrcIdentityMembershipState state);
const char* BhrcRecordRoleName(uint32_t role);

void CountBhrcRecordCache(uint32_t* active_count, uint32_t* inactive_count);


void LogBhrcRecordCacheSummary(const char* reason);


void LogBhrcSessionRecords(const char* reason, void* session_mgr, uint64_t focus_record_key);

void BhrcSetPhase(BhrcPhase phase, const char* reason);


void RememberSuppressedTeamSessionDialog(void* state, uint8_t reason, const char* detail);


bool ShowFixedTeamSessionFailurePrompt(uint8_t reason, const char* failure, const char* detail);


void ReplaySuppressedMatchedPlayersUiAfterReconnectFailure(BhrcRejectReason reason, const char* detail);


void BhrcFail(BhrcRejectReason reason, const char* detail);


bool LoadPersistedReconnectIdentity(PersistedReconnectIdentity* out);


const char* BhrcRecordRoleName(uint32_t role);


bool PersistReconnectIdentity(
    const PersistedReconnectIdentity& id,
    const char* reason);


bool GetSessionRecordBounds(void* session_mgr, SessionRecordBounds* out);


const uint8_t* FindSessionRecordByKey(void* session_mgr, uint64_t record_key);


uint32_t CountSessionRecords(void* session_mgr);


const char* BhrcIdentityMembershipStateName(BhrcIdentityMembershipState state);


struct BhrcStableIdentityHashInput {
  uint32_t schema_version = kBhrcPlayerIdentitySnapshotSchemaVersion;
  uint32_t record_role = kBhrcRecordRoleUnknown;
  uint64_t expedition_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t steam_id = 0;
  uint64_t record_key = 0;
  int32_t player_index = -1;
  int32_t record_d4_id = -1;
  int32_t player_game_data_id = -1;
  int32_t reserved2 = 0;
  uint64_t party_member_handle = 0;
};

uint32_t ComputeBhrcPlayerIdentityHash(
    const BhrcPlayerIdentitySnapshot& snapshot);


BhrcPlayerIdentitySnapshot BuildBhrcPlayerIdentitySnapshot(
    uint64_t steam_id,
    uint64_t lobby_id,
    const void* candidate_record,
    int32_t candidate_player_index,
    int32_t candidate_game_data_id,
    uint32_t expedition_generation);


bool SameBhrcPlayerIdentitySnapshot(
    const BhrcPlayerIdentitySnapshot& left,
    const BhrcPlayerIdentitySnapshot& right);


BhrcPlayerIdentitySnapshot MergeBhrcPlayerIdentitySnapshot(
    const BhrcPlayerIdentitySnapshot& previous,
    BhrcPlayerIdentitySnapshot observed,
    bool status_payload_metadata_valid,
    uint32_t status_payload_hash32);


void LogBhrcSessionRecords(const char* reason, void* session_mgr, uint64_t focus_record_key);


BhrcSteamRecordCacheEntry* CacheReconnectRecordForSteamId(
    uint64_t steam_id,
    uint64_t lobby_id,
    const void* record,
    int32_t player_index,
    int32_t game_data_id,
    int32_t party_slot,
    uint32_t generation,
    const char* reason,
    bool persist);

const uint8_t* SelectSessionRecordForLocalIdentity(
    void* session_mgr,
    void* default_player_game_data,
    SessionRecordKeyToPlayerIndexFn key_to_index,
    PlayerGameDataByIndexFn pgd_by_index,
    PlayerGameDataIdFn pgd_id,
    const char* reason,
    int32_t* out_player_index,
    int32_t* out_game_data_id,
    int32_t* out_party_slot,
    const char** out_source);


void BuildReconnectRecordSection(uint64_t steam_id, wchar_t* section, size_t section_count);


BhrcSteamRecordCacheEntry* FindCachedReconnectRecordBySteamId(uint64_t steam_id);


bool IsBhrcReconnectCacheEntryComplete(const BhrcSteamRecordCacheEntry& entry);


bool PreferCachedReconnectIdentityForLocalSteam(
    PersistedReconnectIdentity* id,
    uint64_t selected_lobby_id,
    const char* reason);


BhrcSteamRecordCacheEntry* FindActiveReconnectRecordBySteamId(uint64_t steam_id);


BhrcSteamRecordCacheEntry* FindInactiveReconnectRecordBySteamId(uint64_t steam_id, uint32_t* out_index);


void RemoveInactiveReconnectRecordAtIndex(uint32_t index);


BhrcSteamRecordCacheEntry* FirstFreeActiveReconnectRecordSlot();


bool SnapshotCachedReconnectRecordBySteamId(
    uint64_t steam_id,
    BhrcSteamRecordCacheEntry* out);


BhrcSteamRecordCacheEntry* AllocateInactiveReconnectRecordBySteamId(
    uint64_t steam_id, const char* reason);


BhrcSteamRecordCacheEntry* AllocateActiveReconnectRecordBySteamId(uint64_t steam_id, const char* reason);


void MoveCachedReconnectRecordToInactive(uint64_t steam_id, const char* reason);


void SetCachedReconnectTransportEvidence(
    uint64_t steam_id, bool active, const char* reason);


void ReconcileCachedReconnectMembershipFromNative(const char* reason);


void PersistCachedReconnectRecord(const BhrcSteamRecordCacheEntry& entry, const char* reason);


void RebindBhrcCachedIdentityOwnersForMigration(
    const BhrcNativeExpeditionAuthorityState& previous,
    const BhrcNativeExpeditionAuthorityState& migrated);


BhrcSteamRecordCacheEntry* CacheReconnectRecordForSteamId(
    uint64_t steam_id,
    uint64_t lobby_id,
    const void* record,
    int32_t player_index,
    int32_t game_data_id,
    int32_t party_slot,
    uint32_t generation,
    const char* reason,
    bool persist);


void CaptureLiveRemoteReconnectIdentities(
    void* session_mgr,
    const char* reason);


void FillRecordRosterEntry(const BhrcSteamRecordCacheEntry& source, uint16_t state, RevenantFixRecordRosterEntry* out);


uint16_t BuildRecordRosterEntries(RevenantFixRecordRosterEntry* entries, uint16_t max_entries);


void ApplyRosterRecordStateHint(const RevenantFixRecordRosterEntry& roster_entry, const char* reason);


bool BroadcastRecordRosterIfDue(const char* reason);


void* ResolveSteamMatchmakingForProbe(const char* reason);


bool InviteBhrcAdmittedMemberToExpeditionLobby(
    uint64_t expedition_lobby_id,
    uint64_t invitee_steam_id,
    uint32_t request_nonce,
    BhrcExpeditionMembership membership);


bool ReopenBhrcMasterLobbyInvitationsAfterHostMigration(
    const BhrcNativeExpeditionAuthorityState& previous,
    const BhrcNativeExpeditionAuthorityState& committed);


struct BhrcMigratedMasterSuccessorDecision {
  bool authority_group_valid = false;
  bool candidate_valid = false;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint64_t candidate_steam_id = 0;
  const char* group_source = "none";
};

struct BhrcFullPgdHotSnapshotMetadata {
  bool valid = false;
  uint64_t updated_tick_ms = 0;
  uint64_t owner_steam_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t authority_commit_seq = 0;
  uint32_t codec_version = 0;
  uint32_t payload_size = 0;
  uint32_t payload_hash32 = 0;
};

BhrcMigratedMasterSuccessorDecision
SelectBhrcMigratedMasterSuccessor(
    uint64_t master_lobby_id,
    uint64_t departed_owner_steam_id);


void ArmBhrcMigratedMasterLeaveLease(
    const BhrcNativeExpeditionAuthorityState& previous,
    const BhrcNativeExpeditionAuthorityState& migrated);


bool CommitBhrcNrscMasterOwnerForMigration(
    uint64_t master_lobby_id,
    uint64_t previous_owner,
    uint64_t owner_steam_id);


bool TryReopenBhrcMasterLobbyInvitationsAtSteamOwnerChange(
    uint64_t master_lobby_id,
    uint64_t previous_owner,
    uint64_t owner_steam_id);


bool RefreshBhrcMasterLobbyOwnerFromSteam(
    uint64_t master_lobby_id,
    const char* reason);


uint64_t RequestNrscMasterLobbyJoin(uint64_t lobby_id, const char* reason);


void PersistNrscSteamControlState(const char* reason);


uint64_t LoadPersistedNrscMasterLobbyId();


uint64_t LoadPersistedExpeditionLobbyId();


void CaptureNrscLobbyScanState(const char* reason, void* context, uint64_t observed_lobby);


void CaptureNrscLobbyEventState(
    const char* reason,
    void* context,
    uint64_t lobby_id);


bool IsLocalNrscHost();


const char* RevenantFixSteamControlPacketTypeName(uint16_t type);


BhrcNativeExpeditionAuthorityState SnapshotNativeExpeditionAuthority();


uint64_t GetSavedNrscSteamLobbyId();


uint64_t GetSavedNrscMasterLobbyId();


uint64_t GetCurrentNrscMasterLobbyId();


void ObserveNativeExplicitLobbyCandidate(
    uint64_t lobby_id, uintptr_t caller_rva, int operation_result);


RevenantFixSteamPeerTestState* FindRevenantFixSteamPeerTestState(uint64_t steam_id);


RevenantFixSteamPeerTestState* AllocateRevenantFixSteamPeerTestState(uint64_t steam_id);


uint32_t GenerateRevenantFixSteamTestNonce(uint64_t peer_steam_id);


uint32_t GenerateDistinctRevenantFixSteamTestNonce(
    uint64_t peer_steam_id, uint32_t prior_nonce);


void FillSteamNetworkingIdentitySteamId(SteamNetworkingIdentityLite* identity, uint64_t steam_id);


uint64_t ReadSteamNetworkingIdentitySteamId(const SteamNetworkingIdentityLite& identity);


bool EnsureBhrcStartupP2PCallbackRegistered(const char* reason);


bool ResolveBhrcStartupP2PTransport(const char* reason);


void HandleBhrcStartupP2PSessionRequest(uint64_t remote_steam_id);


bool TrySendBhrcStartupP2PPacket(
    uint64_t target_steam_id,
    const void* payload,
    uint32_t payload_size,
    uint16_t expected_type,
    const char* reason);


bool ResolveSteamNetworkingMessagesInterface(const char* reason);


bool TrySendRevenantFixSteamPacketRaw(
    uint64_t target_steam_id,
    const void* payload,
    uint32_t payload_size,
    uint16_t type,
    const char* reason,
    bool allow_inactive_authorized_restore,
    RevenantFixSteamSendTrace* send_trace);


bool BroadcastBuddyRecallPacket(
    void* state,
    int buddy_slot,
    const SlotSnapshot& recalled_snapshot,
    const SlotSnapshot& authority_after,
    const char* reason);


bool EnqueueBuddyRecallPacket(
    uint64_t sender_steam_id,
    const RevenantFixBuddyRecallPacket& packet);


const char* BhrcExpeditionProbeTriggerName(BhrcExpeditionProbeTrigger trigger);


const char* BhrcExpeditionHostPhaseName(BhrcExpeditionHostPhase phase);


const char* BhrcExpeditionMembershipName(BhrcExpeditionMembership membership);


const char* BhrcExpeditionProbeStageName(BhrcExpeditionProbeStage stage);


ULONGLONG BhrcNativeAdmissionLifetimeForTrigger(
    BhrcExpeditionProbeTrigger trigger);


bool IsBhrcExpeditionProbePendingStage(BhrcExpeditionProbeStage stage);


BhrcExpeditionProbeAttempt SnapshotBhrcExpeditionProbe();


const char* BhrcOldMemberMatchingFenceKindName(
    BhrcOldMemberMatchingFenceKind kind);


BhrcOldMemberMatchingFence SnapshotBhrcOldMemberMatchingFence();


bool BhrcOldMemberMatchingFenceMatchesCurrent(
    const BhrcOldMemberMatchingFence& fence);


void PublishBhrcOldMemberMatchingFence(
    BhrcOldMemberMatchingFenceKind kind,
    uint32_t source_nonce,
    uint64_t master_lobby_id,
    uint64_t expedition_lobby_id,
    uint64_t host_steam_id,
    uint64_t record_key,
    uint32_t client_generation,
    uint32_t identity_hash32,
    uint64_t authority_commit_seq,
    uint32_t manager_id,
    const char* reason);


bool ClearBhrcOldMemberMatchingFenceForTransport(
    uint64_t master_lobby_id,
    uint64_t host_steam_id,
    uint64_t record_key,
    const char* reason);


bool ClearBhrcOldMemberMatchingFenceForExactProbe(
    const BhrcExpeditionProbeAttempt& probe,
    const char* reason);


void RebindBhrcOldMemberMatchingFenceNonce(
    const BhrcExpeditionProbeAttempt& before,
    const BhrcExpeditionProbeAttempt& after,
    const char* reason);


void ApplyBhrcOldMemberMatchingFenceForAcceptedProbeResponse(
    const BhrcExpeditionProbeAttempt& before,
    const BhrcExpeditionProbeAttempt& after);


bool BhrcExpeditionProbeOwnsReconnect();


bool ArmBhrcExpeditionProbe(
    BhrcExpeditionProbeTrigger trigger,
    uint64_t master_lobby_id,
    uint64_t expedition_lobby_id,
    uint64_t host_steam_id,
    uint64_t record_key,
    uint32_t generation,
    uint32_t identity_hash32,
    const char* reason,
    bool force_replace = false,
    uint64_t expected_attempt_serial = 0,
    bool authority_observer_only = false,
    bool bind_quick_match_attempt = true,
    bool rebind_current_quick_match = true);


bool LoadBhrcStartupReconnectCandidate(
    PersistedReconnectIdentity* identity_out = nullptr,
    uint64_t* host_candidates_out = nullptr,
    uint32_t host_candidate_capacity = 0,
    uint32_t* host_candidate_count_out = nullptr);
BhrcStartupHostPreflightAttempt SnapshotBhrcStartupHostPreflight();

bool ArmBhrcStartupExpeditionProbe(const char* reason);


const char* BhrcStartupHostPreflightStageName(
    BhrcStartupHostPreflightStage stage);


BhrcStartupHostPreflightAttempt SnapshotBhrcStartupHostPreflight();


BhrcStartupTitleCheckPenaltyBridge
SnapshotBhrcStartupTitleCheckPenaltyBridge();


bool BhrcStartupHostPreflightMatchesProbeIdentity(
    const BhrcStartupHostPreflightAttempt& preflight,
    const BhrcExpeditionProbeAttempt& probe);


bool BhrcStartupHostPreflightMatchesProbe(
    const BhrcStartupHostPreflightAttempt& preflight,
    const BhrcExpeditionProbeAttempt& probe);


bool BeginBhrcStartupNativeWait(
    uint32_t request_nonce,
    const char* reason);


bool IsBhrcStartupNativeWaitActive(
    const BhrcStartupHostPreflightAttempt& preflight,
    DWORD now);


bool BhrcStartupHostPreflightOwnsReconnect();


void ReleaseBhrcStartupJoinOwnership(
    uint32_t expected_request_nonce,
    uint64_t expected_host_steam_id,
    const char* reason);


void InvalidateBhrcStartupAuthorizedLatch(
    uint32_t expected_request_nonce,
    uint64_t expected_host_steam_id,
    const char* reason);


void MarkBhrcStartupAuthorizedLatchHandedOff(
    const BhrcExpeditionProbeAttempt& probe);


bool ArmBhrcStartupHostPreflight(const char* reason);


uint64_t ReadBhrcHostMultiplaySessionAuthority(
    const BhrcExpeditionHostPhaseSnapshot& phase,
    bool* native_value_out);


bool HandleBhrcStartupHostPreflightRequest(
    uint64_t sender,
    const RevenantFixStartupHostPreflightRequestPacket& packet);


bool HandleBhrcStartupHostPreflightResponse(
    uint64_t sender,
    const RevenantFixStartupHostPreflightResponsePacket& packet);


void ProcessBhrcStartupP2PReceives();


void AdvanceBhrcStartupHostPreflight();


bool HasAcceptedBhrcTeamSessionRecord(uint64_t record_key);


uint16_t ReadBhrcTeamSessionMemberCountForProbe();


bool IsBhrcCompleteReturningIdentityForAuthority(
    const BhrcSteamRecordCacheEntry& cached,
    uint64_t steam_id,
    const BhrcNativeExpeditionAuthorityState& authority);


BhrcExpeditionMembership ClassifyBhrcExpeditionMembership(
    uint64_t sender,
    const RevenantFixExpeditionProbeRequestPacket& packet,
    const BhrcExpeditionHostPhaseSnapshot& phase,
    uint32_t* detail_flags);


bool HandleBhrcExpeditionProbeRequest(
    uint64_t sender,
    const RevenantFixExpeditionProbeRequestPacket& packet);


bool HandleBhrcExpeditionProbeResponse(
    uint64_t sender,
    const RevenantFixExpeditionProbeResponsePacket& packet);


void AdvanceBhrcExpeditionProbe();


void MarkBhrcExpeditionProbeHandedOff(const char* reason);


bool CommitBhrcActivePreviousSessionOwnerHandoff(uint32_t expected_nonce);


bool IsBhrcActiveFriendAuthorizedStage(BhrcExpeditionProbeStage stage);


BhrcQuickMatchBinding ReadBhrcQuickMatchBinding(
    void* expected_manager);


BhrcQuickMatchAttemptEpoch SnapshotBhrcQuickMatchAttemptEpoch();


bool IsBhrcActiveFriendHandoffReady();


bool IsBhrcOldMemberRouteReady();


BhrcPendingFriendLobbyTarget SnapshotBhrcPendingFriendLobbyTarget();


void ClearBhrcPendingFriendLobbyTarget(
    uint64_t attempt_serial,
    uint64_t master_lobby_id = 0);


bool BhrcPendingFriendLobbyMatches(
    const BhrcQuickMatchAttemptEpoch& epoch,
    uint64_t master_lobby_id,
    uint64_t owner_steam_id,
    uint64_t local_steam_id);


bool ArmBhrcFriendJoinAuthorityObserverFromCommittedScan(
    void* scan_context,
    uint64_t master_lobby_id,
    uint64_t owner_steam_id,
    uint64_t local_steam_id,
    const char* reason);


bool BhrcQuickMatchEpochMatchesBinding(
    const BhrcQuickMatchAttemptEpoch& epoch,
    const BhrcQuickMatchBinding& binding);


bool BhrcQuickMatchEpochMatchesPersistentIdentity(
    const BhrcQuickMatchAttemptEpoch& epoch,
    void* expected_manager);


bool BhrcProbeForbidsGenericState7Event94(
    const BhrcExpeditionProbeAttempt& probe);


void ClearBhrcInboundSlotsForProbeAttempt(
    const BhrcExpeditionProbeAttempt& probe);


void BindBhrcFriendProbeToCurrentQuickMatchAttempt(const char* reason);


bool InvalidateBhrcQuickMatchAttemptBeforeMutatingStart(
    void* quick_match_manager,
    uint64_t active_alias_before,
    const char* reason);


void ResetBhrcActiveFriendHandoffRuntime(const char* reason);


void ObserveBhrcQuickMatchAttemptActivation(
    void* quick_match_manager,
    uint64_t active_alias_before,
    BhrcQuickMatchAttemptOrigin origin,
    const char* reason);


bool BhrcOldMemberHandoffOwnsCompetingQuickMatch(
    const BhrcOldMemberHandoffState& handoff);


bool BhrcOldMemberMatchingFirewallActiveForSnapshots(
    const BhrcOldMemberHandoffState& handoff,
    const BhrcOldMemberMatchingFence& fence);


bool BhrcOldMemberMatchingFirewallActive(
    BhrcOldMemberHandoffState* handoff_out = nullptr,
    BhrcExpeditionProbeAttempt* probe_out = nullptr,
    BhrcOldMemberMatchingFence* fence_out = nullptr);


const char* BhrcOldMemberMatchingFirewallReason(
    const BhrcOldMemberHandoffState& handoff,
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcOldMemberMatchingFence& fence);


void __fastcall HookQuickMatchUiAttemptStart(
    void* quick_match_manager, void* native_descriptor);


void __fastcall HookQuickMatchResultAttemptStart(
    void* quick_match_manager, void* native_descriptor, void* native_result);


void __fastcall HookQuickMatchAlternateAttemptStart(
    void* quick_match_manager, void* native_result);


uint32_t ReadBhrcQuickMatchControllerStep(void* controller);


NativeQuickMatchContextFinalizeFn ResolveBhrcSavedNativeQuickMatchFinalizer(
    void* hook_context);


bool HoldBhrcNrscQuickMatchFinalizer(
    void* hook_context, void* quick_match_context);


enum class BhrcFinalizerResolution : uint8_t {
  None = 0,
  Discard = 1,
  ReplayOrdinary = 2,
  CancelWithoutMatching = 3,
};

void ResolveBhrcHeldQuickMatchFinalizer();


const char* BhrcOldMemberHandoffStageName(
    BhrcOldMemberHandoffStage stage);


BhrcOldMemberHandoffState SnapshotBhrcOldMemberHandoff();


void RebindBhrcOldMemberHandoffNonce(
    const BhrcExpeditionProbeAttempt& before,
    const BhrcExpeditionProbeAttempt& after,
    const char* reason);


bool IsBhrcExecutableGameAddress(uint64_t address);


BhrcPersistentQuickMatchFacadeSnapshot
ReadBhrcPersistentQuickMatchFacadeSnapshot();


bool ValidateBhrcOldMemberAuthorityIdentity(
    const BhrcExpeditionProbeAttempt& probe,
    bool require_active_admission,
    PersistedReconnectIdentity* identity_out,
    const char** reason_out);


bool ValidateBhrcOldMemberAuthority(
    const BhrcExpeditionProbeAttempt& probe,
    bool require_active_admission,
    BhrcPersistentSessionDescriptorSnapshot* saved_out,
    const char** reason_out);


bool ValidateBhrcOwnerHandoffAuthority(
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcOldMemberHandoffState& handoff,
    bool require_active_admission,
    BhrcPersistentSessionDescriptorSnapshot* saved_out,
    const char** reason_out);

void QueueBhrcOldMemberFailClosed(
    uint32_t observer_nonce,
    const char* reason);


bool TryBeginBhrcAuthorizedOldMemberHandoff();


BhrcExistingWorldTeardownSnapshot
ReadBhrcExistingWorldTeardownSnapshot();


void ProcessBhrcAuthorizedOldMemberHandoff();


void LogNr6pfAliveFlagsIfDueMainThread();
void TraceBhrcReconnectEffectApply(void* entity, uint32_t effect_id,
                                 uint8_t mode, uint8_t result, uintptr_t caller_rva);
void __fastcall HookNativeMainGameUpdate(void* game, void* frame);
void BeginBhrcNameRecordRead(void* manager, const void* record, uint32_t type,
    uint32_t size);
void CaptureBhrcNameRecordRead(void* manager, const void* record, uint32_t type,
    void* payload, uint32_t size, int32_t original_result, int32_t result);


bool ValidateBhrcStartupTitleAuthority(
    BhrcExpeditionProbeAttempt* probe_out = nullptr);


bool EnsureBhrcStartupTitleAuthorityLatched(
    BhrcExpeditionProbeAttempt* probe_out = nullptr);


bool LoadBhrcStartupReconnectCandidate(
    PersistedReconnectIdentity* identity_out,
    uint64_t* host_candidates_out,
    uint32_t host_candidate_capacity,
    uint32_t* host_candidate_count_out);


bool ValidateBhrcOwnerHandoffAuthority(
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcOldMemberHandoffState& handoff,
    bool require_active_admission,
    BhrcPersistentSessionDescriptorSnapshot* saved_out,
    const char** reason_out);


uint64_t __fastcall HookNativeTitleOnlineModeSelector();


uint64_t* PublishBhrcNativeTitleMenuResult(
    uint64_t* out_result,
    uint32_t result,
    const char* edge);


uint64_t* InvokeBhrcNativeTitleOnlineGraphGate(
    NativeTitleMenuResultFn original,
    void* functor,
    uint64_t* out_result,
    const char* native_name,
    const char* edge);


uint64_t* __fastcall HookNativeTitleMasterOnlineGate(
    void* functor,
    uint64_t* out_result);


uint64_t* __fastcall HookNativeTitleParentOnlineGate(
    void* functor,
    uint64_t* out_result);


bool IsBhrcStartupNativeDescriptorExact(
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcNativeDescriptorSidecar* exact_sidecar,
    uint32_t* descriptor_hash_out = nullptr);


bool TryRestoreBhrcStartupNativeDescriptorExact(
    const BhrcExpeditionProbeAttempt& probe);


uint64_t* __fastcall HookNativeTitleDescriptorGate(
    void* functor,
    uint64_t* out_result);


bool IsBhrcStartupTitleProviderGraphClaimed(
    BhrcStartupHostPreflightAttempt* preflight_out = nullptr);


bool IsBhrcStartupTitleProviderGraphReleased(
    BhrcStartupHostPreflightAttempt* preflight_out = nullptr);


void RetireBhrcStartupTitleProviderGraph(
    uint32_t expected_nonce,
    const char* reason);


enum BhrcTitlePostConfirmTraceBit : LONG {
  kBhrcTitlePostConfirmTraceConfirmed = 1 << 0,
  kBhrcTitlePostConfirmTraceJob78 = 1 << 5,
  kBhrcTitlePostConfirmTraceGraphGate = 1 << 7,
  kBhrcTitlePostConfirmTraceTaskResultGate = 1 << 8,
};


bool SnapshotBhrcTitlePostConfirmTrace(
    uint32_t* nonce_out = nullptr,
    uint32_t* epoch_out = nullptr);


void ArmBhrcTitlePostConfirmTrace(uint32_t nonce);


bool ClaimBhrcTitlePostConfirmTraceBit(
    LONG bit,
    uint32_t* nonce_out = nullptr,
    uint32_t* epoch_out = nullptr);


bool HasBhrcTitlePostConfirmTraceBit(LONG bit);


void* ReadBhrcTitleContextFromGraphFunctor(void* functor);


bool ReadBhrcTitleContextTraceFields(
    const void* context,
    uint8_t* confirmed_out,
    uint32_t* state_out,
    uint8_t* disabled_out);


bool BhrcTitlePostConfirmTraceContextMatches(const void* context);


void __fastcall HookNativeTitlePositiveConfirmCallback(void* functor);


void* __fastcall HookNativeTitlePostConfirmDispatcherInvoke(
    void* functor,
    void* out_task,
    void* arg3,
    void* arg4);


bool ValidateBhrcTitlePostConfirmHostAuthority(
    void* title_context,
    uint32_t* nonce_out,
    uint32_t* epoch_out,
    uint8_t* confirmed_out,
    uint32_t* state_out,
    uint8_t* disabled_out,
    BhrcExpeditionProbeAttempt* probe_out,
    uint32_t* descriptor_hash_out);

bool ValidateBhrcStartupTitleProviderDescriptor(
    const BhrcExpeditionProbeAttempt& probe,
    uint32_t* descriptor_hash_out);

struct BhrcTitleOnlineSiblingScope {
  void* online_manager = nullptr;
  uint32_t manager_state_before = UINT32_MAX;
  uint32_t preflight_nonce = 0;
  uint32_t formal_probe_nonce = 0;
  uint64_t host_steam_id = 0;
  uint32_t descriptor_hash32 = 0;
  bool manager_state_bridged = false;
};

bool BhrcTitleOnlineSiblingTraceMayBeActive(
    LONG required_trace_bits);


bool BeginBhrcTitleOnlineSiblingScope(
    LONG required_trace_bits,
    BhrcTitleOnlineSiblingScope* scope_out);


bool EndBhrcTitleOnlineSiblingScope(
    const BhrcTitleOnlineSiblingScope& scope);


void* __fastcall HookNativeTitleJob78Builder(
    void* title_context_pair,
    void* out_task);


bool ValidateBhrcStartupTitleProviderDescriptor(
    const BhrcExpeditionProbeAttempt& probe,
    uint32_t* descriptor_hash_out);


bool ValidateBhrcTitlePostConfirmHostAuthority(
    void* title_context,
    uint32_t* nonce_out,
    uint32_t* epoch_out,
    uint8_t* confirmed_out,
    uint32_t* state_out,
    uint8_t* disabled_out,
    BhrcExpeditionProbeAttempt* probe_out,
    uint32_t* descriptor_hash_out);


bool SnapshotBhrcTitleOnlineSiblingAttempt(
    LONG required_trace_bits,
    BhrcTitleOnlineSiblingScope* scope_out);


bool ReadBhrcLocalCharacterValue(uint32_t* value_out);


void ClearBhrcTitlePlayerChrLoginBridge(
    void* expected_task = nullptr);


BhrcTitlePlayerChrLoginCompletionBridge
SnapshotBhrcTitlePlayerChrLoginBridge();


void* __fastcall HookNativeTitlePlayerChrLoginFactoryInvoke(
    void* functor,
    void* out_task);


enum class BhrcTitleOnlineTaskKind : uint8_t {
  None = 0,
  PlayerChrLogin = 1,
  ChaosRank = 2,
  ChaosRate = 3,
};

BhrcTitleOnlineTaskKind IdentifyBhrcTitleOnlineTask(
    const void* native_task);


const char* BhrcTitleOnlineTaskKindName(
    BhrcTitleOnlineTaskKind kind);


bool InvokeBhrcTitlePlayerChrLoginOriginalCompletion(
    void* native_task,
    const BhrcTitlePlayerChrLoginCompletionBridge& bridge,
    const char* source);


uint64_t* __fastcall HookNativeTitleOnlineTaskResultCopy(
    void* native_task,
    uint64_t* out_internal_result,
    const uint64_t* source_internal_result);


int __fastcall HookNativeTitlePlayerChrResultRead(
    void* result_storage,
    uint32_t* out_value);


void __fastcall HookNativeTitlePlayerChrLoginCompletion(
    void* native_task);


uint8_t* __fastcall HookNativeTitleReconnectGraphGate(
    void* title_context,
    uint8_t* out_enabled);


uint64_t* __fastcall HookNativeTitleReconnectTaskResultGate(
    void* callback_functor,
    uint64_t* out_result);


bool BhrcStartupTitleProviderScopeMatchesProbe(
    const BhrcStartupTitleProviderScopeTls& scope,
    const BhrcExpeditionProbeAttempt& probe);


const char* BhrcTitleProviderScopePhaseName(
    BhrcTitleProviderScopePhase phase);


void PopulateBhrcTitleProviderScope(
    BhrcStartupTitleProviderScopeTls* scope_out,
    BhrcTitleProviderScopePhase phase,
    const BhrcStartupHostPreflightAttempt& preflight,
    const BhrcExpeditionProbeAttempt& probe,
    uint32_t descriptor_hash32);


uint64_t* __fastcall HookNativeTitleCheckSessionSubmit(
    void* callback_functor,
    uint64_t* out_result);


uint64_t* __fastcall HookNativeTitleExistenceProviderInvoke(
    void* functor,
    uint64_t* out_result,
    void* frame_time);


uint64_t* __fastcall HookNativeTitleJob78ExistenceProviderInvoke(
    void* functor,
    uint64_t* out_result,
    void* frame_time);


void __fastcall HookNativeTitleSearchStart(void* functor);


void ClearBhrcStartupTitleCheckPenaltyBridge(
    void* expected_task = nullptr);


void __fastcall HookNativeTitleCheckPenaltySubmit(
    void* native_task,
    void* scheduler_context);


uint64_t* __fastcall HookNativeTitleCheckPenaltyResultCopy(
    void* native_task,
    uint64_t* out_internal_result,
    const uint64_t* source_internal_result);


uint64_t __fastcall HookNativeTitleCheckPenaltyPoll(
    void* native_task);


bool TrySendRevenantFixSteamTestPacket(
    uint64_t target_steam_id,
    uint16_t type,
    uint32_t nonce,
    const char* text,
    const char* reason);


void MaybeSendRevenantFixSteamTestPing(uint64_t target_steam_id, const char* reason);


void CaptureLocalReconnectIdentity(const char* reason);


uint32_t BhrcNativeEventMask(uint32_t event_id);


bool IsBhrcNativeEventPayloadSizeValid(
    uint32_t event_id,
    uint32_t payload_size);


bool IsBhrcAuthorizedClientProbe(const BhrcExpeditionProbeAttempt& probe);


bool SnapshotBhrcHostAdmissionAttempt(
    uint64_t steam_id,
    uint32_t nonce,
    BhrcHostAdmissionAttempt* out);


void MarkBhrcHostAdmissionEvent(
    uint64_t steam_id,
    uint32_t nonce,
    uint32_t event_mask,
    bool sent);


void ClearBhrcNativeAdmissionForPeer(uint64_t steam_id, const char* reason);


void ClearBhrcNativeAdmissionState(const char* reason);


bool SendBhrcGenerationPayloadToAttempt(
    const BhrcHostAdmissionAttempt& attempt,
    const BhrcNativeGenerationPayload& generation_payload,
    const char* reason);


void SendCachedBhrcGenerationPayloadsToPeer(uint64_t steam_id);


bool StoreBhrcHostGenerationPayload(
    const BhrcNativeGenerationPayload& captured,
    const char* outcome,
    const char* reason);


bool SameBhrcHostAdmissionTuple(
    const BhrcHostAdmissionAttempt& lhs,
    const BhrcHostAdmissionAttempt& rhs);


bool ReserveBhrcHostAdmissionAttempt(
    uint64_t sender,
    const RevenantFixExpeditionProbeRequestPacket& packet,
    const BhrcExpeditionHostPhaseSnapshot& phase,
    BhrcExpeditionMembership membership,
    BhrcHostAdmissionReservation* reservation);


bool CommitBhrcHostAdmissionAttempt(
    const BhrcHostAdmissionReservation& reservation,
    const BhrcExpeditionHostPhaseSnapshot& phase);


void RollbackBhrcHostAdmissionAttempt(
    const BhrcHostAdmissionReservation& reservation,
    const char* reason);


void CaptureBhrcHostNativeEvent(
    uint32_t event_id,
    uint64_t source_record_key,
    const void* payload,
    uint32_t payload_size,
    const char* reason);


void CaptureBhrcPendingHostNativeEvent94(
    void* team_state,
    const void* payload,
    uint32_t payload_size,
    const char* reason);


void PromoteBhrcPendingHostNativeEvent94();


void CaptureBhrcMigratableNativeEvent94ForGeneration(
    const void* payload,
    uint32_t payload_size,
    uint64_t master_a,
    uint64_t expedition_b,
    uint64_t local,
    uint64_t sender,
    uint64_t source_record_key,
    uint64_t packet_authority_commit_seq,
    uint32_t request_nonce,
    uint32_t client_generation,
    bool transport_authenticated,
    const char* reason);


void CaptureBhrcMigratableNativeEvent94(
    const void* payload,
    uint32_t payload_size,
    const char* reason);


bool PromoteBhrcMigratableNativeEvent94ForAuthority(
    const BhrcNativeExpeditionAuthorityState& authority);


void CaptureBhrcPendingHostNativeEvent1d(
    void* session_mgr,
    const void* payload,
    uint32_t payload_size,
    const char* reason);


void PromoteBhrcPendingHostNativeEvent1d();


uint64_t ResolveBhrcHostLiveSourceRecordKey();


uint64_t ResolveBhrcLocalLiveSourceRecordKey(const char* reason);


bool StoreBhrcInboundNativeEvent(
    uint64_t sender,
    const RevenantFixNativeEventPacket& packet,
    const char* reason);


uint64_t ValidateBhrcTeamSessionEventRecord(const void* record);


bool ConsumeBhrcInboundNativeEvent(
    uint32_t event_id,
    uint64_t source_record_key,
    void* out_payload,
    uint32_t payload_capacity,
    uint32_t* out_payload_size,
    const char* reason);


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
    const char* reason);


bool BuildLocalNativeStatusEventPayload(
    uint8_t* out_payload,
    uint32_t* out_hash,
    const char* reason);


void AdvanceBhrcNativeAdmissionTransport();


void CompleteBhrcClientAdmissionIfWorldReady();


bool ValidateDistributedRecordAuthority(
    uint64_t actual_sender,
    uint64_t header_sender,
    uint64_t packet_owner,
    uint64_t packet_lobby,
    uint32_t roster_generation,
    bool commit_roster_generation,
    const char* packet_kind);


bool HandleRecordRosterAnnounce(
    uint64_t sender,
    const RevenantFixRecordRosterAnnouncePacket& packet);


bool HandleRevenantFixNativeEvent(
    uint64_t sender,
    const RevenantFixNativeEventPacket& packet);


const char* BhrcDeferredReconnectStageName(BhrcDeferredReconnectStage stage);
BhrcDeferredReconnectIntent SnapshotBhrcDeferredReconnectIntent();
bool IsCurrentReason4CleanupIntentArmed();

bool IsBhrcDeferredIntentExactForProbe(
    const BhrcDeferredReconnectIntent& intent,
    const BhrcExpeditionProbeAttempt& probe);


bool IsBhrcLiveFriendJoinAuthorityObserver(
    const BhrcExpeditionProbeAttempt& probe);


bool RetireBhrcDeferredAuthorityTransfer(
    const BhrcExpeditionProbeAttempt& probe,
    bool completed,
    const char* reason);


void RebindBhrcDeferredAuthorityTransfer(
    const BhrcExpeditionProbeAttempt& before,
    const BhrcExpeditionProbeAttempt& after,
    const char* reason);


bool ArmReconnectIntentFromReason4Cleanup(
    void* cleanup_state, const char* trigger_reason, uint64_t* out_serial);


bool MarkReason4CleanupExited(uint64_t serial, const char* reason);


bool MarkDeferredReconnectTransferredToAuthorityProbe(
    uint64_t serial,
    const BhrcExpeditionProbeAttempt& probe,
    const char* reason);


constexpr uint32_t BhrcDeferredReconnectStageMask(
    BhrcDeferredReconnectStage stage);


bool FailDeferredReconnectIntent(
    uint64_t serial, uint32_t expected_stage_mask, const char* reason);


bool AdvanceDeferredReconnectIntent();


void BhrcTick();


bool ProcessRevenantFixSteamControlPacket(
    uint64_t actual_steam_id,
    uint32_t channel,
    const void* payload,
    uint64_t payload_size);


void ReleaseSteamNetworkingMessage(SteamNetworkingMessageLite* message);


void ProcessRevenantFixSteamControlReceives();


bool ShouldSuppressMatchedPlayersUi(uint64_t* out_lobby_id);


const char* DescribeJoinRejectedUiSuppressSkipReason();


struct BhrcNrscOwnershipSnapshot {
  uint64_t state = 0;
  uint64_t master_lobby = 0;
  uint64_t owner_steam_id = 0;
  uint64_t local_steam_id = 0;
  uint32_t active_peers = 0xffffffffu;
};

struct BhrcNativeJoinOwnershipSnapshot {
  uint64_t manager_vtable = 0;
  uint64_t operation = 0;
  uint64_t operation_handle = 0;
  uint64_t operation_callback = 0;
  uint64_t payload_lobby = 0;
  uint64_t async_call = 0;
  uint64_t owned_lobby = 0;
  uint32_t operation_token = 0;
  uint32_t payload_size = 0;
  uint32_t manager_id = 0;
  uint32_t native_state = 0xffffffffu;
  uint32_t state_874 = 0;
};

BhrcNrscOwnershipSnapshot ReadBhrcNrscOwnershipSnapshot(void* leave_hook_context = nullptr);


BhrcNativeJoinOwnershipSnapshot ReadBhrcNativeJoinOwnershipSnapshot(void* manager);


void* ResolveBhrcAuthorizedFullPgdRestoreTarget(
    const BhrcExpeditionProbeAttempt& probe,
    const BhrcNativeExpeditionAuthorityState& local_authority);


uint32_t ReadBhrcTeamRecordCount(void* state);


struct BhrcNativeTeamSessionRecordSnapshot {
  const void* record = nullptr;
  uint64_t key = 0;
  uint64_t provider_c0 = 0;
  uint64_t provider_c8 = 0;
  int32_t player_index_d0 = (-2147483647 - 1);
  int32_t record_id_d4 = (-2147483647 - 1);
  uint32_t value_d8 = UINT32_MAX;
  uint32_t value_dc = UINT32_MAX;
  uint32_t value_e8 = UINT32_MAX;
  uint8_t flag_e0 = 0xff;
  uint8_t flag_e1 = 0xff;
  uint8_t flag_e2 = 0xff;
  uint8_t flag_e3 = 0xff;
  uint8_t flag_e4 = 0xff;
  uint8_t flag_e5 = 0xff;
  bool found = false;
  bool read_fault = false;
};

struct BhrcReturningMemberLeaseDecision {
  bool retain = false;
  const char* reason = "not_evaluated";
  uint64_t lobby_id = 0;
  uint64_t steam_id = 0;
  uint32_t state_flags = 0;
  uint64_t record_key = 0;
  uint32_t current_manager_id = 0;
  void* team_state = nullptr;
  BhrcNativeExpeditionAuthorityState authority{};
  BhrcSteamRecordCacheEntry cached{};
  BhrcNativeTeamSessionRecordSnapshot member{};
};

inline constexpr uint32_t kBhrcReturningMemberRetentionCapacity = 6;

struct BhrcReturningMemberRetention {
  bool active = false;
  uint64_t armed_tick_ms = 0;
  uint64_t steam_id = 0;
  uint64_t record_key = 0;
  uint64_t expedition_lobby_id = 0;
  uint64_t master_lobby_id = 0;
  uint64_t owner_steam_id = 0;
  uint64_t authority_seq = 0;
  uint32_t manager_id = 0;
};

inline SRWLOCK g_bhrc_returning_member_retention_lock = SRWLOCK_INIT;
inline BhrcReturningMemberRetention
    g_bhrc_returning_member_retentions[kBhrcReturningMemberRetentionCapacity]{};

struct BhrcReturningMemberReclassificationValidation {
  bool eligible = false;
  bool already_accepted = false;
  const char* reason = "not_validated";
};

BhrcNativeTeamSessionRecordSnapshot ReadBhrcNativeTeamSessionRecord(
    const void* record);


BhrcNativeTeamSessionRecordSnapshot FindBhrcNativeTeamSessionRecord(
    void* team_state,
    uint64_t key);


BhrcReturningMemberRetention ArmBhrcReturningMemberRetention(
    const BhrcReturningMemberLeaseDecision& decision,
    const char* source);


bool ConsumeBhrcReturningMemberRetention(
    const BhrcReturningMemberRetention& expected);


BhrcReturningMemberRetention SnapshotBhrcReturningMemberRetention(
    uint64_t record_key);


bool HasBhrcReturningMemberRetentionCredential(
    uint64_t steam_id,
    uint64_t record_key,
    const BhrcNativeExpeditionAuthorityState& authority);


bool RefreshBhrcReturningMemberRetentionFromCompleteIdentity(
    const BhrcSteamRecordCacheEntry& identity,
    const BhrcNativeExpeditionAuthorityState& authority,
    const char* reason);


void EnsureBhrcReturningMemberRetentionForAdmission(
    uint64_t steam_id,
    uint64_t record_key,
    const BhrcNativeExpeditionAuthorityState& authority);


void CancelBhrcReturningMemberRetention(
    uint64_t steam_id,
    const char* reason);


BhrcReturningMemberReclassificationValidation
ValidateBhrcReturningMemberReclassification(
    uintptr_t caller_rva,
    void* team_state,
    uint64_t key,
    bool source_provider_read_ok,
    uint64_t source_provider_c0,
    uint64_t source_provider_c8,
    const BhrcNativeTeamSessionRecordSnapshot& before,
    const BhrcNativeTeamSessionRecordSnapshot& after,
    const BhrcReturningMemberRetention& retention);

void __fastcall HookTeamSessionCleanup(void* state, uint8_t reason);


const char* BhrcDeferredReconnectStageName(BhrcDeferredReconnectStage stage);


BhrcDeferredReconnectIntent SnapshotBhrcDeferredReconnectIntent();


bool IsCurrentReason4CleanupIntentArmed();


BhrcNativeManagerRegistryLookup ReadBhrcNativeManagerRegistry(
    void* team_session, uint32_t manager_id);


BhrcCommittedExpeditionDescriptorSnapshot
ReadCommittedNativeExpeditionDescriptor();


uint32_t ReadCurrentBhrcMapFamily();


uint32_t ReadLocalMapObjectFamilyRaw();


uint32_t ReadLocalMapObjectFamily();


void ObserveBhrcExpeditionWorldRegistration(
    void* world_chr_man,
    uint64_t entity_index,
    void* source,
    void* aux);


BhrcExpeditionHostPhaseSnapshot EvaluateBhrcExpeditionHostPhase();


BhrcNativeExpeditionOwnerEvidence SnapshotNativeExpeditionOwnerEvidence();


void ObserveNativeExpeditionSubmitEvidence(
    uintptr_t caller_rva,
    uint64_t payload_lobby,
    uint32_t payload_size,
    int operation_token,
    void* native_manager,
    const BhrcNativeJoinOwnershipSnapshot& manager);


void ObserveNativeExpeditionJoinEvidence(void* native_manager);


void ObserveNativeExpeditionD1Evidence(
    void* native_manager,
    const BhrcNativeJoinOwnershipSnapshot& manager);


void InvalidateNativeExpeditionEvidenceForManager(void* native_manager);


void PublishNativeExpeditionAuthorityCandidate(
    uintptr_t caller_rva,
    bool guest_producer,
    bool host_producer,
    void* team_session,
    uint32_t manager_id,
    const BhrcNativeManagerRegistryLookup& lookup,
    void* native_manager,
    const BhrcNativeJoinOwnershipSnapshot& manager,
    uint64_t descriptor_lobby,
    bool descriptor_lobby_valid);


void PublishNativeExpeditionGuestAuthorityAfterReady();


bool BhrcAuthorizedReturningCandidateMatchesProbe(
    const BhrcNativeExpeditionAuthorityCandidate& candidate,
    const BhrcExpeditionProbeAttempt& probe);


bool TryPublishBhrcAuthorizedReturningAuthorityCandidate();


void TryCommitPendingNativeExpeditionAuthority();

BhrcPersistentSessionDescriptorSnapshot ReadBhrcPersistentSessionDescriptorSnapshot(
    const void* descriptor_override = nullptr);


bool BhrcPersistentSessionDescriptorSnapshotsEqual(
    const BhrcPersistentSessionDescriptorSnapshot& left,
    const BhrcPersistentSessionDescriptorSnapshot& right);


uint32_t ComputeBhrcNativeDescriptorSidecarHash(
    const BhrcNativeDescriptorSidecar& sidecar);


bool CaptureBhrcNativeDescriptorSidecarValue(
    const BhrcPersistentSessionDescriptorSnapshot& descriptor,
    BhrcNativeDescriptorSidecar* out);


bool PersistBhrcNativeDescriptorSidecar(
    BhrcNativeDescriptorSidecar sidecar,
    const PersistedReconnectIdentity& identity,
    const char* source);


bool LoadBhrcNativeDescriptorSidecar(BhrcNativeDescriptorSidecar* out);


void ObserveBhrcPersistentSessionDescriptorLifecycle(const char* edge);


void QueueBhrcNativeDescriptorPersistence(
    const BhrcPersistentSessionDescriptorSnapshot& descriptor,
    const char* source);


void QueueBhrcNativeDescriptorPersistenceForIdentity(
    const PersistedReconnectIdentity& identity,
    const char* source);


void ProcessBhrcNativeDescriptorPersistence();


const char* BhrcDescriptorClearOwnerName(uintptr_t caller_rva);


bool ReadBhrcEmptyNativeDescriptorVector(
    const void* source,
    uint64_t* begin,
    uint64_t* end,
    uint64_t* capacity);


bool ShouldRetainBhrcNativeDescriptor(
    uintptr_t caller_rva,
    const void* source,
    BhrcPersistentSessionDescriptorSnapshot* current_out,
    PersistedReconnectIdentity* identity_out,
    const char** reason_out,
    uint64_t* source_begin_out,
    uint64_t* source_end_out,
    uint64_t* source_capacity_out);


bool CommitBhrcNativeDescriptorOriginal(const void* source);


void __fastcall HookNativePersistentDescriptorCommit(const void* source);

void __fastcall HookNativeBTeamSessionRecordIngress(
    void* team_state,
    const void* live_record);


uint32_t BhrcPreviousSessionBackendGateCallerBit(uintptr_t caller_rva);


bool IsBhrcOldMemberBackendGateAuthorityCurrent(
    const BhrcOldMemberBackendGateTls& gate,
    uint32_t required_online_state = 1);


uint8_t __fastcall HookNativeBackendStatusBitTest(
    void* backend_status,
    uint32_t mask);


void* __fastcall HookNativePreviousSessionFactory(
    void* online_manager,
    void* out_task,
    const uint32_t* descriptor_fields,
    const void* descriptor_string,
    uint8_t mode);


struct BhrcNativeBWrapperActivationSnapshot {
  void* session_manager = nullptr;
  void* wrapper = nullptr;
  void* backend = nullptr;
  void* provider = nullptr;
  uint32_t wrapper_state = UINT32_MAX;
  uint32_t backend_mask = UINT32_MAX;
  uint32_t provider_active_count = UINT32_MAX;
  uint8_t wrapper_enabled = 0xff;
  bool readable = false;
  bool ready = false;
};

BhrcNativeBWrapperActivationSnapshot
ReadBhrcNativeBWrapperActivationSnapshot();


void __fastcall HookNativePreviousSessionOwner(
    void* owner, const void* saved_descriptor);


void __fastcall HookNativeBState2Owner(void* activity_root);


void __fastcall HookQuickMatchGuestWaitSession(void* controller);


void __fastcall HookQuickMatchGuestReadyWait(void* controller);


void __fastcall HookNativeExpeditionDescriptorRead(
    void* team_session, void* output_bytes);


int __fastcall HookNativeOperationSubmit(
    void* registry, uint64_t selector, const void* payload,
    uint32_t payload_size, uint32_t timeout);


bool SnapshotBhrcMasterTransportPreservation(
    BhrcExpeditionProbeAttempt* probe_out,
    BhrcOldMemberHandoffState* handoff_out);


bool CommitBhrcMasterTransportPreserved(
    const BhrcExpeditionProbeAttempt& expected,
    const char* edge);


void __fastcall HookNativeManagerTeardown(void* manager);


void __fastcall HookNativeJoinLobbyBegin(void* manager);


// ---- Channel 0x49 member-verdict path (native-aligned reconnect) ----
// Protocol (from FUN_140dde430 / FUN_140df2a20 phase=1):
//   0x48 = join/member request (host receives)
//   0x49 = 4-byte verdict type for a peer:
//     0 = member ok / continue
//     1 = teardown + match cancel (b67140)
//     2 = teardown only → FUN_140df1180 (reconnect instant fail path)

bool IsBhrcKnownReturningRemoteSteamId(uint64_t steam_id);


// FUN_140df5930: guest join async success → stateC=6 / phase=1.
// FUN_140dde8a0: poll channel 0x49. Returns non-zero when a 4-byte type is ready.
// This is the exact consumer used by FUN_140df2a20 phase=1 before type=2 → df1180.
// FUN_140dde430: host-side pump. Reads 0x48 join request, decides verdict,
// sends 0x49 with type in payload dword0 (0/1/2).
// FUN_140b4f560 / FUN_140b66fb0: matching accept-table lookup used by
// FUN_140dde430. A known returning remote is admitted when the official-server
// table is unavailable under Steam P2P; ordinary and unknown peers retain the
// original result.
uint8_t __fastcall HookNativeMatchingMemberAcceptCheck(
    void* matching_facade,
    void* peer_key_ptr,
    uint32_t arg2,
    uint32_t arg3);


// Whether this manager leave targets the authorized expedition B during
// old-member handoff. Master A is never held.
bool ShouldHoldAuthorizedNativeJoinLobbyLeave(
    void* manager,
    const BhrcNativeJoinOwnershipSnapshot& ownership,
    const BhrcOldMemberHandoffState& handoff);


// FUN_142592480: explicit native LeaveLobby(manager+0x870) used by fail-clean.
// During authorized old-member handoff, hold leave of expedition B so the B
// manager can remain alive long enough for join-complete / provider binding.
// Master A leave is never held.
void __fastcall HookNativeJoinLobbyLeave(void* manager);


// Hold expedition-B fail-clean while old-member handoff is mid-flight. The
// separate exact master-A guard above this path is one-shot: it applies only
// after final PGD restore and original 0x0c publication, then retires the
// handoff so later ordinary session teardown remains native-owned.
bool ShouldHoldAuthorizedExpeditionBLeave(
    uint64_t leave_lobby_id,
    const BhrcNrscOwnershipSnapshot& ownership,
    BhrcOldMemberHandoffState* handoff_out,
    BhrcExpeditionProbeAttempt* probe_out);


bool TryConsumeBhrcMigratedMasterLeaveLease(
    uint64_t lobby_id,
    const BhrcNrscOwnershipSnapshot& nrsc_before,
    BhrcMigratedMasterLeaveLease* consumed_out);


void __fastcall HookNrscLeaveLobbyDetour(
    void* hook_context, void* steam_matchmaking, uint64_t lobby_id);


void __fastcall HookNrscQuickMatchFinalize(
    void* hook_context, void* quick_match_context);


void __fastcall HookJoinFailedMessageBoxText(int64_t* param);


uint32_t TeamSessionDialogTextIdForReason(uint8_t reason);


const char* TeamSessionDialogReasonName(uint8_t reason);


void __fastcall HookTeamSessionDialog(void* state, uint8_t reason);


bool IsBhrcReturningMemberTransportLoss(uint32_t state_flags);


BhrcReturningMemberLeaseDecision EvaluateBhrcReturningMemberLease(
    void* native_manager_hint,
    uint64_t lobby_id,
    uint64_t steam_id,
    uint32_t state_flags);


void __fastcall HookNativePeerEventType1(
    void* manager,
    const uint64_t* peer_key,
    const uint64_t* comparison_key,
    uint8_t role_flag);

uint64_t __fastcall HookNativeLobbyMemberEventQueue(
    void* queue,
    const void* event_data);


void __fastcall HookNrscNativeEvent94Fanout(
    void* hook_context,
    void* team_state,
    void* arg3,
    void* arg4,
    const void* serialized_vector);


void __fastcall HookNrscLobbyMemberEvent(void* context, uint64_t* event_data);


void __fastcall HookNrscLobbyScan(void* context, uint64_t* lobby_id);


void __fastcall HookNrscRegisterPeer(void* steam_friends, void* peer_slot, uint64_t steam_id);


void WriteAbsoluteJump(uint8_t* address, void* destination);


void WriteAbsoluteJumpViaRax(uint8_t* address, void* destination);


bool InstallInlineHook(
    InlineHook& hook,
    uintptr_t target,
    void* detour,
    size_t patch_size,
    void** original,
    const char* name);


bool InstallInlineHookRelocatedCallAtOffset9(
    InlineHook& hook,
    uintptr_t target,
    void* detour,
    void** original,
    const char* name);


bool InstallInlineHookRelocatedConditionAtOffset2(
    InlineHook& hook,
    uintptr_t target,
    void* detour,
    void** original,
    const char* name);


void RemoveInlineHook(InlineHook& hook);


struct BhrcRoundOneHookSpec {
  size_t slot = 0;
  uintptr_t rva = 0;
  const uint8_t* expected = nullptr;
  size_t patch_size = 0;
  void* detour = nullptr;
  void** original_out = nullptr;
  const char* name = nullptr;
};

bool ReadBhrcHookEntryBytes(uintptr_t address, uint8_t* out, size_t size);


void FormatBhrcHookBytes(const uint8_t* bytes, size_t size, char* out, size_t out_size);


bool InstallBhrcOldMemberHandoffAdapter();


bool InstallBhrcNativeDescriptorRetentionHook();


bool VerifyBhrcRoundOneHookEntry(
    const BhrcRoundOneHookSpec& spec,
    const char* source_anchor = kBhrcRoundOneSourceAnchor);


bool BuildRelativeCall(
    uintptr_t callsite,
    uintptr_t destination,
    uint8_t out_call[5]);


void* AllocateRelayNear(uintptr_t callsite, size_t size);


bool RemovePlayerInsControllerAuthorityCallPatch();


bool InstallPlayerInsControllerAuthorityCallPatch();


bool InstallBuddyControllerHooks();


bool InstallVerifiedBhrcNativeStatusReadHook();


bool InstallBhrcNativeEventResultHooks();


bool InstallBhrcFunctionalReconnectHooks();

bool InstallBhrcReturningEvent1dTransactionHooks();

bool InstallBhrcAutomaticMetadataHooks();
bool InstallBhrcReconnectEffectDiagnostics();

bool InstallMatchedPlayersUiHooks();


struct BhrcNrscDependencyPreflightSpec {
  size_t slot = SIZE_MAX;
  uintptr_t rva = 0;
  const uint8_t* expected = nullptr;
  size_t span = 0;
  const char* name = nullptr;
};

bool VerifyBhrcNrscDependencyEntries(uintptr_t nrsc_base);


bool InstallNrscSteamControlHooksForModule(HMODULE nrsc_module);


bool InstallBhrcNativeTitleReconnectProviderVtablePatches();


bool InstallBhrcNativeTitlePostConfirmHooks();


bool InstallBhrcActiveFriendNotificationHandoff();


DWORD WINAPI NrscSteamControlInstallThread(void*);


DWORD WINAPI RevenantFixSteamControlThread(void*);


DWORD WINAPI WorkerThread(void*);
BOOL HandleDllProcessEvent(HMODULE module, DWORD reason);


}  // namespace revenantfix::internal
