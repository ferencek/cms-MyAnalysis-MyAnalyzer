// user include files
#include <fstream>

#include "MyAnalysis/MyAnalyzer/interface/BaseClass.h"

//
// constructors and destructor
//

BaseClass::BaseClass(const edm::ParameterSet& iConfig)
{
   //now do whatever initialization is needed
   fillSkimOrNoCuts_                  = iConfig.getUntrackedParameter<bool>("fillSkimOrNoCuts",true);
   fillAllPreviousCuts_               = iConfig.getUntrackedParameter<bool>("fillAllPreviousCuts",true);
   fillAllOtherCuts_                  = iConfig.getUntrackedParameter<bool>("fillAllOtherCuts",true);
   fillAllSameLevelAndLowerLevelCuts_ = iConfig.getUntrackedParameter<bool>("fillAllSameLevelAndLowerLevelCuts",true);
   fillAllCuts_                       = iConfig.getUntrackedParameter<bool>("fillAllCuts",true);
   writeOptCutFile_                   = iConfig.getUntrackedParameter<bool>("writeOptimizationCutFile",false);
   optCutFileName_                    = iConfig.getUntrackedParameter<string>("optimizationCutFileName","OptimizationCuts.txt");
   skimWasMade_                       = iConfig.getParameter<bool>("skimWasMade");
   eventCounterInputTag_              = iConfig.getUntrackedParameter<edm::InputTag>("eventCounterInputTag",edm::InputTag("nEventsTotal"));
   cutFile_                           = iConfig.getParameter<string>("inputCutFile");
   cutEfficFile_                      = iConfig.getParameter<string>("outputCutEfficiencyFile");

   // read the input cut file
   readCutFile();

   // initialize event counters
   NEvtTotBeforeWeight_ = 0;
   NEvtTot_ = 0;
   eventCountBeforeWeight = 0;
   eventCount = 0;
}

BaseClass::~BaseClass()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
  NEvtTot_ = NEvtTotBeforeWeight_; // This will have to be modified if there are events with generator-level weights. For now, this is the default behavior

  if( !writeCutEfficFile() ) {
      edm::LogError("BaseClass::endJob") << "BaseClass::writeCutEfficFile did not complete successfully.";
      exit(1);
  } 
}

//
// member functions
//

