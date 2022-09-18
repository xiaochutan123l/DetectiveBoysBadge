#include "ESP_GPIO.h"


ESP_GPIO::ESP_GPIO(gpio_num_t pin_num, Pin_mode pin_mode, Pull_mode pull_mode=Pull_mode::PULL_UP) {
    m_pin_num = pin_num;
    m_pin_mode = pin_mode;
    m_pull_mode = pull_mode;
    // initialize GPIO
    init();
}

void ESP_GPIO::init(){
    // init function should be only invoked once statically after all the gpio objects are created.
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt as default
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set pin mode
    switch (m_pin_mode) {
        case Pin_mode::INPUT:
            io_conf.mode = GPIO_MODE_INPUT;
            break;
        case Pin_mode::OUTPUT:
            io_conf.mode = GPIO_MODE_OUTPUT;
            break;
        default:
            io_conf.mode = GPIO_MODE_DISABLE;
            break;
    }
    // set pull mode
    switch (m_pull_mode) {
        case Pull_mode::PULL_UP:
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            break;
        case Pull_mode::PULL_DOWN:
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            break;
        default:
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            break;
    }
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL<<m_pin_num;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}


void ESP_GPIO::set_isr(gpio_isr_t isr_handler, Trigger_mode trigger_mode) {
    //change gpio interrupt type
    gpio_set_intr_type(m_pin_num, gpio_int_type_t(trigger_mode));

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(m_pin_num, isr_handler, (void*) m_pin_num);
}

void ESP_GPIO::reset() {
    gpio_reset_pin(m_pin_num);
};
void ESP_GPIO::writeLevel(uint32_t level) {
    gpio_set_level(m_pin_num, level);
}

int ESP_GPIO::readLevel() {
    return gpio_get_level(m_pin_num);
}

ESP_GPIO::~ESP_GPIO() {
    reset();
}
