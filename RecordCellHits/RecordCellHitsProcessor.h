
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2D.h>
#include <TFile.h>
#include <TTree.h>

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

class RecordCellHitsProcessor: public JEventProcessorSequentialRoot {
private:

    // Declare histogram and tree pointers here. e.g.
    // TH1D* hEraw  = nullptr;
    // TH2D* hEdigi = nullptr ;
    TTree *_tree = nullptr;
    std::vector<dd4hep::rec::CellID> _cellID;
    std::vector<double> _TDC;
    std::vector<double> _ADC;
    std::vector<double> _x, _y, _z;
    double _simx, _simy, _simz;
    double _simc, _simt;
    int _eventID;

    const dd4hep::Detector* _detector                     = nullptr;
    std::unique_ptr<dd4hep::rec::CellIDPositionConverter> _converter;

public:
    RecordCellHitsProcessor() { SetTypeName(NAME_OF_THIS); }

    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
};
