#include "mbed.h"

#define TX PA_9
#define RX PA_10
#define BAUDRATE 9600
#define LED3 PB_5
#define LED2 PB_4
#define BUTTON2 PA_2
#define BUTTON3 PA_3

using namespace std::chrono_literals;
using namespace std::chrono;

static BufferedSerial uart(TX, RX, BAUDRATE);

static DigitalOut led3(LED3);
static DigitalOut led2(LED2);

static DigitalIn button2(BUTTON2, PullUp);
static DigitalIn button3(BUTTON3, PullUp);

//задать для двух светодиодов период импульсов 120 и 240 мс. 
//При нажатии на btn1 увеличить кф заполнения, при нажатии btn2 уменьшить. Коэф заполнения меняется от 0 до 100%

FileHandle *mbed::mbed_override_console(int fd){
    return &uart;
}

// main() runs in its own thread in the OS
int main()
{
    Timer timer_1;
    Timer timer_2; 
    timer_1.start();
    timer_2.start();
    
    int elapsed_time_led3 = 0;
    int elapsed_time_led2 = 0;

    float PULSE_PERIOD_120 = 120;
    float PULSE_PERIOD_240 = 240;

    float TIME_STEP = 0.0001;

    float fill_factor = 0.5;

    while (true) {
        if (!button3) {
            fill_factor  += TIME_STEP;
        }
        if (fill_factor > 1) {
            fill_factor = 1;
        }
        if (!button2) {
            fill_factor -= TIME_STEP;
        }
        if (fill_factor < 0) {
            fill_factor = 0;
        }
        
        //printf("current period is %d ms\n\r", (int)(PULSE_PERIOD_240 * (1 - fill_factor)));

        elapsed_time_led3 = duration_cast<milliseconds>(timer_1.elapsed_time()).count();
        if (elapsed_time_led3 > PULSE_PERIOD_120) {
            led3 = 0;
            timer_1.reset();
        }
        else if (elapsed_time_led3 > PULSE_PERIOD_120 * (1 - fill_factor)) {
            led3 = 1;
        }

        elapsed_time_led2 = duration_cast<milliseconds>(timer_2.elapsed_time()).count();
        if (elapsed_time_led2 > PULSE_PERIOD_240) {
            led2 = 0;
            timer_2.reset();
        }
        else if (elapsed_time_led2 > PULSE_PERIOD_240 * (1 - fill_factor)) {
            led2 = 1;
        }
    }
}

