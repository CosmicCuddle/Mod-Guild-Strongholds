# Staging Playerbots source reader — confirmed bots only

**Status:** C++ read-only optional source adapter + independent public-source compile check. No live install, human proof, raid trophies or housing gameplay.

## Why this is one-way

In the reviewed public mod-playerbots source commit `037c01418b5d01506917a3db9b44fd56ac5f965c`:

- `sPlayerbotsMgr.GetPlayerbotAI(player)` accesses the bot-AI map and returns an AI only when its registry entry is actually bot AI.
- The same method returns **nullptr** if `sPlayerbotAIConfig.enabled` is false, if `player` is null or if there is no matching AI record.
- Thus **nonnull bot AI** can confirm an actively registered bot, but **nullptr absolutely cannot prove human control**.

There may be account-controlled alts, random bots, headless players and customised session logic on the server. No negative lookup or session-style heuristic is allowed to become `VerifiedHuman`.

## Staging opt-ins (all OFF by default)

This integration compiles only with:
1. `NAXX_GS_BUILD_STAGING_RAID_OBSERVER` — the existing read-only death/kill-credit observer.
2. `NAXX_GS_BUILD_STAGING_PLAYERBOTS_READ` — separate opt-in requiring the reviewed public Playerbots headers.

If opted in for a **backed-up isolated staging realm**, the observer also requires its own default-off `NaxxGuildStrongholds.StagingRaidObserver.Enabled=1`, while the Playerbots reader requires both:

- `NaxxGuildStrongholds.StagingPlayerbotsRead.Enabled=1`
- `NaxxGuildStrongholds.StagingPlayerbotsRead.ReviewedSource=1`

The final setting is a manual declaration that the operator has examined the installed fork's implementation. **It is not a safe automatic SHA match or proof that human classification is correct.** Until that source review, leave it off.

## Safe anonymous staging output

When an AzerothCore kill-credit *candidate* passes map/instance/boss-flag filtering, the optional reader may log:

- `BOT_CONFIRMED` when the reviewed active bot registry positively matched the recipient;
- `UNKNOWN_NOT_HUMAN_PROOF` otherwise.

No account, character name, player GUID, IP, guild, group or loot is recorded. Neither result proves real personal encounter participation; the logger never constructs a `TrophyKillProof`, passes `PlayerbotsSourceVerified=true`, grants a guild trophy, executes database queries or summons game objects.

The existing `ReviewRaidParticipation` remains a **pure policy** requiring independently audited human control, original guild generation, group membership, same-instance participation and actual contribution during the encounter. This reader cannot meet those requirements.

## Testing and boundaries

`tests/playerbots_read_tests.cpp` exhaustively validates all 64 combinations of config, source attestation, module enabled, in-world character, registry query and matched bot AI. Only all six true produces `VerifiedPlayerbot`; **zero cases produce `VerifiedHuman`**.

`tests/playerbots_read_source_tests.py` forbids world/database/reward actions and checks config and compile guards; the pinned-public Playerbots source contract test verifies actual header and registry semantics. A fourth upstream C++ compile variant includes reviewed bot source headers and confirms the opt-in read symbol exists only there.

The CI build compiles public source, not the user's customised Naxxramas core and Playerbots. It does **not** link or start a Playerbots-enabled worldserver, nor prove any participant controls an account without a bot AI. Those are future installed-fork staging requirements.

**No SQL, quests, trophies, housing, Playerbots AI or other installed modules have been modified.**
