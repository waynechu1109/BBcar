#include "mbed.h"
#include "bbcar.h"
#include <cstdio>
using namespace std::chrono;

Thread driveThread;
EventQueue driveQueue;

// DigitalInOut qti1(D3), qti2(D2), qti3(D1), qti4(D0);
BusInOut qti(D9, D8, D7, D3);
PwmOut pin5(D5), pin6(D6);
Ticker servo_ticker;
BBCar car(pin5, pin6, servo_ticker);

void drive() {
        qti.output();
        qti = 0b1111;
        wait_us(250);
        qti.input();
        wait_us(250);
        // printf("qti: %d\n", (int)qti);
        // rec = qti;

        // ThisThread::sleep_for(50ms);

        if(qti == 0b0000)      car.goStraight(-40);  // printf("back\n");
        
        else if(qti == 0b0001) {car.turn(85./3, 0.4); ThisThread::sleep_for(60ms);}    // printf("sharp left\n");
        else if(qti == 0b0011) {car.turn(80./3, 0.4); ThisThread::sleep_for(60ms);}    // printf("medium left\n");
        else if(qti == 0b0010) {car.turn(50./3, 0.4); ThisThread::sleep_for(60ms);}    // printf("gentle left\n");
        else if(qti == 0b0110) {car.goStraight(90./3); ThisThread::sleep_for(60ms);}  // printf("straight\n");
        else if(qti == 0b0100) {car.turn(50./3, -0.4); ThisThread::sleep_for(60ms);}   // printf("gentle right\n");
        else if(qti == 0b1100) {car.turn(80./3, -0.4); ThisThread::sleep_for(60ms);}   // printf("medium right\n");
        else if(qti == 0b1000) {car.turn(85./3, -0.4); ThisThread::sleep_for(60ms);}  // printf("sharp right\n");
        
        else if(qti == 0b1111) {car.goStraight(90./3); ThisThread::sleep_for(60ms);}   // printf("straight\n");
        else                   {car.goStraight(90./3); ThisThread::sleep_for(60ms);}   // printf("straight\n");
}


// main() runs in its own thread in the OS
int main() {

    // Thread
    driveThread.start(callback(&driveQueue, &EventQueue::dispatch_forever));

    // EventQueue
    driveQueue.call_every(60ms, drive);
}

