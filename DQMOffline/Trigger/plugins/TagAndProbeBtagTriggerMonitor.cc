#include <cmath>

#include "FWCore/Framework/interface/EventSetup.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "DQMOffline/Trigger/plugins/TagAndProbeBtagTriggerMonitor.h"

//#include "TLorentzVector.h"

// -----------------------------
//  constructors and destructor
// -----------------------------

TagAndProbeBtagTriggerMonitor::TagAndProbeBtagTriggerMonitor(const edm::ParameterSet& iConfig) {
  folderName_ = iConfig.getParameter<std::string>("dirname");
  processname_ = iConfig.getParameter<std::string>("processname");
  triggerobjbtag_ = iConfig.getParameter<std::string>("triggerobjbtag");
  jetPtmin_ = iConfig.getParameter<double>("jetPtMin");
  jetEtamax_ = iConfig.getParameter<double>("jetEtaMax");
  tagBtagmin_ = iConfig.getParameter<double>("tagBtagMin");
  probeBtagmin_ = iConfig.getParameter<double>("probeBtagMin");
  triggerSummaryLabel_ = iConfig.getParameter<edm::InputTag>("triggerSummary");
  triggerSummaryToken_ = consumes<trigger::TriggerEvent>(triggerSummaryLabel_);
  offlineBtagToken_ = consumes<reco::JetTagCollection>(iConfig.getParameter<edm::InputTag>("offlineBtag"));
  genTriggerEventFlag_ = new GenericTriggerEventFlag(
      iConfig.getParameter<edm::ParameterSet>("genericTriggerEventPSet"), consumesCollector(), *this);

  jetPtbins_ = iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<std::vector<double> >("jetPt");
  jetEtabins_ = iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<std::vector<double> >("jetEta");
  jetPhibins_ = iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<std::vector<double> >("jetPhi");
  jetBtagbins_ = iConfig.getParameter<edm::ParameterSet>("histoPSet").getParameter<std::vector<double> >("jetBtag");
}

TagAndProbeBtagTriggerMonitor::~TagAndProbeBtagTriggerMonitor() {
  if (genTriggerEventFlag_)
    delete genTriggerEventFlag_;
}

void TagAndProbeBtagTriggerMonitor::bookHistograms(DQMStore::IBooker& ibooker,
                                                   edm::Run const& iRun,
                                                   edm::EventSetup const& iSetup) {
  std::string currentFolder = folderName_;
  ibooker.setCurrentFolder(currentFolder);

  int ptnbins = jetPtbins_.size() - 1;
  int etanbins = jetEtabins_.size() - 1;
  int phinbins = jetPhibins_.size() - 1;
  int btagnbins = jetBtagbins_.size() - 1;

  std::vector<float> fptbins(jetPtbins_.begin(), jetPtbins_.end());
  std::vector<float> fetabins(jetEtabins_.begin(), jetEtabins_.end());
  std::vector<float> fphibins(jetPhibins_.begin(), jetPhibins_.end());
  std::vector<float> fbtagbins(jetBtagbins_.begin(), jetBtagbins_.end());

  float* ptbins = &fptbins[0];
  float* etabins = &fetabins[0];
  float* phibins = &fphibins[0];
  float* btagbins = &fbtagbins[0];

  pt_jet1_ = ibooker.book1D("pt_jet1", "pt_jet1", ptnbins, ptbins);
  pt_jet2_ = ibooker.book1D("pt_jet2", "pt_jet2", ptnbins, ptbins);
  eta_jet1_ = ibooker.book1D("eta_jet1", "eta_jet1", etanbins, etabins);
  eta_jet2_ = ibooker.book1D("eta_jet2", "eta_jet2", etanbins, etabins);
  phi_jet1_ = ibooker.book1D("phi_jet1", "phi_jet1", phinbins, phibins);
  phi_jet2_ = ibooker.book1D("phi_jet2", "phi_jet2", phinbins, phibins);
  eta_phi_jet1_ = ibooker.book2D("eta_phi_jet1", "eta_phi_jet1", etanbins, etabins, phinbins, phibins);
  eta_phi_jet2_ = ibooker.book2D("eta_phi_jet2", "eta_phi_jet2", etanbins, etabins, phinbins, phibins);

  pt_probe_ = ibooker.book1D("pt_probe", "pt_probe", ptnbins, ptbins);
  pt_probe_match_ = ibooker.book1D("pt_probe_match", "pt_probe_match", ptnbins, ptbins);
  eta_probe_ = ibooker.book1D("eta_probe", "eta_probe", etanbins, etabins);
  eta_probe_match_ = ibooker.book1D("eta_probe_match", "eta_probe_match", etanbins, etabins);
  phi_probe_ = ibooker.book1D("phi_probe", "phi_probe", phinbins, phibins);
  phi_probe_match_ = ibooker.book1D("phi_probe_match", "phi_probe_match", phinbins, phibins);
  eta_phi_probe_ = ibooker.book2D("eta_phi_probe", "eta_phi_probe", etanbins, etabins, phinbins, phibins);
  eta_phi_probe_match_ = ibooker.book2D("eta_phi_probe_match", "eta_phi_match", etanbins, etabins, phinbins, phibins);

  discr_offline_btag_jet1_ = ibooker.book1D("discr_offline_btag_jet1", "discr_offline_btag_jet1", btagnbins, btagbins);
  discr_offline_btag_jet2_ = ibooker.book1D("discr_offline_btag_jet2", "discr_offline_btag_jet2", btagnbins, btagbins);

  // Initialize the GenericTriggerEventFlag
  if (genTriggerEventFlag_ && genTriggerEventFlag_->on())
    genTriggerEventFlag_->initRun(iRun, iSetup);
}

