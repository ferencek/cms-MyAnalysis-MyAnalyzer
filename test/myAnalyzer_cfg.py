###############################
####### Parameters ############
###############################
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing ('python')

options.register('globalTag',
    'START42_V13::All',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Global tag to be used"
)

options.register('outputPrefix',
    '',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Prefix for the output file names"
)

options.register('reportEvery',
    100,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.int,
    "Report every N events (default is N=100)"
)
## 'maxEvents' is already registered by the Framework, changing default value
options.setDefault('maxEvents', 1000)

options.parseArguments()

## For debugging
#print options

import FWCore.ParameterSet.Config as cms

process = cms.Process("USER")

############## IMPORTANT ########################################
# If you run over many samples and you save the log, remember to reduce
# the size of the output by prescaling the report of the event number
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = options.reportEvery
process.MessageLogger.cerr.default.limit = 10
#################################################################

## Make sure to use the same global tag that was used to produce input files
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = options.globalTag

## Options and Output Report
process.options   = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

## Events to process
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

## Output ROOT file
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(((options.outputPrefix + '__') if options.outputPrefix != '' else '') + 'histograms.root')
)

## Input files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/user/ferencek/Jet/Run2011A-05Aug2011-v1_EDMTuple_V00-00-01/6d363a44e73023f68a8f3b11d67becad/EDMTuple_9_1_gAt.root'
    )
)

## MyAnalyzer configuration
process.myAnalyzer = cms.EDFilter('MyAnalyzer',
    HLTInputTag             = cms.InputTag('TriggerResults','','HLT'),
    skimWasMade             = cms.bool(True),
    eventCounterInputTag    = cms.untracked.InputTag('nEventsTotal'),
    inputCutFile            = cms.string('cutFile.txt'),
    outputCutEfficiencyFile = cms.string(((options.outputPrefix + '__') if options.outputPrefix != '' else '') + 'cutEfficiency.txt')
)

## Paths
process.p = cms.Path(process.myAnalyzer)

## Schedule definition
process.schedule = cms.Schedule(process.p)
