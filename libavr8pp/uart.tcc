
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace uart
{
  void Functions::initialize(u32 BAUD)
  {
    u16 a = (1ULL * (F_CPU) / 8 / (BAUD) - 1) & 0x0FFF;

#if defined __AVR_ATmega328P__ || defined __AVR_ATmega168P__
    UBRR0H = (u8)(a >> 8);
    UBRR0L = (u8)a;
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0A = _BV(U2X0);
    UCSR0B = _BV(TXEN0);
#else
    UBRRH = (u8)(a >> 8);
    UBRRL = (u8)a;
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
    UCSRA = _BV(U2X);
    UCSRB = _BV(TXEN);
#endif
  }
  
  void Functions::sendByte(u8 d)
  {
#if defined __AVR_ATmega328P__ || defined __AVR_ATmega168P__
    UDR0 = d;
#else
    UDR = d;
#endif
  }
  
  bool Functions::isSending()
  {
#if defined __AVR_ATmega328P__ || defined __AVR_ATmega168P__
    return !(UCSR0A & _BV(UDRE0));
#else
    return !(UCSRA & _BV(UDRE));
#endif
  }
}
