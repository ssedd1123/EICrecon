// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from TOFPulseGeneration into ADC and TDC values
//
// Author: Souvik Paul, Chun Yuen Tsang
// Date: 22/10/2024

#include "DD4hep/Detector.h"
#include "DDRec/Surface.h"
#include "TF1.h"
#include "TMath.h"
#include <Evaluator/DD4hepUnits.h>
#include <TGraph.h>
#include <bitset>
#include <fmt/format.h>
#include <iostream>
#include <vector>

#include "TOFPulseDigitization.h"
#include "Math/SpecFunc.h"
#include "algorithms/digi/TOFHitDigiConfig.h"
#include <algorithms/geo.h>

// using namespace dd4hep;
// using namespace dd4hep::Geometry;
// using namespace dd4hep::DDRec;
// using namespace eicrecon;
using namespace dd4hep::xml;

namespace eicrecon {

dd4hep::rec::CellID TOFPulseDigitization::_getSensorID(const dd4hep::rec::CellID& cellID) const {
  // fix x-y, what you left with are ids that corresponds to sensor info
  // cellID may change when position changes.
  auto sensorID = cellID; //_converter -> cellID(_converter -> position(hitCell));
  m_decoder->set(sensorID, "x", 0);
  m_decoder->set(sensorID, "y", 0);

  return sensorID;
}

void TOFPulseDigitization::init() {
  auto detector = algorithms::GeoSvc::instance().detector();;
  auto seg       = detector->readout(m_cfg.readout).segmentation();
  auto type      = seg.type();
  if (type != "CartesianGridXY")
    throw std::runtime_error("Unsupported segmentation type: " + type +
                             ". TOFPulseDigitization only works with CartesianGridXY."
			     "It is needed for hit association.");
  // retrieve meaning of cellID bits
  m_decoder = seg.decoder();

}

void TOFPulseDigitization::process(const TOFPulseDigitization::Input& input,
                                   const TOFPulseDigitization::Output& output) const {
  const auto [simPulses, simhits] = input;
  auto [rawhits, associations] = output;

  // prep the association by grouping hits by cellID
  std::unordered_map<dd4hep::rec::CellID, std::vector<edm4hep::SimTrackerHit>> cell_hit_map;
  for(const auto& sim_hit : *simhits) 
    cell_hit_map[this -> _getSensorID(sim_hit.getCellID())].push_back(sim_hit);

  double thres = m_cfg.t_thres;
  // double Vm=-0.05;
  //  SP noted that max dE experienced by LGAD should be 0.8 keV
  double Vm = m_cfg.Vm;
  int adc_range = m_cfg.adc_range;

  // normalized time threshold
  // convert threshold EDep to voltage
  double norm_threshold = -thres * adc_range / Vm;

  for(const auto& pulse : *simPulses) {
    // Added by SP
    //-------------------------------------------------------------
    double intersectionX = 0.0;
    int tdc              = std::numeric_limits<int>::max();
    int adc              = 0;
    double V             = 0.0;

    int time_bin = 0;
    double adc_prev = 0;
    double time_interval = pulse.getInterval();
    auto adcs = pulse.getAdcCounts();
    for (const auto adc : adcs) {
      if (adc_prev >= norm_threshold && adc <= norm_threshold) {
        intersectionX = time_bin*time_interval + time_interval * (norm_threshold - adc_prev) / (adc - adc_prev);
        tdc = ceil(intersectionX / 0.02);
      }
      if (abs(adc) > abs(V)) // To get peak of the Analog signal
        V = adc;
      adc_prev = adc;
      ++time_bin;
    }

    // limit the range of adc values
    adc = std::min(static_cast<double>(adc_range), round(-V));
    // only store valid hits
    if (tdc < std::numeric_limits<int>::max()) {
      auto raw_hit = rawhits->create(pulse.getCellID(), adc, tdc);
      // associate all hits inside the same sensor
      auto sensorID = this -> _getSensorID(pulse.getCellID());
      auto it = cell_hit_map.find(sensorID);
      if(it != cell_hit_map.end()) {
	auto n_hits_sensor = static_cast<double>(it -> second.size());
        for(const auto sim_hit : it -> second) {
          auto hitassoc = associations -> create();
	  hitassoc.setWeight(1./n_hits_sensor);
	  hitassoc.setRawHit(raw_hit);
	  hitassoc.setSimHit(sim_hit);
	}
      }
    }
    //-----------------------------------------------------------

  }
} // TOFPulseDigitization:process
} // namespace eicrecon
