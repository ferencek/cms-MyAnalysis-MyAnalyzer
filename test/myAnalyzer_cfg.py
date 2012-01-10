###############################
####### Parameters ############
###############################
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing ('python')

options.register('processName',
    'USER',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "CMSSW process name"
)

options.register('globalTag',
    'START42_V14B::All',
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

options.register('produceSkim',
    False,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Switch to turn ON/OFF skim production"
)

options.register('skimFilename',
    'skim.root',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Name of the output skim file"
)
## 'maxEvents' is already registered by the Framework, changing default value
options.setDefault('maxEvents', 1000)

options.parseArguments()

## For debugging
#print options

import FWCore.ParameterSet.Config as cms

process = cms.Process(options.processName)

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

if not options.produceSkim:
    ## Output ROOT file
    process.TFileService = cms.Service("TFileService",
        fileName = cms.string(((options.outputPrefix + '__') if options.outputPrefix != '' else '') + 'histograms.root')
    )

## Input files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/user/ferencek/HT/Run2011A-05Aug2011-v1_EDMTuple_V00-00-03/117fa890f807f13ca93974607465b3d6/EDMTuple_9_1_TfJ.root'
    )
)

## MyAnalyzer configuration
process.myAnalyzer = cms.EDFilter('MyAnalyzer',
    fillAllSameLevelAndLowerLevelCuts = cms.untracked.bool(False), # to disable automatic creation of less frequently used histograms
    fillAllCuts                       = cms.untracked.bool(False), # to disable automatic creation of less frequently used histograms
    skimMode                          = cms.untracked.bool(options.produceSkim), # when enabled, the cutEfficiency file and output histograms are not produced
    HLTInputTag                       = cms.InputTag('TriggerResults','','HLT'),
    skimWasMade                       = cms.bool(True),
    eventCounterInputTag              = cms.untracked.InputTag('nEventsTotal'),
    inputCutFile                      = cms.string('cutFile.txt'),
    outputCutEfficiencyFile           = cms.string(((options.outputPrefix + '__') if options.outputPrefix != '' else '') + 'cutEfficiency.txt')
)

## Paths
process.p = cms.Path(process.myAnalyzer)

## Schedule definition
process.schedule = cms.Schedule(process.p)

if options.produceSkim:
    ## Output file
    process.out = cms.OutputModule("PoolOutputModule",
        fileName = cms.untracked.string(options.skimFilename),
        # save only events passing the full path
        SelectEvents   = cms.untracked.PSet( SelectEvents = cms.vstring('p') ),
        dropMetaData = cms.untracked.string("ALL"),
        outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_gtDigis_*_*',
            'keep *_TriggerResults_*_*',
            'drop *_TriggerResults_*_' + options.processName,
            'keep *_hltTriggerSummaryAOD_*_*',
            'keep *_nEventsTotal_*_*',
            'keep *_kt6PFJets_rho_*',
            'keep *_kt6PFJetsForIsolation_rho_*',
            'keep *_AK5CaloJets_*_*',
            'keep *_AK7CaloJets_*_*',
            'keep *_AK5GenJets_*_*',
            'keep *_AK7GenJets_*_*',
            'keep *_AK5PFJets_*_*',
            'keep *_AK7PFJets_*_*',
            'keep *_CaloMET_*_*',
            'keep *_EventSelection_*_*',
            'keep *_GenEventInfo_*_*',
            'keep *_GenParticles_*_*',
            'keep *_Muons_*_*',
            'keep *_PFMET_*_*',
            'keep *_Vertices_*_*'
        )
    )

    ## EndPath definition
    process.outpath = cms.EndPath(process.out)

    ## Updated schedule definition
    process.schedule.append(process.outpath)
