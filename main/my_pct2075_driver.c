#include <stdio.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "pct2075.h"

#define SDA_IO 22
#define SCL_IO 21

void
app_main(void)
{
	i2c_master_bus_config_t i2c_mst_config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = -1,
		.scl_io_num = SCL_IO,
		.sda_io_num = SDA_IO,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};
	i2c_master_bus_handle_t bus_handle;
	i2c_master_dev_handle_t pct2075_handle;
	uint32_t temp;
	uint16_t count, toggle;

	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

	if (pct2075_init(bus_handle, &pct2075_handle) != 0) {
		printf("error initializing the pct2075 device\n");
		while (1);
	}

	pct2075_get_tidle(pct2075_handle);
	count = 0;
	toggle = 0;
	while (1) {
		printf("tj\n");
		temp = pct2075_get_temp(pct2075_handle);
		printf("temperature: %d\n", (unsigned int)temp);

		count++;
		if (count == 1) {
			//printf("toggle %d\n", toggle);
			//if (toggle == 0)
				//pct2075_shutdown(pct2075_handle);
			//else
				pct2075_wakeup(pct2075_handle);
			count = 0;
			//toggle = !toggle;
		}
		vTaskDelay(200);
	}
}
