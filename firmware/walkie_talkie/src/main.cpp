#include "esp_err.h"
#include "esp_log.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ESPNow.h"
#include "audIO.h"
#include "ESP_GPIO.h"

#include<stdio.h>
#include<time.h>

#include<audio_example_file.h>

#define AUDIO_HZ (8000)
#define SEG_NUM_PER_SEC (40) //25ms segment.
#define BUF_LEN (200) // 8 x (8000 / 40) bytes

extern "C" {
    esp_err_t app_main(void);
}

static const char *TAG = "GPIO";

/*
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    flag1 = true;
}*/

void setup() {
    esp_log_level_set(TAG, ESP_LOG_INFO);
}
/*
void onReceivedPacket(example_espnow_event_recv_cb_t& packet) {
    //ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(packet.mac_addr));
    ESP_LOGI(TAG, "received one packet from " MACSTR "\n", MAC2STR(packet.mac_addr));
    //printf("%d-bytes data: %s", packet.data_len, packet.data);
    //printf("DATA: %s\n", packet.data);
    //ESP_LOGI(TAG, "DATA: %s\n", packet.data);
} */

void convert_int32_to_uint8(int32_t *src_buf, uint8_t *des_buf, size_t sample_size) {
  for (int i = 0; i < sample_size; ++i) {
    //convert int32 to int16
    int32_t temp = src_buf[i] >> 11;
    //  convert int16 to uint8
    int16_t temp16 = (temp > INT16_MAX) ? INT16_MAX : (temp < -INT16_MAX) ? -INT16_MAX : (int16_t)temp;
    des_buf[i] = (temp16 + 32768) >> 8;
  }
}

void convert_int16_to_uint8(int16_t *src_buf, uint8_t *des_buf, size_t sample_size) {
  for (int i = 0; i < sample_size; ++i) {
    des_buf[i] = (src_buf[i] + 32768) >> 8;
  }
}

void convert_uint8_to_int16(uint8_t *src_buf, int16_t *des_buf, size_t sample_size) {
  for (int i = 0; i < sample_size; ++i) {
    des_buf[i] = (src_buf[i] - 128) << 8;
  }
}

esp_err_t app_main(void)
{
    //单核无os方案，按下按钮读取然后发送数据，松开按钮，只接收并播放数据。
    //future - 1. 双核无os
    //       - 2. freertos
    setup();

    // set up esp-now.
    ESPNow espNow;
    espNow.init();
    printf("espNow init finished\n");
    //Start handling loop.
    //espNow.registerCallback(onReceivedPacket);
    espNow.start();

    // set up gpio.
    ESP_GPIO gpio1(GPIO_NUM_32, Pin_mode::INPUT, Pull_mode::PULL_UP);
    //gpio1.set_isr(gpio_isr_handler, Trigger_mode::FALLING);
    printf("gpio init finished\n");
    //set up adc-dac-i2s audio IO.
    AudIO audio = AudIO();    
    audio.init();
    printf("AudIO init finished\n");


    // variables.
    int sample_size = BUF_LEN;

    int read_buf_len = BUF_LEN * 2;
    int16_t* read_buf = (int16_t*) calloc(sample_size, sizeof(int16_t)); // record double size audio data, because only half are valid data(8bit).

    uint8_t* send_buf = (uint8_t*) calloc(sample_size, sizeof(uint8_t));
    
    int play_buf_len = BUF_LEN * 2;
    int16_t* play_buf = (int16_t*) calloc(sample_size, sizeof(int16_t));

    printf("running into while loop\n");
    while(1) {
        printf("bttun: %d", gpio1.readLevel());
        if (gpio1.readLevel() == 1) {
            printf("push botton pushed, recording....\n\n");
            if (espNow.get_botton_status() == false) {
                espNow.set_botton_status(true);
            }
            
            size_t start_t = esp_timer_get_time();

            while(esp_timer_get_time() - start_t < 1000000 || gpio1.readLevel() == 1){
                printf("recored %d bytes.\n", audio.audIO_read((char*)read_buf, read_buf_len));
                //vTaskDelay(25 / portTICK_PERIOD_MS);
                convert_int16_to_uint8(read_buf, send_buf, sample_size);
                //memcpy(big_send_buf+ 200*send_count, send_buf, data_len);
                espNow.broadcast(send_buf, sample_size); 
            }
        }
        else {
            printf("push botton released, playing....\n\n");
            if (espNow.get_botton_status() == true) {
                espNow.set_botton_status(false);
            }
            // receive audio data and play. 
            //从回调函数那搞来数据包内容.
            
            example_espnow_event_recv_cb_t packet;
            //size_t s_t = esp_timer_get_time();
            while (gpio1.readLevel() == 0){
                if (espNow.recv_packet(packet) == true) {
                    convert_uint8_to_int16(packet.data, play_buf, packet.data_len);
                    //for(int i = 0; i < 20; i++){
                    //    printf("%d, ", packet.data[i]);
                    //}
                    //printf("\n");
                    printf("played %d bytes.\n", audio.audIO_write((char*)play_buf, play_buf_len));
                    free(packet.data);
                }
                else {
                    vTaskDelay(20 / portTICK_PERIOD_MS);
                }
            }
        }
        //vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    free(read_buf);
    free(send_buf);
    free(play_buf);
    return ESP_OK;
}