/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec step_spec0 = GPIO_DT_SPEC_GET(DT_NODELABEL(step0), gpios);
static const struct gpio_dt_spec step_spec1 = GPIO_DT_SPEC_GET(DT_NODELABEL(step1), gpios);
static const struct gpio_dt_spec step_spec2 = GPIO_DT_SPEC_GET(DT_NODELABEL(step2), gpios);
static const struct gpio_dt_spec step_spec3 = GPIO_DT_SPEC_GET(DT_NODELABEL(step3), gpios);
static const struct gpio_dt_spec step_spec4 = GPIO_DT_SPEC_GET(DT_NODELABEL(step4), gpios);
static const struct gpio_dt_spec step_spec5 = GPIO_DT_SPEC_GET(DT_NODELABEL(step5), gpios);

static const struct gpio_dt_spec dir_spec0 = GPIO_DT_SPEC_GET(DT_NODELABEL(dir0), gpios);
static const struct gpio_dt_spec dir_spec1 = GPIO_DT_SPEC_GET(DT_NODELABEL(dir1), gpios);
static const struct gpio_dt_spec dir_spec2 = GPIO_DT_SPEC_GET(DT_NODELABEL(dir2), gpios);
static const struct gpio_dt_spec dir_spec3 = GPIO_DT_SPEC_GET(DT_NODELABEL(dir3), gpios);
static const struct gpio_dt_spec dir_spec4 = GPIO_DT_SPEC_GET(DT_NODELABEL(dir4), gpios);
static const struct gpio_dt_spec dir_spec5 = GPIO_DT_SPEC_GET(DT_NODELABEL(dir5), gpios);

static const struct gpio_dt_spec lim_spec0 = GPIO_DT_SPEC_GET(DT_NODELABEL(lim0), gpios);
static const struct gpio_dt_spec lim_spec1 = GPIO_DT_SPEC_GET(DT_NODELABEL(lim1), gpios);
static const struct gpio_dt_spec lim_spec2 = GPIO_DT_SPEC_GET(DT_NODELABEL(lim2), gpios); 
static const struct gpio_dt_spec lim_spec3 = GPIO_DT_SPEC_GET(DT_NODELABEL(lim3), gpios);
static const struct gpio_dt_spec lim_spec4 = GPIO_DT_SPEC_GET(DT_NODELABEL(lim4), gpios);
static const struct gpio_dt_spec lim_spec5 = GPIO_DT_SPEC_GET(DT_NODELABEL(lim5), gpios);

// int main(void)
// {
//     if(!device_is_ready(step_spec0.port)){
//         printf("Hello Wo22r");
//         return 1;
//     }

//     int ret = gpio_pin_configure_dt(&step_spec0, GPIO_OUTPUT_ACTIVE);
//     if (ret != 0) {
//         printf("Hello Wor");
//         return 1;
//     }

// 	while (true){
// 		printf("Hello World! %s\n", CONFIG_BOARD_TARGET);
//         ret = gpio_pin_toggle_dt(&step_spec0);
//         if (ret != 0) return 1;

// 		k_msleep(1000);
// 	}

// 	return 0;
// }

int main(void)
{
    int ret;

    // Configure step_spec GPIOs
    if (!device_is_ready(step_spec0.port)) {
        printk("step_spec0 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&step_spec0, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure step_spec0\n");
        return 0;
    }

    if (!device_is_ready(step_spec1.port)) {
        printk("step_spec1 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&step_spec1, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure step_spec1\n");
        return 0;
    }

    if (!device_is_ready(step_spec2.port)) {
        printk("step_spec2 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&step_spec2, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure step_spec2\n");
        return 0;
    }

    if (!device_is_ready(step_spec3.port)) {
        printk("step_spec3 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&step_spec3, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure step_spec3\n");
        return 0;
    }

    if (!device_is_ready(step_spec4.port)) {
        printk("step_spec4 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&step_spec4, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure step_spec4\n");
        return 0;
    }

    if (!device_is_ready(step_spec5.port)) {
        printk("step_spec5 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&step_spec5, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure step_spec5\n");
        return 0;
    }

    // Configure dir_spec GPIOs
    if (!device_is_ready(dir_spec0.port)) {
        printk("dir_spec0 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&dir_spec0, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure dir_spec0\n");
        return 0;
    }

    if (!device_is_ready(dir_spec1.port)) {
        printk("dir_spec1 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&dir_spec1, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure dir_spec1\n");
        return 0;
    }

    if (!device_is_ready(dir_spec2.port)) {
        printk("dir_spec2 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&dir_spec2, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure dir_spec2\n");
        return 0;
    }

    if (!device_is_ready(dir_spec3.port)) {
        printk("dir_spec3 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&dir_spec3, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure dir_spec3\n");
        return 0;
    }

    if (!device_is_ready(dir_spec4.port)) {
        printk("dir_spec4 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&dir_spec4, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure dir_spec4\n");
        return 0;
    }

    if (!device_is_ready(dir_spec5.port)) {
        printk("dir_spec5 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&dir_spec5, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Failed to configure dir_spec5\n");
        return 0;
    }

    // Configure lim_spec GPIOs
    if (!device_is_ready(lim_spec0.port)) {
        printk("lim_spec0 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&lim_spec0, GPIO_INPUT);
    if (ret != 0) {
        printk("Failed to configure lim_spec0\n");
        return 0;
    }

    if (!device_is_ready(lim_spec1.port)) {
        printk("lim_spec1 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&lim_spec1, GPIO_INPUT);
    if (ret != 0) {
        printk("Failed to configure lim_spec1\n");
        return 0;
    }

    if (!device_is_ready(lim_spec2.port)) {
        printk("lim_spec2 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&lim_spec2, GPIO_INPUT);
    if (ret != 0) {
        printk("Failed to configure lim_spec2\n");
        return 0;
    }

    if (!device_is_ready(lim_spec3.port)) {
        printk("lim_spec3 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&lim_spec3, GPIO_INPUT);
    if (ret != 0) {
        printk("Failed to configure lim_spec3\n");
        return 0;
    }

    if (!device_is_ready(lim_spec4.port)) {
        printk("lim_spec4 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&lim_spec4, GPIO_INPUT);
    if (ret != 0) {
        printk("Failed to configure lim_spec4\n");
        return 0;
    }

    if (!device_is_ready(lim_spec5.port)) {
        printk("lim_spec5 not ready\n");
        return 0;
    }
    ret = gpio_pin_configure_dt(&lim_spec5, GPIO_INPUT);
    if (ret != 0) {
        printk("Failed to configure lim_spec5\n");
        return 0;
    }

    printk("GPIO configuration completed successfully!\n");
}