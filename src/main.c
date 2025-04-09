sbit LCD_RS at RD5_bit;
sbit LCD_EN at RD4_bit;
sbit LCD_D4 at RD3_bit;
sbit LCD_D5 at RD2_bit;
sbit LCD_D6 at RD1_bit;
sbit LCD_D7 at RD0_bit;

sbit LCD_RS_Direction at TRISD5_bit;
sbit LCD_EN_Direction at TRISD4_bit;
sbit LCD_D4_Direction at TRISD3_bit;
sbit LCD_D5_Direction at TRISD2_bit;
sbit LCD_D6_Direction at TRISD1_bit;
sbit LCD_D7_Direction at TRISD0_bit;

volatile unsigned long pulseCount = 0UL;
volatile unsigned long displayValue = 0UL;
unsigned long val = 0UL;
volatile unsigned char timerOvf = 0;
volatile unsigned char displayFlag = 0;

void Interrupt()
{
    if (TMR0IF_bit) {
        TMR0IF_bit = 0;
        TMR0 = 6;
        TMR1ON_bit = 0;
        if (++timerOvf >= 125) { // 125ms x 8ms = 1000ms
            displayValue = (pulseCount << 16) | (TMR1H << 8 | TMR1L);
            pulseCount = 0;
            TMR1H = 0x00;
            TMR1L = 0x00;
            timerOvf = 0;
            displayFlag = 1;
        }
        TMR1ON_bit = 1;
    } else if (TMR1IF_bit) {
        TMR1IF_bit = 0;
        pulseCount++;
    }
}

void main() {
    char displayStr[] = "Freq:         Hz";
    char i = 0;
    char j = 0;

    ANSEL = 0x00;
    ANSELH = 0x00;

    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);

    // Timer1 as 16-bit counter (external clock)
    TMR1H = 0x00;
    TMR1L = 0x00;
    T1CON = 0b00000111;

    // Timer0 for precise 8ms interrupts
    TMR0 = 6;
    OPTION_REG = 0b10000100;

    // Interrupts
    INTCON = 0b11100000;
    PIE1 = 0b00000001;

    while (1) {
        if (displayFlag) {
            val = displayValue;
            displayValue = 0;
            displayFlag = 0;

            for (i = 13; i >= 6; i--) {
                displayStr[i] = (val % 10) + '0';
                val /= 10;
                if (val == 0 && i > 6) {
                    for (j = i - 1; j >= 6; j--) {
                        displayStr[j] = ' ';
                    }
                    break;
                }
            }

            Lcd_Out(1, 1, displayStr);
        }
    }
}