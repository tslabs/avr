
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

namespace extint
{
  enum SENSE
  {
    LOW     = 0,
    ANYEDGE = 1,
    NEGEDGE = 2,
    POSEDGE = 3
  };

  enum Int
  {
    EINT0 = 0,
    EINT1
  };

#ifdef __AVR_ATmega328P__
  enum Pcie
  {
    EPCIE0 = 0,
    EPCIE1,
    EPCIE2
  };
#endif

  template<Int>
  class Functions
  {
    public:
      static inline void enable(SENSE);
      static inline void disable();

    private:
      Functions();
  };
}

// High-level access to the peripheral
typedef extint::Functions<extint::EINT0> EXTINT0;
typedef extint::Functions<extint::EINT1> EXTINT1;