void
BaseClass::readCutFile()
{
  string s;

  ifstream is(cutFile_.c_str());
  if(is.good()) {
      edm::LogInfo("BaseClass::readCutFile") << "Reading cutFile: "<< cutFile_;
      int id = 0;
      int optimize_count = 0;
      while( getline(is,s) ) {
          edm::LogInfo("BaseClass::readCutFile") << "Reading line: " << s;
          if ( s.empty() ) continue;
          vector<string> v = split(s);
          if ( v.size() == 0 || v[0][0] == '#') continue;
          edm::LogInfo("BaseClass::readCutFile") << "Starting optimizer code";
          if ( v[1] == "OPT") {
              if (optimizeName_cut_.size()>=6) {
                  edm::LogWarning("BaseClass::readCutFile") << "Optimizer can only accept up to 6 variables.\nVariable "<<v[0]<<" is not being included.";
                  continue;
              }
              bool found = false;
              for ( unsigned int i=0; i<optimizeName_cut_.size(); ++i ) {
                  if ( optimizeName_cut_[i].variableName == v[0] ) {
                      edm::LogWarning("BaseClass::readCutFile") << "variableName = "<<v[0]<<" is already being optimized in optimizedName_cut_.  Skipping.";
                      found = true;
                      break;
                  }
              }
              if ( found ) continue;

              int level_int = atoi(v[5].c_str());
              bool greaterthan = true;
              if (v[2] == "<") greaterthan = false;
              double minval = atof(v[3].c_str());
              double maxval = atof(v[4].c_str());
              Optimize opt(optimize_count, v[0], minval, maxval, greaterthan, level_int);
              optimizeName_cut_[optimize_count] = opt; // order cuts by cut #, rather than name, so that optimization histogram is consistently ordered
              ++optimize_count;
              continue;
          }

          map<string, cut>::iterator cc = cutName_cut_.find(v[0]);
          if ( cc != cutName_cut_.end() ) {
              edm::LogError("BaseClass::readCutFile") << "variableName = "<< v[0] << " exists already in cutName_cut_. Returning.";
              exit(1);
          }

          int level_int = atoi(v[5].c_str());
          if ( level_int == -1 ) {
              map<string, preCut>::iterator cc = preCutName_cut_.find(v[0]);
              if( cc != preCutName_cut_.end() ) {
                  edm::LogError("BaseClass::readCutFile") << "variableName = "<< v[0] << " exists already in preCutName_cut_. Returning.";
                  exit(1);
              }
              preCutInfo_ << "### Preliminary cut values: " << s <<endl;
              preCut thisPreCut;
              thisPreCut.variableName = v[0];
              thisPreCut.value1  = decodeCutValue( v[1] );
              thisPreCut.value2  = decodeCutValue( v[2] );
              thisPreCut.value3  = decodeCutValue( v[3] );
              thisPreCut.value4  = decodeCutValue( v[4] );
              preCutName_cut_[thisPreCut.variableName] = thisPreCut;
              continue;
          }
          cut thisCut;
          thisCut.variableName = v[0];
          string m1 = v[1];
          string M1 = v[2];
          string m2 = v[3];
          string M2 = v[4];
          if( m1 == "-" || M1 == "-" ) {
              edm::LogError("BaseClass::readCutFile") << "minValue1 and maxValue1 have to be provided. Returning.";
              exit(1);
          }
          if( (m2 == "-" && M2 != "-") || (m2 != "-" && M2 == "-") ) {
              edm::LogError("BaseClass::readCutFile") << "If any of minValue2 and maxValue2 is -, then both have to be -. Returning.";
              exit(1);
          }
          if( m2 == "-") m2 = "+inf";
          if( M2 == "-") M2 = "-inf";
          thisCut.minValue1  = decodeCutValue( m1 );
          thisCut.maxValue1  = decodeCutValue( M1 );
          thisCut.minValue2  = decodeCutValue( m2 );
          thisCut.maxValue2  = decodeCutValue( M2 );
          thisCut.level_int  = level_int;
          thisCut.level_str  = v[5];
          thisCut.histoNBins = atoi( v[6].c_str() );
          thisCut.histoMin   = atof( v[7].c_str() );
          thisCut.histoMax   = atof( v[8].c_str() );
          // Not filled from file
          thisCut.id = ++id;
          string s1;
          if(skimWasMade_) {
              s1 = "cutHisto_skim___________________" + thisCut.variableName;
          } else {
              s1 = "cutHisto_noCuts_________________" + thisCut.variableName;
          }
          string s2 = "cutHisto_allPreviousCuts________" + thisCut.variableName;
          string s3 = "cutHisto_allOthrSmAndLwrLvlCuts_" + thisCut.variableName;
          string s4 = "cutHisto_allOtherCuts___________" + thisCut.variableName;
          string s5 = "cutHisto_allCuts________________" + thisCut.variableName;
          if ( fillSkimOrNoCuts_ )                  thisCut.histo1 = fs->make<TH1D>(s1.c_str(),"", thisCut.histoNBins, thisCut.histoMin, thisCut.histoMax);
          if ( fillAllPreviousCuts_ )               thisCut.histo2 = fs->make<TH1D>(s2.c_str(),"", thisCut.histoNBins, thisCut.histoMin, thisCut.histoMax);
          if ( fillAllSameLevelAndLowerLevelCuts_ ) thisCut.histo3 = fs->make<TH1D>(s3.c_str(),"", thisCut.histoNBins, thisCut.histoMin, thisCut.histoMax);
          if ( fillAllOtherCuts_ )                  thisCut.histo4 = fs->make<TH1D>(s4.c_str(),"", thisCut.histoNBins, thisCut.histoMin, thisCut.histoMax);
          if ( fillAllCuts_ )                       thisCut.histo5 = fs->make<TH1D>(s5.c_str(),"", thisCut.histoNBins, thisCut.histoMin, thisCut.histoMax);
          thisCut.histo1->Sumw2();
          thisCut.histo2->Sumw2();
          thisCut.histo3->Sumw2();
          thisCut.histo4->Sumw2();
          thisCut.histo5->Sumw2();
          // Filled event by event
          thisCut.filled = false;
          thisCut.value = 0;
          thisCut.weight = 1.;
          thisCut.passed = false;
          thisCut.nEvtPassedBeforeWeight = 0;
          thisCut.nEvtPassed = 0;
          thisCut.nEvtPassedErr2 = 0;
          thisCut.nEvtPassedBeforeWeight_alreadyFilled = false;

          orderedCutNames_.push_back(thisCut.variableName);
          cutName_cut_[thisCut.variableName] = thisCut;

      }
      edm::LogInfo("BaseClass::readCutFile") << "Finished reading cutFile: " << cutFile_;
  } else {
      edm::LogError("BaseClass::readCutFile") << "Error opening cutFile: " << cutFile_;
      exit (1);
  }
  // make optimizer histogram
  if ( optimizeName_cut_.size() > 0 ) {
      h_Optimizer_ = fs->make<TH1D>("Optimizer","Optimization of cut variables",(int)pow(10,optimizeName_cut_.size()),0,
                               pow(10,optimizeName_cut_.size()));
  }

  // Create a histogram that will show events passing cuts
  int cutsize = orderedCutNames_.size() + 1;
  if ( skimWasMade_ ) ++cutsize;
  EventCuts_ = fs->make<TH1D>("EventsPassingCuts","Events Passing Cuts",cutsize,0,cutsize);

  is.close();
}

