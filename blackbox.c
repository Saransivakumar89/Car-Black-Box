#include <xc.h>
#include "matrix_keypad.h"
#include "clcd.h"
#include "blackbox.h"
#include "adc.h"
#include "i2c.h"
#include "ds1307.h"
#include "ext_eeprom.h"
#include<string.h>

#if 1
extern unsigned char key;
char* arr[15] = {"ON", "GN", "G1", "G2", "G3", "G4", "G5", "GR", "C ","VL","ST","DL","CL","CP"};
static unsigned int i = 0;
static unsigned int flag = 0;
static unsigned int cr = 0;
unsigned short adc_reg_val;
char speed[3] = {};
char password[5] = {};
char pass[5] = {};

unsigned int wait, enter, c_wait;
unsigned long delay;
int screen_flag;
static int idx = 0;
static int cursor = 0;
static int attempts = 0;
int pos = 6;
static int x = 5;
int count = 0, mark = 0, star = 0, m_count = 0;
char *main_menu[6] = {"View Log        ", "Set Time    ", "Download Log", "Clear Log   ", "Change Pass "};
static int menu_pos = 0, arrow_pos = 1;
extern int pre_key;
unsigned int press_delay;
int scrn_delay;
int time_delay;

#endif

void get_pass(void) {
    for (int i = 0; i < 5; i++) {
        pass[i] = read_external_eeprom(i);
    }
}

void clr_pass(void) {
    password[0] = password[1] = password[2] = password[3] = 0;
}

unsigned char time[9] = {"10:00:00"};
unsigned char clock_reg[4] = {};
extern int seconds;

void timeout(void) {

    GIE = PEIE = TMR0IE = 1;
    clcd_print("  Please Try   ", LINE1(0));
    clcd_print("Again in ", LINE2(0));
    clcd_putch(seconds / 10 + 48, LINE2(9));
    clcd_putch(seconds % 10 + 48, LINE2(10));
    clcd_print(" sec", LINE2(11));
    //    while(1){;}
    if (seconds == 0) {
        GIE = PEIE = TMR0IE = 0;
        CLEAR_DISP_SCREEN;
        screen_flag = DEFAULT;
        seconds = 10;
        attempts = 0;
        idx = 0;
        clr_pass();
        enter = 0;
    }



}

void get_time(void) {
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    if (clock_reg[0] & 0x40) {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    } else {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);
    time[8] = '\0';
}

int position = -1;
int overflow = 0;
char log[17] = {};
int index;

void event_capture() {
    char address;
    if (++position == 10) {
        position = 0;
        overflow = 1;
    }
    address = (position * 16) + 5;
    for (char i = 0; i < 16; i++) {
        write_external_eeprom(address + i, log[i]);
    }

    if (index < 9) {
        index++;
    }
}

void car_log() {

    for (int i = 0; i < 8; i++) {
        log[i] = time[i];
    }
    log[8] = ' ';
    log[9] = ' ';
    log[10] = arr[i][0];
    log[11] = arr[i][1];
    log[12] = ' ';
    log[13] = ' ';
    log[14] = speed[0];
    log[15] = speed[1];
    log[16] = '\0';


    event_capture();
}

