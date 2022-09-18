#include "ESPNow.h"

#define ESPNOW_MAXDELAY 512

static const char *TAG = "ESPNow";

static QueueHandle_t queue;

static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len);
// record the push botton status.
static bool pushed = false;

static std::deque<example_espnow_event_recv_cb_t> recv_deque;
static int recv_deque_size = 100; // default 10.
static int play_available_size = 80;
static bool ready_play = false;
static int queue_item_count = 0;

static bool send_ok = true;

/* WiFi should start before using ESPNOW */
static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
    ESP_LOGI(TAG, "Init wifi finished");
}

static void espNow_init()
{
    ESP_LOGI(TAG, "ESPNow init begin");
    /* Initialize ESPNOW and register sending and receiving callback function. */

    //queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(example_espnow_event_t));
    //queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(example_espnow_event_recv_cb_t));
    queue = xQueueCreate(recv_deque_size, sizeof(example_espnow_event_recv_cb_t));
    if (queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        //return ESP_FAIL;
        return;
    }

    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_recv_cb) );

    #if CONFIG_ESP_WIFI_STA_DISCONNECTED_PM_ENABLE
        ESP_ERROR_CHECK( esp_now_set_wake_window(65535) );
    #endif
    /* Set primary master key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );
    ESP_LOGI(TAG, "ESPNow init finished");

    // add broadcast as the first default peer.
    esp_now_peer_info_t *peer = (esp_now_peer_info_t*)malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(queue);
        esp_now_deinit();
        return;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;  
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);
}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    /*
    ESP_LOGI(TAG, "into send callback");
    example_espnow_event_t evt;
    example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = EXAMPLE_ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    //if (xQueueSend(queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
    //    ESP_LOGW(TAG, "Send send queue fail");
    //}
    */
    ESP_LOGI(TAG, "into send callback");
    send_ok = true;
}

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    if (pushed == false) {
        
        ESP_LOGI(TAG, "into recev callback, %d-bytes", len);
        example_espnow_event_t evt;
        example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

        if (mac_addr == NULL || data == NULL || len <= 0) {
            ESP_LOGE(TAG, "Receive cb arg error");
            return;
        }
       
        evt.id = EXAMPLE_ESPNOW_RECV_CB;
        memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
        recv_cb->data = (uint8_t*)malloc(len);
        if (recv_cb->data == NULL) {
            ESP_LOGE(TAG, "Malloc receive data fail");
            return;
        }
        memcpy(recv_cb->data, data, len);
        recv_cb->data_len = len;
        
        if (xQueueSend(queue, recv_cb, ESPNOW_MAXDELAY) != pdTRUE) {
            ESP_LOGW(TAG, "Send receive queue fail");
            free(recv_cb->data);
        }
        queue_item_count++;
        //ESP_LOGI(TAG, "emplace_back");
        /*
        recv_deque.push_back(evt.info.recv_cb);
        if (recv_deque.size() >= recv_deque_size){
            printf("add one packet\n");
            recv_deque.pop_front();
            
        }
        
        
        if (recv_deque.size() < play_available_size){
            //printf("add one packet\n");
            //recv_deque.pop_front();
            recv_deque.push_back(evt.info.recv_cb);
            
        }
        */
        //free(recv_cb->data);
    }
    else{
        //printf("push botton is pushed, do nothing\n");
    }
}
/*
static void add_boradcast_peer() {
    // Add broadcast peer information to peer list. 
    esp_now_peer_info_t *peer = (esp_now_peer_info_t*)malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;  
    memcpy(peer->peer_addr, s_example_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);
}
*/

/* ---------- Class member functions------------------*/

void ESPNow::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    wifi_init();
    espNow_init();
}

void ESPNow::sendTo(uint8_t *peer_addr, uint8_t *data, size_t len)  {
    if (esp_now_send(peer_addr, data, len) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
        deinit();
    }
}

void ESPNow::sendToAll(uint8_t *data, size_t len)  {
    // dest_mac -> null -> Send to all peers.
    if (esp_now_send(nullptr, data, len) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
        deinit();
    }
}

void my_delay_us(size_t time) {
    size_t start_time = esp_timer_get_time();
    while (true){
        if (esp_timer_get_time() - start_time >= time){
            break;
        }
    }
}

