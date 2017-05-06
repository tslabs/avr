
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace onewire
{
  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  void Functions<DATA_PORT, DATA_PIN, TIMER>::set0()
  {
    DATA::setLow();
    DATA::setOut();
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  void Functions<DATA_PORT, DATA_PIN, TIMER>::setZ()
  {
    DATA::setIn();
    DATA::setHigh();
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  bool Functions<DATA_PORT, DATA_PIN, TIMER>::get()
  {
    return DATA::getPin();
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  void Functions<DATA_PORT, DATA_PIN, TIMER>::initialize()
  {
    setZ();
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  bool Functions<DATA_PORT, DATA_PIN, TIMER>::reset()
  {
    set0(); TIMER(D_RESET);
    setZ(); TIMER(D_RS_WAIT);
    bool i = !get();
    TIMER(D_RS_EPI);
    return i;
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  u8 Functions<DATA_PORT, DATA_PIN, TIMER>::readByte()
  {
    u8 byte = 0;

    for (u8 i = 0; i < 8; i++)
    {
      byte >>= 1;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        set0(); delay_us(2);
        setZ(); delay_us(10);
        byte |= get() ? 0x80 : 0;
      }
      TIMER(D_RW_EPI);
    }

    return byte;
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  void Functions<DATA_PORT, DATA_PIN, TIMER>::writeByte(u8 byte)
  {
    for (u8 i = 0; i < 8; i++)
    {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        set0(); delay_us(2);
        if (byte & 1) setZ();
      }
      TIMER(D_RW_EPI);
      setZ(); delay_us(10);
      byte >>= 1;
    }
  }
}
