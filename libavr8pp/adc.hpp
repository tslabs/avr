
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

namespace adc
{
  template<u8 NUMBER>
  class Functions
  {
    public:
      static inline u8 read8(); // !!! deprecated

    private:
      Functions();
  };

  void initialize(u8, u8);
  void selectChannel(u8);
  u16 read16();
}
