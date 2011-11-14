#ifndef BASE_CLASS_H
#define BASE_CLASS_H

// -*- C++ -*-
//
// Package:    MyAnalyzer
// Class:      BaseClass
// 
/**\class BaseClass BaseClass.h MyAnalysis/MyAnalyzer/interface/BaseClass.h

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Dinko Ferencek
//         Created:  Mon Sep 12 15:06:41 CDT 2011
// $Id: BaseClass.h,v 1.1 2011/09/16 06:45:00 ferencek Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

// ROOT
#include <TH1D.h>
#include <TH2D.h>

// TFileService
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

using namespace std;

//
// class declaration
//

struct cut {
  string variableName;
  double minValue1;
  double maxValue1;
  double minValue2;
  double maxValue2;
  int level_int;
  string level_str;
  int histoNBins;
  double histoMin;
  double histoMax;
  // Not filled from file
  int id;
  TH1D *histo1;
  TH1D *histo2;
  TH1D *histo3;
  TH1D *histo4;
  TH1D *histo5;
  // Filled event by event
  bool filled;
  double value;
  double weight;
  bool passed;
  double nEvtInput;
  double nEvtPassedBeforeWeight;
  double nEvtPassed;
  double nEvtPassedErr2;
  bool nEvtPassedBeforeWeight_alreadyFilled;
  bool saveVariableInReducedSkim;
};

struct preCut {
  string variableName;
  double value1;
  double value2;
  double value3;
  double value4;
  int level_int;
  string level_str;
};

// Create class to hold the optimization information
class Optimize {
  public:
   Optimize(){count=0; variableName=""; minvalue=0; maxvalue=0; testgreater=false; level_int=-10;};
   Optimize(int x0, string x1, double x2, double x3, bool x4, int x5) {
       count=x0;
       variableName=x1;
       minvalue=x2;
       maxvalue=x3;
       if (minvalue>maxvalue) {
           maxvalue=x2;
           minvalue=x3;
       }
       increment=(maxvalue-minvalue)/9.;
       if (increment<=0)
         increment=1;
       testgreater=x4;
       level_int=x5;
       value=-99999999999; // dummy start value
   };
   ~Optimize(){};

   int count; // store number for ordering of optimization cuts
   string variableName; // store name of variable
   double minvalue; // minimum threshold value to test
   double maxvalue; // maximum threshold to test
   double increment; // max-min, divided into 10 parts
   bool testgreater; // tests whether value should be greater or less than threshold
   int level_int; // cut level -- not used?
   double value;  // value to check against threshold

   bool Compare(int counter) {
       // compare value to threshold # <counter>

       // if testing that value is greater than some threshold, start with lowest threshold first
       bool passed = false;
       if (testgreater) {
           double thresh = minvalue + increment*counter; // convert counter # to physical threshold
           value > thresh ? passed = true : passed = false;
       }
       // if testing that value is less than threshold, start with largest threshold first.  This keep the number of \events "monotonically decreasing" over a series of 10 cuts.
       else {
           double thresh=maxvalue-increment*counter;
           value < thresh ? passed = true : passed = false;
       }
       return passed;
   }; // comparison function
}; // class Optimize


class BaseClass {
   public:
      explicit BaseClass(const edm::ParameterSet&);
      ~BaseClass();

      void resetCuts(const string& s = "newEvent");
      void fillVariableWithValue(const string&, const double&, const double& weight = 1.);
      void evaluateCuts();

      bool passedCut(const string& s);
      bool passedAllPreviousCuts(const string& s);
      bool passedAllOtherCuts(const string& s);
      bool passedAllOtherSameAndLowerLevelCuts(const string& s);
      bool variableIsFilled(const string& s);
      double getVariableValue(const string& s);
      double getPreCutValue1(const string& s);
      double getPreCutValue2(const string& s);
      double getPreCutValue3(const string& s);
      double getPreCutValue4(const string& s);
      double getCutMinValue1(const string& s);
      double getCutMaxValue1(const string& s);
      double getCutMinValue2(const string& s);
      double getCutMaxValue2(const string& s);

      const TH1D& getHisto_skim_or_noCuts(const string& s);
      const TH1D& getHisto_allPreviousCuts(const string& s);
      const TH1D& getHisto_allOthrSmAndLwrLvlCuts(const string& s);
      const TH1D& getHisto_allOtherCuts(const string& s);
      const TH1D& getHisto_allCuts(const string& s);

      int    getHistoNBins(const string& s);
      double getHistoMin(const string& s);
      double getHistoMax(const string& s);

      void CreateAndFillUserTH1D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup, Double_t value, Double_t weight=1);
      void CreateUserTH1D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup);
      void FillUserTH1D(const string& nameAndTitle, Double_t value, Double_t weight=1);
      void CreateAndFillUserTH2D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup,  Double_t value_x,  Double_t value_y, Double_t weight=1);
      void CreateUserTH2D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup);
      void FillUserTH2D(const string& nameAndTitle, Double_t value_x,  Double_t value_y, Double_t weight=1);

      bool skimWasMade_;
      edm::InputTag eventCounterInputTag_;
      double NEvtTotBeforeWeight_;
      double NEvtTot_;
      double eventCountBeforeWeight;
      double eventCount;

   private:

      void readCutFile();
      bool fillCutHistos();
      bool writeCutHistos();
      bool writeUserHistos();
      bool updateCutEffic();
      bool writeCutEfficFile();
      bool sortCuts(const cut&, const cut&);
      vector<string> split(const string& s);
      double decodeCutValue(const string& s);

      // Optimization stuff
      void fillOptimizerWithValue(const string& s, const double& d);
      void runOptimizer();
      
      // ----------member data ---------------------------
      map<string, bool> combCutName_passed_;

      string configFile_;
      edm::Service<TFileService> fs;
      string cutFile_;
      string cutEfficFile_;
      stringstream preCutInfo_;
      map<string, preCut> preCutName_cut_;
      map<string, cut> cutName_cut_;
      vector<string> orderedCutNames_;
      map<string, TH1D*> userTH1Ds_;
      map<string, TH2D*> userTH2Ds_;

      // Which plots to fill
      bool fillSkimOrNoCuts_;
      bool fillAllPreviousCuts_;
      bool fillAllOtherCuts_;
      bool fillAllSameLevelAndLowerLevelCuts_;
      bool fillAllCuts_;

      // Skim mode (in this mode, the BaseClass does not produce any output which is useful when producing a skim)
      bool skimMode_;

      TH1D *EventCuts_; // number of events passing each cut
      
      // Optimization stuff
      map<int, Optimize> optimizeName_cut_;
      TH1D *h_Optimizer_; // optimization histogram
      bool writeOptCutFile_;
      string optCutFileName_;
};

#endif