void view_log() {
    char *vlog[17] = {};
    static int shift_pos = 0, vlog_pos;
    char address;

    vlog_pos = (overflow) ? ((pos + 1) % 10) : 0;
    if (key != ALL_RELEASED) {
        press_delay++;
        pre_key = key;
        if (press_delay == 400) {
            press_delay = 0;
            if (key == MK_SW6) {

                CLEAR_DISP_SCREEN;
                screen_flag = MAINMENU;
            }
        }
    } else if (press_delay > 0 && press_delay < 400) {
        press_delay = 0;
        clcd_print("#   TIME   EV SP", LINE1(0));
        if (pre_key == MK_SW6 && shift_pos < index) {
            if (shift_pos < 10)
                shift_pos++;
            clcd_print("                ", LINE2(0));
        } else if (pre_key == MK_SW5 && shift_pos > 0) {
            if (shift_pos > -1)
                shift_pos--;
            clcd_print("                ", LINE2(0));
        }
    } else {
        press_delay = 0;
    }
    vlog_pos = (vlog_pos + shift_pos) % 10;
    for (int i = 0; i < 16; i++) {
        address = vlog_pos * 16 + 5 + i;
        vlog[i] = read_external_eeprom(address);
    }

    clcd_putch(shift_pos + '0', LINE2(0));
    clcd_putch(vlog[0], LINE2(2));
    clcd_putch(vlog[1], LINE2(3));
    clcd_putch(vlog[2], LINE2(4));
    clcd_putch(vlog[3], LINE2(5));
    clcd_putch(vlog[4], LINE2(6));
    clcd_putch(vlog[5], LINE2(7));
    clcd_putch(vlog[6], LINE2(8));
    clcd_putch(vlog[7], LINE2(9));
    clcd_putch(vlog[8], LINE2(10));
    clcd_putch(vlog[10], LINE2(11));
    clcd_putch(vlog[11], LINE2(12));
    clcd_putch(vlog[12], LINE2(13));
    clcd_putch(vlog[14], LINE2(14));
    clcd_putch(vlog[15], LINE2(15));

}

void default_screen(unsigned char key) {
    speed_display(speed);
    get_time();
    clcd_print("  TIME    EV  SP", LINE1(0));
    clcd_print(time, LINE2(0));
    clcd_print(speed, LINE2(14));
    event(key);

}

void event(unsigned char key) {
    if (flag == 0) {
        clcd_print(arr[0], LINE2(10));
        flag = 1;
        car_log();
    }

    if (key == MK_SW2 && flag == 1) {
        if (i >= 0 && i < 7) {
            cr = 0;
            ++i;
        }
        car_log();
    }
    if (key == MK_SW3 && flag == 1) {
        if (i > 1) {
            --i;
            cr = 0;
        }
        car_log();
    }
    if (key == MK_SW1) {
        clcd_print(arr[8], LINE2(10));
        i = 8;
        cr = 1;
        car_log();
    }
    if (key == MK_SW3 && cr == 1) {
        ++i;
        cr = 0;
        car_log();
    }
    clcd_print(arr[i], LINE2(10));

    if (i == 8 && (key == MK_SW2 || key == MK_SW3)) {
        i = 1;
    }
}

void speed_display(char * speed) {
    adc_reg_val = read_adc(CHANNEL4);
    adc_reg_val = (adc_reg_val / 10.33);
    speed[0] = (char) (adc_reg_val / 10 % 10) + 48;
    speed[1] = (char) (adc_reg_val % 10) + 48;
}

void blink(int x) {
    if (c_wait++ == 10000) {
        if (cursor == 0) {
            clcd_putch('_', LINE2(x));
        } else {
            clcd_putch(' ', LINE2(x));
        }
        cursor = !cursor;
        c_wait = 0;
    }
}

void login(unsigned char key) {
    if (enter == 0) {
        CLEAR_DISP_SCREEN;
        enter = 1;
    }
    count = 0 ;
    if (wait++ == 1500) {
        clcd_print(" Enter Password ", LINE1(0));
        wait = 0;
    }

    blink(x);


    if (idx < 4) {

        if (key == ALL_RELEASED && delay++ == 100000) {
            CLEAR_DISP_SCREEN;
            screen_flag = DEFAULT;
            enter = 0;
            pos = 6;
            cursor = 0;
            idx = 0;
            star = 0;
            count = 0;
            mark = 0;
            x = 5;
        }
        if ((key == MK_SW5 || key == MK_SW6)) {
            count++;
            star++;

            if (key == MK_SW5) {
                if (count > 1)
                    password[idx++] = '0';
            }
            if (key == MK_SW6)
                password[idx++] = '1';

            if (star > 1 && star < 6) {
                clcd_putch('*', LINE2(pos));

                pos++;
            }
            x++;
            if (star == 5) {
                star = 1;
                pos = 6;
                x = 6;
            }
            delay = 0;
        }
    } else if (idx == 4) {
        get_pass();
        if (strcmp(password, pass) == 0) {
            for (unsigned long int i = 900000; i--;);
            CLEAR_DISP_SCREEN;
            clcd_print("  Login ", LINE1(3));
            clcd_print("Successfull...", LINE2(1));
            for (unsigned long int i = 500000; i--;);
            CLEAR_DISP_SCREEN;
            attempts = 0;
            idx = 0;
            clr_pass();
            pre_key = 1;
            screen_flag = MAINMENU;
        } else {

            attempts++;
            if (attempts < MAX_ATTEMPTS) {
                for (unsigned long int i = 900000; i--;);
                CLEAR_DISP_SCREEN;
                clcd_print("Incorrect  ", LINE1(3));
                clcd_print("Attempts Left:", LINE2(0));
                clcd_putch('0' + 3 - (attempts), LINE2(15));
                for (unsigned long int i = 900000; i--;);
                idx = 0;
                clr_pass();
                enter = 0;


            } else {

                //                for (unsigned long int i = 900000; i--;);
                //                CLEAR_DISP_SCREEN;
                //                clcd_print("Max Attempts", LINE1(1));
                //                clcd_print(" Reached", LINE2(3));

                //                for (unsigned long int i = 900000; i--;);


                // <Timeout>
                timeout();
                //                CLEAR_DISP_SCREEN;
                // enter = 0;
                // screen_flag = DEFAULT;
            }
        }
    }
}

