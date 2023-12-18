#include <driver/i2c.h>
#include <stdio.h>
#include "pct2075.h"

/* TODO: return esp_err_t and data through arguments */

static int pct2075_conf_backend(struct pct2075_handle_t pct2075_dev,
				uint8_t val, uint8_t val_mask, uint8_t mask,
				uint8_t offset);
static inline uint8_t pct2075_alter_conf_bits(uint8_t current_conf, uint8_t val,
					      uint8_t val_mask, uint8_t mask,
					      uint8_t offset);
static int pct2075_set_1byte_register(struct pct2075_handle_t pct2075_dev,
				      uint8_t address, uint8_t val);
static uint8_t pct2075_get_1byte_register(struct pct2075_handle_t pct2075_dev,
					  uint8_t address);
static uint16_t pct2075_get_2byte_register(struct pct2075_handle_t pct2075_dev,
					   uint8_t address);
static int pct2075_set_2byte_register(struct pct2075_handle_t pct2075_dev,
				      uint8_t address, uint16_t val);

/*
 * adds the pct2075 sensor to the i2c bus given by bus_handle and returns in
 * ret_handle its handle.
 * returns 0 on success and 1 on failure
 */
int
pct2075_init(int master_port, uint8_t dev_addr, int timeout_in_ms,
		 struct pct2075_handle_t *ret_handle)
{
	if (ret_handle == NULL)
		return 1;

	ret_handle->master_port = master_port;
	ret_handle->dev_addr = dev_addr;
	ret_handle->timeout_ms = timeout_in_ms;

	return 0;
}

/*
 * reads the contents of the configuration register and returns what was read
 */
uint8_t
pct2075_get_conf(struct pct2075_handle_t pct2075_dev)
{
	return pct2075_get_1byte_register(pct2075_dev, PCT2075_CONF_POINTER);
}

/*
 * sets the contents of the configuration register
 * returns 0 on success and 1 otherwise
 */
int
pct2075_set_conf(struct pct2075_handle_t pct2075_dev, uint8_t cfg)
{
	return pct2075_set_1byte_register(pct2075_dev, PCT2075_CONF_POINTER,
					  cfg);
}

/*
 * sets the OS_FAULT_QUEUE bit(s) in the configuration register by:
 *	reading the configuration register and storing the value
 *	on the stored value, manipulate the bits accordingly
 *	write to the configuration register the manipulated data
 * go to the definition of pct2075_conf_backend
 *
 * returns 0 on success and 1 otherwise
 */
int
pct2075_conf_os_fault_queue(struct pct2075_handle_t pct2075_dev,
			    enum OS_FAULT_QUEUE val)
{
	return pct2075_conf_backend(pct2075_dev, val, 0b11, OS_FAULT_QUEUE_MASK,
				    OS_FAULT_QUEUE_OFFSET);
}

/*
 * sets the OS_POL bit(s) in the configuration register by:
 *	reading the configuration register and storing the value
 *	on the stored value, manipulate the bits accordingly
 *	write to the configuration register the manipulated data
 * go to the definition of pct2075_conf_backend
 *
 * returns 0 on success and 1 otherwise
 */
int
pct2075_conf_os_pol(struct pct2075_handle_t pct2075_dev, enum OS_POL val)
{
	return pct2075_conf_backend(pct2075_dev, val, 0b1, OS_POL_MASK,
				    OS_POL_OFFSET);
}

/*
 * sets the OS_COMP_INT bit(s) in the configuration register by:
 *	reading the configuration register and storing the value
 *	on the stored value, manipulate the bits accordingly
 *	write to the configuration register the manipulated data
 * go to the definition of pct2075_conf_backend
 *
 * returns 0 on success and 1 otherwise
 */
int
pct2075_conf_os_comp_int(struct pct2075_handle_t pct2075_dev,
		         enum OS_COMP_INT val)
{
	return pct2075_conf_backend(pct2075_dev, val, 0b1, OS_COMP_INT_MASK,
				    OS_COMP_INT_OFFSET);
}

/*
 * sets the MODE bit(s) in the configuration register by:
 *	reading the configuration register and storing the value
 *	on the stored value, manipulate the bits accordingly
 *	write to the configuration register the manipulated data
 * go to the definition of pct2075_conf_backend
 *
 * returns 0 on success and 1 otherwise
 */
