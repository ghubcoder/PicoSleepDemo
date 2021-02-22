#include "pico/sleep.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"



static bool awake;

static void sleep_callback(void) {
    printf("RTC woke us up\n");
    uart_default_tx_wait_blocking();
    awake = true;
    return;
}

static void rtc_sleep(int8_t minute_to_sleep_to, int8_t second_to_sleep_to) {

    datetime_t t_alarm = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = minute_to_sleep_to,
            .sec   = second_to_sleep_to
    };

    printf("Going to sleep.......\n");
    uart_default_tx_wait_blocking();

    sleep_goto_sleep_until(&t_alarm, &sleep_callback);
}


void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig){

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    stdio_init_all();

    return;
}

void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);
    uart_default_tx_wait_blocking();
    // Can't measure clk_ref / xosc as it is the ref
}

int main() {
    const uint LED_PIN = 25;
    stdio_init_all();
    printf("Starting\n");

    //save values for later
    uint scb_orig = scb_hw->scr;
    uint clock0_orig = clocks_hw->sleep_en0;
    uint clock1_orig = clocks_hw->sleep_en1;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    //crudely reset the clock each time
    //to the value below
    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 00
    };

    // Start the Real time clock
    rtc_init();

    measure_freqs();
    uart_default_tx_wait_blocking();
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(2000);

        awake = false;
        printf("Sleep from xosc\n");
        uart_default_tx_wait_blocking();

        sleep_run_from_xosc();
        //Reset real time clock to a value
        rtc_set_datetime(&t);
        //sleep here, in this case for 1 min
        rtc_sleep(46,0);

        //will return here and awake should be true
        while (!awake) {
            //This should not fire
            printf("Should be sleeping\n");
            uart_default_tx_wait_blocking();
        }

        //reset processor and clocks back to defaults
        recover_from_sleep(scb_orig, clock0_orig, clock1_orig);

        //clocks should be restored
        measure_freqs();

        printf("switch off led\n");
        uart_default_tx_wait_blocking();
        gpio_put(LED_PIN, 0);

        //prove we can use sleep_ms once more without going into deep sleep
        //(this will not return unless recover_from_sleep() is ran as above)
        printf("Sleep from sleep_ms\n");
        uart_default_tx_wait_blocking();
        sleep_ms(2000);
    }
}
