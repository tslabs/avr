
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

// PCF8574T pins:
//  P0 - RS
//  P1 - RW
//  P2 - E
//  P3 - Backlight
//  P4..7 - D4..7
//  A0..2 = 111

#include "softi2c.hpp"

namespace hd44780
{
  enum
  {
    DEVICE_ADDR  = 0b0100111,
    RS_MASK   = 1,
    RW_MASK   = 2,
    E_MASK    = 4,
    BL_MASK   = 8,
    DATA_LSB  = 4,
    DATA_MASK = 0xF0
  };

  enum
  {
    CMD_CLR_DISP        = 0x01,

    CMD_RET_HOME        = 0x02,

    CMD_ENT_MD_SET      = 0x04,
    CMD_ENT_MD_SET_S    = 0x01,
    CMD_ENT_MD_SET_I_D  = 0x02,

    CMD_DISP_CTR        = 0x08,
    CMD_DISP_CTR_B      = 0x01,
    CMD_DISP_CTR_C      = 0x02,
    CMD_DISP_CTR_D      = 0x04,

    CMD_CUR_DISP_SH     = 0x10,
    CMD_CUR_DISP_SH_R_L = 0x04,
    CMD_CUR_DISP_SH_S_C = 0x08,

    CMD_FUNC_SET        = 0x20,
    CMD_FUNC_SET_F      = 0x04,
    CMD_FUNC_SET_N      = 0x08,
    CMD_FUNC_SET_DL     = 0x10,

    CMD_SET_CGR_ADDR    = 0x40,

    CMD_SET_DDR_ADDR    = 0x80
  };

  enum TYPE
  {
    TYPE_16x2,
    TYPE_40x2,
    TYPE_20x4
  };

  enum MODE
  {
    MODE_XX,
    MODE_IR,
    MODE_DR
  };

  template<typename>
  class Functions
  {
    public:
      u8 x, y;

      inline void Initialize(TYPE);
      inline void Cls();
      inline void SetXY(u8, u8);
      inline void Putchar(u8);
      inline void Cr();

    private:
      u8 pins;
      MODE mode;
			TYPE type;
			u8 lines;

      inline void setIrMode();
      inline void setDrMode();
      inline void writeByte(u8);
      inline void writeNibble(u8);
      inline void clk();
      inline void setDdrAddr(u8);
      inline void setCgrAddr(u8);
  };
}