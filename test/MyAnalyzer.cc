// -*- C++ -*-
//
// Package:    MyAnalyzer
// Class:      MyAnalyzer
//
/**\class MyAnalyzer MyAnalyzer.cc MyAnalysis/MyAnalyzer/src/MyAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Dinko Ferencek
//         Created:  Mon Sep 12 15:06:41 CDT 2011
// $Id: MyAnalyzer.cc,v 1.3 2011/10/06 01:03:59 ferencek Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Common/interface/MergeableCounter.h"

// BaseClass
#include "MyAnalysis/MyAnalyzer/interface/BaseClass.h"

// TFileService
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// ROOT
#include <TH1D.h>
#include <TH2D.h>
#include <TLorentzVector.h>

using namespace std;

//
// class declaration
//

class MyAnalyzer : public BaseClass, public edm::EDFilter {
   public:
      explicit MyAnalyzer(const edm::ParameterSet&);
      ~MyAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginJob() ;
      virtual bool filter(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      virtual bool beginRun(edm::Run&, edm::EventSetup const&);
      virtual bool endRun(edm::Run&, edm::EventSetup const&);
      virtual bool beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
      virtual bool endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);

      // ----------member data ---------------------------

};

//
// constructors and destructor
//

MyAnalyzer::MyAnalyzer(const edm::ParameterSet& iConfig) :
  BaseClass(iConfig)
{
   //now do whatever initialization is needed
}

MyAnalyzer::~MyAnalyzer()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// member functions
//

// ------------ method called once each job just before starting event loop  ------------
void
MyAnalyzer::beginJob()
{
   //#####################################################################################
   //########################### User's code starts here #################################/
   
   // book your histograms here
   CreateUserTH1D("h1_M_m1m2", 100, 0, 1000);
   CreateUserTH1D("h1_M_j1j2", getHistoNBins("pTMu1"), getHistoMin("pTMu1"), getHistoMax("pTMu1"));
   CreateUserTH1D("h1_ST",     getHistoNBins("pTMu1"), getHistoMin("pTMu1"), getHistoMax("pTMu1"));

   // initialize your variables here

   
   //############################# User's code ends here #################################
   //#####################################################################################
}

// ------------ method called on each new Event  ------------
bool
MyAnalyzer::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   bool ret = true;
   // event weight (by default set to 1)
   double eventWeight = 1;

   //#####################################################################################
   //########################### User's code starts here #################################

   // grab necessary objects from the event
   edm::Handle<bool> passPrimaryVertex;
   iEvent.getByLabel(edm::InputTag("EventSelection:PassPrimaryVertex"), passPrimaryVertex);
   edm::Handle<bool> passBeamScraping;
   iEvent.getByLabel(edm::InputTag("EventSelection:PassBeamScraping"), passBeamScraping);
   
   edm::Handle<vector<double> > MuonPt;
   iEvent.getByLabel(edm::InputTag("Muons:Pt"), MuonPt);
   edm::Handle<vector<double> > MuonEta;
   iEvent.getByLabel(edm::InputTag("Muons:Eta"), MuonEta);
   edm::Handle<vector<double> > MuonPhi;
   iEvent.getByLabel(edm::InputTag("Muons:Phi"), MuonPhi);
   edm::Handle<vector<double> > MuonE;
   iEvent.getByLabel(edm::InputTag("Muons:Energy"), MuonE);

   edm::Handle<vector<double> > PFJetPt;
   iEvent.getByLabel(edm::InputTag("AK5PFJets:Pt"), PFJetPt);
   edm::Handle<vector<double> > PFJetEta;
   iEvent.getByLabel(edm::InputTag("AK5PFJets:Eta"), PFJetEta);
   edm::Handle<vector<double> > PFJetPhi;
   iEvent.getByLabel(edm::InputTag("AK5PFJets:Phi"), PFJetPhi);
   edm::Handle<vector<double> > PFJetE;
   iEvent.getByLabel(edm::InputTag("AK5PFJets:Energy"), PFJetE);
   
   // muons
   vector<int> v_idx_mu_final;
   for(unsigned int i=0; i<MuonPt->size(); i++) {
       // select muons inside a fiducial region
       if( fabs(MuonEta->at(i)) < getPreCutValue1("muFidRegion") ) v_idx_mu_final.push_back(i);
   }

   // jets
   vector<int> v_idx_jet_final;
   for(unsigned int i=0; i<PFJetPt->size(); i++) {
       // select jets inside a fiducial region
       if( fabs(PFJetEta->at(i)) < getPreCutValue1("jetFidRegion") ) v_idx_jet_final.push_back(i);
   }

   // Set the evaluation of the cuts to false and clear the variable values and filled status
   resetCuts();

   fillVariableWithValue("PassPrimaryVertex", ( *passPrimaryVertex ? 1 : 0 ) );
   fillVariableWithValue("PassBeamScraping", ( *passBeamScraping ? 1 : 0 ) );
   
   fillVariableWithValue("nMuFinal", v_idx_mu_final.size()) ;
   if( v_idx_mu_final.size() >= 1 ) {
       fillVariableWithValue( "pTMu1", MuonPt->at(v_idx_mu_final[0]) );
   }
   if( v_idx_mu_final.size() >= 2 ) {
       fillVariableWithValue( "pTMu2", MuonPt->at(v_idx_mu_final[1]) );
       // Calculate M_e1e2
       TLorentzVector v_m1m2, v_m1, v_m2;
       v_m1.SetPtEtaPhiE(MuonPt->at(v_idx_mu_final[0]),MuonEta->at(v_idx_mu_final[0]),MuonPhi->at(v_idx_mu_final[0]),MuonE->at(v_idx_mu_final[0]));
       v_m2.SetPtEtaPhiE(MuonPt->at(v_idx_mu_final[1]),MuonEta->at(v_idx_mu_final[1]),MuonPhi->at(v_idx_mu_final[1]),MuonE->at(v_idx_mu_final[1]));
       v_m1m2 = v_m1 + v_m2;
       FillUserTH1D("h1_M_m1m2", v_m1m2.M() );
   }

   fillVariableWithValue("nJetFinal", v_idx_jet_final.size()) ;
   if( v_idx_jet_final.size() >= 1 ) {
       fillVariableWithValue( "pTJet1", PFJetPt->at(v_idx_jet_final[0]) );
   }
   if( v_idx_jet_final.size() >= 2 ) {
       fillVariableWithValue( "pTJet2", PFJetPt->at(v_idx_jet_final[1]) );
       // Calculate M_j1j2
       TLorentzVector v_j1j2, v_j1, v_j2;
       v_j1.SetPtEtaPhiE(PFJetPt->at(v_idx_jet_final[0]),PFJetEta->at(v_idx_jet_final[0]),PFJetPhi->at(v_idx_jet_final[0]),PFJetE->at(v_idx_jet_final[0]));
       v_j2.SetPtEtaPhiE(PFJetPt->at(v_idx_jet_final[1]),PFJetEta->at(v_idx_jet_final[1]),PFJetPhi->at(v_idx_jet_final[1]),PFJetE->at(v_idx_jet_final[1]));
       v_j1j2 = v_j1 + v_j2;
       FillUserTH1D("h1_M_j1j2", v_j1j2.M() );
   }

   // Evaluate cuts (but do not apply them)
   evaluateCuts();

   // retrieve value of previously filled variables (after making sure that they were filled)
   double ST = -99.;
   if ( variableIsFilled("pTMu1") && variableIsFilled("pTMu2") && variableIsFilled("pTJet1") && variableIsFilled("pTJet2") )
       ST = getVariableValue("pTMu1") + getVariableValue("pTMu2") + getVariableValue("pTJet1") + getVariableValue("pTJet2");
   FillUserTH1D("h1_ST", ST );

   // select only those events that pass the full selection
   ret = passedCut("all");

   //############################# User's code ends here #################################
   //#####################################################################################
   
   // increment event counters
   eventCountBeforeWeight++;
   eventCount += eventWeight;
   
   return ret;
}

// ------------ method called when starting to processes a run  ------------
bool
MyAnalyzer::beginRun(edm::Run&, edm::EventSetup const&)
{
  return true;
}

// ------------ method called when starting to processes a luminosity block  ------------
bool
MyAnalyzer::beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
  return true;
}

// ------------ method called when ending the processing of a luminosity block  ------------
bool
MyAnalyzer::endLuminosityBlock(edm::LuminosityBlock& iLumi, edm::EventSetup const& iSetup)
{
  if ( skimWasMade_ ) {
      edm::Handle<edm::MergeableCounter> eventCounter;

      if (iLumi.getByLabel(eventCounterInputTag_, eventCounter) && eventCounter.isValid()) {
          NEvtTotBeforeWeight_ += (double) eventCounter->value;
      } else {
          edm::LogError("MyAnalyzer::endLuminosityBlock") << "Can't get the product " << eventCounterInputTag_ <<". Please make sure the skimWasMade and eventCounterInputTag parameters were set correctly.";
          exit(1);
      }
  }

  return true;
}

// ------------ method called when ending the processing of a run  ------------
bool
MyAnalyzer::endRun(edm::Run&, edm::EventSetup const&)
{
  return true;
}

// ------------ method called once each job just after ending the event loop  ------------
void
MyAnalyzer::endJob()
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
MyAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(MyAnalyzer);
