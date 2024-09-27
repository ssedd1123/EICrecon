
//
// Template for this file generated with eicmkplugin.py
//

#include "RecordCellHitsProcessor.h"
#include "services/rootfile/RootFile_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

// Include appropriate class headers. e.g.
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new RecordCellHitsProcessor);
    }
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void RecordCellHitsProcessor::InitWithGlobalRootLock(){
    auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
    auto rootfile = rootfile_svc->GetHistFile();
    rootfile->mkdir("RecordCellHits")->cd();

    // Create histograms here. e.g.
    _tree = new TTree("cellHits", "ADC TDC hits for all cells");
    _tree -> Branch("eventID", &_eventID);
    _tree -> Branch("cellID", &_cellID);
    _tree -> Branch("TDC", &_TDC);
    _tree -> Branch("ADC", &_ADC);
    _tree -> Branch("x", &_x);
    _tree -> Branch("y", &_y);
    _tree -> Branch("z", &_z);
    _tree -> Branch("simx", &_simx);
    _tree -> Branch("simy", &_simy);
    _tree -> Branch("simz", &_simz);
    _tree -> Branch("simc", &_simc);
    _tree -> Branch("simt", &_simt);

    // Create histograms here. e.g.
    _detector = GetApplication() -> GetService<DD4hep_service>() -> detector();
    _converter = std::make_unique<dd4hep::rec::CellIDPositionConverter>(*_detector);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void RecordCellHitsProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
    // Data objects we will need from JANA e.g.
    const auto &truehits = *(event->GetCollection<edm4hep::SimTrackerHit>("TOFBarrelHits"));
    _eventID = event -> GetEventNumber();
    // this script only works when there is only one hit per event
    // Because I don't want to perform any hit association for simplicity
    if(truehits.size() != 1) 
        return;
    for(auto hit : truehits) {
        auto pos = hit.getPosition();
	_simx = pos.x;
	_simy = pos.y;
	_simz = pos.z;
	_simc = hit.getEDep();
	_simt = hit.getTime();
    }

    _cellID.clear();
    _TDC.clear();
    _ADC.clear();
    _x.clear();
    _y.clear();
    _z.clear();
    const auto &rawhits = *(event->GetCollection<edm4eic::RawTrackerHit>("TOFBarrelADCTDC"));

    for(auto hit: rawhits) {
	auto cellID = hit.getCellID();
        auto pos = _converter -> position(cellID);

	_cellID.push_back(cellID);
	_ADC.push_back(hit.getCharge());
        _TDC.push_back(hit.getTimeStamp());
	_x.push_back(pos.X());
	_y.push_back(pos.Y());
	_z.push_back(pos.Z());
    }

    _tree -> Fill();

}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void RecordCellHitsProcessor::FinishWithGlobalRootLock() {

    // Do any final calculations here.
    //auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
    //auto rootfile = rootfile_svc->GetHistFile();
    //rootfile -> cd("RecordCellHits");

    //_tree -> Write();

}

