#ifndef __GPIO_H__
#define __GPIO_H__

int gpio_set_dir(int dir);
int gpio_read_bit(int idx, int *value);
int gpio_write_bit(int idx, int value);
int gpio_read_byte(int idx, int *value);
int gpio_write_byte(int idx, int value);
int gpio_read_int(int *value);
int gpio_write_int(int value);
int gpio_enb_irq(void);
int gpio_dis_irq(void);
int gpio_reg_info(int gpio_num);
int gpio_read_input(int gpio);
void gpio_set_led(int gpio, int on, int off, int blinks, int rests, int times);

#endif


