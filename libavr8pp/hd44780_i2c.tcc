
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace hd44780
{
  // Initialize
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::initialize()
  {
    DEVICE::initialize();
    DEVICE::sendStart();
    DEVICE::sendByte(DEVICE_ADDR << 1);

    mode = MODE_XX;
    pins = BL_MASK;
    DEVICE::sendByte(pins);
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

  // Clear screen
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::cls()
  {
    x = y = 0;
    set_ir_mode();
    write_byte(CMD_CLR_DISP);
    delay_us(1640);
  }

  // Set screen coordinates
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::setXY(u8 x, u8 y)
  {
    u8 addr;
    addr = (y & 1) ? 64 : 0;
    addr += x;

    switch(TP)
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

  // Put character on the screen
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::putchr(u8 c)
  {
    set_dr_mode();
    write_byte(c);
  }

  // Move cursor to a new line
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::cr()
  {
    x = 0; y++;
    if (y >= NY) y = 0;
    setXY(x, y);
  }

  // Set instruction mode
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::set_ir_mode()
  {
    if (mode == MODE_IR) return;

    pins &= ~RS_MASK;
    mode = MODE_IR;
    DEVICE::sendByte(pins);
  }

  // Set data mode
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::set_dr_mode()
  {
    if (mode == MODE_DR) return;

    pins |= RS_MASK;
    mode = MODE_DR;
    DEVICE::sendByte(pins);
  }

  // Write byte
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::write_byte(u8 d)
  {
    write_nibble(d >> 4);
    write_nibble(d & 0x0F);
    delay_us(40);
  }

  // Write half-byte
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::write_nibble(u8 d)
  {
    pins &= ~DATA_MASK;
    pins |= d << DATA_LSB;
    DEVICE::sendByte(pins);
    clk();
  }

  // Generate clock pulse
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::clk()
  {
    pins |= E_MASK;
    DEVICE::sendByte(pins);
    delay_us(2);
    pins &= ~E_MASK;
    DEVICE::sendByte(pins);
    delay_us(2);
  }

  // Set DDR (text) address
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::set_ddr_addr(u8 addr)
  {
    set_ir_mode();
    write_byte(CMD_SET_DDR_ADDR | addr);     // AC = DDRAM area address
  }

  // Set CGR (font) address
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ, TYPE TP>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ, TP>::set_cgr_addr(u8 addr)
  {
    set_ir_mode();
    write_byte(CMD_SET_CGR_ADDR | addr);     // AC = DDRAM area address
  }
}