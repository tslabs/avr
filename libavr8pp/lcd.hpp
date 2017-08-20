
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace lcd
{
  enum
  {
    LCD_DATA_MASK = 0xF << LCD_DATA_LSB
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

  enum
  {
    TYPE_16x2,
    TYPE_40x2,
    TYPE_20x4
  };

  u8 lcd_type = TYPE_16x2;

  void set_ir_mode()
  {
    set_gpio(LCD_RS, 0);
  }

  void set_dr_mode()
  {
    set_gpio(LCD_RS, 1);
  }

  void clk()
  {
    set_gpio(LCD_E, 1);
    delay_us(2);
    set_gpio(LCD_E, 0);
    delay_us(2);
  }

  void write_nibble(u8 d)
  {
    set_gpios(LCD_DATA, d << LCD_DATA_LSB)
    clk();
  }

  void write_byte(u8 d)
  {
    write_nibble(d >> 4);
    write_nibble(d & 0x0F);
    delay_us(40);
  }

  void cls(void)
  {
    set_ir_mode();
    write_byte(CMD_CLR_DISP);
    delay_us(1640);
  }

  void init(void)
  {
    set_gpio_mode(LCD_E, GPIO_OUT, 0);
    set_gpio_mode(LCD_RS, GPIO_OUT, 0);
    set_gpio_mode(LCD_RW, GPIO_OUT, 0);   // !!! obsolete
    set_gpios_mode(LCD_DATA, GPIO_OUT, 0);
    delay_ms(15);   // Power delay

    set_ir_mode();
    write_nibble((CMD_FUNC_SET | CMD_FUNC_SET_DL) >> 4);    // 0x3x - set databus width 8
    delay_us(4100);
    clk();
    delay_us(100);
    clk();
    write_nibble(CMD_FUNC_SET >> 4);                  // 0x2x - set databus width 4
    delay_us(40);
    write_byte(CMD_FUNC_SET | CMD_FUNC_SET_N);        // D/L = 0, N = 1, F = 0
    write_byte(CMD_DISP_CTR);                         // D = 0, C = 0, B = 0
    cls();
    write_byte(CMD_ENT_MD_SET | CMD_ENT_MD_SET_I_D);  // I/D = 1, S = 0
    write_byte(CMD_DISP_CTR | CMD_DISP_CTR_D);        // D = 1, C = 0, B = 0
  }

  void set_ddr_addr(u8 addr)
  {
    set_ir_mode();
    write_byte(CMD_SET_DDR_ADDR | addr);     // AC = DDRAM area address
  }

  void set_cgr_addr(u8 addr)
  {
    set_ir_mode();
    write_byte(CMD_SET_CGR_ADDR | addr);     // AC = DDRAM area address
  }

  void set_xy(u8 x, u8 y)
  {
    u8 addr;
    addr = (y & 1) ? 64 : 0;
    addr += x;

    switch(lcd_type)
    {
      case TYPE_20x4:
        addr += (y & 2) ? 20 : 0;
      break;

      case TYPE_16x2:
      case TYPE_40x2:
      break;

      default:
        return;
    }

    set_ddr_addr(addr);
  }

  void print_str(char *ptr)
  {
    set_dr_mode();

    char c;
    while (c = *ptr++)
      write_byte(c);
  }
}