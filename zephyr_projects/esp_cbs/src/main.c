
// extern "C"{
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
// void main(void);
// }

// #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>

 //INPUT-TYPE_PANEL-INDEX_INPUT-INDEX NAH
// #define BUTTON_0_0_PIN 13
// #define BUTTON_0_1_PIN 12
// #define BUTTON_0_2_PIN


#define DIG_PIN_0 13
#define DIG_PIN_1 12
#define DIG_PIN_2 14
#define DIG_PIN_3 27
#define DIG_PIN_4 26

#define TX_BUFF_MAX_SIZE 255

#define NUM_BUTTONS 2


#define BUTTON_GPIO_FLAGS GPIO_INPUT | GPIO_PULL_UP


// void tx(std::string out_msg, const struct device uart){
//   size_t len = out_msg.length();
//         const char* data = out_msg.c_str();
//     for(size_t i = 0; i < len; i++){
//         i+= uart_fifo_fill(uart, (const uint8_t *)(data + i), len - i);
//     }

// }


//* $panel_desc(size, f f f f f f)

int main(void)
{
    // while (1) {
    //     printk("MEOW MEOW MEOW! %s\n", CONFIG_ARCH);
    //     k_sleep(K_MSEC(100));
    // }

	const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	// const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    char* msg = "Hello World!\n";
    int ret = gpio_pin_configure(gpio_dev, DIG_PIN_0, BUTTON_GPIO_FLAGS);
    ret = gpio_pin_configure(gpio_dev, DIG_PIN_1, BUTTON_GPIO_FLAGS);

    if (ret < 0) {
        printk("Error %d: Failed to configure button GPIO pin.\n", ret);
        return 0;
    }

    if (!device_is_ready(uart_dev)) {
        return 0;
    }


    while (1) {
		// if()
    //* Simple superloop arch.

    //* Refresh button states
	int button_0_state = gpio_pin_get(gpio_dev, DIG_PIN_0);
	int button_1_state = gpio_pin_get(gpio_dev, DIG_PIN_1);


    char tx_buffer[TX_BUFF_MAX_SIZE];
    char id_msg[10] = "$PANEL_0(\0";
    sprintf(tx_buffer, "%s%i,%i,%i)\n\0", id_msg, NUM_BUTTONS, button_0_state, button_1_state);
        
        // for (int i = 0; i < sizeof(msg) - 1; i++) {
        for (int i = 0; tx_buffer[i] != '\0'; i++){
            uart_poll_out(uart_dev, tx_buffer[i]);
        }
        


      

        k_sleep(K_MSEC(100));  // Adjust the delay as needed
    }
}



// to read data

/*

int ret = uart_poll_in(uart_dev, &rx_data);



*/