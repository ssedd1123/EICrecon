// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/digi/TOFPulseDigitization.h"
#include <iostream>

namespace eicrecon {

class TOFPulseDigitization_factory : public JOmniFactory<TOFPulseDigitization_factory, TOFHitDigiConfig> {
private:
  // Underlying algorithm
  std::unique_ptr<eicrecon::TOFPulseDigitization> m_algo;

  // Declare inputs
  PodioInput<edm4hep::RawTimeSeries> m_in_sim_pulse{this}; //, "TOFBarrelRawHits"};
  PodioInput<edm4hep::SimTrackerHit> m_in_sim_hits{this}; //, "TOFBarrelRawHits"};

  // Declare outputs
  PodioOutput<edm4eic::RawTrackerHit> m_out_reco_particles{this};
  PodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_assoc_output {this};

  // Declare services here, e.g.
  Service<DD4hep_service> m_geoSvc{this};

  ParameterRef<double> m_t_thres{this, "t_thres", config().t_thres};
public:
  void Configure() {
    // This is called when the factory is instantiated.
    // Use this callback to make sure the algorithm is configured.
    // The logger, parameters, and services have all been fetched before this is called
    m_algo = std::make_unique<eicrecon::TOFPulseDigitization>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    // Pass config object to algorithm
    m_algo->applyConfig(config());

    // If we needed geometry, we'd obtain it like so
    // m_algo->init(m_geoSvc().detector(), m_geoSvc().converter(), logger());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {
    // This is called whenever the run number is changed.
    // Use this callback to retrieve state that is keyed off of run number.
    // This state should usually be managed by a Service.
    // Note: You usually don't need this, because you can declare a Resource instead.
  }

  void Process(int64_t run_number, uint64_t event_number) {
    // This is called on every event.
    // Use this callback to call your Algorithm using all inputs and outputs
    // The inputs will have already been fetched for you at this point.
    m_algo->process({m_in_sim_pulse(), m_in_sim_hits()},
                    {m_out_reco_particles().get(), m_assoc_output().get()});
    // JANA will take care of publishing the outputs for you.
  }
};
} // namespace eicrecon
