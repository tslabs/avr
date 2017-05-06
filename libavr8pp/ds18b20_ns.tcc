
namespace ds18b20
{
  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  void Functions<DATA_PORT, DATA_PIN, TIMER>::initialize()
  {
    is_conv = false;
    DEVICE::initialize();
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  bool Functions<DATA_PORT, DATA_PIN, TIMER>::convertTemperature()
  {
    if (!DEVICE::reset()) return false;

    // Start the temperature conversion
    DEVICE::writeByte(SKIP_ROM);
    DEVICE::writeByte(CONVERT_T);

    return true;
  }

  template<gpio::Address DATA_PORT, u8 DATA_PIN, TFUNC TIMER>
  bool Functions<DATA_PORT, DATA_PIN, TIMER>::readTemperature(s16 &temp)
  {
    bool rc = false;

    // check for device presence
    if (DEVICE::reset())
    {
      if (is_conv)
      {
        // Read scratchpad
        DEVICE::writeByte(SKIP_ROM);
        DEVICE::writeByte(READ);

        // 8T.4t is tttt0000, TTTTTTTT
        temp = DEVICE::readByte() << 4;
        temp |= DEVICE::readByte() << 12;
        rc = true;
      }

      convertTemperature();
      is_conv = true;
    }
    else
      is_conv = false;

    return rc;
  }
}
