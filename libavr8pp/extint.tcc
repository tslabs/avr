
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace extint
{
  template<Int EXTINT>
  void Functions<EXTINT>::enable(SENSE mode)
  {
#ifdef __AVR_ATmega328P__
    EICRA = (EICRA & ~(3 << (EXTINT * 2))) | (mode << (EXTINT * 2));
    EIMSK |= 1 << (EXTINT + INT0);
#else
    // MCUCR |= _BV(ISC01) | _BV(ISC00);
  #ifdef __AVR_ATtiny13A__
      // GIMSK  |= _BV(INT0);
  #else
      MCUCR = (MCUCR & ~(3 << (EXTINT * 2))) | (mode << (EXTINT * 2));
      GICR |= 1 << (EXTINT + INT0);
  #endif
#endif
  }
  
  template<Int EXTINT>
  void Functions<EXTINT>::disable()
  {
#ifdef __AVR_ATmega328P__
    EIMSK &= ~(1 << EXTINT);
#endif
  }
}
