
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace gpio
{
  template<Address P, u8 N>
  void Pin<P, N>::setPin(bool v)
  {
    switch (P)
    {
#ifdef PORTA
      case GPIOA:
        if (v) PORTA |= _BV(N);
        else PORTA &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOA, "No PORT A in this part!");
#endif
#ifdef PORTB
      case GPIOB:
        if (v) PORTB |= _BV(N);
        else PORTB &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOB, "No PORT B in this part!");
#endif
#ifdef PORTC
      case GPIOC:
        if (v) PORTC |= _BV(N);
        else PORTC &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOC, "No PORT C in this part!");
#endif
#ifdef PORTD
      case GPIOD:
        if (v) PORTD |= _BV(N);
        else PORTD &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOD, "No PORT D in this part!");
#endif
#ifdef PORTE
      case GPIOE:
        if (v) PORTE |= _BV(N);
        else PORTE &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOE, "No PORT E in this part!");
#endif
#ifdef PORTF
      case GPIOF:
        if (v) PORTF |= _BV(N);
        else PORTF &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOF, "No PORT F in this part!");
#endif
#ifdef PORTG
      case GPIOG:
        if (v) PORTG |= _BV(N);
        else PORTG &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOG, "No PORT G in this part!");
#endif
    } // switch (P)
  }

  template<Address P, u8 N>
  void Pin<P, N>::setHigh()
  {
    Pin<P, N>::setPin(1);
  }

  template<Address P, u8 N>
  void Pin<P, N>::setLow()
  {
    Pin<P, N>::setPin(0);
  }

  template<Address P, u8 N>
  void Pin<P, N>::setDir(Direction v)
  {
    switch (P)
    {
#ifdef PORTA
      case GPIOA:
        if (v == OUT) DDRA |= _BV(N);
        else DDRA &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOA, "No PORT A in this part!");
#endif
#ifdef PORTB
      case GPIOB:
        if (v == OUT) DDRB |= _BV(N);
        else DDRB &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOB, "No PORT B in this part!");
#endif
#ifdef PORTC
      case GPIOC:
        if (v == OUT) DDRC |= _BV(N);
        else DDRC &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOC, "No PORT C in this part!");
#endif
#ifdef PORTD
      case GPIOD:
        if (v == OUT) DDRD |= _BV(N);
        else DDRD &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOD, "No PORT D in this part!");
#endif
#ifdef PORTE
      case GPIOE:
        if (v == OUT) DDRE |= _BV(N);
        else DDRE &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOE, "No PORT E in this part!");
#endif
#ifdef PORTF
      case GPIOF:
        if (v == OUT) DDRF |= _BV(N);
        else DDRF &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOF, "No PORT F in this part!");
#endif
#ifdef PORTG
      case GPIOG:
        if (v == OUT) DDRG |= _BV(N);
        else DDRG &= ~_BV(N);
      break;
#else
      static_assert(P != GPIOG, "No PORT G in this part!");
#endif
    } // switch (P)
  }

  template<Address P, u8 N>
  void Pin<P, N>::setIn()
  {
    Pin<P, N>::setDir(IN);
  }

  template<Address P, u8 N>
  void Pin<P, N>::setOut()
  {
    Pin<P, N>::setDir(OUT);
  }

  template<Address P, u8 N>
  void Pin<P, N>::pullUp()
  {
    Pin<P, N>::setDir(IN);
    Pin<P, N>::setHigh();
  }

  template<Address P, u8 N>
  void Pin<P, N>::setZ()
  {
    Pin<P, N>::setDir(IN);
    Pin<P, N>::setLow();
  }

  template<Address P, u8 N>
  bool Pin<P, N>::getPin()
  {
    switch (P)
    {
#ifdef PORTA
      case GPIOA:
        return (PINA & _BV(N)) != 0;
#else
      static_assert(P != GPIOA, "No PORT A in this part!");
#endif
#ifdef PORTB
      case GPIOB:
        return (PINB & _BV(N)) != 0;
#else
      static_assert(P != GPIOB, "No PORT B in this part!");
#endif
#ifdef PORTC
      case GPIOC:
        return (PINC & _BV(N)) != 0;
#else
      static_assert(P != GPIOC, "No PORT C in this part!");
#endif
#ifdef PORTD
      case GPIOD:
        return (PIND & _BV(N)) != 0;
#else
      static_assert(P != GPIOD, "No PORT D in this part!");
#endif
#ifdef PORTE
      case GPIOE:
        return (PINE & _BV(N)) != 0;
#else
      static_assert(P != GPIOE, "No PORT E in this part!");
#endif
#ifdef PORTF
      case GPIOF:
        return (PINF & _BV(N)) != 0;
#else
      static_assert(P != GPIOF, "No PORT F in this part!");
#endif
#ifdef PORTG
      case GPIOG:
        return (PING & _BV(N)) != 0;
#else
      static_assert(P != GPIOG, "No PORT G in this part!");
#endif
    } // switch (P)
  }

  template<Address P>
  void Port<P>::setPort(u8 v)
  {
    switch (P)
    {
#ifdef PORTA
      case GPIOA:
        PORTA = v;
      break;
#else
      static_assert(P != GPIOA, "No PORT A in this part!");
#endif
#ifdef PORTB
      case GPIOB:
        PORTB = v;
      break;
#else
      static_assert(P != GPIOB, "No PORT B in this part!");
#endif
#ifdef PORTC
      case GPIOC:
        PORTC = v;
      break;
#else
      static_assert(P != GPIOC, "No PORT C in this part!");
#endif
#ifdef PORTD
      case GPIOD:
        PORTD = v;
      break;
#else
      static_assert(P != GPIOD, "No PORT D in this part!");
#endif
#ifdef PORTE
      case GPIOE:
        PORTE = v;
      break;
#else
      static_assert(P != GPIOE, "No PORT E in this part!");
#endif
#ifdef PORTF
      case GPIOF:
        PORTF = v;
      break;
#else
      static_assert(P != GPIOF, "No PORT F in this part!");
#endif
#ifdef PORTG
      case GPIOG:
        PORTG = v;
      break;
#else
      static_assert(P != GPIOG, "No PORT G in this part!");
#endif
    } // switch (P)
  }

  template<Address P>
  u8 Port<P>::getPort()
  {
    switch (P)
    {
#ifdef PORTA
      case GPIOA:
        return PORTA;
#else
      static_assert(P != GPIOA, "No PORT A in this part!");
#endif
#ifdef PORTB
      case GPIOB:
        return PORTB;
#else
      static_assert(P != GPIOB, "No PORT B in this part!");
#endif
#ifdef PORTC
      case GPIOC:
        return PORTC;
#else
      static_assert(P != GPIOC, "No PORT C in this part!");
#endif
#ifdef PORTD
      case GPIOD:
        return PORTD;
#else
      static_assert(P != GPIOD, "No PORT D in this part!");
#endif
#ifdef PORTE
      case GPIOE:
        return PORTE;
#else
      static_assert(P != GPIOE, "No PORT E in this part!");
#endif
#ifdef PORTF
      case GPIOF:
        return PORTF;
#else
      static_assert(P != GPIOF, "No PORT F in this part!");
#endif
#ifdef PORTG
      case GPIOG:
        return PORTG;
#else
      static_assert(P != GPIOG, "No PORT G in this part!");
#endif
    } // switch (P)
  }
}
