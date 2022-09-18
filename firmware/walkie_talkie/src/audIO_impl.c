/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/* ADC/DAC are not supported in the new I2S driver, but still available in the legacy I2S driver for backward compatibility
 * Please turn to the dedicated ADC/DAC driver instead */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "spi_flash_mmap.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "driver/i2s.h"
#include "driver/adc.h"
//#include "audio_example_file.h"
#include "esp_adc_cal.h"
#include "esp_rom_sys.h"
#include "audIO_impl.h"

#define I2S_WS 12
#define I2S_SD 14
#define I2S_SCK 13

#define I2S_MIC_PORT I2S_NUM_0

#define I2S_SPK_PORT I2S_NUM_1
/**
 * @brief I2S ADC/DAC mode init.
 */
/*
void i2s_init(void)
{
    // Set up I2S microfon configuration
  const i2s_config_t i2s_mic_config = {
    .mode = (I2S_MODE_MASTER | I2S_MODE_RX),
    //.sample_rate = 44100,
    .sample_rate = 8000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .communication_format = (i2s_comm_format_t)1,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .tx_desc_auto_clear = true,
    //.use_apll = true
  };

  // Set up I2S speaker configuration
  const i2s_config_t i2s_spk_config = {
    .mode = (I2S_MODE_MASTER | I2S_MODE_TX),
    //.sample_rate = 44100,
    .sample_rate = 8000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .communication_format = (i2s_comm_format_t)1,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .tx_desc_auto_clear = true,
    //.use_apll = true
  };
 
  i2s_driver_install(I2S_MIC_PORT, &i2s_mic_config, 0, NULL);
  i2s_driver_install(I2S_SPK_PORT, &i2s_spk_config, 0, NULL);
}
*/

void i2s_setpin() {
  // Set I2S microfon pin configuration
  const i2s_pin_config_t mic_pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
 
  // Set I2S speaker pin configuration
  const i2s_pin_config_t spk_pin_config = {
      .bck_io_num = 5,
      .ws_io_num = 18,
      .data_out_num = 17,
      .data_in_num = -1   //Not used
  };
  i2s_set_pin(I2S_MIC_PORT, &mic_pin_config);

  i2s_set_pin(I2S_SPK_PORT, &spk_pin_config);
}  

void i2s_init(void)
{
// Set up I2S microfon configuration
  const i2s_config_t i2s_mic_config = {
    .mode = (I2S_MODE_MASTER | I2S_MODE_RX),
    //.sample_rate = 44100,
    .sample_rate = 8000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .communication_format = (i2s_comm_format_t)1,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 100,
    .tx_desc_auto_clear = true,
    //.use_apll = true
  };

  // Set up I2S speaker configuration
  const i2s_config_t i2s_spk_config = {
    .mode = (I2S_MODE_MASTER | I2S_MODE_TX),
    //.sample_rate = 44100,
    .sample_rate = 8000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    //.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .communication_format = (i2s_comm_format_t)1,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 100,
    .tx_desc_auto_clear = true,
    //.use_apll = true
  };
 
  i2s_driver_install(I2S_MIC_PORT, &i2s_mic_config, 0, NULL);
  i2s_driver_install(I2S_SPK_PORT, &i2s_spk_config, 0, NULL);

  i2s_setpin();
}

size_t read_audio(char *i2s_read_buff, int i2s_read_len) {
    size_t bytes_read;
    /*
     8 bits adc samples. (8/12/16/24/32 bit available, 8 bit is enough for walkie talkie,
     and 8 bit doesn't requires scale the size. If later high sample accuracy required, 
     need to add adc_scale method, and double(for 12/16 bit) the buffer size)
    */
    // manually enable and disable i2s_adc if built-in adc in use.
    //i2s_adc_enable(EXAMPLE_I2S_NUM);
    int ret = i2s_read(EXAMPLE_I2S_MIC_NUM, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    //i2s_adc_disable(EXAMPLE_I2S_NUM);
    return bytes_read;
}

void _read_enable() {
    i2s_adc_enable(EXAMPLE_I2S_MIC_NUM);
    //i2s_set_sample_rates(EXAMPLE_I2S_NUM, 8000);
}

void _read_disable() {
    i2s_adc_disable(EXAMPLE_I2S_MIC_NUM);
}


size_t write_audio(char *i2s_write_buff, int i2s_write_len) {
    // adapt the buffer size and scale data if sample bits higher than 8 bits.
    size_t bytes_written;
    i2s_write(EXAMPLE_I2S_SPK_NUM, i2s_write_buff, i2s_write_len, &bytes_written, portMAX_DELAY);
    return bytes_written;
}



/* ADC_DAC Without I2S. Using FreeRTOS.
void adc_read_task(void* arg)
{
    ESP_LOGI(TAG, "Start read task");
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN_11db);
    esp_adc_cal_characteristics_t characteristics;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, V_REF, &characteristics);
    while(1) {
        uint32_t voltage;
        vTaskDelay(200 / portTICK_PERIOD_MS);
        esp_adc_cal_get_voltage(ADC1_TEST_CHANNEL, &characteristics, &voltage);
        ESP_LOGI(TAG, "%d mV", voltage);
    }
}

*/