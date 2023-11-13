/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* defining each motor pin */
#define MOTOR_OUT1_PIN 26
#define MOTOR_OUT2_PIN 27
#define MOTOR_ENA_PIN  14
#define LED_PIN 2

#define SLEEPTIME 1000

//defining a threadA initialized with a stacksize of 1024
K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
static struct k_thread threadA_data;
K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadB_data;

void threadA(void *dummy1, void *dummy2, void *dummy3){
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	
	const struct device *gpio_dev, *pwm_dev;
	
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	int ret;
	ret = gpio_pin_configure(gpio_dev, LED_PIN, GPIO_OUTPUT);
	if (gpio_dev == NULL){
		printk("Could not bind\n");
		return;
	}
	pwm_dev = DEVICE_DT_GET(DT_NODELABEL(mcpwm0));
    	if (pwm_dev == NULL) {
		printk("Error: Unable to find PWM device\n");
		return;
    	}


        gpio_pin_configure(gpio_dev, MOTOR_OUT1_PIN, GPIO_OUTPUT_HIGH);
        gpio_pin_configure(gpio_dev, MOTOR_OUT2_PIN, GPIO_OUTPUT_HIGH);
        gpio_pin_configure(gpio_dev, MOTOR_ENA_PIN, GPIO_OUTPUT_HIGH);

	pwm_pin_set_usec(pwm_dev, 0, MOTOR_ENA_PIN, 30);
        pwm_pin_set_period(pwm_dev, 0, MOTOR_ENA_PIN, 1000000);  // 1 Hz
	
	printk("thread a: started \n");
	
	while(1)
	{
		gpio_pin_set(gpio_dev, LED_PIN, 1);
		printk("ON: Moving forward \n");
		gpio_pin_set(gpio_dev, MOTOR_OUT1_PIN, 1);
    		gpio_pin_set(gpio_dev, MOTOR_OUT2_PIN, 0);
		k_msleep(SLEEPTIME);
		
		printk("ON: Moving backward \n");
		gpio_pin_set(gpio_dev, MOTOR_OUT1_PIN, 0);
    		gpio_pin_set(gpio_dev, MOTOR_OUT2_PIN, 1);
		k_msleep(SLEEPTIME);
		
		gpio_pin_set(gpio_dev, LED_PIN, 0);
		printk("OFF \n");
		gpio_pin_set(gpio_dev, MOTOR_OUT1_PIN, 0);
    		gpio_pin_set(gpio_dev, MOTOR_OUT2_PIN, 0);
		k_msleep(SLEEPTIME);
	}
	
}

void threadB(void *dummy1, void *dummy2, void *dummy3){
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);
	
	const struct device *pwm_dev;
	pwm_dev = DEVICE_DT_GET(DT_NODELABEL(mcpwm0));
	
	printk("thread b: started \n");
	
	while(1)
	{
		printk("SPEED UPDATED: 30 \n");
		pwm_pin_set_usec(pwm_dev, 0, MOTOR_ENA_PIN, 30);
		k_msleep(2000);
		printk("SPEED UPDATED: 70 \n");
		pwm_pin_set_usec(pwm_dev, 0, MOTOR_ENA_PIN, 70);
		k_msleep(2000);
	}
}

void main(void){
	k_thread_create(&threadA_data, threadA_stack_area,
			K_THREAD_STACK_SIZEOF(threadA_stack_area),
			threadA, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadA_data, "thread_a");
	
	k_thread_create(&threadB_data, threadB_stack_area,
			K_THREAD_STACK_SIZEOF(threadB_stack_area),
			threadB, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadB_data, "thread_b");
	
	k_thread_start(&threadA_data);
	k_thread_start(&threadB_data);
}