void menu(unsigned char key) {
    if (arrow_pos == 1) {
        clcd_print("->", LINE1(0));
        clcd_print(main_menu[menu_pos], LINE1(3));
        clcd_print(main_menu[menu_pos + 1], LINE2(3));
    } else {
        clcd_print("->", LINE2(0));
        clcd_print(main_menu[menu_pos - 1], LINE1(3));
        clcd_print(main_menu[menu_pos], LINE2(3));
    }

    if (key == MK_SW5 || key == MK_SW6) {
        delay = 0;
    }

    if (key != ALL_RELEASED) {
        press_delay++;
        pre_key = key;


        if (press_delay == 1000) {

            if (key == MK_SW5) {
                press_delay = 0;
                clcd_putch(menu_pos % 10 + 48, LINE1(15));

                if (menu_pos == 0) {
                    //for (unsigned long int i = 5000; i--;);
                    //   CLEAR_DISP_SCREEN;
                    screen_flag = VIEWLOG;

                }
                if (menu_pos == 1) {
                    //  CLEAR_DISP_SCREEN;
                    screen_flag = SET_TIME;
                }
                if (menu_pos == 2) {
                    //     CLEAR_DISP_SCREEN;
                    screen_flag = DOWNLOAD_LOG;
                }
                if (menu_pos == 3) {
                    screen_flag = CLEAR_LOG;
                }
                if (menu_pos == 4) {
                    screen_flag = CHANGE_PASS;
                }
            } else if (key == MK_SW6) {
                screen_flag = DEFAULT;
                enter = 0;
                pre_key = 0;
                press_delay = 0;
                count = 0 ;
                CLEAR_DISP_SCREEN;
            }

        }
    } else if (press_delay > 0 && press_delay < 1000) {
        press_delay = 0;
        m_count++;
        if (pre_key == MK_SW6 && menu_pos < 4 && m_count > 1) {
            menu_pos++;
            if (arrow_pos < 2)
                arrow_pos++;
            CLEAR_DISP_SCREEN;
        } else if (pre_key == MK_SW5 && menu_pos > 0) {
            menu_pos--;
            if (arrow_pos > 1)
                arrow_pos--;
            CLEAR_DISP_SCREEN;
        }
    } else {
        press_delay = 0;
    }



    if (key == ALL_RELEASED && delay++ == 300000) {
        CLEAR_DISP_SCREEN;
        screen_flag = DEFAULT;
        menu_pos = 0;
        m_count = 0;
        arrow_pos = 1;

        enter = 0;
        pos = 6;
        cursor = 0;
        idx = 0;
        star = 0;
        count = 0;
        mark = 0;
        x = 5;
        pre_key = 0;
    }

}

/*void set_time(unsigned char key) {

}*/

int once = 0;

