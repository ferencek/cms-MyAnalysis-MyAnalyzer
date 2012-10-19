-------------------
Creating CRAB Jobs:
-------------------

For automatic creation of CRAB jobs for several different datasets, the createCrabJobs.py script, located
in the test/ subdirectory of the MyAnalyzer package, can be used. For a brief description of the input
parameters, execute

./createCrabJobs.py --help

which will produce the following output

Usage: createCrabJobs.py [options]
Example: ./createCrabJobs.py -w CRAB_Jobs -d datasetList.txt -c myAnalyzer_cfg.py -f cutFile.txt -t crab_template.cfg

Options:
  -h, --help            show this help message and exit
  -w MAIN_WORKDIR, --main_workdir=MAIN_WORKDIR
                        Main working directory
  -d DATASET_LIST, --dataset_list=DATASET_LIST
                        Text file containing a list of datasets to be
                        processed
  -c CMSSW_CFG, --cmssw_cfg=CMSSW_CFG
                        CMSSW configuration file
  -f CUT_FILE, --cut_file=CUT_FILE
                        Cut file
  -t CRAB_CFG_TEMPLATE, --crab_cfg_template=CRAB_CFG_TEMPLATE
                        CRAB configuration file template
  -n, --no_creation     Create the necessary configuration files and skip the
                        job creation (This parameter is optional)

The -w (--main_workdir) parameter defines the name of the main working directory, the -d (--dataset_list)
parameter specifies the location of a text file containing a list of datasets to be processed, the -c (--cmssw_cfg)
parameter specifies the location of a common CMSSW configuration file, the -f (--cut_file) parameter specifies the
location of a cut file, and the -t (--crab_cfg_template) parameter specifies the location of a CRAB configuration file
template. With the input parameters provided, the createCrabJobs.py script creates the following file and directory structure

MAIN_WORKDIR/ (Main working directory)
    |
    ------> datasetList.txt (Text file defining a list of datasets to be processed)
    |
    ------> cfg_files/ (Directory containing CMSSW and CRAB configuration files)
    |          |
    |          ------> cutFile.txt (Common cut file)
    |          |
    |          ------> CMSSW_cfg.py (Common CMSSW configuration file)
    |          |
    |          ------> DATASET1_crab.cfg (CRAB configuration file for DATASET1)
    |          |
    |          ------> DATASET1_lumi_mask.txt (Lumi mask file for DATASET1)
    |          |
    |          ------> DATASET2_crab.cfg (CRAB configuration file for DATASET2)
    |          |
    |          ------> DATASET2_lumi_mask.txt (Lumi mask file for DATASET2)
    |          |
    |          ...
    |
    ------> DATASET1/ (CRAB working directory for DATASET1)
    |
    ------> DATASET2/ (CRAB working directory for DATASET2)
    |
    ...

The dataset list file contains the following entries

# Dataset_name                                                                                   Number_of_lumis   Number_of_jobs   Run_selection                                                      Lumi_mask   Scheduler   Use_server             DBS_instance   Publish_data                                  Publication_name   PyCfg_parameters (optional)
/Jet/ferencek-Run2011A-May10ReReco-v1_EDMTuple_V00-00-01-6d363a44e73023f68a8f3b11d67becad/USER                -1               80               -   Cert_160404-163869_7TeV_May10ReReco_Collisions11_JSON_v3.txt      condor            0   cms_dbs_ph_analysis_01              0   Run2011A-May10ReReco-v1_EDMTuple_V00-00-01_skim   globalTag=GR_R_42_V20::All
/Jet/ferencek-Run2011A-PromptReco-v4_EDMTuple_V00-00-01-6d363a44e73023f68a8f3b11d67becad/USER                 -1              100               -       Cert_160404-177515_7TeV_PromptReco_Collisions11_JSON.txt      condor            0   cms_dbs_ph_analysis_01              0    Run2011A-PromptReco-v4_EDMTuple_V00-00-01_skim   globalTag=GR_R_42_V20::All
/Jet/ferencek-Run2011A-05Aug2011-v1_EDMTuple_V00-00-01-6d363a44e73023f68a8f3b11d67becad/USER                  -1               40               -    Cert_170249-172619_7TeV_ReReco5Aug_Collisions11_JSON_v2.txt      condor            0   cms_dbs_ph_analysis_01              0     Run2011A-05Aug2011-v1_EDMTuple_V00-00-01_skim   globalTag=GR_R_42_V20::All
/Jet/ferencek-Run2011A-PromptReco-v6_EDMTuple_V00-00-01-6d363a44e73023f68a8f3b11d67becad/USER                 -1               40               -       Cert_160404-177515_7TeV_PromptReco_Collisions11_JSON.txt      condor            0   cms_dbs_ph_analysis_01              0    Run2011A-PromptReco-v6_EDMTuple_V00-00-01_skim   globalTag=GR_R_42_V20::All

