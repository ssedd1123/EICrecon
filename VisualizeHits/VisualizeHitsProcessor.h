
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2D.h>
#include <TFile.h>

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

class VisualizeHitsProcessor: public JEventProcessorSequentialRoot {
private:
    struct HitInfo {double x, y, z; int charge, time; };
    const dd4hep::Detector* _detector                     = nullptr;
    std::unique_ptr<dd4hep::rec::CellIDPositionConverter> _converter;
    std::vector<HitInfo> _hits;

    // It generates a root file "geo.root"
    // To draw everything, open the file with root, when enter the following in the command line
    // $> _file0->Get("Default");
    // $> gGeoManager -> SetVisLevel(10000);
    // $> gGeoManager -> GetVolume("BarrelTOFSubAssembly") -> Draw("ogl");
    // If still can't see the Staves, increase SetVisLevel

    void Export3DNeighbors(int nNeighbors, const std::string& filename);
public:
    VisualizeHitsProcessor() { SetTypeName(NAME_OF_THIS); }

    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
};
