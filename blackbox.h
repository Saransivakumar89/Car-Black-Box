#ifndef BLACKBOX_H
#define	BLACKBOX_H
#include "timer0.h"
#include "uart.h"
#define DEFAULT 0
#define LOGIN   1
#define MAINMENU 2
#define VIEWLOG 3
#define SET_TIME 4
#define DOWNLOAD_LOG 5
#define CLEAR_LOG 6
#define CHANGE_PASS 7

 #define CLEAR_LINE1 "                 ";
 #define CLEAR_LINE2 "                 ";
#define MAX_ATTEMPTS 3
#define TIMEOUT_SECONDS 60

void default_screen(unsigned char key);
void event(unsigned char key);
void speed_display(char *speed);
void login(unsigned char key);
void blink(int pos);
void menu(unsigned char key);
void clear_screen(void);
void delay_(void);
void view_log(void);
void get_time(void);
void event_capture(void);
void car_log(void);
void clear_log(void);
void download_log(void);
void timeout(void);
void change_pass(void);

#endif	

