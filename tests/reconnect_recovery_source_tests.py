#!/usr/bin/env python3
"""Reconnect policy must not introduce a live SQL/delete/teleport pathway."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
head=(root/"src/StrongholdVisitRecovery.h").read_text()
cpp=(root/"src/StrongholdVisitRecovery.cpp").read_text()
tests=(root/"tests/visit_recovery_tests.cpp").read_text()
assert "RecoveryResumeDecision ReviewInterruptedVisit" in cpp
assert "CandidateRetryReturning" in head
assert "CandidateReviewCompletedReturn" in tests
assert "InvalidTicket" in cpp
for forbidden in ("CharacterDatabase.", "WorldDatabase.", "LoginDatabase.",
                  "SaveToDB(", "SetPhaseMask(", "DELETE FROM ",
                  "SummonCreature(", "SummonGameObject("):
    assert forbidden not in cpp, forbidden
print("PASS: crash-resume review cannot mutate world or clear a visit ticket")
