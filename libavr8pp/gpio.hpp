
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

namespace gpio
{
  enum Address
  {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    GPIOF,
    GPIOG
  };

  enum Direction
  {
    IN,
    OUT
  };

  template<Address P, u8 N>
  class Pin
  {
    public:
      static inline void setPin(bool);
      static inline bool getPin();
      static inline void setHigh();
      static inline void setLow();
      static inline void setDir(Direction);
      static inline void setIn();
      static inline void setOut();
      static inline void pullUp();
      static inline void setZ();

    private:
      Pin();
  };

  template<Address P>
  class Port
  {
    public:
    static inline void setPort(u8);
    static inline u8 getPort();
    // static inline void setDir(u8);

    private:
    Port();
  };
} // namespace gpio

typedef gpio::Port<gpio::GPIOA> GPIOA;
typedef gpio::Port<gpio::GPIOB> GPIOB;
typedef gpio::Port<gpio::GPIOC> GPIOC;
typedef gpio::Port<gpio::GPIOD> GPIOD;
typedef gpio::Port<gpio::GPIOE> GPIOE;
typedef gpio::Port<gpio::GPIOF> GPIOF;
typedef gpio::Port<gpio::GPIOG> GPIOG;

#undef PA0
#undef PA1
#undef PA2
#undef PA3
#undef PA4
#undef PA5
#undef PA6
#undef PA7

#undef PB0
#undef PB1
#undef PB2
#undef PB3
#undef PB4
#undef PB5
#undef PB6
#undef PB7

#undef PC0
#undef PC1
#undef PC2
#undef PC3
#undef PC4
#undef PC5
#undef PC6
#undef PC7

#undef PD0
#undef PD1
#undef PD2
#undef PD3
#undef PD4
#undef PD5
#undef PD6
#undef PD7

#undef PE0
#undef PE1
#undef PE2
#undef PE3
#undef PE4
#undef PE5
#undef PE6
#undef PE7

#undef PF0
#undef PF1
#undef PF2
#undef PF3
#undef PF4
#undef PF5
#undef PF6
#undef PF7

#undef PG0
#undef PG1
#undef PG2
#undef PG3
#undef PG4
#undef PG5
#undef PG6
#undef PG7

typedef gpio::Pin<gpio::GPIOA, 0> PA0;
typedef gpio::Pin<gpio::GPIOA, 1> PA1;
typedef gpio::Pin<gpio::GPIOA, 2> PA2;
typedef gpio::Pin<gpio::GPIOA, 3> PA3;
typedef gpio::Pin<gpio::GPIOA, 4> PA4;
typedef gpio::Pin<gpio::GPIOA, 5> PA5;
typedef gpio::Pin<gpio::GPIOA, 6> PA6;
typedef gpio::Pin<gpio::GPIOA, 7> PA7;

typedef gpio::Pin<gpio::GPIOB, 0> PB0;
typedef gpio::Pin<gpio::GPIOB, 1> PB1;
typedef gpio::Pin<gpio::GPIOB, 2> PB2;
typedef gpio::Pin<gpio::GPIOB, 3> PB3;
typedef gpio::Pin<gpio::GPIOB, 4> PB4;
typedef gpio::Pin<gpio::GPIOB, 5> PB5;
typedef gpio::Pin<gpio::GPIOB, 6> PB6;
typedef gpio::Pin<gpio::GPIOB, 7> PB7;

typedef gpio::Pin<gpio::GPIOC, 0> PC0;
typedef gpio::Pin<gpio::GPIOC, 1> PC1;
typedef gpio::Pin<gpio::GPIOC, 2> PC2;
typedef gpio::Pin<gpio::GPIOC, 3> PC3;
typedef gpio::Pin<gpio::GPIOC, 4> PC4;
typedef gpio::Pin<gpio::GPIOC, 5> PC5;
typedef gpio::Pin<gpio::GPIOC, 6> PC6;
typedef gpio::Pin<gpio::GPIOC, 7> PC7;

typedef gpio::Pin<gpio::GPIOD, 0> PD0;
typedef gpio::Pin<gpio::GPIOD, 1> PD1;
typedef gpio::Pin<gpio::GPIOD, 2> PD2;
typedef gpio::Pin<gpio::GPIOD, 3> PD3;
typedef gpio::Pin<gpio::GPIOD, 4> PD4;
typedef gpio::Pin<gpio::GPIOD, 5> PD5;
typedef gpio::Pin<gpio::GPIOD, 6> PD6;
typedef gpio::Pin<gpio::GPIOD, 7> PD7;

typedef gpio::Pin<gpio::GPIOE, 0> PE0;
typedef gpio::Pin<gpio::GPIOE, 1> PE1;
typedef gpio::Pin<gpio::GPIOE, 2> PE2;
typedef gpio::Pin<gpio::GPIOE, 3> PE3;
typedef gpio::Pin<gpio::GPIOE, 4> PE4;
typedef gpio::Pin<gpio::GPIOE, 5> PE5;
typedef gpio::Pin<gpio::GPIOE, 6> PE6;
typedef gpio::Pin<gpio::GPIOE, 7> PE7;

typedef gpio::Pin<gpio::GPIOF, 0> PF0;
typedef gpio::Pin<gpio::GPIOF, 1> PF1;
typedef gpio::Pin<gpio::GPIOF, 2> PF2;
typedef gpio::Pin<gpio::GPIOF, 3> PF3;
typedef gpio::Pin<gpio::GPIOF, 4> PF4;
typedef gpio::Pin<gpio::GPIOF, 5> PF5;
typedef gpio::Pin<gpio::GPIOF, 6> PF6;
typedef gpio::Pin<gpio::GPIOF, 7> PF7;

typedef gpio::Pin<gpio::GPIOG, 0> PG0;
typedef gpio::Pin<gpio::GPIOG, 1> PG1;
typedef gpio::Pin<gpio::GPIOG, 2> PG2;
typedef gpio::Pin<gpio::GPIOG, 3> PG3;
typedef gpio::Pin<gpio::GPIOG, 4> PG4;
typedef gpio::Pin<gpio::GPIOG, 5> PG5;
typedef gpio::Pin<gpio::GPIOG, 6> PG6;
typedef gpio::Pin<gpio::GPIOG, 7> PG7;
