#include <xc.h>
#include "clcd.h"
#include "matrix_keypad.h"
#include "blackbox.h"
#include "adc.h"
#include "i2c.h"
#include "ds1307.h"
#include "ext_eeprom.h"
#include "uart.h"



unsigned char key;
extern int screen_flag = DEFAULT;
int pre_key;

static void init_config(void) {
    init_clcd();
    init_matrix_keypad();
    init_adc();

    init_i2c();
    init_ds1307();

    init_timer0();

    write_external_eeprom(0x00, '1');
    write_external_eeprom(0x01, '1');
    write_external_eeprom(0x02, '1');
    write_external_eeprom(0x03, '1');
    write_external_eeprom(0x04, '\0');
}

void main(void) {

    init_config();

    while (1) {
        if (!pre_key) {
            key = read_switches(STATE_CHANGE);
        } else {
            key = read_switches(LEVEL_CHANGE);
        }

        if (screen_flag == DEFAULT) {

            default_screen(key);
        }

        if (key == MK_SW5 && screen_flag == DEFAULT) {


            screen_flag = LOGIN;
        }

        if (screen_flag == LOGIN) {


            login(key);
        }

        if (screen_flag == MAINMENU) {
            menu(key);
        }

        if (screen_flag == VIEWLOG) {
            view_log();
        }
        if (screen_flag == CLEAR_LOG) {
            clear_log();
        }
        if (screen_flag == DOWNLOAD_LOG) {
            download_log();
        }
        if(screen_flag == CHANGE_PASS)
        {
            change_pass();
        }
        if(screen_flag==SET_TIME)
        {
            //
        }
    }

    return;
}