void download_log(void) {
    init_uart();

    clcd_print("  Downloaded  ", LINE1(0));
    clcd_print(" Successfully...", LINE2(0));
    GIE = PEIE = TMR2IE = 0;

    car_log();
    unsigned char address;
    char vlog[17];
    int vlog_pos;
    int next = -1;
    clcd_putch(once + '0', LINE2(0));
    if (once == 0) {
        puts("*----------Downloading --------*\n\r");
        puts("# TIME     EVENT SPEED\n\r");
        while (next < index) {
            next++;
            vlog_pos = (overflow) ? ((pos + 1) % 10) : 0;
            vlog_pos = (vlog_pos + next) % 10;
            for (int i = 0; i < 16; i++) {
                address = vlog_pos * 16 + 5 + i;
                vlog[i] = read_external_eeprom(address);
            }
            putch(next + '0');
            puts(" ");
            putch(vlog[0]);
            putch(vlog[1]);
            putch(vlog[2]);
            putch(vlog[3]);
            putch(vlog[4]);
            putch(vlog[5]);
            putch(vlog[6]);
            putch(vlog[7]);
            putch(vlog[8]);
            putch(vlog[9]);
            putch(vlog[10]);
            putch(vlog[11]);
            putch(vlog[12]);
            putch(vlog[13]);
            putch(vlog[14]);
            putch(vlog[15]);
            putch('\n');
            putch('\r');
        }
        once = 1;
    }
    if (wait++ == 1000) {
        CLEAR_DISP_SCREEN;
        screen_flag = DEFAULT;
        pre_key = 0;
        wait = 0;

    }

}

void clear_log() {
    CLEAR_DISP_SCREEN;
    index = 0;
    overflow = 0;
    for (int i = 0; i < 17; i++) {
        log[i] = '\0';
    }
    clcd_print("   Logs Cleared   ", LINE1(0));
    clcd_print("<<.......>>", LINE2(4));
    for (unsigned long int i = 3000000; i--;);
    screen_flag = MAINMENU;

}

