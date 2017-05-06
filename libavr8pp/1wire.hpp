
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
#include "gpio.hpp"

namespace onewire
{
  // Delays in us for 1-Wire signaling, us
  enum Delay
  {
    D_RESET    = 480,
    D_RS_WAIT  = 60,
    D_RS_EPI   = 250,
    D_RW_EPI   = 60
  };

  typedef void (*TFUNC)(u16);   // timer function, provided by caller, delay is microseconds

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  class Functions
  {
    typedef gpio::Pin<DATA_PORT, DATA_PIN> DATA;

    public:
      static inline void initialize();
      static inline bool reset();
      static inline u8 readByte();
      static inline void writeByte(u8);

    private:
      static inline void set0();
      static inline void setZ();
      static inline bool get();
      Functions();
  };
}
