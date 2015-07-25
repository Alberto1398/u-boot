#ifndef GAUGE_EG2801_H_
#define GAUGE_EG2801_H_

#define EG2801_I2C_ADDR		(0x55)
#define REG_ChargeState_H		(0x2c)
#define REG_ChargeState_L		(0x2d)

int eg2801_gasgauge_get_capacity(void);

#endif
