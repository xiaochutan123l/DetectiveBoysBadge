#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_rom_sys.h"

extern void i2s_init(void);
extern size_t read_audio(char *i2s_read_buff, int i2s_read_len);
extern size_t write_audio(char *i2s_write_buff, int i2s_write_len);
extern void _read_enable();
extern void _read_disable();
