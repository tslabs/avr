
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace adc
{
  // !!! deprecated
  template<u8 NUMBER>
  u8 Functions<NUMBER>::read8()
  {
    ADMUX = _BV(REFS0) | _BV(ADLAR) | ((NUMBER & 0x0F) << MUX0);    // REFS[1:0] = 01 (Avcc), result right justified
    ADCSRA = _BV(ADEN) | _BV(ADSC) | (7 << ADPS0);                  // start conversion, free running disabled, prescaler = XTAL/128

    // wait for conversion end
    while(!(ADCSRA & _BV(ADIF)));

    // clear conversion flag, disable ADC
    ADCSRA = _BV(ADIF);
    return ADCH;
  }

  void initialize(u8 m, u8 n)
  {
    ADCSRA = m;
    ADMUX = m;
  }

  void selectChannel(u8 c)
  {
    ADMUX = (ADMUX & ~0x1F) | c;
  }

  u16 read16()
  {
    ADCSRA |= _BV(ADEN) | _BV(ADSC);
    while(!(ADCSRA & _BV(ADIF)));
    ADCSRA |= _BV(ADIF);
    ADCSRA &= ~_BV(ADEN);
    
    return ADCW;
  }
}
