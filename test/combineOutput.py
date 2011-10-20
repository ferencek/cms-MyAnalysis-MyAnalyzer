#!/usr/bin/env python

import sys, os, subprocess, string, re
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
        if( j == 0):
            table[int(j)] = {'variableName':       table[int(j)]['variableName'],
                             'min1':        table[int(j)]['min1'],
                             'max1':        table[int(j)]['max1'],
                             'min2':        table[int(j)]['min2'],
                             'max2':        table[int(j)]['max2'],
                             'level':       table[int(j)]['level'],
                             'N':          float(table[j]['N']) ,
                             'errN':       int(0),
                             'Npass':      float(table[j]['Npass']) ,
                             'errNpass':   int(0),
                             'EffRel':     int(1),
                             'errEffRel':  int(0),
                             'EffAbs':     int(1),
                             'errEffAbs':  int(0),
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


def WriteTable(table, precutlist, file):
    for myline in precutlist:
        print>>file, myline
    print >>file, "#id".rjust(3),
    print >>file, "variableName".rjust(24),
    print >>file, "min1".rjust(14),
    print >>file, "max1".rjust(14),
    print >>file, "min2".rjust(14),
    print >>file, "max2".rjust(14),
    print >>file, "level".rjust(14),
    print >>file, "N".rjust(14),
    print >>file, "Npass".rjust(14),
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
            print >>file, ("%.04f" % table[j]['N']).rjust(14),
        else:
            print >>file, ("%.04e" % table[j]['N']).rjust(14),
        ###
        if( table[j]['Npass'] >= 0.1 ):
            print >>file, ("%.04f" % table[j]['Npass']).rjust(14),
        else:
            print >>file, ("%.04e" % table[j]['Npass']).rjust(14),
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
  usage = "Usage: combineOutput.py [options] \nExample: ./combineOutput.py -w CRAB_Jobs"

  # input parameters
  parser = OptionParser(usage=usage)

  parser.add_option("-w", "--main_workdir", dest="main_workdir",
                    help="Main working directory",
                    metavar="MAIN_WORKDIR")

  parser.add_option("-o", "--output_dir", dest="output_dir",
                    help="Output directory (This parameter is optional)",
                    metavar="OUTPUT_DIR")

  (options, args) = parser.parse_args()

  # make sure all necessary input parameters are provided
  if not options.main_workdir:
    print usage
    sys.exit()

  main_workdir = options.main_workdir

  # redefine main_workdir as an absolute path (if not defined in such form already)
  if not re.search("^/", main_workdir):
    main_workdir = os.path.join(os.getcwd(),main_workdir)

  output_dir = main_workdir
  if options.output_dir:
    output_dir = options.output_dir
    # redefine output_dir as an absolute path (if not defined in such form already)
    if not re.search("^/", output_dir):
      output_dir = os.path.join(os.getcwd(),output_dir)

  # open and read the dataset_list file
  dataset_list_file = open(os.path.join(main_workdir,'datasetList.txt'),"r")
  dataset_list_lines = dataset_list_file.readlines()

  # loop over datasets
  for line in dataset_list_lines:
   
    line_elements = line.split()
    if (len(line_elements)==0 or line_elements[0][0]=='#'): continue

    workdir = line_elements[0].lstrip('/').rstrip('/USER')[:-33].replace('/','__')
    print workdir
    
    # final output
    output_root_file = os.path.join(output_dir,workdir + '__histograms.root')
    output_txt_file  = os.path.join(output_dir,workdir + '__cutEfficiency.txt')

    crab_output_dir = os.path.join(main_workdir,workdir,'output')

    # merge .root files
    print 'Merging .root files...'
    cmd = 'hadd -f ' + output_root_file + ' ' + os.path.join(crab_output_dir,'*.root')
    proc = subprocess.Popen( cmd, shell=True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )
    output = proc.communicate()[0]
    if proc.returncode != 0:
      print output
      sys.exit(1)

    # merge .txt files
    print 'Merging .txt files...'
    
    dict_final_table = {}
    preCut_list = []
    list_crab_output_dir = os.listdir(crab_output_dir)
    
    count_txt = -1

    # read .txt files for current dataset
    for file in list_crab_output_dir:

      if not re.search(".txt$", file): continue
      
      txt_file = os.path.join(crab_output_dir,file)

      count_txt = count_txt + 1
      
      data = {}
      column = []
      lineCounter = int(0)

      for j, line in enumerate( open(txt_file) ):

          line = string.strip(line,"\n")

          #skip precuts
          if( re.search("^###", line) ):
              ############################
              ##only once for each dataset
              if(count_txt==0):
                  preCut_list.append(line)
              ############################
              continue

          #print "---> lineCounter: " , lineCounter
          #print line

          if lineCounter == 0:
              for i,piece in enumerate(line.split()):
                  column.append(piece)
          else:
              for i,piece in enumerate(line.split()):
                  if i == 0:
                      data[int(piece)] = {}
                      row = int(piece)
                  else:
                      data[row][ column[i] ] = piece
                      #print data[row][ column[i] ]

          lineCounter = lineCounter + 1

      # create new table
      newtable = {}
      Ntot = float(data[0]['N'])

      for j, line in enumerate( data ):
          if(j == 0):
              newtable[int(j)]={'variableName': data[j]['variableName'],
                                'min1': "-",
                                'max1': "-",
                                'min2': "-",
                                'max2': "-",
                                'level': "-",
                                'N': Ntot,
                                'errN': int(0),
                                'Npass': Ntot ,
                                'errNpass': int(0),
                                }
          else:
              N = ( float(data[j]['N'])  )
              errN = ( float(data[j-1]["errEffAbs"]) * Ntot )
              #print data[j]['variableName']
              #print "errN: " , errN
              if(str(errN) == "nan"):
                  errN = 0

                  #            if( float(N) > 0 and float(errN) > 0 ):
                  #                errRelN = errN / N
                  #            else:
                  #                errRelN = float(0)

              Npass = ( float(data[j]['Npass']) )
              errNpass = ( float(data[j]["errEffAbs"]) * Ntot )
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

      #print newtable

      # combine different .txt files from different jobs in one table
      UpdateTable(newtable, dict_final_table)

    # calculate efficiency for each step of the analysis
    CalculateEfficiency(dict_final_table)
    #print dict_final_table

    # write table
    output_table_file = open(output_txt_file,'w')
    WriteTable(dict_final_table, preCut_list, output_table_file)
    output_table_file.close()

    # final output for this dataset
    print ''
    print "Final .root file: " + output_root_file
    print "Final .txt file:  " + output_txt_file
    print ''

if __name__ == "__main__":
  main()

