#ifndef PCT2075_H
#define PCT2075_H

#include <driver/i2c_master.h>

#define PCT2075_HARDCODED_ADDRESS 0b1001000
#define PCT2075_CONF_POINTER 0x01
#define PCT2075_TEMP_POINTER 0x00
#define PCT2075_TOS_POINTER 0x03
#define PCT2075_THYST_POINTER 0x02
#define PCT2075_TIDLE_POINTER 0x04

#define OS_FAULT_QUEUE_MASK 0b00011000
#define OS_FAULT_QUEUE_OFFSET 3
#define OS_POL_MASK 0b00000100
#define OS_POL_OFFSET 2
#define OS_COMP_INT_MASK 0b00000010
#define OS_COMP_INT_OFFSET 1
#define MODE_MASK 0b00000001
#define MODE_OFFSET 0

enum OS_FAULT_QUEUE {
	OS_QUEUE_VAL1,
	OS_QUEUE_VAL2,
	OS_QUEUE_VAL4,
	OS_QUEUE_VAL6
};

enum OS_POL {
	OS_POL_LOW,
	OS_POL_HIGH
};

enum OS_COMP_INT {
	OS_COMP,
	OS_INT
};

enum MODE {
	MODE_NORMAL,
	MODE_SHUTDOWN
};

/* adds the pct2075 to a already initialized bus */
int pct2075_init(i2c_master_bus_handle_t bus_handle,
		 i2c_master_dev_handle_t *ret_handle);
uint8_t pct2075_get_conf(i2c_master_dev_handle_t pct2075_dev);
int pct2075_set_conf(i2c_master_dev_handle_t pct2075_dev, uint8_t cfg);
int pct2075_conf_os_fault_queue(i2c_master_dev_handle_t pct2075_dev,
				enum OS_FAULT_QUEUE val);
int pct2075_conf_os_pol(i2c_master_dev_handle_t pct2075_dev, enum OS_POL val);
int pct2075_conf_os_comp_int(i2c_master_dev_handle_t pct2075_dev,
			     enum OS_COMP_INT val);
int pct2075_conf_mode(i2c_master_dev_handle_t pct2075_dev, enum MODE val);
int pct2075_shutdown(i2c_master_dev_handle_t pct2075_dev);
int pct2075_wakeup(i2c_master_dev_handle_t pct2075_dev);
int32_t pct2075_get_temp(i2c_master_dev_handle_t pct2075_dev);

/*
 * warning: the following 5 functions don't convert between pct2075 register
 * contents and human readable values. it also doesn't shift the values
 * to the right by 5 bits
 */
uint16_t pct2075_get_temp_raw(i2c_master_dev_handle_t pct2075_dev);
uint16_t pct2075_get_tos(i2c_master_dev_handle_t pct2075_dev);
int pct2075_set_tos(i2c_master_dev_handle_t pct2075_dev, uint16_t val);
uint16_t pct2075_get_thyst(i2c_master_dev_handle_t pct2075_dev);
int  pct2075_set_thyst(i2c_master_dev_handle_t pct2075_dev, uint16_t val);
/*
 * end of warning
 */

uint8_t pct2075_get_tidle(i2c_master_dev_handle_t pct2075_dev);
uint8_t pct2075_set_tidle(i2c_master_dev_handle_t pct2075_dev, uint8_t val);

#endif /* #ifndef PCT2075_H */