// split a line into a vector of strings separated by spaces in the original line
vector<string>
BaseClass::split(const string& s)
{
  vector<string> ret;
  string::size_type i = 0;
  while (i != s.size()) {
    while (i != s.size() && isspace(s[i]))
      ++i;
    string::size_type j = i;
    while (j != s.size() && !isspace(s[j]))
      ++j;
    if (i != j) {
      ret.push_back(s.substr(i, j -i));
      i = j;
    }
  }
  return ret;
}

double
BaseClass::decodeCutValue(const string& s)
{
  double ret;
  if( s == "inf" || s == "+inf" ) {
       ret = 99999999999;
  } else if ( s == "-inf" || s == "-" ) {
       ret = -99999999999;
  } else {
       ret = atof( s.c_str() );
  }
  return ret;
}

void
BaseClass::resetCuts(const string& s)
{
  for (map<string, cut>::iterator cc = cutName_cut_.begin(); cc != cutName_cut_.end(); cc++) {
      cut *c = &(cc->second);
      c->filled = false;
      c->value = 0;
      c->weight = 1;
      c->passed = false;
      if( s == "newEvent" ) {
          c->nEvtPassedBeforeWeight_alreadyFilled = false;
      } else if( s != "sameEvent" ) {
          edm::LogError("BaseClass::resetCuts") << "Unrecognized option. Only allowed options are 'sameEvent' and 'newEvent'; no option = 'newEvent'.";
          exit(1);
      }
  }
  combCutName_passed_.clear();
  return;
}

void
BaseClass::fillVariableWithValue(const string& s, const double& d, const double& weight)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if ( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::fillVariableWithValue") << "variableName = "<< s << " not found in cutName_cut_. Returning.";
      exit(1);
  } else {
      cut *c = &(cc->second);
      c->filled = true;
      c->value = d;
      c->weight = weight;
  }
  fillOptimizerWithValue(s, d);
  return;
}

