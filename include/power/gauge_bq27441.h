#ifndef GAUGE_EQ27441_H_
#define GAUGE_EQ27441_H_

#define EQ27441_I2C_ADDR		(0x55)

int bq27441_gasgauge_get_capacity(void);
int bq27441_gasgauge_get_voltage(void);

#endif

