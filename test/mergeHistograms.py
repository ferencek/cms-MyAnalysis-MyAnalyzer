#!/usr/bin/env python

import sys, os, string, re, copy
from optparse import OptionParser
from math import *
from ROOT import *


def main():
  # usage description
  usage = "Usage: mergeTables.py [options] \nExample: ./mergeTables.py -m datasetListForMerging.txt -x xsections.txt -l 3000 -i CRAB_Jobs"

  # input parameters
  parser = OptionParser(usage=usage)

  parser.add_option("-m", "--dataset_list_for_merging", dest="dataset_list_for_merging",
                    help="List of datasets to be merged",
                    metavar="DATASET_LIST_FOR_MERGING")

  parser.add_option("-x", "--xsection_file", dest="xsection_file",
                    help="File containing cross sections (in pb) for all datasets to be merged",
                    metavar="XSECTION_FILE")

  parser.add_option("-l", "--int_lumi", dest="int_lumi",
                    help="Integrated luminosity (in /pb)",
                    metavar="INT_LUMI")

  parser.add_option("-i", "--input_dir", dest="input_dir",
                    help="Input directory",
                    metavar="INPUT_DIR")

  parser.add_option("-o", "--output_dir", dest="output_dir",
                    help="Output directory (This parameter is optional)",
                    metavar="OUTPUT_DIR")

  parser.add_option("-a", "--analyzer_module", dest="analyzer_module",
                    help="Name of the analyzer module (This parameter is optional and is set to 'myAnalyzer' by default)",
                    default='myAnalyzer',
                    metavar="OUTPUT_DIR")

  (options, args) = parser.parse_args()

  # make sure all necessary input parameters are provided
  if not (options.dataset_list_for_merging and options.xsection_file and options.int_lumi and options.input_dir):
    print usage
    sys.exit()

  input_dir = options.input_dir
  # redefine input_dir as an absolute path (if not defined in such form already)
  if not re.search('^/', input_dir):
    input_dir = os.path.join(os.getcwd(),input_dir)

  output_dir = input_dir
  if options.output_dir:
    output_dir = options.output_dir
    # redefine output_dir as an absolute path (if not defined in such form already)
    if not re.search('^/', output_dir):
      output_dir = os.path.join(os.getcwd(),output_dir)

  xsections = {}
  # open and read the xsection_file
  xsection_file = open(options.xsection_file,'r')
  xsection_file_lines = xsection_file.readlines()

  for line in xsection_file_lines:

    line_elements = line.split()
    if (len(line_elements)==0 or line_elements[0][0]=='#'): continue

    dataset_key = line_elements[0].lstrip('/').rstrip('/USER')[:-33].replace('/','__')
    xsection_val = line_elements[1]

    xsections[dataset_key] = xsection_val

  final_histos = {}
  group = ''
  count_datasets = -1
  # open and read the dataset_list_for_merging
  dataset_list_for_merging = open(options.dataset_list_for_merging,'r')
  dataset_list_for_merging_lines = dataset_list_for_merging.readlines()

  for line in dataset_list_for_merging_lines:

    line_elements = line.split()
    if (len(line_elements)==0 or line_elements[0][0]=='#'): continue

    if re.search(':', line):
      group = line_elements[0].rstrip(':')
      final_histos[group] = {}
      count_datasets = -1
      print group
    else:
      dataset = line_elements[0].lstrip('/').rstrip('/USER')[:-33].replace('/','__')
      if dataset not in xsections.keys():
        print 'ERROR: Cross section for ' + line_elements[0] + ' not found'
        print 'Aborting'
        sys.exit(1)
      input_txt_file  = os.path.join(input_dir,dataset + '__cutEfficiency.txt')
      if(os.path.isfile(input_txt_file) == False):
        print 'ERROR: File ' + input_txt_file + ' not found'
        print 'Aborting'
        sys.exit(1)
      input_root_file  = os.path.join(input_dir,dataset + '__histograms.root')
      if(os.path.isfile(input_root_file) == False):
        print 'ERROR: File ' + input_root_file + ' not found'
        print 'Aborting'
        sys.exit(1)
        
      count_datasets = count_datasets + 1

      data = {}
      column = []
      lineCounter = float(0)
      row = int(-1)

      for j, line in enumerate( open(input_txt_file) ):

          line = string.strip(line,'\n')

          #skip precuts
          if( re.search('^###', line) ):
              continue

          if lineCounter == 0:
              for i,element in enumerate(line.split()):
                  column.append(element)
          else:
              for i,element in enumerate(line.split()):
                  if i == 0:
                      row = int(element)
                      data[row] = {}
                  else:
                      data[row][ column[i] ] = element
                      #print data[row][ column[i] ]

          lineCounter = lineCounter + 1

      # calculate weight
      Ntot = float(data[0]['N'])
      weight = 1.0
      xsection_x_intLumi = Ntot
      if( xsections[dataset] != '-1' ):
          xsection_x_intLumi = float(xsections[dataset]) * float(options.int_lumi)
          if( Ntot == 0 ):
              weight = float(0)
          else:
              weight = xsection_x_intLumi / Ntot
      print line_elements[0] + ' weight: ' + str(weight)

      # open input ROOT file
      root_file = TFile(input_root_file)

      # get the number of histograms
      nHistos = root_file.Get(options.analyzer_module).GetListOfKeys().GetEntries()
      
      # loop over histograms in the input ROOT file
      for h in range(0, nHistos):
        histoName = root_file.Get(options.analyzer_module).GetListOfKeys()[h].GetName()
        #print histoName
        htemp = root_file.Get(os.path.join(options.analyzer_module,histoName))

        if(count_datasets==0):
          final_histos[group][histoName] = copy.deepcopy(htemp)
          final_histos[group][histoName].SetName(group + '__' + histoName)
          final_histos[group][histoName].Scale(weight)
        else:
          final_histos[group][histoName].Add(htemp, weight)

  print ''
  # final output file
  output_root_file = TFile( os.path.join(output_dir,'Final__histograms.root'), 'RECREATE' )

  # write histrograms
  groups = final_histos.keys()
  groups.sort()
  for group in groups:
    histos = final_histos[group].keys()
    histos.sort()
    for histo in histos:
      print "Writing histo: " , final_histos[group][histo].GetName()
      final_histos[group][histo].Write()

  output_root_file.Close()
    
  print ''
  print 'Final histograms file: ' + os.path.join(output_dir,'Final__histograms.root')
  print ''


if __name__ == '__main__':
  main()