bool
BaseClass::variableIsFilled(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if ( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::variableIsFilled") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cc->second);
  return (c->filled);
}

double
BaseClass::getVariableValue(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if ( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getVariableValue") << "Did not find variableName = "<<s<<" in cutName_cut_.";
      exit(1);
  }
  cut *c = &(cc->second);
  if ( !variableIsFilled(s) ) {
      edm::LogError("BaseClass::getVariableValue") <<  "Requesting value of a variable that has not yet been filled"<<s;
      exit(1);
  }
  return (c->value);
}

void BaseClass::fillOptimizerWithValue(const string& s, const double& d)
{
  for (unsigned int i=0; i<optimizeName_cut_.size(); ++i) {
      if (optimizeName_cut_[i].variableName == s) {
          optimizeName_cut_[i].value = d;
          return;
      }
  }
  return;
}

void
BaseClass::evaluateCuts()
{
  combCutName_passed_.clear();
  for (vector<string>::iterator it = orderedCutNames_.begin(); it != orderedCutNames_.end(); it++) {
      cut *c = &(cutName_cut_.find(*it)->second);
      if( ! ( c->filled && ( (c->value > c->minValue1 && c->value <= c->maxValue1) || (c->value > c->minValue2 && c->value <= c->maxValue2) ) ) ) {
          c->passed = false;
          combCutName_passed_[c->level_str.c_str()] = false;
          combCutName_passed_["all"] = false;
      } else {
          c->passed = true;
          map<string,bool>::iterator cp = combCutName_passed_.find( c->level_str.c_str() );
          if ( cp == combCutName_passed_.end() ) combCutName_passed_[c->level_str.c_str()] = true;
          map<string,bool>::iterator ap = combCutName_passed_.find("all");
          if ( ap == combCutName_passed_.end() ) combCutName_passed_["all"] = true;
      }
  }

  // run optimizer
  runOptimizer();

  if( !fillCutHistos() ) {
      edm::LogError("BaseClass::evaluateCuts") << "BaseClass::fillCutHistos did not complete successfully.";
      exit(1);
  }

  if( !updateCutEffic() ) {
      edm::LogError("BaseClass::evaluateCuts") << "BaseClass::updateCutEffic did not complete successfully.";
      exit(1);
  }

  return;
}

void
BaseClass::runOptimizer()
{
  // don't run optimizer if no optimized cuts specified
  if (optimizeName_cut_.size()==0)
    return;

  // first, check that all cuts (except those to be optimized) have been passed
  for (vector<string>::iterator it = orderedCutNames_.begin(); it != orderedCutNames_.end(); it++) {
      bool ignorecut=false;
      for (unsigned int i=0; i < optimizeName_cut_.size(); ++i) {
          if (optimizeName_cut_[i].variableName == *it) {
              ignorecut=true;
              break;
          }
      }
      if (ignorecut) continue;
      if (passedCut(*it) == false)
        return;
  }

  // loop over up to 6 cuts
  int thesize = optimizeName_cut_.size();
  int mysize = thesize;
  vector<bool> counterbins;
  for (unsigned int i=0; i<pow(10,thesize); ++i) counterbins.push_back(true); // assume true

  // lowest-numbered cut appears first in cut ordering
  // that is, for cut:  ABCDEF
  // A is the index of cut0, B is cut 1, etc.
  for (int cc=0; cc<thesize; ++cc) { // loop over all cuts, starting at cut 0
      --mysize;
      for (unsigned int i=0; i<10; ++i) {// loop over 10 cuts for each
          if (!optimizeName_cut_[cc].Compare(i)) { // cut failed; set all values associated with cut to false
              // loop over all cut values starting with current cut
              for (unsigned int j = (i*pow(10,mysize)); j < pow(10,thesize); ++j) {
                  // if relevant digit of the cut value matches the current (failed) cut, set this cut to false
                  if ((j/int(pow(10,mysize)))%10==i)
                    counterbins[j]=false;
                  if (j>counterbins.size())
                    continue; // shouldn't ever happen
              }
          } // if (cut failed)
      } // for (int i=0;i<10;++i)
  }
  // now fill histograms
  for (unsigned int i=0; i<counterbins.size(); ++i) {
      if (counterbins[i]==true)
        h_Optimizer_->Fill(i,cutName_cut_[orderedCutNames_.at(orderedCutNames_.size()-1)].weight); // take the event weight from the last cut in the cut file
  }

  return;
}

bool
BaseClass::fillCutHistos()
{
  bool ret = true;
  for (map<string, cut>::iterator it = cutName_cut_.begin(); it != cutName_cut_.end(); it++) {
      cut *c = &(it->second);
      if( c->filled ) {
          if ( fillSkimOrNoCuts_ )                                                                          c->histo1->Fill( c->value, c->weight );
          if ( fillAllPreviousCuts_ && passedAllPreviousCuts(c->variableName) )                             c->histo2->Fill( c->value, c->weight );
          if ( fillAllSameLevelAndLowerLevelCuts_ && passedAllOtherSameAndLowerLevelCuts(c->variableName) ) c->histo3->Fill( c->value, c->weight );
          if ( fillAllOtherCuts_ && passedAllOtherCuts(c->variableName) )                                   c->histo4->Fill( c->value, c->weight );
          if ( fillAllCuts_ && passedCut("all") )                                                           c->histo5->Fill( c->value, c->weight );
      }
  }
  return ret;
}

bool
BaseClass::updateCutEffic()
{
  bool ret = true;
  for (map<string, cut>::iterator it = cutName_cut_.begin(); it != cutName_cut_.end(); it++) {
      cut *c = &(it->second);
      if( passedAllPreviousCuts(c->variableName) ) {
          if( passedCut(c->variableName) ) {
              if ( c->nEvtPassedBeforeWeight_alreadyFilled == false) {
                  c->nEvtPassedBeforeWeight += 1;
                  c->nEvtPassedBeforeWeight_alreadyFilled = true;
              }
              c->nEvtPassed += c->weight;
              c->nEvtPassedErr2 += (c->weight)*(c->weight);
          }
      }
  }
  return ret;
}

bool
BaseClass::passedCut(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc != cutName_cut_.end() ) {
      cut *c = &(cutName_cut_.find(s)->second);
      return (c->filled && c->passed);
  }
  map<string, bool>::iterator cp = combCutName_passed_.find(s);
  if( cp != combCutName_passed_.end() ) {
      return (cp->second);
  }
  edm::LogWarning("BaseClass::passedCut") << "Did not find variableName = "<<s<<" neither in cutName_cut_ nor combCutName_passed_. Returning false.";
  return false;
}

