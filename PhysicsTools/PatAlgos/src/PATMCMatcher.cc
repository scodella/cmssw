#include "PhysicsTools/PatAlgos/interface/PATCandMatcher.h"
#include "PhysicsTools/PatAlgos/interface/PATMatchSelector.h"
#include "PhysicsTools/PatUtils/interface/PATMatchByDRDPt.h"
#include "PhysicsTools/PatUtils/interface/PATMatchLessByDPt.h"

#include "DataFormats/JetReco/interface/GenJetfwd.h"
#include "DataFormats/JetReco/interface/GenJet.h"

// Match by deltaR and deltaPt, ranking by deltaR (default)
typedef pat::PATCandMatcher<
  reco::CandidateView,
  reco::CandidateCollection,
  pat::PATMatchSelector<reco::Candidate>,
  pat::PATMatchByDRDPt<reco::CandidateView::value_type,
		    reco::CandidateCollection::value_type>
> PATMCMatcher;

// Alternative: match by deltaR and deltaPt, ranking by deltaPt
typedef pat::PATCandMatcher<
   reco::CandidateView,
   reco::CandidateCollection,
   pat::PATMatchSelector<reco::Candidate>,
   pat::PATMatchByDRDPt<reco::CandidateView::value_type,
 		    reco::CandidateCollection::value_type>,
   pat::PATMatchLessByDPt<reco::CandidateView,
 			 reco::CandidateCollection>
 > PATMCMatcherByPt;

// Alternative: pure deltaR matching (explicit) & sorting (default)
// typedef pat::PATCandMatcher<
//   reco::CandidateView,
//   reco::CandidateCollection,
//   helpers::MCTruthPairSelector<reco::Candidate>,
//   pat::PATMatchByDR<reco::CandidateView::value_type,
// 		    reco::CandidateCollection::value_type>
// > PATMCMatcher;

// or, using also the default (deltaR) for the match
// typedef pat::PATCandMatcher<
//   reco::CandidateView,
//   reco::CandidateCollection,
//   helpers::MCTruthPairSelector<reco::Candidate>
// > PATMCMatcher;

// JET Match by deltaR, ranking by deltaR (default)
typedef pat::PATCandMatcher<
  reco::CandidateView,
  reco::GenJetCollection,
  pat::PATMatchSelector<reco::Candidate>,
  pat::PATMatchByDR<reco::CandidateView::value_type,
		    reco::CandidateCollection::value_type>
> PATGenJetMatcher;




#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE( PATMCMatcher );
DEFINE_FWK_MODULE( PATMCMatcherByPt );
DEFINE_FWK_MODULE( PATGenJetMatcher );