void ESPNow::broadcast(uint8_t *data, size_t len) {
    //if (esp_now_send(broadcast_mac, data, len) != ESP_OK) {
    //    ESP_LOGE(TAG, "Send error");
    //    deinit();
    //}
    //my_delay_us(30000);
    esp_err_t error = esp_now_send(broadcast_mac, data, len);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Error num: %d\n", error);
        ESP_LOGE(TAG, "Send error");
        deinit();
    }
}

bool ESPNow::isSendAvalaible() {
    return sendAvalaible;
}

bool ESPNow::play_available() {
    return recv_deque.size() >= play_available_size;
}

void ESPNow::task() {
    example_espnow_event_t evt;
    while (xQueueReceive(queue, &evt, portMAX_DELAY) == pdTRUE) {
        switch (evt.id) {
            case EXAMPLE_ESPNOW_SEND_CB:
            {
                sendAvalaible = evt.info.send_cb.status;
                break;
            }
            case EXAMPLE_ESPNOW_RECV_CB:
            {
                if (_handler != nullptr){
                _handler(evt.info.recv_cb);
                }
                else {
                    handle_recv_packet(evt.info.recv_cb);
                }
                break;
            }
            default:
                ESP_LOGE(TAG, "Callback type error: %d", evt.id);
                break;
        }
    }
}

/*
void ESPNow::task_impl(void* _this) {
    static_cast<ESPNow*>(_this)->task();
}
*/

void ESPNow::start() {
    /* Start */
    //xTaskCreate(this->task_impl, "example_espnow_task", 2048, this, 4, NULL);
    //xTaskCreate(this->task_impl, "example_espnow_task", 2048, this, 3, NULL);
    //task();
}

void ESPNow::set_botton_status(bool status) {
    if (pushed != status) {
        pushed = status;
    }
}

bool ESPNow::get_botton_status() {
    return pushed;
}

void ESPNow::handle_recv_packet(example_espnow_event_recv_cb_t& packet) {
    //TODO maybe change to a ring buffer if deque consums too much resources.
    recv_deque.emplace_back(packet);
    if (recv_deque.size() > recv_deque_size){
        recv_deque.pop_front();
    }
}

bool ESPNow::recv_packet(example_espnow_event_recv_cb_t& packet) {
    /*
    printf("recv queue size: %d\n", recv_deque.size());
    //if (recv_deque.empty()){
    if (recv_deque.size()< play_available_size && ready_play == false){
        //printf("recv_packet0\n");
        packet = nullptr;
        return false;
    }
    else{
        ready_play = true;
        //printf("recv_packet2\n");
        packet->data = recv_deque.front().data;
        packet->data_len = recv_deque.front().data_len;
        //printf("recv_packet3\n");
        recv_deque.pop_front();
        //printf("recv_packet4\n");
        return true;
    }
    //example_espnow_event_t evt;
    */
    /*
    //if (xQueueReceive(queue, &packet, portMAX_DELAY) == pdTRUE) {
    if (recv_count < play_available_size && ready_play == false){
        //printf("recv_packet0\n");
        //packet = nullptr;
        return false;
    }
    else{
        ready_play = true;
        if (xQueueReceive(queue, &packet, portMAX_DELAY) == pdTRUE) {
            //handle_recv_packet(evt.info.recv_cb);
            return true;
        }
        else {
            return false;
        }
    }*/
    if (queue_item_count > 0){
        if (xQueueReceive(queue, &packet, portMAX_DELAY) == pdTRUE) {
            queue_item_count--;
            //handle_recv_packet(evt.info.recv_cb);
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

void ESPNow::del_peer(const uint8_t *peer_addr) {
    esp_now_del_peer(peer_addr);
}

void ESPNow::add_peer(esp_now_peer_info_t *peer) {
    esp_now_add_peer(peer);
}

void ESPNow::get_peer(const uint8_t *peer_addr, esp_now_peer_info_t *peer) {
    esp_now_get_peer(peer_addr, peer);
}

bool ESPNow::is_peer_exist(const uint8_t *peer_addr) {
    return esp_now_is_peer_exist(peer_addr);
}

int ESPNow::get_peer_num() {
    esp_now_peer_num_t *num = 0;
    esp_now_get_peer_num(num);
    return num->total_num;
}

void ESPNow::deinit()
{
    vSemaphoreDelete(queue);
    esp_now_deinit();
}

/*
void AsyncUDP::onPacket(ESPNowPacketHandlerFunctionWithArg cb, void * arg)
{
    onPacket(std::bind(cb, arg, std::placeholders::_1));
}
*/
void ESPNow::registerCallback(ESPNowPacketHandlerFunction cb)
{
    _handler = cb;
}

