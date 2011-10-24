#!/usr/bin/env python

import sys, os, string, re
from optparse import OptionParser
from math import *


def UpdateTable(inputTable, outputTable):
    if not outputTable:
        for j,line in enumerate( inputTable ):
            outputTable[int(j)]={'variableName': inputTable[j]['variableName'],
                                 'min1': inputTable[j]['min1'],
                                 'max1': inputTable[j]['max1'],
                                 'min2': inputTable[j]['min2'],
                                 'max2': inputTable[j]['max2'],
                                 'level': inputTable[j]['level'],
                                 'N':       float(inputTable[j]['N']),
                                 'errN':    pow( float(inputTable[j]['errN']), 2 ),
                                 'Npass':       float(inputTable[j]['Npass']),
                                 'errNpass':    pow( float(inputTable[j]['errNpass']), 2 ),
                                 'EffRel':      float(0),
                                 'errEffRel':   float(0),
                                 'EffAbs':      float(0),
                                 'errEffAbs':   float(0),
                                 }
    else:
        for j,line in enumerate( inputTable ):
            outputTable[int(j)]={'variableName': inputTable[j]['variableName'],
                                 'min1': inputTable[j]['min1'],
                                 'max1': inputTable[j]['max1'],
                                 'min2': inputTable[j]['min2'],
                                 'max2': inputTable[j]['max2'],
                                 'level': inputTable[j]['level'],
                                 'N':       float(outputTable[int(j)]['N']) + float(inputTable[j]['N']),
                                 'errN':    float(outputTable[int(j)]['errN']) + pow( float(inputTable[j]['errN']), 2 ),
                                 'Npass':       float(outputTable[int(j)]['Npass']) + float(inputTable[j]['Npass']),
                                 'errNpass':    float(outputTable[int(j)]['errNpass']) + pow( float(inputTable[j]['errNpass']), 2 ),
                                 'EffRel':      float(0),
                                 'errEffRel':   float(0),
                                 'EffAbs':      float(0),
                                 'errEffAbs':   float(0),
                                 }
    return


def CalculateEfficiency(table):
    for j,line in enumerate( table ):
        if( j == 0 ):
            table[int(j)] = {'variableName':       table[int(j)]['variableName'],
                             'min1':        table[int(j)]['min1'],
                             'max1':        table[int(j)]['max1'],
                             'min2':        table[int(j)]['min2'],
                             'max2':        table[int(j)]['max2'],
                             'level':       table[int(j)]['level'],
                             'N':          float(table[j]['N']) ,
                             'errN':       float(0),
                             'Npass':      float(table[j]['Npass']) ,
                             'errNpass':   float(0),
                             'EffRel':     float(1),
                             'errEffRel':  float(0),
                             'EffAbs':     float(1),
                             'errEffAbs':  float(0),
                             }
        else:
            N = float(table[j]['N'])
            errN = sqrt(float(table[j]["errN"]))
            if( float(N) > 0 ):
                errRelN = errN / N
            else:
                errRelN = float(0)

            Npass = float(table[j]['Npass'])
            errNpass = sqrt(float(table[j]["errNpass"]))
            if( float(Npass) > 0 ):
                errRelNpass = errNpass / Npass
            else:
                errRelNpass = float(0)

            if(Npass > 0  and N >0 ):
                EffRel = Npass / N
                errRelEffRel = sqrt( errRelNpass*errRelNpass + errRelN*errRelN )
                errEffRel = errRelEffRel * EffRel
                if(Npass==N):
                    errEffRel = 0

                EffAbs = Npass / float(table[0]['N'])
                errEffAbs = errNpass / float(table[0]['N'])
            else:
                EffRel = 0
                errEffRel = 0
                EffAbs = 0
                errEffAbs = 0

            table[int(j)]={'variableName': table[int(j)]['variableName'],
                           'min1': table[int(j)]['min1'],
                           'max1': table[int(j)]['max1'],
                           'min2': table[int(j)]['min2'],
                           'max2': table[int(j)]['max2'],
                           'level': table[int(j)]['level'],
                           'N':       N,
                           'errN':    errN,
                           'Npass':       Npass,
                           'errNpass':    errNpass,
                           'EffRel':      EffRel,
                           'errEffRel':   errEffRel,
                           'EffAbs':      EffAbs,
                           'errEffAbs':   errEffAbs,
                           }
            #print table[j]
    return


