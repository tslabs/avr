
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace ds18b20
{
  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  void Functions<DATA_PORT, DATA_PIN, TIMER>::initialize()
  {
    DEVICE::initialize();
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  bool Functions<DATA_PORT, DATA_PIN, TIMER>::convertTemperature()
  {
    if (!DEVICE::reset()) return false;

    // Start the temperature conversion
    DEVICE::writeByte(SKIP_ROM);
    DEVICE::writeByte(CONVERT_T);

    return true;
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  bool Functions<DATA_PORT, DATA_PIN, TIMER>::readTemperature(u16 &temp)
  {
    temp = 0;

    if (!DEVICE::reset()) return false;

    // Read scratchpad
    DEVICE::writeByte(SKIP_ROM);
    DEVICE::writeByte(READ);

    // 8T.4t is tttt0000, TTTTTTTT
    temp = DEVICE::readByte() << 4;
    temp |= DEVICE::readByte() << 12;
    temp ^= 0x8000;  // make signed-unsingned conversion

    return true;
  }
}
