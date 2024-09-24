
//
// Template for this file generated with eicmkplugin.py
//

#include "VisualizeHitsProcessor.h"
#include "services/rootfile/RootFile_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

// Include appropriate class headers. e.g.
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include "TGeoMatrix.h"
#include "TColor.h"

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new VisualizeHitsProcessor);
    }
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void VisualizeHitsProcessor::InitWithGlobalRootLock(){
    auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
    auto rootfile = rootfile_svc->GetHistFile();
    rootfile->mkdir("VisualizeHits")->cd();

    // Create histograms here. e.g.
    _detector = GetApplication() -> GetService<DD4hep_service>() -> detector();
    _converter = std::make_unique<dd4hep::rec::CellIDPositionConverter>(*_detector);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void VisualizeHitsProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
    // Data objects we will need from JANA e.g.
    const auto &rawhits = *(event->GetCollection<edm4eic::RawTrackerHit>("TOFBarrelADCTDC"));

    for(auto hit: rawhits) {
	auto cellID = hit.getCellID();
        auto pos = _converter -> position(cellID);
	if(_cellDim.size() ==  0) 
	    _cellDim = _converter -> cellDimensions(cellID);
	_hits.push_back({pos.X(), pos.Y(), pos.Z(), hit.getCharge(), hit.getTimeStamp()});
    }

    const auto &truehits = *(event->GetCollection<edm4hep::SimTrackerHit>("TOFBarrelHits"));
    for(auto hit : truehits) {
        auto pos = hit.getPosition();
	_geantHits.push_back({pos.x/10, pos.y/10, pos.z/10, 0, 0});
    }

    const auto &recohits = *(event->GetCollection<edm4eic::TrackerHit>("TOFBarrelRecHit"));
    for(auto hit : recohits) {
        auto pos = hit.getPosition();
	_recoHits.push_back({pos.x/10, pos.y/10, pos.z/10, 0, 0});
    }

    // Fill histograms here. e.g.
    // for (auto hit : rawhits) hEraw->Fill(hit.getEnergy());
}

void VisualizeHitsProcessor::Export3DNeighbors(int nNeighbors, const std::string& filename) {
  // get volume
  auto manager =
      GetApplication()->GetService<DD4hep_service>()->detector()->world().volume()->GetGeoManager();
  // add all hit points as sphere
  auto volume    = manager->GetVolume("BarrelTOFSubAssembly");
  auto matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  auto Vacuum    = new TGeoMedium("Vacuum", 1, matVacuum);

  auto maxCharge = std::max_element(_hits.begin(), _hits.end(), [](const HitInfo& l, const HitInfo& r) {
		  return l.charge < r.charge; });

  std::vector<int> colors{kRed-10, kRed-9, kRed-7, kRed-4, kRed,
	                  kRed+1, kRed+2, kRed+3, kRed+4};
  std::reverse(colors.begin(), colors.end());
  int counter = 0;
  for (const auto& hit : _hits) {
    ++counter;
    if(counter > nNeighbors) break;

    double x = hit.x;
    double y = hit.y;
    double z = hit.z;

    // find volume
    auto node     = manager->FindNode(x, y, z);
    auto HMatrix  = manager->GetCurrentMatrix();
    auto rotation = new TGeoRotation();
    rotation->SetMatrix(HMatrix->GetRotationMatrix());

    auto hitGeo = manager->MakeBox("Hits", Vacuum, _cellDim[0]/2., _cellDim[1]/2, 0.05);
    hitGeo -> SetLineColor(colors[std::max(int(colors.size())-1, int(double(hit.charge)/maxCharge -> charge*colors.size()))]);//TColor::GetColor(std::min(255, int((hit.charge/1000.)*255)), 0, 0));
    volume->AddNode(hitGeo, 1, new TGeoCombiTrans(x, y, z, rotation));
  }

  for (const auto& hit : _geantHits) {
    double x = hit.x;
    double y = hit.y;
    double z = hit.z;

    // find volume
    auto hitGeo = manager->MakeSphere("Hits", Vacuum, 0., 0.2);
    hitGeo -> SetLineColor(kGreen);
    volume->AddNode(hitGeo, 1, new TGeoTranslation(x, y, z));
  }

  for (const auto& hit : _recoHits) {
    double x = hit.x;
    double y = hit.y;
    double z = hit.z;

    auto hitGeo = manager->MakeSphere("Hits", Vacuum, 0., 0.2);
    hitGeo -> SetLineColor(kBlue);
    volume->AddNode(hitGeo, 1, new TGeoTranslation(x, y, z));
  }



  manager->Export(filename.c_str());
}


//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void VisualizeHitsProcessor::FinishWithGlobalRootLock() {

    // Do any final calculations here.
    this -> Export3DNeighbors(INT_MAX, "geo.root");
}

