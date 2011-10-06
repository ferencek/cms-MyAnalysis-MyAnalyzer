Introduction:
-------------

This package provides a small facility to analyze events stored in the CMSSW EDM format.
This analysis package can therefore be run over RECO, AOD, various types of PAT-tuples as
well as simple ROOT trees as long as they are saved in the EDM format.

This package is based on a standalone analysis framework developed by Paolo Rumerio. Different
versions of this framework can be found in the following CVS locations (listed in chronological order
from older to more recent versions):
http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/prumerio/Leptoquark/rootNtupleAnalyzer/
http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/Leptoquarks/rootNtupleAnalyzer/
http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/Leptoquarks/rootNtupleAnalyzerPAT/
http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/Leptoquarks/rootNtupleaAnalyzerV2/
Additional contributions to the framework development were made by Edmund Berry, Dinko Ferencek,
Jeff Temple, and Francesco Santanastasio.


Instructions:
-------------

1) Set up your CMSSW working area:

   scram project -n CMSSW_4_2_4_MyAnalysis CMSSW CMSSW_4_2_4
   cd CMSSW_4_2_4_MyAnalysis/src
   cmsenv

   NOTE: You can skip this step if you already have your working area set up.
         CMSSW_4_2_4 is used just as an example.

2) Checkout the package:

   cvs co -d MyAnalysis/MyAnalyzer/ UserCode/ferencek/MyAnalysis/MyAnalyzer/

   The package has the following structure:

   BuildFile.xml
   doc/README.txt
   interface/BaseClass.h
   src/BaseClass.cc
   test/MyAnalyzer.cc
   test/cutFileExample.txt
   test/myAnalyzerExample_cfg.py

3) Copy the example analysis file into the src subdirectory:

   cd MyAnalysis/MyAnalyzer/
   cp -i test/MyAnalyzer.cc src/MyAnalyzer.cc

   and start developing your analysis code by modifying the src/MyAnalyzer.cc file.

   NOTE: You might end up having several different versions of the analysis code, such as

         MyAnalyzer_ver1.cc
         MyAnalyzer_ver2.cc
         MyAnalyzer_ver3.cc
         ...

         In such a case, you might find it more convenient to keep your code in a separate
         location and create a link to the version of the analysis code you would like to
         be used:

         ln -f /...fullPath.../MyAnalyzer_ver1.cc  src/MyAnalyzer.cc
         
4) Compile the code:

   scram b -j4

5) Run the code:

   cd test/
   cmsRun myAnalyzerExample_cfg.py

   NOTE: If you end up having multiple versions of the analysis code, you will presumably also
         have multiple versions of the cut file and the Python configuration file

         cutFile_ver1.txt
         myAnalyzer_cfg_ver1.py
         cutFile_ver2.txt
         myAnalyzer_cfg_ver2.py
         cutFile_ver3.txt
         myAnalyzer_cfg_ver3.py
         ...

         You can then create symbolic links to the version of the cut file and the Python
         configuration file

         ln -sf /...fullPath.../cutFile_ver1.txt  cutFile.txt
         ln -sf /...fullPath.../myAnalyzer_cfg_ver1.txt  myAnalyzer_cfg.py

         and use those to run your analysis code.


More details:
-------------

- Providing cuts via a text file:

A list of cut variable names and cut limits can be provided through a file (see test/cutFileExample.txt).
The variable names in such a file have to be filled with a value calculated by the user analysis code. For
this purpose, a function "fillVariableWithValue" is provided (see example code).
Once all the cut variables have been filled, the cuts can be evaluated by calling "evaluateCuts" (see
example code). Do not forget to reset the cuts by calling "resetCuts" at each event before filling the
variables (see example code).

