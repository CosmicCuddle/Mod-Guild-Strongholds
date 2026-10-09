#ifndef NAXX_GUILD_STRONGHOLDS_PRIVACY_MATRIX_H
#define NAXX_GUILD_STRONGHOLDS_PRIVACY_MATRIX_H
#include "StrongholdLifecycle.h"
#include <cstdint>
#include <vector>
namespace NaxxGuildStrongholds
{
// Only a staging *review coverage* model; not proof or a runtime adapter.
enum class PrivacyContext : std::uint8_t
{
    Baseline, MixedIpTiers, Playerbots, Relog, Restart, GuildChanged, Count
};
enum class PrivacySurface : std::uint8_t
{
    Player, Npc, GameObject, Pet, Gossip, ObjectUse, Aura, Combat, Count
};
struct PrivacyObservation
{
    PrivacyContext Context = PrivacyContext::Baseline;
    PrivacySurface Surface = PrivacySurface::Player;
    std::uint8_t ObserverGuild = 0; // 0=A, 1=B
    std::uint8_t TargetGuild = 1;
    bool ServerObservationAttested = false;
    bool VisibleOrInteractive = false;
    std::uint32_t ObserverMap = 0;
    std::uint32_t TargetMap = 0;
};
struct PrivacyMatrixInput
{
    GuildIdentity GuildA, GuildB;
    std::uint32_t LogicalSite = 0;
    bool ExactInstalledSourceReviewed = false;
    bool StagingBuildVerified = false;
    bool OriginalGuildIdentitiesVerified = false;
    std::vector<PrivacyObservation> Observations;
};
enum class PrivacyReviewDecision : std::uint8_t
{
    UnverifiedSource, InvalidGuildIdentity, InvalidEvidence,
    DuplicateEvidence, CrossGuildLeak, OwnGuildHidden,
    MissingCoverage, CandidateForManualStagingReview
};
struct PrivacyReviewResult
{
    PrivacyReviewDecision Decision = PrivacyReviewDecision::UnverifiedSource;
    bool PrivacyIsolationVerified = false;
    bool HousingEnabled = false;
};
PrivacyReviewResult ReviewPrivacyMatrix(PrivacyMatrixInput const& input);
}
#endif