bool
BaseClass::passedAllPreviousCuts(const string& s)
{
  bool ret = true;
  
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogWarning("BaseClass::passedAllPreviousCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning false.";
      return false;
  }

  for (vector<string>::iterator it = orderedCutNames_.begin(); it != orderedCutNames_.end(); it++) {
      cut *c = &(cutName_cut_.find(*it)->second);
      if( c->variableName == s ) {
          return true;
      } else {
          if( ! (c->filled && c->passed) ) return false;
      }
  }
  
  return ret;
}

bool
BaseClass::passedAllOtherCuts(const string& s)
{
  bool ret = true;
  
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogWarning("BaseClass::passedAllOtherCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning false.";
      return false;
  }

  for (map<string, cut>::iterator cc = cutName_cut_.begin(); cc != cutName_cut_.end(); cc++) {
      cut *c = &(cc->second);
      if( c->variableName == s ) {
          continue;
      } else {
          if( ! (c->filled && c->passed) ) return false;
      }
  }
  
  return ret;
}

bool
BaseClass::passedAllOtherSameAndLowerLevelCuts(const string& s)
{
  bool ret = true;
  int cutLevel;
  
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogWarning("BaseClass::passedAllOtherSameAndLowerLevelCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning false.";
      return false;
  } else {
      cutLevel = cc->second.level_int;
  }

  for (map<string, cut>::iterator cc = cutName_cut_.begin(); cc != cutName_cut_.end(); cc++) {
      cut *c = &(cc->second);
      if( c->level_int > cutLevel || c->variableName == s ) {
          continue;
      } else {
          if( ! (c->filled && c->passed) ) return false;
      }
  }
  
  return ret;
}

