
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
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::initialize()
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
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::sendStart()
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
  void Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::sendStop()
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
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::sendByte(u8 d)
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
  u8 Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::recvByte(bool ack)
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
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::readReg(u8 addr, u8 reg, u8 &data)
  {
    sendStart();
    if (!sendByte(addr << 1)) goto err;
    if (!sendByte(reg)) goto err;
    sendStart();
    if (!sendByte((addr << 1) | 1)) goto err;
    data = recvByte(false);
    sendStop();
    return true;

  err:
    sendStop();
    return false;
  }

  // write register to device: <S><SLAW><reg><data><P>
  template<gpio::Address SCL_PORT, u8 SCL_PIN, gpio::Address SDA_PORT, u8 SDA_PIN, u32 FREQ>
  bool Functions<SCL_PORT, SCL_PIN, SDA_PORT, SDA_PIN, FREQ>::writeReg(u8 addr, u8 reg, u8 data)
  {
    sendStart();
    if (!sendByte(addr << 1)) goto err;
    if (!sendByte(reg)) goto err;
    if (!sendByte(data)) goto err;
    sendStop();
    return true;

  err:
    sendStop();
    return false;
  }
}