def WriteTable(group, table, precutlist, file):
    print >>file, group
    for myline in precutlist:
        print>>file, myline
    print >>file, "#id".rjust(3),
    print >>file, "variableName".rjust(24),
    print >>file, "min1".rjust(14),
    print >>file, "max1".rjust(14),
    print >>file, "min2".rjust(14),
    print >>file, "max2".rjust(14),
    print >>file, "level".rjust(14),
    print >>file, "N".rjust(20),
    print >>file, "Npass".rjust(20),
    print >>file, "EffRel".rjust(14),
    print >>file, "errEffRel".rjust(14),
    print >>file, "EffAbs".rjust(14),
    print >>file, "errEffAbs".rjust(14)

    for j, line in enumerate(table):
        print >>file, repr(j).rjust(3),
        print >>file, table[j]['variableName'].rjust(24),
        print >>file, table[j]['min1'].rjust(14),
        print >>file, table[j]['max1'].rjust(14),
        print >>file, table[j]['min2'].rjust(14),
        print >>file, table[j]['max2'].rjust(14),
        print >>file, table[j]['level'].rjust(14),
        ###
        if( table[j]['N'] >= 0.1 ):
            print >>file, ("%.04f" % table[j]['N']).rjust(20),
        else:
            print >>file, ("%.04e" % table[j]['N']).rjust(20),
        ###
        if( table[j]['Npass'] >= 0.1 ):
            print >>file, ("%.04f" % table[j]['Npass']).rjust(20),
        else:
            print >>file, ("%.04e" % table[j]['Npass']).rjust(20),
        ###
        if( table[j]['EffRel'] >= 0.1 ):
            print >>file, ("%.04f" % table[j]['EffRel']).rjust(14),
        else:
            print >>file, ("%.04e" % table[j]['EffRel']).rjust(14),
        ###
        if( table[j]['errEffRel'] >= 0.1 ):
            print >>file, ("%.04f" % table[j]['errEffRel']).rjust(14),
        else:
            print >>file, ("%.04e" % table[j]['errEffRel']).rjust(14),
        ###
        if( table[j]['EffAbs'] >= 0.1  ):
            print >>file, ("%.04f" % table[j]['EffAbs']).rjust(14),
        else:
            print >>file, ("%.04e" % table[j]['EffAbs']).rjust(14),
        ###
        if( table[j]['errEffAbs'] >=0.1 ):
            print >>file, ("%.04f" % table[j]['errEffAbs']).rjust(14)
        else:
            print >>file, ("%.04e" % table[j]['errEffAbs']).rjust(14)

        ##########

    print >>file, "\n"

    #--- print to screen

    print ""
    print group
    for myline in precutlist:
        print myline
    print "#id".rjust(3),
    print "variableName".rjust(24),
    print "min1".rjust(14),
    print "max1".rjust(14),
    print "min2".rjust(14),
    print "max2".rjust(14),
    print "level".rjust(14),
    print "N".rjust(14),
    print "Npass".rjust(14),
    print "EffRel".rjust(14),
    print "errEffRel".rjust(14),
    print "EffAbs".rjust(14),
    print "errEffAbs".rjust(14)

    for j, line in enumerate(table):
        print repr(j).rjust(3),
        print table[j]['variableName'].rjust(24),
        print table[j]['min1'].rjust(14),
        print table[j]['max1'].rjust(14),
        print table[j]['min2'].rjust(14),
        print table[j]['max2'].rjust(14),
        print table[j]['level'].rjust(14),
        ###
        if( table[j]['N'] >= 0.1 ):
            print ("%.04f" % table[j]['N']).rjust(14),
        else:
            print ("%.04e" % table[j]['N']).rjust(14),
        ###
        if( table[j]['Npass'] >= 0.1 ):
            print ("%.04f" % table[j]['Npass']).rjust(14),
        else:
            print ("%.04e" % table[j]['Npass']).rjust(14),
        ###
        if( table[j]['EffRel'] >= 0.1 ):
            print ("%.04f" % table[j]['EffRel']).rjust(14),
        else:
            print ("%.04e" % table[j]['EffRel']).rjust(14),
        ###
        if( table[j]['errEffRel'] >= 0.1 ):
            print ("%.04f" % table[j]['errEffRel']).rjust(14),
        else:
            print ("%.04e" % table[j]['errEffRel']).rjust(14),
        ###
        if( table[j]['EffAbs'] >= 0.1  ):
            print ("%.04f" % table[j]['EffAbs']).rjust(14),
        else:
            print ("%.04e" % table[j]['EffAbs']).rjust(14),
        ###
        if( table[j]['errEffAbs'] >=0.1 ):
            print ("%.04f" % table[j]['errEffAbs']).rjust(14)
        else:
            print ("%.04e" % table[j]['errEffAbs']).rjust(14)

        #######################

    return


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

  preCut_lists = {}
  final_tables = {}
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
      preCut_lists[group] = []
      final_tables[group] = {}
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
        
      count_datasets = count_datasets + 1

      data = {}
      column = []
      lineCounter = float(0)
      row = int(-1)

      for j, line in enumerate( open(input_txt_file) ):

          line = string.strip(line,"\n")

          #skip precuts
          if( re.search("^###", line) ):
              ######################################
              ##only once for each group of datasets
              if(count_datasets==0):
                  preCut_lists[group].append(line)
              ######################################
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
      if( xsections[dataset] != "-1" ):
          xsection_x_intLumi = float(xsections[dataset]) * float(options.int_lumi)
          if( Ntot == 0 ):
              weight = float(0)
          else:
              weight = xsection_x_intLumi / Ntot
      print line_elements[0] + " weight: " + str(weight)

      # create new table using weights
      newtable = {}

      for j, line in enumerate( data ):
          if(j == 0):
              newtable[int(j)]={'variableName': data[j]['variableName'],
                                'min1': "-",
                                'max1': "-",
                                'min2': "-",
                                'max2': "-",
                                'level': "-",
                                'N': ( Ntot * weight ),
                                'errN': float(0),
                                'Npass': ( Ntot * weight ),
                                'errNpass': float(0),
                                }
          else:
              N = ( float(data[j]['N']) * weight )
              errN = ( float(data[j-1]["errEffAbs"]) * xsection_x_intLumi )
              #print data[j]['variableName']
              #print "errN: " , errN
              if(str(errN) == "nan"):
                  errN = 0

                  #            if( float(N) > 0 and float(errN) > 0 ):
                  #                errRelN = errN / N
                  #            else:
                  #                errRelN = float(0)

              Npass = ( float(data[j]['Npass']) * weight )
              errNpass = ( float(data[j]["errEffAbs"]) * xsection_x_intLumi )
              #print "errNPass " , errNpass
              #print ""
              if(str(errNpass) == "nan"):
                  errNpass = 0

                  #            if( float(Npass) > 0 and float(errNpass) > 0 ):
                  #                errRelNpass = errNpass / Npass
                  #            else:
                  #                errRelNpass = float(0)

              newtable[int(j)]={'variableName': data[j]['variableName'],
                                'min1': data[j]['min1'],
                                'max1': data[j]['max1'],
                                'min2': data[j]['min2'],
                                'max2': data[j]['max2'],
                                'level': data[j]['level'],
                                'N':     N,
                                'errN':  errN,
                                'Npass': Npass,
                                'errNpass': errNpass,
                                }
                             
      # combine efficiency tables from different datasets in one table
      UpdateTable(newtable, final_tables[group])

  # final output file
  output_txt_file = open( os.path.join(output_dir,'Final__cutEfficiency.txt'), 'w' )

  groups = final_tables.keys()
  groups.sort()
  for group in groups:
   
    # calculate efficiency for each step of the analysis
    CalculateEfficiency(final_tables[group])

    # write table
    WriteTable(group, final_tables[group], preCut_lists[group], output_txt_file)

  output_txt_file.close()
    
  print ''
  print "Final cutEfficiency file: " + os.path.join(output_dir,'Final__cutEfficiency.txt')
  print ''


if __name__ == "__main__":
  main()

