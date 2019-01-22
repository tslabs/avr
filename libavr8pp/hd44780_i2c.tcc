
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
  template<typename I2C>
  void Functions<I2C>::Initialize(TYPE tp)
  {
    type = tp;
		lines = (tp == TYPE_20x4) ? 4 : 2;
		
		I2C::SendStart();
    I2C::SendByte(DEVICE_ADDR << 1);

    mode = MODE_XX;
    pins = BL_MASK;
    I2C::SendByte(pins);
    delay_ms(15);   // Power delay

    setIrMode();
    writeNibble((CMD_FUNC_SET | CMD_FUNC_SET_DL) >> 4);    // 0x3x - set databus width 8
    delay_us(4100);
    clk();
    delay_us(100);
    clk();
    writeNibble(CMD_FUNC_SET >> 4);                  // 0x2x - set databus width 4
    delay_us(40);
    writeByte(CMD_FUNC_SET | CMD_FUNC_SET_N);        // D/L = 0, N = 1, F = 0
    writeByte(CMD_DISP_CTR);                         // D = 0, C = 0, B = 0
    I2C::SendStop();
    Cls();
    I2C::SendStart();
    I2C::SendByte(DEVICE_ADDR << 1);
    writeByte(CMD_ENT_MD_SET | CMD_ENT_MD_SET_I_D);  // I/D = 1, S = 0
    writeByte(CMD_DISP_CTR | CMD_DISP_CTR_D);        // D = 1, C = 0, B = 0
    I2C::SendStop();
  }

  // Clear screen
  template<typename I2C>
  void Functions<I2C>::Cls()
  {
    x = y = 0;
    I2C::SendStart();
    I2C::SendByte(DEVICE_ADDR << 1);
    setIrMode();
    writeByte(CMD_CLR_DISP);
    I2C::SendStop();
    delay_us(1640);
  }

  // Set screen coordinates
  template<typename I2C>
  void Functions<I2C>::SetXY(u8 x, u8 y)
  {
    u8 addr;
    addr = (y & 1) ? 64 : 0;
    addr += x;

    switch(type)
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

    I2C::SendStart();
    I2C::SendByte(DEVICE_ADDR << 1);
    setDdrAddr(addr);
    I2C::SendStop();
  }

  // Put character on the screen
  template<typename I2C>
  void Functions<I2C>::Putchar(u8 c)
  {
    I2C::SendStart();
    I2C::SendByte(DEVICE_ADDR << 1);
    setDrMode();
    writeByte(c);
    I2C::SendStop();
  }

  // Move cursor to a new line
  template<typename I2C>
  void Functions<I2C>::Cr()
  {
    x = 0; y++;
    if (y >= lines) y = 0;
    SetXY(x, y);
  }

  // Set instruction mode
  template<typename I2C>
  void Functions<I2C>::setIrMode()
  {
    if (mode == MODE_IR) return;

    pins &= ~RS_MASK;
    mode = MODE_IR;
    I2C::SendByte(pins);
  }

  // Set data mode
  template<typename I2C>
  void Functions<I2C>::setDrMode()
  {
    if (mode == MODE_DR) return;

    pins |= RS_MASK;
    mode = MODE_DR;
    I2C::SendByte(pins);
  }

  // Write byte
  template<typename I2C>
  void Functions<I2C>::writeByte(u8 d)
  {
    writeNibble(d >> 4);
    writeNibble(d & 0x0F);
    delay_us(40);
  }

  // Write half-byte
  template<typename I2C>
  void Functions<I2C>::writeNibble(u8 d)
  {
    pins &= ~DATA_MASK;
    pins |= d << DATA_LSB;
    I2C::SendByte(pins);
    clk();
  }

  // Generate clock pulse
  template<typename I2C>
  void Functions<I2C>::clk()
  {
    pins |= E_MASK;
    I2C::SendByte(pins);
    delay_us(2);
    pins &= ~E_MASK;
    I2C::SendByte(pins);
    delay_us(2);
  }

  // Set DDR (text) address
  template<typename I2C>
  void Functions<I2C>::setDdrAddr(u8 addr)
  {
    setIrMode();
    writeByte(CMD_SET_DDR_ADDR | addr);     // AC = DDRAM area address
  }

  // Set CGR (font) address
  template<typename I2C>
  void Functions<I2C>::setCgrAddr(u8 addr)
  {
    setIrMode();
    writeByte(CMD_SET_CGR_ADDR | addr);     // AC = DDRAM area address
  }
}