
#pragma once

#include "1wire.hpp"

namespace ds18b20
{
  // DS18B20 commands
  enum
  {
    READ_ROM   = 0x33,
    MATCH_ROM  = 0x55,
    SKIP_ROM   = 0xCC,
    SEARCH_ROM = 0xF0,
    CONVERT_T  = 0x44,
    READ       = 0xBE,
    WRITE      = 0x4E,
    COPY       = 0x48,
    RECALL_EE  = 0xB8,
    READ_PS    = 0xB4
  };

  typedef void (*TFUNC)(u16);

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  class Functions
  {
    typedef onewire::Functions<DATA_PORT, DATA_PIN, TIMER> DEVICE;

    public:
      inline void initialize();
      inline bool readTemperature(s16&);

    private:
      bool is_conv;

      inline bool convertTemperature();
  };
}
