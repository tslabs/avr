
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

namespace uart
{
  class Functions
  {
    public:
      static inline void initialize(u32);
      static inline void sendByte(u8);
      static inline bool isSending();

    private:
      Functions();
  };
}

// High-level access to the peripheral
typedef uart::Functions UART;
