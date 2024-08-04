// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#include "BTOFHitReconstruction.h"

#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <spdlog/common.h>
#include <stddef.h>
#include <iterator>
#include <utility>
#include <vector>

namespace eicrecon {

void BTOFHitReconstruction::init(const dd4hep::rec::CellIDPositionConverter* converter, std::shared_ptr<spdlog::logger>& logger) {

    m_log = logger;

    m_converter = converter;
}

dd4hep::rec::CellID BTOFHitReconstruction::getDetInfos(const dd4hep::rec::CellID& id) {
    // retrieve segmentation class if that hasn't been done
    if(! m_decoder) {
        const auto det = m_converter -> findContext(id) -> element;
        auto readout = m_converter -> findReadout(det);
        auto seg = readout.segmentation();
        m_decoder = seg.decoder();
    }

    // CellID for BarrelTOF is composed of 6 parts
    // system, layer, module, sensor, x, y
    // If we fix x and y to zero, what remains will be the detector information only
    auto id_return = id;
    m_decoder -> set(id_return, "x", 0);
    m_decoder -> set(id_return, "y", 0);
    return id_return;
}

std::unique_ptr<edm4eic::TrackerHitCollection> BTOFHitReconstruction::process(const edm4eic::RawTrackerHitCollection& TDCADC_hits) {
    using dd4hep::mm;

    auto rec_hits { std::make_unique<edm4eic::TrackerHitCollection>() };

    // collection of ADC values from all sensors
    std::unordered_map<dd4hep::rec::CellID, 
	               std::vector<HitInfo>> hitsBySensors;

    for (const auto& TDCADC_hit : TDCADC_hits) {

        auto id = TDCADC_hit.getCellID();

        // Get position and dimension
        auto pos = m_converter->position(id);
	// Get sensors info
	auto detID = this -> getDetInfos(id);
	hitsBySensors[detID].emplace_back(pos.x(), pos.y(), pos.z(), int(TDCADC_hit.getCharge()), int(TDCADC_hit.getTimeStamp()), id);

    }

    // loop through each sensors for Hit information
    for (const auto& sensor : hitsBySensors) {
	// INSERT clustering algorithm for each sensors here
	// Right now I just perform a simple average over all hits in a sensors
	// Will be problematic near the edges, but it's just an illustration
        double ave_x = 0, ave_y = 0, ave_z = 0;
	double tot_charge = 0;
	const auto& hits = sensor.second;
	// find cellID for the cell with maximum ADC value within a sensor
	// I don't know why you need cellID for reconstructed hits, but we'll do it anyway
	auto id = hits[0].id;
	auto curr_adc = hits[0].adc;
	auto first_tdc = hits[0].tdc;
	for(const auto& hit : hits) {
            // weigh all hits by ADC value
            ave_x += hit.adc*hit.x;
	    ave_y += hit.adc*hit.y;
	    ave_z += hit.adc*hit.z;

	    tot_charge += hit.adc;
	    if(hit.adc > curr_adc) {
	        curr_adc = hit.adc;
		id = hit.id;
	    }
	    first_tdc = std::min(first_tdc, hit.tdc);
	}

	ave_x /= tot_charge;
	ave_y /= tot_charge;
	ave_z /= tot_charge;


        // >oO trace
        rec_hits->create(
	    id,
            edm4hep::Vector3f{static_cast<float>(ave_x / mm), 
	                      static_cast<float>(ave_y / mm), 
			      static_cast<float>(ave_z / mm)}, // mm
            edm4eic::CovDiag3f{1./mm, 1./mm, 1./mm}, // should be the covariance of position
            static_cast<float>((double)(first_tdc)), // ns
            0.0F,                            // covariance of time
            static_cast<float>(tot_charge / 1000.),   // total ADC sum
            0.0F);                                       // Error on the energy

    }

    return std::move(rec_hits);
}

} // namespace eicrecon
