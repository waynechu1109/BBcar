#include "mbed.h"
#include "bbcar.h"
#include <cstdio>
#include "drivers/DigitalOut.h"

#include "erpc_simple_server.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "bbcar_control_server.h"

using namespace std::chrono;

volatile int steps;
volatile int last;

bool nextRight = false;
bool nextLeft = false;
double length = 0;
double pingRec = 0;
int rec = 0;
int speed = 0;

/**
 * Macros for setting console flow control.
 */
#define CONSOLE_FLOWCONTROL_RTS     1
#define CONSOLE_FLOWCONTROL_CTS     2
#define CONSOLE_FLOWCONTROL_RTSCTS  3
#define mbed_console_concat_(x) CONSOLE_FLOWCONTROL_##x
#define mbed_console_concat(x) mbed_console_concat_(x)
#define CONSOLE_FLOWCONTROL mbed_console_concat(MBED_CONF_TARGET_CONSOLE_UART_FLOW_CONTROL)

mbed::DigitalOut led1(LED1, 1);
mbed::DigitalOut led2(LED2, 1);
mbed::DigitalOut led3(LED3, 1);
mbed::DigitalOut* leds[] = { &led1, &led2, &led3 };
// Uncomment for actual BB Car operations
// BBCar* cars[] = {&car}; //Control only one car

/****** erpc declarations *******/

// GET DISTANCE TRAVELED
void stop(uint8_t car){
    if(car == 1) { //there is only one car
          *leds[car - 1] = 0;
        // Uncomment for actual BB Car operations
        // (*cars[car -1]).stop();
        // printf("Car %d stop.\n", car);

        if(rec == 0b0000) speed = -40;  // printf("back\n");
        
        else if(rec == 0b0001) speed = 90./ 2.135;   // printf("sharp left\n");
        else if(rec == 0b0011) speed = 80./ 2.135;   // printf("medium left\n");
        else if(rec == 0b0010) speed = 50./ 2.135;   // printf("gentle left\n");
        else if(rec == 0b0110) speed = 90./ 2.135;   // printf("straight\n");
        else if(rec == 0b0100) speed = 50./ 2.135;   // printf("gentle right\n");
        else if(rec == 0b1100) speed = 80./ 2.135;   // printf("medium right\n");
        else if(rec == 0b1000) speed = 90./ 2.135;   // printf("sharp right\n");

        printf("Distance: %f cm, Speed: %d\n", length, speed);
    }
}

void goStraight(uint8_t car, int32_t  speed){
    if(car == 1) { //there is only one car
          *leds[car - 1] = 0;
        // Uncomment for actual BB Car operations
        // (*cars[car -1]).goStraight(speed);
        printf("Car %d go straight at speed %d.\n", car, speed);
  }
}

void turn(uint8_t car, int32_t speed, double factor){
    if(car == 1) { //there is only one car
          *leds[car - 1] = 0;
        // Uncomment for actual BB Car operations
        // (*cars[car -1]).turn(speed, factor);
        printf("Car %d turn at speed %d with a factor of %f.\n", car, speed, factor);
  }
}

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;

/** LED service */
BBCarService_service car_control_service;


Thread driveThread(osPriorityHigh);
Thread pingThread;
Thread encoderThread;
Thread erpcThread(osPriorityHigh);

EventQueue driveQueue, encoderQueue, pingQueue, erpcQueue;

// DigitalInOut qti1(D3), qti2(D2), qti3(D1), qti4(D0);
BufferedSerial pc(USBTX, USBRX);
DigitalIn encoder(D11);
DigitalInOut ping(D10);
BusInOut qti(D9, D8, D7, D3);
PwmOut pin5(D5), pin6(D6);

Timer ping_timer;
Ticker servo_ticker;
Ticker encoder_ticker;

BBCar car(pin5, pin6, servo_ticker);

void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
   length = steps * 10.31 * 3.14 / 32;
//    printf("steps: %d\n", steps);
//    printf("distance: %f\n", steps * 10.31 * 3.14 / 32);
}