bool
BaseClass::writeCutEfficFile()
{
  bool ret = true;

  // set bin labels for event counter histogram
  int bincounter = 1;
  EventCuts_->GetXaxis()->SetBinLabel(bincounter,"NoCuts");
  ++bincounter;
  if (skimWasMade_) {
      EventCuts_->GetXaxis()->SetBinLabel(bincounter,"Skim");
      ++bincounter;
  }
  for (unsigned int i=0;i<orderedCutNames_.size();++i) {
      EventCuts_->GetXaxis()->SetBinLabel(bincounter,orderedCutNames_[i].c_str());
      ++bincounter;
  }

  bincounter=1;

  double nEvtTotBeforeWeight = (skimWasMade_ ? NEvtTotBeforeWeight_ : eventCountBeforeWeight );
  double nEvtTot             = (skimWasMade_ ? NEvtTot_ : eventCount );

  ofstream os(cutEfficFile_.c_str());

  os << "################################## Preliminary Cut Values ###################################################################\n"
     << "########################### variableName                 value1          value2          value3          value4          level\n"
     << preCutInfo_.str();

  int cutIdPed=0;
  double minForFixed = 0.1;
  int precision = 4;
  os.precision(precision);
  os << "################################## Cuts #####################################################################################\n"
     <<"#id             variableName           min1           max1           min2           max2          level              N          Npass         EffRel      errEffRel         EffAbs      errEffAbs"<<endl
     << fixed
     << setw(3) << cutIdPed
     << setw(25) << "nocut"
     << setprecision(4)
     << setw(15) << "-"
     << setw(15) << "-"
     << setw(15) << "-"
     << setw(15) << "-"
     << setw(15) << "-"
     << setw(15) << nEvtTot
     << setw(15) << nEvtTot
     //<< setprecision(11)
     << setw(15) << 1.
     << setw(15) << 0.
     << setw(15) << 1.
     << setw(15) << 0.
     << endl;

  double effRel;
  double effRelErr;
  double effAbs;
  double effAbsErr;

  EventCuts_->SetBinContent(bincounter, nEvtTot);
  if (optimizeName_cut_.size())
    h_Optimizer_->SetBinContent(0, nEvtTot);

  double nEvtPassedBeforeWeight_previousCut = nEvtTotBeforeWeight;
  double nEvtPassed_previousCut = nEvtTot;

  if(skimWasMade_) {
      ++bincounter;
      EventCuts_->SetBinContent(bincounter, eventCount);
      effRel = eventCount / nEvtPassed_previousCut;
      effRelErr = sqrt( effRel * (1.0 - effRel) / nEvtPassed_previousCut );
      effAbs = effRel;
      effAbsErr = effRelErr;
      os << fixed
         << setw(3) << ++cutIdPed
         << setw(25) << "skim"
         << setprecision(4)
         << setw(15) << "-"
         << setw(15) << "-"
         << setw(15) << "-"
         << setw(15) << "-"
         << setw(15) << "-"
         << setw(15) << nEvtPassed_previousCut
         << setw(15) << eventCount
         << setw(15) << ( (effRel                 < minForFixed) ? (scientific) : (fixed) ) << effRel
         << setw(15) << ( (effRelErr              < minForFixed) ? (scientific) : (fixed) ) << effRelErr
         << setw(15) << ( (effAbs                 < minForFixed) ? (scientific) : (fixed) ) << effAbs
         << setw(15) << ( (effAbsErr              < minForFixed) ? (scientific) : (fixed) ) << effAbsErr
         << fixed << endl;
      nEvtPassedBeforeWeight_previousCut = eventCount;
      nEvtPassed_previousCut = eventCount;
  }

  for (vector<string>::iterator it = orderedCutNames_.begin(); it != orderedCutNames_.end(); it++) {
      cut * c = & (cutName_cut_.find(*it)->second);
      ++bincounter;
      EventCuts_->SetBinContent(bincounter, c->nEvtPassed);
      effRel = c->nEvtPassed / nEvtPassed_previousCut;
      double N = nEvtPassedBeforeWeight_previousCut;
      double Np = c->nEvtPassedBeforeWeight;
      double p = Np / N;
      double q = 1-p;
      double w = c->nEvtPassed / c->nEvtPassedBeforeWeight;
      effRelErr = sqrt(p*q/N)*w;
      effAbs = c->nEvtPassed / nEvtTot;
      N = nEvtTot;
      p = Np / N;
      q = 1-p;
      effAbsErr = sqrt(p*q/N)*w;

      stringstream ssm1, ssM1, ssm2,ssM2;
      ssm1 << fixed << setprecision(4) << c->minValue1;
      ssM1 << fixed << setprecision(4) << c->maxValue1;
      if(c->minValue2 == -99999999999) {
          ssm2 << "-inf";
      } else {
          ssm2 << fixed << setprecision(4) << c->minValue2;
      }
      if(c->maxValue2 ==  99999999999) {
          ssM2 << "+inf";
      }
      else {
          ssM2 << fixed << setprecision(4) << c->maxValue2;
      }
      os << setw(3) << cutIdPed+c->id
         << setw(25) << c->variableName
         << setprecision(precision)
         << fixed
         << setw(15) << ( ( c->minValue1 == -99999999999.0 ) ? "-inf" : ssm1.str() )
         << setw(15) << ( ( c->maxValue1 ==  99999999999.0 ) ? "+inf" : ssM1.str() )
         << setw(15) << ( ( c->minValue2 > c->maxValue2 ) ? "-" : ssm2.str() )
         << setw(15) << ( ( c->minValue2 > c->maxValue2 ) ? "-" : ssM2.str() )
         << setw(15) << c->level_int
         << setw(15) << ( (nEvtPassed_previousCut < minForFixed) ? (scientific) : (fixed) ) << nEvtPassed_previousCut
         << setw(15) << ( (c->nEvtPassed          < minForFixed) ? (scientific) : (fixed) ) << c->nEvtPassed
         << setw(15) << ( (effRel                 < minForFixed) ? (scientific) : (fixed) ) << effRel
         << setw(15) << ( (effRelErr              < minForFixed) ? (scientific) : (fixed) ) << effRelErr
         << setw(15) << ( (effAbs                 < minForFixed) ? (scientific) : (fixed) ) << effAbs
         << setw(15) << ( (effAbsErr              < minForFixed) ? (scientific) : (fixed) ) << effAbsErr
         << fixed << endl;
      nEvtPassedBeforeWeight_previousCut = c->nEvtPassedBeforeWeight;
      nEvtPassed_previousCut = c->nEvtPassed;
  }

  // write optimization cut file
  if ( optimizeName_cut_.size() && writeOptCutFile_ ) {
      ofstream optFile( optCutFileName_.c_str() );
      if ( !optFile.good() ) {
          edm::LogError("BaseClass::writeCutEfficFile") << "Cannot open file "<< optCutFileName_.c_str();
          exit(1);
      }

      int Nbins = h_Optimizer_->GetNbinsX();
      for (int i=0; i<Nbins; ++i) {
          vector<int> cutindex;
          // cutindex will store histogram bin as a series of integers
          // 12345 = {1,2,3,4,5}, etc.

          optFile <<"Bin = "<<i+1; // bins in ROOT histograms start from 1 (bin 0 is the underflow)
          for (int j=Nbins/10; j>=1; j/=10) {
              cutindex.push_back((i/j)%10);
          }  // for (int j=(int)log10(Nbins);...)

          for (unsigned int j=0; j<cutindex.size(); ++j) {
              optFile <<"\t"<< optimizeName_cut_[j].variableName;
              if (optimizeName_cut_[j].testgreater==true)
                optFile <<" > "<<optimizeName_cut_[j].minvalue+optimizeName_cut_[j].increment*cutindex[j];
              else
                optFile <<" < "<<optimizeName_cut_[j].maxvalue-optimizeName_cut_[j].increment*cutindex[j];
          } //for (unsigned int j=0;...)
          optFile <<endl;
      } // for (int i=0;...)
  }

  // Any failure mode to implement?
  return ret;
} // writeCutEfficFile

