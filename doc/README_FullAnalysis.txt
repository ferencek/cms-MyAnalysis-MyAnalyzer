This file gives a short overview of the steps necessary to run a full analysis chain over several data
and/or MC datasets. However, before you continue reading this file, you should read the README.txt and
README_CRAB.txt files located in the same directory as this file and familiarize yourself with their
content.

-------------
Instructions:
-------------

1) Download, compile and test locally your analysis code following instructions in the README.txt file.
   A good way to manage your analysis code and the corresponding cut file is to have them in a separate
   location (preferably also stored in CVS) and simply create links to them, as described in the README.txt
   file.

2) Once your analysis code has been tested and works as expected, you are ready to run it over datasets of
   interest to your analysis. For this, you will have to create a dataset list file. As with the analysis code,
   a good way to mange your dataset list files is to store them in a separate location and simply create
   a symbolic link to one of them in the test/ subdirectory of the MyAnalyzer package. An example dataset list
   file can be found at the following link:
   http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/ferencek/MyAnalysis/AnalyzerCode/datasetList_DijetBBTag_2011.txt
   
3) Once the dataset list file has been prepared, you should follow the instructions described in the
   README_CRAB.txt file. A likely sequence of steps is the following one (all executed in the test/
   subdirectory of the MyAnalyzer package):

   ./createCrabJobs.py -w CRAB_Jobs -d datasetList.txt -c myAnalyzer_cfg.py -f cutFile.txt -t crab_template.cfg  ---> to create CRAB jobs

   ./otherOperationsWithCrabJobs.py -w CRAB_Jobs/ -s                                                             ---> to submit the CRAB jobs

   ./otherOperationsWithCrabJobs.py -w CRAB_Jobs/                                                                ---> to check the status of the CRAB jobs

   ./otherOperationsWithCrabJobs.py -w CRAB_Jobs/ -g                                                             ---> to get the output of the CRAB jobs

   ./otherOperationsWithCrabJobs.py -w CRAB_Jobs/ -r                                                             ---> to get the final report for the CRAB jobs
                                                                                                                      (this can be skipped for MC datasets)

4) Once the CRAB jobs have successfully finished and their output has been obtained, you will have to combine
   their output in order to get a single cut efficiency and histograms file per dataset:

   ./combineOutput.py -w CRAB_Jobs/

Pre-5) In case both data and MC datasets have been processed, and MC datasets have to be scaled to match data,
       the processed integrated luminosity has to be calculated. To do this, the lumiCal2.py script, described
       in https://twiki.cern.ch/twiki/bin/view/CMS/LumiCalc, is used:

       lumiCalc2.py -i CRAB_Jobs/DATA_DATASET1/res/lumiSummary.json overview
       lumiCalc2.py -i CRAB_Jobs/DATA_DATASET2/res/lumiSummary.json overview
       ...

       Finally, add up the processed luminosities for all real data datasets to get the total processed luminosity.
 
5) In the final step, you will have to merge histograms and cut efficiency tables for different datasets since
   it is often the case that a given physics process is divided into several separate datasets. The same is true
   for real data as well. To perform this step, you will have to create a list of datasets to be merged and a file
   containing cross sections for datasets being merged. As already suggested above for other types of files, a good
   way to manage these files is to have them stored in a separate location and create symbolic links to them in their
   test/ subdirectory of the MyAnalyzer package. An example dataset list for merging file can be found at the
   following link:
   http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/ferencek/MyAnalysis/AnalyzerCode/datasetListForMerging_DijetBBTag_2011.txt
   and an example cross sections file at:
   http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/ferencek/MyAnalysis/AnalyzerCode/xsections_DijetBBTag_2011.txt

   A possible set of commands to merge histograms and cut efficiency tables is the following one:

   ./mergeTables.py -m datasetListForMerging.txt -x xsections.txt -l 2176 -i CRAB_Jobs/
   ./mergeHistograms.py -m datasetListForMerging.txt -x xsections.txt -l 2176 -i CRAB_Jobs/