void drive() {

    // while(true) { 
        qti.output();
        qti = 0b1111;
        wait_us(250);
        qti.input();
        wait_us(250);
        // printf("qti: %d\n", (int)qti);
        rec = qti;

        // printf("ping: %f\n", pingRec);


        // if(pingRec < 20 && !nextLeft && !nextRight) {   // if there's an obstacle
        //     // printf("ping: %f\n", pingRec);
        //     printf("obstacle in front\n");
        //     car.stop();                                 // and there's also no branch in front
        //     ThisThread::sleep_for(1000ms);
        //     car.bigTurn(53, 0.4);                       // uturn
        //     ThisThread::sleep_for(1700ms);
        // }
        if(qti == 0b0000) car.goStraight(-40);  // printf("back\n");
        
        else if(qti == 0b0001) {car.turn(90./2.135, 0.47); ThisThread::sleep_for(60ms);}    // printf("sharp left\n");
        else if(qti == 0b0011) {car.turn(80./2.135, 0.5); ThisThread::sleep_for(60ms);}    // printf("medium left\n");
        else if(qti == 0b0010) {car.turn(50./2.135, 0.55); ThisThread::sleep_for(60ms);}    // printf("gentle left\n");
        else if(qti == 0b0110) {car.goStraight(90./2.135); ThisThread::sleep_for(60ms);}   // printf("straight\n");
        else if(qti == 0b0100) {car.turn(50./2.135, -0.55); ThisThread::sleep_for(60ms);}   // printf("gentle right\n");
        else if(qti == 0b1100) {car.turn(80./2.135, -0.5); ThisThread::sleep_for(60ms);}   // printf("medium right\n");
        else if(qti == 0b1000) {car.turn(90./2.135, -0.47); ThisThread::sleep_for(60ms);}   // printf("sharp right\n");
        
        // encounter the branch intersection
        else if(qti == 0b1111) {
            if(nextLeft) {
                // car.turn(85./2.135, 0.4); 
                car.stop();
                car.goStraight(-100);
                ThisThread::sleep_for(60ms);
                // car.turn(85./2.135, 0.4);
                car.bigTurn(70, 0.4);
                ThisThread::sleep_for(640ms);
                nextLeft = false;                
            }
            if(nextRight) {
                // car.turn(85./2.135, -0.4);
                car.stop();
                car.goStraight(-100);
                ThisThread::sleep_for(60ms);
                // car.turn(85./2.135, -0.4);
                car.bigTurn(68, -0.4);
                ThisThread::sleep_for(550ms);
                nextRight = false;
            }
            car.goStraight(90./2.135); ThisThread::sleep_for(60ms);       // printf("straight\n");
        }   

        // recognize turning pattern
        else if(qti == 0b0111){
            nextLeft = true; 
            nextRight = false;          // make sure that the car won't get the double turn signs 
            car.goStraight(90./2.135); 
            ThisThread::sleep_for(60ms);
        }
        else if(qti == 0b1110){
            nextRight = true;
            nextLeft = false;           // make sure that the car won't get the double turn signs 
            car.goStraight(90./2.135); 
            ThisThread::sleep_for(60ms);
        }      

        else {car.goStraight(90./2.135); ThisThread::sleep_for(60ms);}    // default: go straight

        // printf("nextLeft: %d, nextRight: %d\n", nextLeft, nextRight); 
        // printf("qti: %d\n", rec);    
    // }     
}

void u_turn() {
    printf("obstacle in front\n");
    car.stop();                                
    ThisThread::sleep_for(1000ms);
    car.bigTurn(53, 0.4);                       // uturn
    ThisThread::sleep_for(1700ms);
}

void pingScan() {
    float val;
    // while(true) {
        ping.output();
        ping = 0; wait_us(200);
        ping = 1; wait_us(5);
        ping = 0; wait_us(5);

        ping.input();
        while(ping.read() == 0);
        ping_timer.start();
        while(ping.read() == 1);
        val = ping_timer.read();
        pingRec = val* 14500;

        if(pingRec < 20 && !nextLeft && !nextRight) { // if there's an obstacle
            driveQueue.call(u_turn);                  // and there's also no branch in front
        }

        printf("Ping = %lf\r\n", pingRec);
        ping_timer.stop();
        ping_timer.reset();
    // }
}

void run_erpc() {
    printf("Initializing server.\n");
    rpc_server.setTransport(&uart_transport);
    rpc_server.setCodecFactory(&basic_cf);
    rpc_server.setMessageBufferFactory(&dynamic_mbf);


    // Add the led service to the server
    printf("Adding BBCar server.\n");
    rpc_server.addService(&car_control_service);

    // Run the server. This should never exit
    printf("Running server.\n\n");
    rpc_server.run();
}


int main() {

    pc.set_baud(9600);

    steps = 0;
    last = 0;

    // Thread
    driveThread.start(callback(&driveQueue, &EventQueue::dispatch_forever));
    // driveThread.start(drive);
    encoderThread.start(callback(&encoderQueue, &EventQueue::dispatch_forever));
    pingThread.start(callback(&pingQueue, &EventQueue::dispatch_forever));
    // erpcThread.start(callback(&erpcQueue, &EventQueue::dispatch_forever));

    // EventQueue
    driveQueue.call_every(60ms, drive);
    // driveQueue.call(drive);
    encoderQueue.call_every(1ms, encoder_control);
    pingQueue.call_every(500ms, pingScan);


    // Initialize the rpc server
    uart_transport.setCrc16(&crc16);

    // Set up hardware flow control, if needed
#if CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTS
  uart_transport.set_flow_control(mbed::SerialBase::RTS, STDIO_UART_RTS, NC);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_CTS
  uart_transport.set_flow_control(mbed::SerialBase::CTS, NC, STDIO_UART_CTS);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTSCTS
  uart_transport.set_flow_control(mbed::SerialBase::RTSCTS, STDIO_UART_RTS, STDIO_UART_CTS);
#endif

    // erpcQueue.call(run_erpc);
    erpcThread.start(run_erpc);
        
}