double
BaseClass::getPreCutValue1(const string& s)
{
  map<string, preCut>::iterator cc = preCutName_cut_.find(s);
  if( cc == preCutName_cut_.end() ) {
      edm::LogError("BaseClass::getPreCutValue1") << "Did not find variableName = "<<s<<" in preCutName_cut_. Returning.";
      exit(1);
  }
  preCut *c = &(cc->second);
  if(c->value1 == -99999999999) {
      edm::LogError("BaseClass::getPreCutValue1") << "value1 of preliminary cut "<<s<<" was not provided.";
      exit(1);
  }
  return (c->value1);
}

double
BaseClass::getPreCutValue2(const string& s)
{
  map<string, preCut>::iterator cc = preCutName_cut_.find(s);
  if( cc == preCutName_cut_.end() ) {
      edm::LogError("BaseClass::getPreCutValue2") << "Did not find variableName = "<<s<<" in preCutName_cut_. Returning.";
      exit(1);
  }
  preCut *c = &(cc->second);
  if(c->value2 == -99999999999) {
      edm::LogError("BaseClass::getPreCutValue2") << "value2 of preliminary cut "<<s<<" was not provided.";
      exit(1);
  }
  return (c->value2);
}

double
BaseClass::getPreCutValue3(const string& s)
{
  map<string, preCut>::iterator cc = preCutName_cut_.find(s);
  if( cc == preCutName_cut_.end() ) {
      edm::LogError("BaseClass::getPreCutValue3") << "Did not find variableName = "<<s<<" in preCutName_cut_. Returning.";
      exit(1);
  }
  preCut *c = &(cc->second);
  if(c->value3 == -99999999999) {
      edm::LogError("BaseClass::getPreCutValue3") << "value3 of preliminary cut "<<s<<" was not provided.";
      exit(1);
  }
  return (c->value3);
}

double
BaseClass::getPreCutValue4(const string& s)
{
  map<string, preCut>::iterator cc = preCutName_cut_.find(s);
  if( cc == preCutName_cut_.end() ) {
      edm::LogError("BaseClass::getPreCutValue4") << "Did not find variableName = "<<s<<" in preCutName_cut_. Returning.";
      exit(1);
  }
  preCut *c = &(cc->second);
  if(c->value4 == -99999999999) {
      edm::LogError("BaseClass::getPreCutValue4") << "value4 of preliminary cut "<<s<<" was not provided.";
      exit(1);
  }
  return (c->value4);
}

double
BaseClass::getCutMinValue1(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getCutMinValue1") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cc->second);
  return (c->minValue1);
}

double
BaseClass::getCutMaxValue1(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getCutMaxValue1") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cc->second);
  return (c->maxValue1);
}

double
BaseClass::getCutMinValue2(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getCutMinValue2") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cc->second);
  return (c->minValue2);
}

double
BaseClass::getCutMaxValue2(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getCutMaxValue2") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cc->second);
  return (c->maxValue2);
}

const TH1D&
BaseClass::getHisto_skim_or_noCuts(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHisto_skim_or_noCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (*(c->histo1));
}

const TH1D&
BaseClass::getHisto_allPreviousCuts(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHisto_allPreviousCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (*(c->histo2));
}

const TH1D&
BaseClass::getHisto_allOthrSmAndLwrLvlCuts(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHisto_allOthrSmAndLwrLvlCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (*(c->histo3));
}

