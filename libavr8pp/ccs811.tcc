
/*******************************************************************************
 *
 * Copyright (C) 2019 TSL
 *
 * Licenced with GNU General Public License
 * <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

namespace ccs811
{
  template<typename I2C>
  bool Functions<I2C>::Initialize()
  {
    u8 reg;

/*
    // check ID
    if (!I2C::ReadReg(DEVICE_ADDR, REG_ID, reg)) return false;
    if (reg != REG_ID_VAL) return false;

    // read calibration
    for (u8 i = 0; i <= (REG_CALIB_21 - REG_CALIB_00); i++)
      if (!I2C::ReadReg(DEVICE_ADDR, REG_CALIB_00 + i, c[i ^ 1])) return false;
*/
    return true;
  }
}
