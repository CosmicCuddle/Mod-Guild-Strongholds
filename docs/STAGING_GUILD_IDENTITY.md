# Staging-only authoritative AzerothCore Guild identity read

**State:** Real upstream-API C++ adapter + pure source validation tests. Not a production guild property feature. No database writes, automatic NPC spawn or housing release.

## The exact verified fields

A future real Guild Steward must not assume `Player::GetGuildId()` means it owns a stronghold. Guild IDs can be reused and guild membership can change.

The staging-only `src/StrongholdStagingGuildAdapter.cpp` checks:
1. `Player::GetGuildId() != 0`: character reports a guild.
2. `Player::GetGuild()` resolves an actual AzerothCore `Guild` object.
3. `Guild::GetId() == Player::GetGuildId()`: registry agrees with player ID.
4. `Guild::GetMember(player->GetGUID()) != nullptr`: the exact character is currently registered as a member.
5. `Guild::GetCreatedDate() > 0`: the guild's original creation timestamp exists.

Only then does `VerifyGuildIdentity` return `GuildIdentity {GuildId, CreatedAt}` and a verified status. Else it returns no trusted identity. Every gossip selection rechecks the latest state, including after kick/leave/disband or GM-mode changes.

The verified guild generation and membership appear in the **Guild and IP Evidence** page as the original creation timestamp. The page still states **property ownership NOT verified**. No character stronghold, level, IP rank, trophy, guild supply or private housing data is loaded.

## Source and runtime boundaries

The exact API names were inspected on **public AzerothCore** `7b2cecef92b271a468e39d89831b520b20ae06a8`:
- `src/server/game/Entities/Player/Player.h`
- `src/server/game/Guilds/Guild.h`

The actual installed Naxxramas fork may have different APIs or hooks. On staging, the build must include every installed module, Playerbots and Individual Progression, and verify `GetGuild()` semantics are unchanged.

The direct adapter only exists in C++ when `NAXX_GS_BUILD_STAGING_STEWARD` is explicitly defined; runtime `NaxxGuildStrongholds.StagingSteward.Enabled=0` stays default. Normal builds do not register any steward or attach this adapter to a WorldScript. In fact, no creature entry or NPC spawn SQL has been created.

The source-only CI test refuses use of suspicious state mutators such as `SetGuildId`, `TeleportTo`, `SetPhaseMask`, world/character DB operations and quest changes. The C++ pure tests cover stale guild identity, missing registry/member and unknown creation stamps. These tests are helpful but not substitutes for full integration and security review.

## Still missing for any housing

- Backed-up, fork-specific staging source/asset copy and successful **whole-worldserver** startup.
- Authoritative read-only character and module version compatible Individual Progression stage/cap adapter.
- Actual `naxx_gs_settlement` lookup, original owner generation matching, active/archived state and verified settlement level, with transactional C++ access.
- Working private per-guild map/phase destination, two-guild visibility tests, safe teleport return and Playerbots compatibility.
- Staged, reversible owned world assets, NPC entry/spawn, real guild claims and progression quests.
- Preservation-first uninstall/reinstall and mandatory source/database backups.

**A verified guild member and creation date alone must NEVER enable housing.**
