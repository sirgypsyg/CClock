#include <8051.h>

#define FALSE 0
#define TRUE  1
#define T100 960 // pó³okres LED

#define ENTER 1
#define ESC 2
#define RIGHT 4
#define UP 8
#define DOWN 16
#define LEFT 32

unsigned char SS = 0;
unsigned char MM = 0;
unsigned char HH = 0;


__bit editMode = FALSE;
__sbit __at (0xB5) P3_5 ;
__sbit __at (0x97) DIODE;

// szablony cyfr (od 0 do 9) dla wys´wietlacza LED
// przechowywane w pamie?ci programu (code)
__code unsigned char WZOR[10] = { 0b0111111, 0b0000110,
                                  0b1011011, 0b1001111, 0b1100110, 0b1101101,
                                  0b1111101, 0b0000111, 0b1111111, 0b1101111
                                };

// bit 6 portu 1 w³a?cza/wy³a?cza wys´wietlacz LED 11:
__bit __at (0x96) SEG_OFF;

// bufor wybieraja?cy bitowo aktywny wys´wietlacz
__xdata unsigned char * led_wyb = (__xdata unsigned char *) 0xFF30;

// bufor wybieraja?cy aktywne segmenty wys´wietlacza
__xdata unsigned char * led_led = (__xdata unsigned char *) 0xFF38;

//wskaŸnik na adres 0xFF21 w obszarze xdata
__xdata unsigned char * key_addr = (__xdata unsigned char *) 0xFF21;

__bit t0_flag = 0;
unsigned int counter = 0;
unsigned char led_b = 1;
unsigned char key = 0;
unsigned char curSeg = 1;
unsigned char prev = 0;
unsigned char curr = 0;


void incHH() {
    ++HH;
    if(HH == 24) {
        HH = 0;
    }
}

void incMM() {
    ++MM;
    if(MM == 60) {
        MM = 0;
        incHH();
    }
}
void incSS() {
    ++SS;
    if(SS == 60) {
        SS = 0;
        incMM();
    }
}
void decHH() {
    --HH;
    if(HH == 00) {
        HH = 11;
    }
}
void decMM() {
    --MM;
    if(MM == 0) {
        MM = 59;
        decHH();
    }
}
void decSS() {
    --SS;
    if(SS == 0) {
        SS = 59;
        decMM();
    }
}
void timer_int(void) __interrupt(1) {
    TH0 = 226;
    t0_flag = 1;
}

void handleUp() {
    switch(curSeg) {
    case 1:
        ++SS;
        if(SS == 60) {
            SS = 0;
        }
        break;
    case 2:
        ++MM;
        if(MM == 60) {
            MM = 0;
        }
        break;
    case 3:
    	++HH;
        if(HH == 24) {
            HH = 0;
        }
        break;
    }
}
void handleDown() {
    switch(curSeg) {
    case 1:
        --SS;
        if(SS == 0 || SS == 255) {
            SS = 59;
        }
        break;
    case 2:
        --MM;
        if(MM == 0 || MM == 255) {
            MM = 59;
        }
        break;
    case 3:
        --HH;
        if(HH == 0 || HH == 255) {
            HH = 23;
        }
        break;
    }
}

void checkKeybord() {
    curr = prev;
        if(key == ENTER) {
            editMode = !editMode;
            DIODE = !DIODE;
            counter = 0;
        }
        else if(editMode == TRUE) {
            switch(key) {
            case UP:
                handleUp();
                break;
            case DOWN:
                handleDown();
                break;
            case RIGHT:
                if(curSeg != 1) {
                    --curSeg;
                }
                break;
            case LEFT:
            	if(curSeg != 3){
                    ++curSeg;
            	}
                break;
            }
        }
}


void t0_serv() {
    ++counter;
    if (counter == T100) {
        if(editMode == FALSE) {
            incSS();
        }
        counter = 0;
    }
}

void main() {
    IE = 0b10000010;
    TMOD = 0b01110000;
    TH0 = 226;
    TR0 = TRUE;  // uruchom licznik T0
    SEG_OFF = FALSE;
    while (TRUE) {
        if (t0_flag) {
            t0_flag = FALSE;
            SEG_OFF = TRUE;
            prev = led_b;
            *led_wyb = led_b;
            switch (led_b) {
            case 1:
            	if(editMode && curSeg == 1){
                   *led_led = WZOR[SS % 10] + 128;
             	}else{
                    *led_led = WZOR[SS % 10];
                }
                if(P3_5){
                    key = led_b;
                }
                break;
            case 2:
            	if(editMode && curSeg == 1){
                   *led_led = WZOR[SS / 10] + 128;
             	}else{
                    *led_led = WZOR[SS / 10];
              	}
                if(P3_5){
                    key =led_b;
                }
                break;
            case 4:
            	if(editMode && curSeg == 2){
                   *led_led = WZOR[MM % 10] + 128;
             	}else{
                    *led_led = WZOR[MM % 10];
       	        }
                if(P3_5){
                    key =led_b;
                }
                break;
            case 8:
            	if(editMode && curSeg == 2){
                   *led_led = WZOR[MM / 10] + 128;
             	}
             	else{
                    *led_led = WZOR[MM / 10];
              	}
                if(P3_5){
                    key =led_b;
                }
                break;
            case 16:
            	if(editMode && curSeg == 3){
                   *led_led = WZOR[HH % 10] + 128;
             	}else{
                    *led_led = WZOR[HH % 10];
                }
                if(P3_5){
                    key =led_b;
                }
                break;
            case 32:
            	if(editMode && curSeg == 3){
                   *led_led = WZOR[HH / 10] + 128;
             	}
             	else{
                    *led_led = WZOR[HH / 10];
              	}
                if(P3_5){
                    key =led_b;
                }
                break;
            }
            led_b += led_b;
            if (led_b > 32) {
                led_b = 1;
            }
            if(P3_5 && curr == 0) {
                checkKeybord();
            } else if(!P3_5 && curr == prev) {
                curr =0;
            }
            t0_serv();
            SEG_OFF = FALSE;
        }
    }
}
