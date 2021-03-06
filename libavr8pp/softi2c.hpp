
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

namespace softi2c
{
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  class Functions
  {
    enum
    {
      DELAY = 1ULL * F_CPU / FREQ / 4 / 2
    };

    typedef gpio::Pin<SCL_PORT, SCL_PIN> SCL;
    typedef gpio::Pin<SDA_PORT, SDA_PIN> SDA;

    public:
      static inline void Initialize();
      static inline void SendStart();
      static inline void SendStop();
      static inline bool SendByte(u8);
      static inline u8   RecvByte(bool);
      static inline bool ReadReg(u8, u8, u8&);
      static inline bool ReadReg(u8, u8, void*, u8);
      static inline bool WriteReg(u8, u8, u8);
      static inline bool WriteReg(u8, u8, void*, u8);

    private:
      static inline void waitScl();
  };
}
