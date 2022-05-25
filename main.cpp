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

bool nextRight = false;
bool nextLeft = false;

void drive() {
        qti.output();
        qti = 0b1111;
        wait_us(250);
        qti.input();
        wait_us(250);
        // printf("qti: %d\n", (int)qti);
        int rec = qti;

        // ThisThread::sleep_for(50ms);

        if(qti == 0b0000)      car.goStraight(-40);  // printf("back\n");
        
        else if(qti == 0b0001) {car.turn(83./3, 0.4); ThisThread::sleep_for(60ms);}    // printf("sharp left\n");
        else if(qti == 0b0011) {car.turn(75./3, 0.4); ThisThread::sleep_for(60ms);}    // printf("medium left\n");
        else if(qti == 0b0010) {car.turn(50./3, 0.4); ThisThread::sleep_for(60ms);}    // printf("gentle left\n");
        else if(qti == 0b0110) {car.goStraight(90./3); ThisThread::sleep_for(60ms);}  // printf("straight\n");
        else if(qti == 0b0100) {car.turn(50./3, -0.4); ThisThread::sleep_for(60ms);}   // printf("gentle right\n");
        else if(qti == 0b1100) {car.turn(75./3, -0.4); ThisThread::sleep_for(60ms);}   // printf("medium right\n");
        else if(qti == 0b1000) {car.turn(83./3, -0.4); ThisThread::sleep_for(60ms);}  // printf("sharp right\n");
        
        // encoter the branch intersection
        else if(qti == 0b1111) {
            if(nextLeft) {
                // car.turn(85./3, 0.4); 
                car.stop();
                car.goStraight(-100);
                ThisThread::sleep_for(50ms);
                // car.turn(85./3, 0.4);
                car.bigTurn(85./3, 0.4);
                ThisThread::sleep_for(1000ms);
                nextLeft = false;
            }

            if(nextRight) {
                // car.turn(85./3, -0.4);
                car.stop();
                car.goStraight(-100);
                ThisThread::sleep_for(50ms);
                // car.turn(85./3, -0.4);
                car.bigTurn(85./3, -0.4);
                ThisThread::sleep_for(1000ms);
                nextRight = false;
            }
            car.goStraight(90./3); ThisThread::sleep_for(60ms);       // printf("straight\n");
        }   

        // recognize turning pattern
        else if(qti == 0b0111) {nextLeft = true; car.goStraight(90./3); ThisThread::sleep_for(60ms);}
        else if(qti == 0b1110) {nextRight = true; car.goStraight(90./3); ThisThread::sleep_for(60ms);}     

        printf("nextLeft: %d, nextRight: %d\n", nextLeft, nextRight); 
        // printf("qti: %d\n", rec);         
}


// main() runs in its own thread in the OS
int main() {

    // Thread
    driveThread.start(callback(&driveQueue, &EventQueue::dispatch_forever));

    // EventQueue
    driveQueue.call_every(60ms, drive);
}

