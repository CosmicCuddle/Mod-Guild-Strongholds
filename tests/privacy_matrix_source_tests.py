#!/usr/bin/env python3
"""Never allow synthetic privacy checks to enable world access."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
head=(root/"src/StrongholdPrivacyMatrix.h").read_text()
cpp=(root/"src/StrongholdPrivacyMatrix.cpp").read_text()
runner=(root/"tests/run-catalog-tests.sh").read_text()
assert "PrivacyIsolationVerified = false" in head
assert "HousingEnabled = false" in head
assert "CandidateForManualStagingReview" in cpp
assert "CrossGuildLeak" in cpp
assert "StrongholdPrivacyMatrix.cpp" in runner
for text in (head,cpp):
    for mutation in ("SetPhaseMask(", "TeleportTo(", "SaveToDB(",
                     "CharacterDatabase.", "WorldDatabase.", "SummonCreature(",
                     "SummonGameObject(", "DevelopmentCapabilities.PrivacyIsolation ="):
        assert mutation not in text, mutation
print("PASS: offline privacy matrix cannot enable a stronghold")
