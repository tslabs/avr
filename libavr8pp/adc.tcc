
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
  template<u8 NUMBER>
  u8 Functions<NUMBER>::read()
  {
    ADMUX = _BV(REFS0) | _BV(ADLAR) | ((NUMBER & 0x0F) << MUX0);    // REFS[1:0] = 01 (Avcc), result right justified
    ADCSRA = _BV(ADEN) | _BV(ADSC) | (7 << ADPS0);                  // start conversion, free running disabled, prescaler = XTAL/128

    // wait for conversion end
    while(!(ADCSRA & _BV(ADIF)));

    // clear conversion flag, disable ADC
    ADCSRA = _BV(ADIF);
    return ADCH;
  }
}
