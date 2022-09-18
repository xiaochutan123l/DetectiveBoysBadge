#pragma once

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "espnow_example.h"

#include <functional>
#include <deque>

typedef std::function<void(example_espnow_event_recv_cb_t& packet)> ESPNowPacketHandlerFunction;
//typedef std::function<void(void * arg, example_espnow_event_recv_cb_t& packet)> ESPNowHandlerFunctionWithArg;

class ESPNow {
public:
    //ESPNow();
    void init();
    void sendTo(uint8_t *peer_addr, uint8_t *data, size_t len);
    void sendToAll(uint8_t *data, size_t len);
    void broadcast(uint8_t *data, size_t data_len);
    bool isSendAvalaible();

    void send_packet();
    void send();

    void start();
    // peer operation
    void del_peer(const uint8_t *peer_addr);
    void add_peer(esp_now_peer_info_t *peer);
    void get_peer(const uint8_t *peer_addr, esp_now_peer_info_t *peer);
    bool is_peer_exist(const uint8_t *peer_addr);
    int get_peer_num();
    void deinit();
    void registerCallback(ESPNowPacketHandlerFunction cb);    
    
    // receve packet.
    bool recv_packet(example_espnow_event_recv_cb_t& packet);
    //int get_recv_queue_size(){return recv_deque_size;}
    //void set_recv_queue_size(int size){recv_deque_size = size;}

    // push botton.
    void set_botton_status(bool status);
    bool get_botton_status();

    // play available
    bool play_available();
private:
    void task();
    static void task_impl(void* _this);

    void handle_recv_packet(example_espnow_event_recv_cb_t& packet);

    bool sendAvalaible;
    ESPNowPacketHandlerFunction _handler = nullptr;

    // receive buffer.
    //std::deque<example_espnow_event_recv_cb_t> recv_deque;
    //int recv_deque_size = 20; // default 10.
    //int play_available_size = 10;
};