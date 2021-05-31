#include "mbed.h"
#include <cstdio>
#include <ctime>
#include <string.h>

#define TX PA_9
#define RX PA_10
#define BTN1 PA_0
#define LED PB_5
#define ADC PA_5
#define RES PA_6

#define BAUDRATE 9600

using namespace std::chrono_literals;
using namespace std::chrono;

static BufferedSerial uart(TX, RX, BAUDRATE);
static AnalogIn adc(ADC);
static AnalogIn res(RES);

static DigitalOut led(LED);
Timer t;


FileHandle *mbed::mbed_override_console(int fd) {
    return &uart;
}

int main() {
    t.start();

    float previusVoltage = 0;
    float voltage = 0, signal = 0;

    while (true) {
        signal = res.read();
        voltage = 3.3 * signal;
        if (abs(voltage - previusVoltage) > 0.1) {
            printf("v=%f\r\n", round(voltage * 10) / 10);
        }
        previusVoltage = voltage;

        led = !led;
        ThisThread::sleep_for(500ms);

    }
}