#include "OLED.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "font.h"
#include <string.h>

#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ 10000

#define SLAVE_ADRESS_OLED 0x78

static const char *TAG = "OLED";

// implemented with help from:
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
// https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
// https://github.com/yanbe/ssd1306-esp-idf-i2c/blob/master/main/main.c
void init_i2c_driver(void){
    int i2c_master_port = I2C_NUM_0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,         // select SDA GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,         // select SCL GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,  // select frequency specific to your project
        .clk_flags = 0,                          // optional; you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here
    };

    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}


void init_OLED(void){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SLAVE_ADRESS_OLED) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);
    esp_err_t espRc;
    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

void clear_oled(void) {
	i2c_cmd_handle_t cmd;

	uint8_t zero[128] = {0};
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (SLAVE_ADRESS_OLED) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true); // reset page
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
    ESP_LOGI(TAG, "Cleared screen.");
}


void oled_text(char *text, int inverted) {
	// https://www.educative.io/answers/splitting-a-string-using-strtok-in-c
	uint8_t text_len = strlen(text);
	char dest[text_len];
	strcpy(dest, text);
	char * token = strtok(dest, "\n");
	uint8_t cur_line = 0;
	while( token != NULL ) {
      oled_println(token, inverted, cur_line);
	  ESP_LOGI(TAG, "Print out '%s' to display.", token);
      token = strtok(NULL, "\n");
	  cur_line++;
   }

}




void oled_println(char *text, int inverted, uint8_t line){
	uint8_t text_len = strlen(text);
	i2c_cmd_handle_t cmd;
	uint8_t letter[8];

	// init coms
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SLAVE_ADRESS_OLED) | I2C_MASTER_WRITE, true);

	// set writing location on display.
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column
	i2c_master_write_byte(cmd, 0x10, true);
	i2c_master_write_byte(cmd, 0xB0 | line, true); // set line

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	for (uint8_t i = 0; i < text_len; i++) {
			memcpy(letter, font8x8_basic_tr[(uint8_t)text[i]], 8);
			if (inverted > 0) _invert_text(letter, 8);
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (SLAVE_ADRESS_OLED) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, letter, 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
	}
}


void _invert_text(uint8_t *buf, size_t buflen){
	uint8_t inverted;
	for(int i=0; i<buflen; i++){
		inverted = buf[i];
		buf[i] = ~inverted;
	}
}
