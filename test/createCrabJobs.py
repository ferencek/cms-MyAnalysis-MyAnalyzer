#!/usr/bin/env python

import sys, os, shutil, re
from optparse import OptionParser


def main():
  # usage description
  usage = "Usage: createCrabJobs.py [options] \nExample: ./createCrabJobs.py -w CRAB_Jobs -d datasetListExample.txt -c myAnalyzer_cfg.py -f cutFileExample.txt -t crab_template.cfg"

  # input parameters
  parser = OptionParser(usage=usage)

  parser.add_option("-w", "--main_workdir", dest="main_workdir",
                    help="Main working directory",
                    metavar="MAIN_WORKDIR")

  parser.add_option("-d", "--dataset_list", dest="dataset_list",
                    help="Text file containing a list of datasets to be processed",
                    metavar="DATASET_LIST")

  parser.add_option("-c", "--cmssw_cfg", dest="cmssw_cfg",
                    help="CMSSW configuration file",
                    metavar="CMSSW_CFG")

  parser.add_option("-f", "--cut_file", dest="cut_file",
                    help="Cut file",
                    metavar="CUT_FILE")

  parser.add_option("-t", "--crab_cfg_template", dest="crab_cfg_template",
                    help="CRAB configuration file template",
                    metavar="CRAB_CFG_TEMPLATE")

  parser.add_option("-n", "--no_creation",
                    action="store_true", dest="no_creation", default=False,
                    help="Create the necessary configuration files and skip the job creation (This parameter is optional)")

  (options, args) = parser.parse_args()

  # make sure all necessary input parameters are provided
  if not (options.main_workdir and options.dataset_list and options.cmssw_cfg and options.crab_cfg_template and options.cut_file):
    print usage
    sys.exit()

  main_workdir = options.main_workdir
  dataset_list = options.dataset_list
  cmssw_cfg = options.cmssw_cfg
  cut_file = options.cut_file
  crab_cfg_template = options.crab_cfg_template

  # redefine main_workdir as an absolute path (if not defined in such form already)
  if not re.search("^/", main_workdir):
    main_workdir = os.path.join(os.getcwd(),main_workdir)

  # define path for the cfg_files_dir
  cfg_files_dir = os.path.join(main_workdir,'cfg_files')

  # create the main working directory and 'cfg_files' subdirectory
  os.mkdir(main_workdir)
  os.mkdir(cfg_files_dir)

  # copy the dataset list file to the main_workdir
  shutil.copyfile(dataset_list,os.path.join(main_workdir,'datasetList.txt'))

  # copy the CMSSW cfg file to the cfg_files_dir
  shutil.copyfile(cmssw_cfg,os.path.join(cfg_files_dir,'CMSSW_cfg.py'))

  # copy the cut file to the cfg_files_dir
  shutil.copyfile(cut_file,os.path.join(cfg_files_dir,'cutFile.txt'))

  # read the CRAB cfg template
  crab_cfg_template_file = open(crab_cfg_template,'r')
  crab_cfg_template_content = crab_cfg_template_file.read()

  # open and read the dataset_list file
  dataset_list_file = open(dataset_list,"r")
  dataset_list_lines = dataset_list_file.readlines()

  # loop over datasets
  for line in dataset_list_lines:
    line_elements = line.split()
    if (len(line_elements)==0 or line_elements[0][0]=='#'): continue

    prefix = line_elements[0].lstrip('/').rstrip('/USER')[:-33].replace('/','__')

    crab_cfg_content = crab_cfg_template_content
    crab_cfg_content = re.sub('SCHEDULER',line_elements[5],crab_cfg_content)
    crab_cfg_content = re.sub('USE_SERVER',line_elements[6],crab_cfg_content)
    crab_cfg_content = re.sub('DATASET_NAME',line_elements[0],crab_cfg_content)
    if(line_elements[7] != '-'):
      crab_cfg_content = re.sub('DBS_INSTANCE',line_elements[7],crab_cfg_content)
    else:
      crab_cfg_content = re.sub('dbs_url','#dbs_url',crab_cfg_content)
    crab_cfg_content = re.sub('CFG_FILE',os.path.join(cfg_files_dir,'CMSSW_cfg.py'),crab_cfg_content)
    if(line_elements[3] != '-'):
      crab_cfg_content = re.sub('RUN_SELECTION',line_elements[3],crab_cfg_content)
    else:
      crab_cfg_content = re.sub('runselection','#runselection',crab_cfg_content)
    if(line_elements[4] != '-'):
      # location of the lumi mask file (can be specified as an absolute path or a relative path with respect to the dataset list file)
      lumi_mask_location = (line_elements[4] if re.search("^/", line_elements[4]) else os.path.join(os.path.split(dataset_list)[0],line_elements[4]))
      # copy the lumi mask file to the cfg_files_dir
      shutil.copyfile(lumi_mask_location,os.path.join(cfg_files_dir,prefix + '_lumi_mask.txt'))
      crab_cfg_content = re.sub('LUMI_MASK',os.path.join(cfg_files_dir,prefix + '_lumi_mask.txt'),crab_cfg_content)
    else:
      crab_cfg_content = re.sub('lumi_mask','#lumi_mask',crab_cfg_content)
    crab_cfg_content = re.sub('TOTAL_LUMIS',line_elements[1],crab_cfg_content)
    crab_cfg_content = re.sub('N_JOBS',line_elements[2],crab_cfg_content)

    crab_cfg_content = re.sub('OUTPUT_FILES','%s__histograms.root, %s__cutEfficiency.txt'%(prefix,prefix),crab_cfg_content)
    cfg_parameters = 'outputPrefix=%s'%prefix
    if( len(line_elements)>10 ):
      for param in range(10,len(line_elements)):
        cfg_parameters = cfg_parameters + ' ' + line_elements[param]
    crab_cfg_content = re.sub('CFG_PARAMETERS',cfg_parameters,crab_cfg_content)
    crab_cfg_content = re.sub('INPUT_FILES',os.path.join(cfg_files_dir,'cutFile.txt'),crab_cfg_content)
    crab_cfg_content = re.sub('WORKING_DIR',os.path.join(main_workdir,prefix),crab_cfg_content)
    crab_cfg_content = re.sub('OUTPUT_DIR',os.path.join(main_workdir,prefix,'output'),crab_cfg_content)
    crab_cfg_content = re.sub('PUBLICATION_NAME',line_elements[9] + (('_' + line_elements[3]) if line_elements[3] != '-' else ''),crab_cfg_content)

    # create a CRAB cfg file
    crab_cfg_name = os.path.join(cfg_files_dir,prefix + '_crab.cfg')
    crab_cfg = open(crab_cfg_name,'w')
    crab_cfg.write(crab_cfg_content)
    crab_cfg.close()
    if not options.no_creation:
      # create CRAB jobs
      os.system('crab -create -cfg ' + crab_cfg_name)

  # close all open files
  crab_cfg_template_file.close()
  dataset_list_file.close()


if __name__ == "__main__":
  main()

