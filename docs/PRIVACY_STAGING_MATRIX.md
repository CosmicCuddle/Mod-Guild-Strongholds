# Two-guild privacy acceptance matrix — offline contract

**Not playable, not a certified privacy mechanism.** The earlier phase model only simulated one visibility rule. A real shared settlement must also avoid cross-guild NPC gossip, object use, summons, auras, combat and Playerbot interaction.

The new review model calls for **132 observations** over six test contexts: baseline, mixed Individual Progression tiers, Playerbots, relog, worldserver restart and guild membership change.

For each context, test eight cross-guild surfaces (player, NPC, gameobject, pet/summon, gossip, object use, aura and combat), **A-to-B and B-to-A**. Also positively confirm own-guild player, NPC and gameobject visibility for each guild. Same-guild services may legitimately be locked by personal IP, so own gossip/quests aren't mandatory positive controls.

The matrix requires independently attested exact deployed source, a staging build and separate original guild generations. Incomplete, duplicate, malformed, unreviewed or contradictory observations are rejected. Any known cross-guild leak overrides missing coverage and rejects the candidate.

**All 132 matching synthetic records still produce only CandidateForManualStagingReview.** No possible result sets PrivacyIsolationVerified or HousingEnabled true, grants guild access or changes any phase. Operator attestation is not a security proof; the exact deployed fork, other modules and manual in-world reproduction need separate verification.

It does not yet cover collision/LOS, logout emergency safe returns, guild disband evacuation, exact phase restoration, IP services, module uninstall or server load limits. These are independent release blockers.

Test using the existing test shell runner and source guard. Revert the development commit to remove this entirely; it installs no SQL, NPC, GO, assets or gameplay scripts.