int
pct2075_conf_mode(struct pct2075_handle_t pct2075_dev, enum MODE val)
{

	return pct2075_conf_backend(pct2075_dev, val, 0b1, MODE_MASK,
				    MODE_OFFSET);
}

/*
 * uses the pct2075_conf_mode function in order to set the mode to shutdown
 * returns 0 on success and 1 otherwise
 */
int
pct2075_shutdown(struct pct2075_handle_t pct2075_dev)
{
	return pct2075_conf_mode(pct2075_dev, MODE_SHUTDOWN);
}

/*
 * uses the pct2075_conf_mode function in order to set the mode to normal
 * returns 0 on success and 1 otherwise
 */
int
pct2075_wakeup(struct pct2075_handle_t pct2075_dev)
{
	return pct2075_conf_mode(pct2075_dev, MODE_NORMAL);
}

/*
 * this functions reads the temperature and then does calculations according
 * to the formula on page 11 of the datasheet. the value is calculated with
 * 3 fixed point accurracy
 *
 * returns the calculated temperature (with 3 fixed point accurracy)
 */
int32_t
pct2075_get_temp(struct pct2075_handle_t pct2075_dev)
{
	int32_t to_return = 0;
	uint16_t raw_temp;

	raw_temp = pct2075_get_temp_raw(pct2075_dev);

	if (!(raw_temp & 0x8000)) {
		to_return = ((((int32_t)raw_temp) >> 5) & 0b1111111111) * 125;
	} else {
		to_return =
		    ((((~((int32_t) raw_temp)) >> 5) & 0b1111111111) +
		     1) * 125;
		to_return = -1 * to_return;
	}
	
	return to_return;
}

/*
 * warning: the following 5 functions don't convert between pct2075 register
 * contents and human readable values. it also doesn't shift the values
 * to the right by 5 bits
 */

/*
 * reads the temperature register and returns the data as-is (not shifted to
 * the right by 5 bits)
 */
uint16_t
pct2075_get_temp_raw(struct pct2075_handle_t pct2075_dev)
{
	return pct2075_get_2byte_register(pct2075_dev, PCT2075_TEMP_POINTER);
}

/*
 * reads the over temperature shutdown register and returns the data as-is (not
 * shifted to the right by 5 bits)
 */
uint16_t
pct2075_get_tos(struct pct2075_handle_t pct2075_dev)
{
	return pct2075_get_2byte_register(pct2075_dev, PCT2075_TOS_POINTER);
}

/*
 * sets the over temperature shutdown register to val (given as is, not shifted
 * to the left by 5 bits)
 * returns 0 on success and 1 otherwise
 */
int
pct2075_set_tos(struct pct2075_handle_t pct2075_dev, uint16_t val)
{
	return pct2075_set_2byte_register(pct2075_dev, PCT2075_TOS_POINTER,
					  val);
}

/*
 * reads the temperature hysteresis register and returns the data as-is (not
 * shifted to the right by 5 bits)
 */
uint16_t
pct2075_get_thyst(struct pct2075_handle_t pct2075_dev)
{
	return pct2075_get_2byte_register(pct2075_dev, PCT2075_THYST_POINTER);
}

/*
 * sets the over temperature shutdown register to val (given as is, not shifted
 * to the left by 5 bits)
 * returns 0 on success and 1 otherwise
 */
int
pct2075_set_thyst(struct pct2075_handle_t pct2075_dev, uint16_t val)
{
	return pct2075_set_2byte_register(pct2075_dev, PCT2075_THYST_POINTER,
					  val);
}

/*
 * end of the warning
 */

/*
 * reads the data in the tidle register and returns the read value
 */
uint8_t
pct2075_get_tidle(struct pct2075_handle_t pct2075_dev)
{
	return pct2075_get_1byte_register(pct2075_dev, PCT2075_TIDLE_POINTER);
}

/*
 * sets the tidle register to val
 * returns 0 on success and 1 otherwise
 */
uint8_t
pct2075_set_tidle(struct pct2075_handle_t pct2075_dev, uint8_t val)
{
	return pct2075_set_1byte_register(pct2075_dev, PCT2075_TIDLE_POINTER,
					  val);
}

/* start of static functions */

/*
 * this is a internal function used to read the 8 bits registers of the pct2075
 * reads the register pointed to by address and returns the read value
 */
