
/*******************************************************************************
 *
 * Copyright (C) 2016 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace softi2c
{
  // initialize IOs
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::Initialize()
  {
    SDA::setIn();
    SCL::setIn();
    SDA::setLow();
    SCL::setLow();
  }

  // check if SCL line is free or the slave holds it
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::waitScl()
  {
    u16 t;
    while (!SCL::getPin())
    {
      if (!(++t)) return;
    }
  }

  // send I2C START condition
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::SendStart()
  {
    SDA::setIn();
    delay(DELAY);
    SCL::setIn();
    delay(DELAY);
    waitScl();
    SDA::setOut();
    delay(DELAY);
    SCL::setOut();
  }

  // send I2C STOP condition
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::SendStop()
  {
    SDA::setOut();
    delay(DELAY);
    SCL::setIn();
    delay(DELAY);
    waitScl();
    SDA::setIn();
  }

  // send byte
  // returns ACK status: 0 - NACK, 1 - ACK
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::SendByte(u8 d)
  {
    for (u8 i = 0; i < 8; i++)
    {
      (d & 0x80) ? SDA::setIn() : SDA::setOut();
      delay(DELAY);
      SCL::setIn();
      d <<= 1;
      delay(DELAY);
      waitScl();
      SCL::setOut();
    }

    SDA::setIn();
    delay(DELAY);
    SCL::setIn();
    delay(DELAY);
    waitScl();
    bool rc = !SDA::getPin();
    SCL::setOut();

    return rc;
  }

  // receive byte
  // input: return 0 - NACK, 1 - ACK
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  u8 Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::RecvByte(bool ack)
  {
    u8 d = 0;
    SDA::setIn();

    for (u8 i = 0; i < 8; i++)
    {
      delay(DELAY);
      SCL::setIn();
      delay(DELAY);
      waitScl();
      d <<= 1; if (SDA::getPin()) d++;
      SCL::setOut();
    }

    // send I2C ACK/NACK condition
    if (ack)
      SDA::setOut();
    else
      SDA::setIn();
    delay(DELAY);
    SCL::setIn();
    delay(DELAY);
    waitScl();
    SCL::setOut();

    return d;
  }

  // read register from device: <S><SLAW><reg><R><SLAR><data><P>
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::ReadReg(u8 addr, u8 reg, u8 &data)
  {
    return ReadReg(addr, reg, (u8*)data, 1);
  }

  // read register from device: <S><SLAW><reg><R><SLAR><data><P>
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::ReadReg(u8 addr, u8 reg, u8 *data, u8 len)
  {
    SendStart();
    if (!SendByte(addr << 1)) goto err;
    if (!SendByte(reg)) goto err;
    SendStart();
    if (!SendByte((addr << 1) | 1)) goto err;

    while (len--)
      *data++ = RecvByte(len != 0);

    SendStop();
    return true;

  err:
    SendStop();
    return false;
  }

  // write register to device: <S><SLAW><reg><data><P>
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::WriteReg(u8 addr, u8 reg, u8 data)
  {
    return WriteReg(addr, reg, (u8*)&data, 1);
  }

  // write register to device: <S><SLAW><reg><data><P>
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::WriteReg(u8 addr, u8 reg, u8 *data, u8 len)
  {
    SendStart();
    if (!SendByte(addr << 1)) goto err;
    if (!SendByte(reg)) goto err;

    while (len--)
      if (!SendByte(*data++)) goto err;

    SendStop();
    return true;

  err:
    SendStop();
    return false;
  }
}
