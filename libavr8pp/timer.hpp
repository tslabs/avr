
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#pragma once

#include "defs.hpp"

namespace timer
{
  enum Timer
  {
    TC0,
    TC1,
    TC2
  };

  template<Timer TIMER>
  class Functions
  {
    public:
      static inline void initialize();

    private:
      Functions();
  };
}

// High-level access to the peripheral
typedef timer::Functions<timer::TC0> TC0;
typedef timer::Functions<timer::TC1> TC1;
typedef timer::Functions<timer::TC2> TC2;