static uint8_t
pct2075_get_1byte_register(struct pct2075_handle_t pct2075_dev, uint8_t address)
{
	uint8_t to_send[1], read_val[1];

	to_send[0] = address;

	/* if this solution doesn't work, then make a custom
	 * command
	 */
	i2c_master_write_read_device(pct2075_dev.master_port,
				   pct2075_dev.dev_addr,
				   to_send, sizeof(to_send),
				   read_val, sizeof(read_val),
				   pct2075_dev.timeout_ms / portTICK_PERIOD_MS);

	return *read_val;
}

/*
 * this is a internal function used to set the 8 bits registers of the pct2075
 * sets the register pointed to by address to val
 * returns 0 on success and 1 otherwise
 */
static int
pct2075_set_1byte_register(struct pct2075_handle_t pct2075_dev, uint8_t address,
			   uint8_t val)
{
	int wret = 0;
	uint8_t to_send[2];

	to_send[0] = address;
	to_send[1] = val;

	wret = i2c_master_write_to_device(pct2075_dev.master_port,
				   pct2075_dev.dev_addr,
				   to_send, sizeof(to_send),
				   pct2075_dev.timeout_ms / portTICK_PERIOD_MS);
	
	return !(wret == ESP_OK);
}

/*
 * this is a internal function used to read the 16 bits registers of the pct2075
 * reads the register pointed to by address and returns the read value
 * constructed into a 16 bit int. (i assume they are returned in little endian
 * format by the pct2075 )
 */
static uint16_t
pct2075_get_2byte_register(struct pct2075_handle_t pct2075_dev, uint8_t address)
{
	uint16_t raw_data_constructed;
	uint8_t to_send[1] = {0}, raw_data[2];

	to_send[0] = address;
	
	i2c_master_write_read_device(pct2075_dev.master_port,
				   pct2075_dev.dev_addr,
				   to_send, sizeof(to_send),
				   raw_data, sizeof(raw_data),
				   pct2075_dev.timeout_ms / portTICK_PERIOD_MS);

	raw_data_constructed = (raw_data[0] << 8) | raw_data[1];

	return raw_data_constructed;
}

/*
 * this is a internal function used to set the 8 bits registers of the pct2075
 * sets the register pointed to by address to val (i assume the pct2075 expects
 * data in little endian format)
 * returns 0 on success and 1 otherwise
 */
static int
pct2075_set_2byte_register(struct pct2075_handle_t pct2075_dev, uint8_t address,
			   uint16_t val)
{
	int wret = 0;
	uint8_t to_send[3] = {0};

	to_send[0] = address;
	to_send[1] = val >> 8;
	to_send[2] = val & 0xff;
	
	wret = i2c_master_write_to_device(pct2075_dev.master_port,
				   pct2075_dev.dev_addr,
				   to_send, sizeof(to_send),
				   pct2075_dev.timeout_ms / portTICK_PERIOD_MS);

	return !(wret != ESP_OK);
}

/*
 * this is a internal function used to configure the bits pointed to by the
 * mask and offset inside of the configuration register
 * first, the configuration register is read and stored, the using the function
 * pct_alter_conf_bits, and then the altered value is written back into the
 * configuration register
 *
 * returns 1 on success and 0 otherwise
 */
static int
pct2075_conf_backend(struct pct2075_handle_t pct2075_dev, uint8_t val,
		     uint8_t val_mask, uint8_t mask, uint8_t offset)
{
	uint8_t current_conf = 0;

	current_conf = pct2075_get_conf(pct2075_dev);
	current_conf = pct2075_alter_conf_bits(current_conf, val, val_mask,
					       mask, offset);

	return pct2075_set_conf(pct2075_dev, current_conf);
}

/*
 * this is a internal function used to alted the bits of current_conf according
 * to this rule:
 * the bits that are set in mask will be cleared in current_conf
 * for val, only the required bits are kept and the shifted to the left
 * by offset and the these bits are set inside of the current_conf
 * the altered value is returned
 */
static inline uint8_t
pct2075_alter_conf_bits(uint8_t current_conf, uint8_t val, uint8_t val_mask,
			uint8_t mask, uint8_t offset)
{
	current_conf = current_conf & (~mask);
	current_conf |= ((val & val_mask) << offset);

	return current_conf;
}
