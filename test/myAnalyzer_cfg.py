import FWCore.ParameterSet.Config as cms

process = cms.Process("USER")

## MessageLogger
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.MessageLogger.cerr.default.limit = 10

## Make sure to use the same global tag that was used to produce input files
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag.globaltag = 'START42_V13::All'

## Options and Output Report
process.options   = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

## Events to process
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

## Output ROOT file
process.TFileService = cms.Service("TFileService",
    fileName = cms.string('histograms.root')
)

## Input files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'file:MyNtupleMaker_output.root'
    )
)

## MyAnalyzer configuration
process.myAnalyzer = cms.EDFilter('MyAnalyzer',
    skimWasMade             = cms.bool(True),
    eventCounterInputTag    = cms.untracked.InputTag('nEventsTotal'),
    inputCutFile            = cms.string('cutFileExample.txt'),
    outputCutEfficiencyFile = cms.string('cutEfficiency.txt')
)

## Paths
process.p = cms.Path(process.myAnalyzer)

## Schedule definition
process.schedule = cms.Schedule(process.p)