const TH1D&
BaseClass::getHisto_allOtherCuts(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHisto_allOtherCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (*(c->histo4));
}

const TH1D&
BaseClass::getHisto_allCuts(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHisto_allCuts") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (*(c->histo5));
}

int
BaseClass::getHistoNBins(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHistoNBins") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (c->histoNBins);
}

double
BaseClass::getHistoMin(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHistoMin") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (c->histoMin);
}

double
BaseClass::getHistoMax(const string& s)
{
  map<string, cut>::iterator cc = cutName_cut_.find(s);
  if( cc == cutName_cut_.end() ) {
      edm::LogError("BaseClass::getHistoMax") << "Did not find variableName = "<<s<<" in cutName_cut_. Returning.";
      exit(1);
  }
  cut *c = &(cutName_cut_.find(s)->second);
  return (c->histoMax);
}

void
BaseClass::CreateAndFillUserTH1D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup, Double_t value, Double_t weight)
{
  map<string, TH1D*>::iterator nh_h = userTH1Ds_.find(nameAndTitle);
  if( nh_h == userTH1Ds_.end() ) {
      TH1D *h = fs->make<TH1D>(nameAndTitle.c_str(), nameAndTitle.c_str(), nbinsx, xlow, xup);
      h->Sumw2();
      h->Fill(value, weight);
      userTH1Ds_[nameAndTitle] = h;
  } else {
      nh_h->second->Fill(value, weight);
  }
}

void
BaseClass::CreateUserTH1D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup)
{
  map<string, TH1D*>::iterator nh_h = userTH1Ds_.find(nameAndTitle);
  if( nh_h == userTH1Ds_.end() ) {
      TH1D *h = fs->make<TH1D>(nameAndTitle.c_str(), nameAndTitle.c_str(), nbinsx, xlow, xup);
      h->Sumw2();
      userTH1Ds_[nameAndTitle] = h;
  } else {
      edm::LogError("BaseClass::CreateUserTH1D") << "Trying to define already existing histogram "<<nameAndTitle;
      exit(1);
  }
}

void
BaseClass::FillUserTH1D(const string& nameAndTitle, Double_t value, Double_t weight)
{
  map<string, TH1D*>::iterator nh_h = userTH1Ds_.find(nameAndTitle);
  if( nh_h == userTH1Ds_.end() ) {
      edm::LogError("BaseClass::FillUserTH1D") << "Trying to fill histogram "<<nameAndTitle<<" that was not defined.";
      exit(1);
  } else {
      nh_h->second->Fill(value, weight);
  }
}

void
BaseClass::CreateAndFillUserTH2D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup,  Double_t value_x,  Double_t value_y, Double_t weight)
{
  map<string, TH2D*>::iterator nh_h = userTH2Ds_.find(nameAndTitle);
  if( nh_h == userTH2Ds_.end() ) {
      TH2D *h = fs->make<TH2D>(nameAndTitle.c_str(), nameAndTitle.c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup);
      h->Sumw2();
      h->Fill(value_x, value_y, weight);
      userTH2Ds_[nameAndTitle] = h;
  } else {
      nh_h->second->Fill(value_x, value_y, weight);
  }
}

void
BaseClass::CreateUserTH2D(const string& nameAndTitle, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup)
{
  map<string, TH2D*>::iterator nh_h = userTH2Ds_.find(nameAndTitle);
  if( nh_h == userTH2Ds_.end() ) {
      TH2D *h = fs->make<TH2D>(nameAndTitle.c_str(), nameAndTitle.c_str(), nbinsx, xlow, xup, nbinsy, ylow, yup);
      h->Sumw2();
      userTH2Ds_[nameAndTitle] = h;
  } else {
      edm::LogError("BaseClass::CreateUserTH2D") << "Trying to define already existing histogram "<<nameAndTitle;
      exit(1);
  }
}

void
BaseClass::FillUserTH2D(const string& nameAndTitle, Double_t value_x,  Double_t value_y, Double_t weight)
{
  map<string, TH2D*>::iterator nh_h = userTH2Ds_.find(nameAndTitle);
  if( nh_h == userTH2Ds_.end() ) {
      edm::LogError("BaseClass::FillUserTH2D") << "Trying to fill histogram "<<nameAndTitle<<" that was not defined.";
      exit(1);;
  } else {
      nh_h->second->Fill(value_x, value_y, weight);
  }
}

bool
BaseClass::sortCuts(const cut& X, const cut& Y)
{
  return X.id < Y.id;
}
