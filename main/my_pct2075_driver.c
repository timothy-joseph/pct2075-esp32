#include <stdio.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "pct2075.h"

#define SDA_IO 22
#define SCL_IO 21

void
app_main(void)
{
	struct pct2075_handle_t pct2075_handle = {0};
	int i2c_master_port = I2C_NUM_0;
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_IO,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_io_num = SCL_IO,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 200000,
		.clk_flags = 0,
	};

	uint32_t temp;
	uint16_t count, toggle;

	i2c_param_config(i2c_master_port, &conf);
	i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);

	pct2075_init(i2c_master_port, PCT2075_HARDCODED_ADDRESS, 200,
		     &pct2075_handle);

	/* this function will generate a error, comment it out for the
	 * temperature demonstration
	 */

	pct2075_shutdown(pct2075_handle);
	count = 0;
	toggle = 0;
	while (1) {
		printf("tj\n");
		temp = pct2075_get_temp(pct2075_handle);
		printf("temperature %d: %d\n", toggle, (unsigned int)temp);
		printf("configuration: %hhu", pct2075_get_conf(pct2075_handle));

		count++;
		if (count == 10) {
			toggle = !toggle;
			if (toggle == 0)
				pct2075_shutdown(pct2075_handle);
			else
				pct2075_wakeup(pct2075_handle);
			count = 0;
		}
		vTaskDelay(200);
	}
}
