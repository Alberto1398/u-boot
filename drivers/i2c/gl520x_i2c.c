/*
 * gl520x_i2c.c
 *
 *  Created on: Aug 11, 2014
 *      Author: clamshell
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>


typedef struct
{
    uint32_t    io_base;
    uint8_t     clk_reg_bit;
    uint8_t     rst_reg_bit;
    uint32_t    curr_clk_freq;

} acts_gl520x_i2c_obj_t;

/**
 * i2c_set_bus_speed - set i2c bus speed
 *  @speed: bus speed (in HZ)
 */
int i2c_set_bus_speed(unsigned int speed)
{
    return 0;
}

/**
 * i2c_get_bus_speed - get i2c bus speed
 *  @speed: bus speed (in HZ)
 */
unsigned int i2c_get_bus_speed(void)
{
}

/**
 * i2c_init - initialize the i2c bus
 *  @speed: bus speed (in HZ)
 *  @slaveaddr: address of device in slave mode (0 - not slave)
 *
 *  Slave mode isn't actually implemented.  It'll stay that way until
 *  we get a real request for it.
 */
void i2c_init(int speed, int slaveaddr)
{
}

/**
 * i2c_probe - test if a chip exists at a given i2c address
 *  @chip: i2c chip addr to search for
 *  @return: 0 if found, non-0 if not found
 */
int i2c_probe(uchar chip)
{
}

/**
 * i2c_read - read data from an i2c device
 *  @chip: i2c chip addr
 *  @addr: memory (register) address in the chip
 *  @alen: byte size of address
 *  @buffer: buffer to store data read from chip
 *  @len: how many bytes to read
 *  @return: 0 on success, non-0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
}

/**
 * i2c_write - write data to an i2c device
 *  @chip: i2c chip addr
 *  @addr: memory (register) address in the chip
 *  @alen: byte size of address
 *  @buffer: buffer holding data to write to chip
 *  @len: how many bytes to write
 *  @return: 0 on success, non-0 on failure
 */
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
}

/**
 * i2c_set_bus_num - change active I2C bus
 *  @bus: bus index, zero based
 *  @returns: 0 on success, non-0 on failure
 */
int i2c_set_bus_num(unsigned int bus)
{
}

/**
 * i2c_get_bus_num - returns index of active I2C bus
 */
unsigned int i2c_get_bus_num(void)
{
}
