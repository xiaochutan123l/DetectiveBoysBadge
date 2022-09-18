/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/* ADC/DAC are not supported in the new I2S driver, but still available in the legacy I2S driver for backward compatibility
 * Please turn to the dedicated ADC/DAC driver instead */

#include "audIO_impl.h"

#define I2S_MIC_WS 12
#define I2S_MIC_SD 14
#define I2S_MIC_SCK 13

#define I2S_SPK_LRC 18
#define I2S_SPK_DIN 17
#define I2S_SPK_SCK 5

#define I2S_MIC_PORT I2S_NUM_0
#define I2S_SPK_PORT I2S_NUM_1

#define DMA_BUF_COUNT 4
#define DMA_BUF_LEN 100

#define SAMPLE_RATE 8000

void i2s_setpin() {
  // Set I2S microfon pin configuration
  const i2s_pin_config_t mic_pin_config = {
    .bck_io_num = I2S_MIC_SCK,
    .ws_io_num = I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SD
  };
 
  // Set I2S speaker pin configuration
  const i2s_pin_config_t spk_pin_config = {
      .bck_io_num = I2S_SPK_SCK,
      .ws_io_num = I2S_SPK_LRC,
      .data_out_num = I2S_SPK_DIN,
      .data_in_num = I2S_PIN_NO_CHANGE   //Not used
  };

  i2s_set_pin(I2S_MIC_PORT, &mic_pin_config);
  i2s_set_pin(I2S_SPK_PORT, &spk_pin_config);
}  

void i2s_init(void)
{
// Set up I2S microfon configuration
  const i2s_config_t i2s_mic_config = {
    .mode = (I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)1,
    .intr_alloc_flags = 0,
    .dma_buf_count = DMA_BUF_COUNT,
    .dma_buf_len = DMA_BUF_LEN,
    .tx_desc_auto_clear = true,
    //.use_apll = true
  };

  // Set up I2S speaker configuration
  const i2s_config_t i2s_spk_config = {
    .mode = (I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)1,
    .intr_alloc_flags = 0,
    .dma_buf_count = DMA_BUF_COUNT,
    .dma_buf_len = DMA_BUF_LEN,
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
    i2s_read(I2S_MIC_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    //i2s_adc_disable(EXAMPLE_I2S_NUM);
    return bytes_read;
}

void _read_enable() {
    i2s_adc_enable(I2S_MIC_PORT);
}

void _read_disable() {
    i2s_adc_disable(I2S_MIC_PORT);
}


size_t write_audio(char *i2s_write_buff, int i2s_write_len) {
    // adapt the buffer size and scale data if sample bits higher than 8 bits.
    size_t bytes_written;
    i2s_write(I2S_SPK_PORT, i2s_write_buff, i2s_write_len, &bytes_written, portMAX_DELAY);
    return bytes_written;
}