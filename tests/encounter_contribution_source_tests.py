#!/usr/bin/env python3
"""Contribution proposals must never be accidentally wired into player rewards."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
cpp = (root / "src/StrongholdEncounterContribution.cpp").read_text()
header = (root / "src/StrongholdEncounterContribution.h").read_text()
runner = (root / "tests/run-catalog-tests.sh").read_text()
docs = (root / "docs/ENCOUNTER_CONTRIBUTION.md").read_text()

assert "CandidateForStagingReview" in cpp
assert "EncounterParticipationVerified = false" in header
assert "TrophyGranted = false" in header
assert "StrongholdEncounterContribution.cpp" in runner
assert "encounter_contribution_tests.cpp" in runner
assert "human classification" in docs.lower() and "always false" in docs.lower()

for text in (cpp, header):
    for forbidden in (
        "CharacterDatabase.", "WorldDatabase.", "LoginDatabase.",
        "TeleportTo(", "SetPhaseMask(", "SaveToDB(", "SummonGameObject(",
        "SummonCreature(", "RewardQuest(", "CompleteQuest(",
        "ProposeTrophyUnlock(", "GetPlayerbotAI(", "IsHeadless()",
        "ParticipantControl::VerifiedHuman", "RaidMemberEvidence",
    ):
        assert forbidden not in text, f"Unsafe contribution contract: {forbidden}"

print("PASS: encounter action evidence is source-bound and never authorizes a trophy")
