#include "StrongholdPrivacyMatrix.h"
#include <iostream>
using namespace NaxxGuildStrongholds;
namespace
{
PrivacyMatrixInput Example()
{
    PrivacyMatrixInput v;
    v.GuildA = {12, 1800000000};
    v.GuildB = {23, 1800000001};
    v.LogicalSite=1;
    v.ExactInstalledSourceReviewed=true;
    v.StagingBuildVerified=true;
    v.OriginalGuildIdentitiesVerified=true;
    for (unsigned c=0;c<static_cast<unsigned>(PrivacyContext::Count);++c)
        for (unsigned s=0;s<static_cast<unsigned>(PrivacySurface::Count);++s)
            for (unsigned a=0;a<2;++a)
                for (unsigned b=0;b<2;++b)
                    if (a!=b || s<=2)
                        v.Observations.push_back({static_cast<PrivacyContext>(c),
                            static_cast<PrivacySurface>(s),
                            static_cast<std::uint8_t>(a),
                            static_cast<std::uint8_t>(b),true,a==b,1,1});
    return v;
}
}
int main()
{
    unsigned checks=0, failed=0;
    auto check=[&](bool yes,char const* label) {
        ++checks;
        if (!yes) { ++failed; std::cerr << "FAIL: " << label << '\n'; }
    };
    auto full=Example();
    auto result=ReviewPrivacyMatrix(full);
    check(full.Observations.size()==132,"All bidirectional and own guild controls");
    check(result.Decision==PrivacyReviewDecision::CandidateForManualStagingReview,
          "Complete synthetic set only qualifies for human review");
    check(!result.PrivacyIsolationVerified && !result.HousingEnabled,
          "No synthetic data activates real housing");
    auto v=full; v.ExactInstalledSourceReviewed=false;
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::UnverifiedSource,
          "Missing actual installed source review blocks");
    v=full; v.StagingBuildVerified=false;
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::UnverifiedSource,
          "Unverified staging build blocks");
    v=full; v.GuildB=v.GuildA;
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::InvalidGuildIdentity,
          "One guild does not prove separation");
    v=full; v.GuildB.GuildId=v.GuildA.GuildId;
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::CandidateForManualStagingReview,
          "Recycled numeric guild ID is a distinct generation");
    v=full; v.Observations.pop_back();
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::MissingCoverage,
          "Missing measurement blocks completeness");
    v=full; v.Observations.clear();
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::InvalidEvidence,
          "Empty matrix denied");
    v=full; v.Observations[0]=v.Observations[1];
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::DuplicateEvidence,
          "Duplicate observation does not satisfy another direction");
    v=full; v.Observations[0].ServerObservationAttested=false;
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::InvalidEvidence,
          "Unattested observation refused");
    v=full; v.Observations[0].Context=static_cast<PrivacyContext>(88);
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::InvalidEvidence,
          "Invalid context cannot index beyond array");
    v=full; v.Observations[0].Surface=static_cast<PrivacySurface>(88);
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::InvalidEvidence,
          "Invalid surface cannot index beyond array");
    v=full; v.Observations[0].ObserverGuild=9;
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::InvalidEvidence,
          "Invalid guild side denied");
    v=full; v.Observations[0].TargetMap=2;
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::InvalidEvidence,
          "Different physical maps do not verify same site isolation");
    v=full;
    for (auto& o:v.Observations)
        if (o.ObserverGuild!=o.TargetGuild &&
            o.Context==PrivacyContext::Playerbots &&
            o.Surface==PrivacySurface::Pet)
        {o.VisibleOrInteractive=true;break;}
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::CrossGuildLeak,
          "Cross-guild Playerbot pet exposure blocked");
    v=full;
    for (auto& o:v.Observations)
        if (o.ObserverGuild!=o.TargetGuild && o.Surface==PrivacySurface::Gossip)
        {o.VisibleOrInteractive=true;break;}
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::CrossGuildLeak,
          "Foreign guild gossip vulnerability blocks");
    v=full;
    for (auto& o:v.Observations)
        if (o.ObserverGuild==o.TargetGuild && o.Surface==PrivacySurface::GameObject)
        {o.VisibleOrInteractive=false;break;}
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::OwnGuildHidden,
          "Own guild decorations must be visible");
    v=full;v.Observations.pop_back();
    for (auto& o:v.Observations)
        if (o.ObserverGuild!=o.TargetGuild){o.VisibleOrInteractive=true;break;}
    check(ReviewPrivacyMatrix(v).Decision==PrivacyReviewDecision::CrossGuildLeak,
          "Observed privacy leak outranks missing coverage");
    if(failed) return 1;
    std::cout<<"PASS: "<<checks<<" synthetic privacy matrix safety tests\n";
    return 0;
}