These entries are used to define the working directories and CRAB configuration files for different datasets. Note that
the PyCfg_parameters entry is optional and can be left out. However, all other entries must be defined (if you don't want
to apply any run selection or lumi mask, just set them to -). The lumi mask file can be specified as an absolute path or
as a relative path with respect to the dataset list file.


--------------------------------
Other Operations with CRAB Jobs:
--------------------------------

Once the CRAB jobs for different datasets have been created, you can manually submit them

crab -submit -c MAIN_WORKDIR/DATASET1/

check their status

crab -status -c MAIN_WORKDIR/DATASET1/

and finally get their output.

crab -getoutput -c MAIN_WORKDIR/DATASET1/

To obtain the final report (including the output lumi summary file), execute one final time

crab -status -c MAIN_WORKDIR/DATASET1/

followed by

crab -report -c MAIN_WORKDIR/DATASET1/

For publication of the output EDM files, execute

crab -publish -c MAIN_WORKDIR/DATASET1/

However, keep in mind that the publication step can be relatively long. It is therefore recommended to start the publication
step inside a screen session.

If you have many datasets in your dataset list file, you will probably find it more convenient to use the otherOperationsWithCrabJobs.py
script provided in the test/ subdirectory of the MyAnalyzer package. This script enables you to perform the following
operations on your CRAB jobs:

    status
    submit
    getoutput
    report
    publish
    kill

Note that the otherOperationsWithCrabJobs.py script can only perform the above operations on all jobs of a given dataset. To
get output, kill, or (re)submit a job range, a manual procedure described above has to be followed. For a brief description of
available input parameters, execute

./otherOperationsWithCrabJobs.py --help

which will produce the following output

Usage: otherOperationsWithCrabJobs.py [options]
Example: ./otherOperationsWithCrabJobs.py -w CRAB_Jobs

Options:
  -h, --help            show this help message and exit
  -w MAIN_WORKDIR, --main_workdir=MAIN_WORKDIR
                        Main working directory
  -s, --submit          Submit CRAB jobs (This parameter is optional)
  -g, --getoutput       Get output from CRAB jobs (This parameter is optional)
  -r, --report          Get CRAB report (This parameter is optional)
  -p, --publish         Publish the output of CRAB jobs (This parameter is
                        optional)
  -k, --kill            Kill CRAB jobs (This parameter is optional)

Note that only one optional parameter at a time is allowed. With no optional parameters specified, the CRAB -status command is called.

Once the CRAB jobs have successfully finished and their output has been obtained, you can combine their output using the combineOutput.py
script provided in the test/ subdirectory of the MyAnalyzer package. Using this script, you will get one cut efficiency and one
histograms file per dataset. For a brief description of available input parameters, execute

./combineOutput.py --help

which will produce the following output

Usage: combineOutput.py [options]
Example: ./combineOutput.py -w CRAB_Jobs

Options:
  -h, --help            show this help message and exit
  -w MAIN_WORKDIR, --main_workdir=MAIN_WORKDIR
                        Main working directory
  -o OUTPUT_DIR, --output_dir=OUTPUT_DIR
                        Output directory (This parameter is optional)

The -w (--main_workdir) parameter specifies the main working directory containing individual CRAB working directories, and the
-o (--output_dir) parameter specifies the output directory where the combined files will be stored. Note that the -o (--output_dir)
parameter is optional, and if it is not explicitly defined, the combined files will be stored in the main working directory.

