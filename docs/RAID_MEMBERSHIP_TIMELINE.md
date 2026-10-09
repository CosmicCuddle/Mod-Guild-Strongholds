# Raid membership windows — source-independent prototype

Status: pure C++ historical roster policy, **not real AzerothCore membership or human verification**.

A member can join a 40-person raid late, leave before a boss dies, rejoin after a wipe, switch groups or move guild. A final group roster alone must not imply that all characters actively completed the encounter.

Each synthetic membership window includes a server attempt token, map, instance, player GUID, server group token, original guild ID+creation timestamp, join and departure times and source-review flags. Windows are half-open: a departure exactly at 4000 means no membership at 4000; a new join at 4000 begins immediately. Zero departure time represents a still-open interval.

The policy rejects impossible times, missing server source flags, foreign encounter attempt or instance, overlapping/duplicated windows, absent members, gap activity, group switches and incorrect original guild generations. It allows adjacent leave/rejoin intervals and 40-person mixed guild simulations. Its cap of 160 historical records is a source-data safety bound, not a raid design decision.

Even if the synthetic timeline appears perfect, a result is only CandidateForStagingReview. MemberAtEventConfirmed, HumanControlConfirmed and TrophyGranted are **always false**. It is **never a human classifier**, a runtime guild/group source, a player-activity verifier or a reward authorisation. Do not set the existing StrongholdRaidParticipation proof booleans from this candidate.

The next step still needs the exact deployed AzerothCore/Playerbots/IP modules reviewed, real event-time group and original guild records, encounter attempt reset semantics and 40-player mixed guild staging. See RAID_SOURCE_PREFLIGHT.md and RAID_PARTICIPATION.md. No server configuration, SQL, NPC, GO or player is touched.

Test with the existing C++ runner plus tests/raid_membership_timeline_source_tests.py. Revert the development commit to remove the prototype; it owns no production data.
