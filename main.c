#include <xc.h>
#include "clcd.h"

static void init_config(void) {
    init_clcd();
}

void main(void) {
    init_config();
    unsigned long wait = 0;

    char str[] = "  Saran 23017A";
    unsigned int i = 0;

    while (1) {
        if (wait++ == 100000) {
            char temp = str[15];
            for (i = 15; i >= 0; i--) {
                str[i] = str[i - 1];
            }
            str[0] = temp;

            clcd_print("_Right_scrolling", LINE1(0));
            clcd_print(str, LINE2(0));
            wait = 0;
        }
    }
}