The function "evaluateCuts" determines whether the cuts are satisfied or not, stores the pass/failed result
of each cut, calculates cut efficiencies and fills histograms for each cut variable (binning provided by the
cut file, see test/cutFileExample.txt). The user has access to the cut results via a set of functions
(see interface/BaseAnalyzer.h):

  bool passedCut(const string& s);
  bool passedAllPreviousCuts(const string& s);
  bool passedAllOtherCuts(const string& s);
  bool passedAllOtherSameAndLowerLevelCuts(const string& s)

where the string to be passed is the cut variable name. The cuts are evaluated following the order of their
apperance in the cut file (test/cutFileExample.txt). One can simply change the order of lines in the cut file
to have the cuts applied in a different order and perform cut efficiency studies (no changes or recompilation
of the C++ code are required).

Also, the user can assign to each cut a level (0,1,2,3,4 ... n) and use a function

  bool passedCut(const string& s);

to have the pass/failed info on all cuts with the same level.

There are also cuts with level = -1. These cuts are not actually evaluated, and the corresponding lines
in the cut file (test/cutFileExample.txt) are used to pass values to the user analysis code (such as fiducial
region limits, etc.). The user can access these values usning the following functions

  double getPreCutValue1(const string& s);
  double getPreCutValue1(const string& s);
  double getPreCutValue2(const string& s);
  double getPreCutValue2(const string& s);

It is also possible to get values of the cuts with level >= 0 using

  double getCutMinValue1(const string& s);
  double getCutMaxValue1(const string& s);
  double getCutMinValue2(const string& s);
  double getCutMaxValue2(const string& s);


- Automatic histograms for cuts:

The following histograms are generated for each cut variable with level >= 0:

  skim or no cuts applied
  passedAllPreviousCuts
  passedAllOtherSameLevelCuts
  passedAllOtherCuts
  passedAllCut

and by default they are all saved to the output root file. The behavior can be controlled
via the following Python configuration file switches

   fillSkimOrNoCuts
   fillAllPreviousCuts
   fillAllOtherCuts
   fillAllSameLevelAndLowerLevelCuts
   fillAllCuts


- Automatic cut efficiency:

The absolute and relative efficiency is calculated for each cut and stored in an output file
(the name of this output file is controlled by the 'outputCutEfficiencyFile' parameter in
test/myanalyzer_cfg.py).


Using the Optimizer (Jeff Temple):
----------------------------------

The input cut file can also specify variables to be used in optimization studies.
To do so, add a line in the file for each variable to optimize. The first field of a line
must be the name of the variable, second field must be "OPT", third field either ">" or "<".
(The ">" sign will pass values greater than the applied threshold, and "<" will pass
those less than the threshold). 4th and 5th fields should be the minimum
and maximum thresholds you wish to apply when scanning for optimal cuts.
An example of the optimization syntax is:

# VariableName     must be OPT   > or <    RangeMin        RangeMax        unused
#-------------     -----------   ------    ------------    -------------   ------
MuonPt                OPT          >          10              55              5

This optimizer will scan 10 different values, evenly distributed over
the inclusive range [RangeMin, RangeMax]. At the moment, the 6th value is not used and
does not need to be specified. The optimization cuts are always run after all the other
cuts in the file, and are only run when all other cuts are passed. The above line will
make 10 different cuts on MuonPt, at [10, 15, 20, 25, ..., 55]. ('5' in the 6th field
is meaningless here). The output of the optimization will be a 10-bin histogram, showing
the number of events passing each of the 10 thresholds.

Multiple optimization cuts may be applied in the same file.  In the case where N optimization cuts
are applied, a histogram of 10^N bins will be produced, with each bin corresponding to a unique cut
combination. No more than 6 variables may be optimized at one time (limitation in the number of bins
for a TH1F ~ 10^6). A file that lists the cut values applied for each bin can be produced by setting
the 'writeOptimizationCutFile' parameter test/myanalyzer_cfg.py to True (the name of this file is
controlled by the 'optimizationCutFileName' parameter and its default value is "OptimizationCuts.txt").
Since this file can be quite large (10^N lines), by default it is not created.
