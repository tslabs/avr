
/*******************************************************************************
 *
 * Copyright (C) 2019 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#pragma once

#include "defs.hpp"
#include "gpio.hpp"
#include "softi2c.hpp"

namespace ccs811
{
  // address
  enum
  {
    DEVICE_ADDR = 0x5B
  };

  template<typename>
  class Functions
  {
    public:
      static bool Initialize();
    private:
  };
}
