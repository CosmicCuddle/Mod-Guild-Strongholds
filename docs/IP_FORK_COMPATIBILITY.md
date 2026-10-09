# Grimfeather Individual Progression — reviewed source compatibility

**Research checkpoint: 8 October 2026. No runtime compatibility is claimed.**

This is the actual public fork supplied for this server:
- Repository: [Grimfeather/mod-individual-progression](https://github.com/Grimfeather/mod-individual-progression)
- Reviewed public commit: [706740808fee328b8557607f87b0548cf961e047](https://github.com/Grimfeather/mod-individual-progression/commit/706740808fee328b8557607f87b0548cf961e047)
- Parent: `ZhengPeiRu21/mod-individual-progression`
- The **deployed server checkout** and installed source revision remain **NOT VERIFIED**.

## What the fork really does

The public header declares:
- `GetPlayerProgressionFromQuests(Player*) const` — read-only observer of awarded hidden progression quests.
- `hasPassedProgression(Player*, ProgressionState) const` — checks `enabled` and `progressionLimit`, **then** the character's rewarded-quest state.
- `enabled` and `progressionLimit` are public members.
- `PROGRESSION_PRE_TBC = 8`, `PROGRESSION_TBC_TIER_5 = 13` (pre-Wrath), and `PROGRESSION_WOTLK_TIER_3 = 16`.
- IP phase spell IDs `89509, 89511, 89513, 89515, 89517, 89519`.

The implementation's raw getter loops through rewarded hidden quests `66000 + stage` and retains the maximum rewarded stage. `hasPassedProgression` additionally checks the module enable and progression limit. The fork also responds to `OnPlayerUpdateZone` and `OnPlayerUpdateArea`, recasting phase spells through `checkIPPhasing`.

**Critical finding:** A high awarded quest rank does **not** imply a higher era is currently open if `progressionLimit` is smaller. Guild Strongholds must never offer an Outland or Northrend service based on raw stage alone.

## Effective stage contract

`StrongholdIpCompatibility.h/.cpp` requires a source-verified snapshot of:

- actual IP module enabled status;
- player is genuinely in-world;
- raw rewarded-quest progression stage from IP's public getter;
- actual configured `progressionLimit` (0 = uncapped in reviewed fork);
- verified source API/behavior matched to the current deployed revision.

If valid, effective stage is the lower of raw stage and positive cap. Otherwise eligibility is unverified, even for a rank-zero guild activity. IP cap changes must be reflected immediately at the next activity check.

Example: character's stored quest stage = 18, cap = 7. Effective rank is 7; `PROGRESSION_PRE_TBC=8` activities stay locked.

No direct live adapter exists yet. Never call IP's `UpdateProgressionState` or `ForceUpdateProgressionState` to gain activity access. Never update rewarded hidden quest IDs, IP auras, global phase flags, or world IP quest rows.

## Source validation on CI

A new GitHub Actions job checks out the public fork at its **immutable reviewed commit**, not its moving `master`, then executes:

```bash
python3 scripts/check-grimfeather-ip-contract.py reviewed-ip-source --expect-revision 706740808fee328b8557607f87b0548cf961e047
```

The script is read-only. It checks the public enum values, phase spell IDs, getters/flags, rewarded quest loop, off-world guard, module enabled/limit guard, effective comparison, and zone/area phasing hook presence. It rejects changed semantics, but matching text **does not prove** the fork is safely compiled/linked with Guild Strongholds. Negative tests simulate source changes to ensure accidental bypasses are detected.

When inspecting the real server's source checkout later, the same script can examine it, but **successful shape checks are not enough**: an exact deployed SHA, module load/config state, real core build and functional staging tests remain mandatory.

## Compatibility requirements before implementation

1. Record the actually deployed IP Git SHA (which may differ from the reviewed public commit), AzerothCore fork SHA, Playerbots source revision and every other module.
2. Confirm public IP header/signatures and correct source inclusion/link order on the deployed build. **Never overwrite any IP files** to make an adapter compile.
3. Verify actual IP `enabled`, `progressionLimit`, rewarded quest stage and phase spells for two characters at distinct tiers.
4. Test global cap changes when characters already have greater awarded quest progress.
5. Validate mixed-era guild services, quests, NPC menus, shared scenery and no extra progression unlocks.
6. Test IP phase restoration after entry/exit, logout, realm restart, zone change, encounter death and player summon.
7. Block the feature if any actual fork mismatch, phasing regression or Playerbot exploit is found.

**Status:** Source research and standalone policy/tests ready; real integration, actual private space, and installed-module validation NOT READY.
