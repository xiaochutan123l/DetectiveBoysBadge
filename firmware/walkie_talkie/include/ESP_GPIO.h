#include "driver/gpio.h"

#define ESP_INTR_FLAG_DEFAULT 0
// add new enum elements/numbers if more options should be set.

enum class Pin_mode {
    INPUT = GPIO_MODE_INPUT,
    OUTPUT = GPIO_MODE_OUTPUT,
};

enum class Pull_mode {
    PULL_UP = GPIO_PULLUP_ONLY,
    PULL_DOWN = GPIO_PULLDOWN_ONLY,
};

enum class Trigger_mode {
    DISABLE = GPIO_INTR_DISABLE,
    RISING = GPIO_INTR_POSEDGE,
    FALLING = GPIO_INTR_NEGEDGE,
};

class ESP_GPIO {
public:
// TODO: add option default output voltage level.
    // GPIO_NUM_X
    ESP_GPIO(gpio_num_t pin_num, Pin_mode pin_mode, Pull_mode pull_mode);

    // Attention: init() method should be invoked after all the gpios objects are created 
    // and all their constructors are finished! (Because of bit mask.)
    
    void set_isr(gpio_isr_t isr_handler, Trigger_mode trigger_mode);
    /* need to further adapt the situation if a single gpio requires both input and output */
    // output mode only.
    void writeLevel(uint32_t level);
    // input mode only.
    int readLevel();
    void reset();
    ~ESP_GPIO();

private:
    void init();

    gpio_num_t m_pin_num;
    Pin_mode m_pin_mode;
    Pull_mode m_pull_mode;

};