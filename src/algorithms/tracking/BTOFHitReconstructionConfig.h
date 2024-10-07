// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#pragma once

namespace eicrecon {
    struct BTOFHitReconstructionConfig {
        // parameters that convert ADC to EDep
	double c_slope = 0, c_intercept = 0;
	// parameters that convert TDC to hit time (ns)
	double t_slope = 0, t_intercept = 0;
    };
}
