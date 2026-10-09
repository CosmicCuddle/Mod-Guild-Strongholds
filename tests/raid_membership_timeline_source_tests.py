#!/usr/bin/env python3
"""Keep proposed roster interval history free of executable reward effects."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
header = (root / "src/StrongholdRaidMembershipTimeline.h").read_text()
cpp = (root / "src/StrongholdRaidMembershipTimeline.cpp").read_text()
runner = (root / "tests/run-catalog-tests.sh").read_text()
docs = (root / "docs/RAID_MEMBERSHIP_TIMELINE.md").read_text()
assert "MemberAtEventConfirmed = false" in header
assert "HumanControlConfirmed = false" in header
assert "TrophyGranted = false" in header
assert "CandidateForStagingReview" in cpp
assert "StrongholdRaidMembershipTimeline.cpp" in runner
assert "raid_membership_timeline_tests.cpp" in runner
assert "never a human classifier" in docs.lower()
for text in (header, cpp):
    for denied in ("CharacterDatabase.", "WorldDatabase.", "LoginDatabase.",
                   "SaveToDB(", "TeleportTo(", "SetPhaseMask(",
                   "SummonCreature(", "SummonGameObject(", "RewardQuest(",
                   "ProposeTrophyUnlock(", "GetPlayerbotAI(",
                   "ParticipantControl::VerifiedHuman", "RaidMemberEvidence"):
        assert denied not in text, denied
print("PASS: offline raid membership cannot grant any real gameplay proof")