void TagAndProbeBtagTriggerMonitor::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup) {
  //    bool accept = false;
  bool match1 = false;
  bool match2 = false;

  edm::Handle<reco::JetTagCollection> offlineJetTagPFHandler;
  iEvent.getByToken(offlineBtagToken_, offlineJetTagPFHandler);

  if (!offlineJetTagPFHandler.isValid())
    return;

  // applying selection for event; tag & probe -> selection  for all events
  if (genTriggerEventFlag_->on() && !genTriggerEventFlag_->accept(iEvent, iSetup)) {
    reco::JetTagCollection jettags = *(offlineJetTagPFHandler.product());
    if (jettags.size() > 1) {
      const reco::Jet jet1 = *(jettags.key(0).get());
      const reco::Jet jet2 = *(jettags.key(1).get());

      float btag1 = jettags.value(0);
      float btag2 = jettags.value(1);

      if (jet1.pt() > jetPtmin_ && jet2.pt() > jetPtmin_ && fabs(jet1.eta()) < jetEtamax_ &&
          fabs(jet2.eta()) < jetEtamax_) {
        if (btag1 > tagBtagmin_ && btag2 > probeBtagmin_) {
          pt_jet1_->Fill(jet1.pt());
          pt_jet2_->Fill(jet2.pt());
          eta_jet1_->Fill(jet1.eta());
          eta_jet2_->Fill(jet2.eta());
          phi_jet1_->Fill(jet1.phi());
          phi_jet2_->Fill(jet2.phi());
          eta_phi_jet1_->Fill(jet1.eta(), jet1.phi());
          eta_phi_jet2_->Fill(jet2.eta(), jet2.phi());
          if (btag1 < 0)
            btag1 = -0.5;
          if (btag2 < 0)
            btag2 = -0.5;
          discr_offline_btag_jet1_->Fill(btag1);
          discr_offline_btag_jet2_->Fill(btag2);

          // trigger objects matching
          std::vector<trigger::TriggerObject> onlinebtags;
          edm::Handle<trigger::TriggerEvent> triggerEventHandler;
          iEvent.getByToken(triggerSummaryToken_, triggerEventHandler);
          const unsigned int filterIndex(
              triggerEventHandler->filterIndex(edm::InputTag(triggerobjbtag_, "", processname_)));
          if (filterIndex < triggerEventHandler->sizeFilters()) {
            const trigger::Keys& keys(triggerEventHandler->filterKeys(filterIndex));
            const trigger::TriggerObjectCollection& triggerObjects = triggerEventHandler->getObjects();
            for (auto& key : keys) {
              onlinebtags.reserve(onlinebtags.size() + keys.size());
              onlinebtags.push_back(triggerObjects[key]);
            }
          }
          for (auto const& to : onlinebtags) {
            if (reco::deltaR(jet1, to))
              match1 = true;
            if (reco::deltaR(jet2, to))
              match2 = true;
          }

          if (match1)  // jet1 is the tag
          {
            pt_probe_->Fill(jet2.pt());
            eta_probe_->Fill(jet2.eta());
            phi_probe_->Fill(jet2.phi());
            eta_phi_probe_->Fill(jet2.eta(), jet2.phi());
            if (match2)  // jet2 is the probe
            {
              pt_probe_match_->Fill(jet2.pt());
              eta_probe_match_->Fill(jet2.eta());
              phi_probe_match_->Fill(jet2.phi());
              eta_phi_probe_match_->Fill(jet2.eta(), jet2.phi());
            }
          }
        }  // offline jets btag
      }    // offline jets kinematic selection
    }      // at least two offline jets
  }        // accept trigger
}
void TagAndProbeBtagTriggerMonitor::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {}

// Define this as a plug-in
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(TagAndProbeBtagTriggerMonitor);
