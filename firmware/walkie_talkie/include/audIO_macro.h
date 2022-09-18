/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/* ADC/DAC are not supported in the new I2S driver, but still available in the legacy I2S driver for backward compatibility
 * Please turn to the dedicated ADC/DAC driver instead */

#pragma once
#pragma message("ADC/DAC on ESP32 will no longer supported via I2S driver")

//#include <stdio.h>
//#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "spi_flash_mmap.h"
#include "esp_err.h"
#include "esp_log.h"
//#include "esp_partition.h"
#include "driver/i2s.h"
#include "driver/adc.h"
//#include "audio_example_file.h"
#include "esp_adc_cal.h"
#include "esp_rom_sys.h"

#if CONFIG_IDF_TARGET_ESP32
//static const char* TAG = "i2s_adc_dac";
//#define V_REF   1100
#define ADC1_TEST_CHANNEL (ADC1_CHANNEL_7) //GPIO35(SP)

#define PARTITION_NAME   "storage"

/*---------------------------------------------------------------
                            EXAMPLE CONFIG
---------------------------------------------------------------*/


//i2s number
#define EXAMPLE_I2S_MIC_NUM           (0)
#define EXAMPLE_I2S_SPK_NUM           (1)
//i2s sample rate
//#define EXAMPLE_I2S_SAMPLE_RATE   (16000)
#define EXAMPLE_I2S_SAMPLE_RATE   (8000)
//i2s data bits
//#define EXAMPLE_I2S_SAMPLE_BITS   (16)
#define EXAMPLE_I2S_SAMPLE_BITS   (8)
//enable display buffer for debug
#define EXAMPLE_I2S_BUF_DEBUG     (0)
//I2S read buffer length
#define EXAMPLE_I2S_READ_LEN      (16 * 1024)
//I2S data format
//#define EXAMPLE_I2S_FORMAT        (I2S_CHANNEL_FMT_RIGHT_LEFT)
#define EXAMPLE_I2S_FORMAT        (I2S_CHANNEL_FMT_ONLY_LEFT)
//I2S channel number
#define EXAMPLE_I2S_CHANNEL_NUM   ((EXAMPLE_I2S_FORMAT < I2S_CHANNEL_FMT_ONLY_RIGHT) ? (2) : (1))
//I2S built-in ADC unit
#define I2S_ADC_UNIT              ADC_UNIT_1
//I2S built-in ADC channel
#define I2S_ADC_CHANNEL           ADC1_CHANNEL_0

#define DMA_BUF_COUNT (2)
#define DMA_BUF_LEN (1024)

/*
//enable record sound and save in flash
#define RECORD_IN_FLASH_EN        (1)
//enable replay recorded sound in flash
#define REPLAY_FROM_FLASH_EN      (1)
//flash record size, for recording 5 seconds' data
#define FLASH_RECORD_SIZE         (EXAMPLE_I2S_CHANNEL_NUM * EXAMPLE_I2S_SAMPLE_RATE * EXAMPLE_I2S_SAMPLE_BITS / 8 * 5)
#define FLASH_ERASE_SIZE          (FLASH_RECORD_SIZE % FLASH_SECTOR_SIZE == 0) ? FLASH_RECORD_SIZE : FLASH_RECORD_SIZE + (FLASH_SECTOR_SIZE - FLASH_RECORD_SIZE % FLASH_SECTOR_SIZE)
//sector size of flash
#define FLASH_SECTOR_SIZE         (0x1000)
//flash read / write address
#define FLASH_ADDR                (0x200000)
*/
#endif