/*
static int cdx;
int cp_count = 0, cp_mark = 0, cp_star = 0;
int cp_pos = 6;
static int cp_cursor = 0;
char *pwd[5] = {};
int cp_enter = 0, cp_wait;
int c_x = 5;

void cp_blink(int c_x) {

    if (c_wait++ == 10000) {
        if (cp_cursor == 0) {
            clcd_putch('_', LINE2(c_x));
        } else {
            clcd_putch(' ', LINE2(c_x));
        }
        cp_cursor = !cp_cursor;
        c_wait = 0;
    }
}

void change_pass(void) {

    if (cp_enter == 0) {
        CLEAR_DISP_SCREEN;
        cp_enter = 1;
    }
    if (cp_wait++ == 1500) {

        clcd_print("   Enter Pwd ", LINE1(0));
        cp_wait = 0;

    }
    cp_blink(c_x);

    if ((key == MK_SW5 || key == MK_SW6)) {
        cp_count++;
        cp_star++;
        if (key == MK_SW5) {
            if (cp_count > 1)
                pwd[cdx++] = '0';
        }
        if (key == MK_SW6)
            pwd[cdx++] = '1';

        if (cp_star > 1 && cp_star < 6) {
            clcd_putch('*', LINE2(pos));
            cp_pos++;
        }
        if (cp_star == 5) {
            cp_star = 1;
            cp_pos = 6;
        }
        delay = 0;
    }
    if (cdx == 4) {
        pwd[4] = '\0';
        get_pass();
        if (strcmp(pass, pwd) == 0) {

            for (unsigned long int i = 900000; i--;);
            CLEAR_DISP_SCREEN;
            clcd_print(" Correct ", LINE1(4));
            clcd_print("Password ", LINE2(4));
            for (unsigned long int i = 500000; i--;);
            CLEAR_DISP_SCREEN;


        }
    }
}*/
char old_pd[5]={};
char new_pd[5]={};
int return_delay, return_delay1,k,l;
int sec;
void change_pass(void) {
    static int attempt1;
    static char c = 0;

        k = 0;
        attempt1 = 3;
        return_delay1 = 0;

        l = 0;
        key = ALL_RELEASED;
       i = 12;
        car_log();
        CLEAR_DISP_SCREEN;
        clcd_print(" Enter pwd ", LINE1(0));
      //  clcd_putch(' ', LINE2(5));
        c = 0;
    
    if (return_delay1++ == 50000) { 
        TMR2ON = 0;
        
       pre_key = 1;
        screen_flag = MAINMENU;

        CLEAR_LINE1;
        CLEAR_LINE2;
        GIE = PEIE = TMR2IE = 0;
        return_delay1 = 0;
    }
    if (c == 0) {
        clcd_print("               ", LINE2(0));
        c = 1;
    }
    if (l < 4) {
        if (key == MK_SW5 || key == MK_SW6) { // To enter old password
            clcd_putch('*', LINE2(6 + l));
           return_delay1 = 0;
            if (key == MK_SW5) {
                old_pd[l++] = '1';
            } else if (key == MK_SW6) {
                old_pd[l++] = '0';
            }
        }
    } else if (l == 4) {
        attempt1--;
        old_pd[4] = '\0';
        TMR2ON = 1;
        GIE = PEIE = TMR2IE = 1;
  
        if (strcmp(old_pd, pass) == 0) {
            if (k == 4) {
                CLEAR_LINE2;
            }
            if (k < 4) {
                clcd_print("Enter new password", LINE1(0));
                clcd_putch(' ', LINE2(5));
                if (key == MK_SW5 || key == MK_SW6) {
                    clcd_putch('*', LINE2(6 + k));
                    return_delay1 = 0;
                    if (key == MK_SW5) {
                        new_pd[k++] = '0';
                    } else if (key == MK_SW6) {
                        new_pd[k++] = '1';
                    }
                }
            } else if (k > 3 && k < 8) {
                clcd_print("Re_enter  pwd", LINE1(0));
                clcd_putch(' ', LINE2(0));
                if (key == MK_SW5 || key == MK_SW6) {
                    clcd_putch('*', LINE2(6 + (k % 4)));
                    return_delay1 = 0;
                    if (key == MK_SW5) {
                        new_pd[k++] = '1';
                    } else if (key == MK_SW6) {
                        new_pd[k++] = '0';
                    }
                }
            }
            if (c == 1) {
                clcd_print("               ", LINE2(0));
                c = 2;
            }
            if (k == 8) {
                TMR2ON = 0;
                GIE = PEIE = TMR2IE = 0;
                new_pd[8] = '\0';
                if (strncmp(new_pd, &new_pd[4], 4) == 0) {
                    c = 0;
                    write_external_eeprom(0x00, &new_pd[4]);
                    get_pass();
                    CLEAR_DISP_SCREEN;
                    clcd_print("Password changed", LINE1(0));
                    clcd_print("Successfully", LINE2(2));
                    for (unsigned long long int i = 300000; i--;);
                    CLEAR_DISP_SCREEN;
                    for (unsigned int i = 100; i--;);
                    screen_flag = MAINMENU;
                 
                 
                    pre_key = 1;

                } else {
                    c = 0;
                    CLEAR_DISP_SCREEN;
                    clcd_print("Password  changed", LINE1(0));
                    clcd_print("failed", LINE2(5));
                    for (unsigned long long int i = 300000; i--;);
                    CLEAR_DISP_SCREEN;
                    for (unsigned int i = 100; i--;);
                    screen_flag = MAINMENU;
   
                    pre_key = 1;
                    CLEAR_LINE1;
                    CLEAR_LINE2;
                }
            }
        } else {
            
            CLEAR_DISP_SCREEN;
            if (attempt1 == 0) {
                c = 0;
                screen_flag = MAINMENU;

                CLEAR_LINE1;
                CLEAR_LINE2;

                 pre_key = 1;   
                attempt1 = 3;
                TMR2ON = 0;
                GIE = PEIE = TMR2IE = 0;
            } else {
                clcd_print(" WRONG PASSWORD ", LINE1(0));
                clcd_putch(attempt1 + '0', LINE2(0));
                clcd_print("attempt left", LINE2(2));
                sec = 5;
                while (sec);
            }
            CLEAR_DISP_SCREEN;
            clcd_print(" Enter old password ", LINE1(0));
            l = 0;
            return_delay = 0;
        }
    }
}















