/*
  IMU Capture

  This example uses the on-board IMU to start reading acceleration and gyroscope
  data from on-board IMU and prints it to the Serial Monitor for one second
  when the significant motion is detected.

  You can also use the Serial Plotter to graph the data.

  The circuit:
  - Arduino Nano 33 BLE or Arduino Nano 33 BLE Sense board.

  Created by Don Coleman, Sandeep Mistry
  Modified by Dominic Pajak, Sandeep Mistry

  This example code is in the public domain.
*/

#include "Arduino_BMI270_BMM150.h"
#include "Arduino_LSM9DS1.h"
#include "mbed.h"
#include "rtos.h"

// model input
const int capture_point = 10;
// +-range with extra data
const int extra_point = 5;
// total capture data = capture - extra point ~ capture + extra point 
const int total_capture = capture_point + 2*extra_point;

const int buf_scale = 2;
// buffer size = total capture data * scale
const int total_buffer_size = total_capture * buf_scale; 

// delay time between two capture
int capture_ms = 20;


int current_point = 0;
float aX[total_buffer_size], aY[total_buffer_size], aZ[total_buffer_size]; 
float gX[total_buffer_size], gY[total_buffer_size], gZ[total_buffer_size];
rtos::Thread thread;

// capture data thread
void capture(){
  while(1){

    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      IMU.readAcceleration(aX[current_point], aY[current_point], aZ[current_point]);
      IMU.readGyroscope(gX[current_point], gY[current_point], gZ[current_point]);
      current_point += 1;
    }

    if(current_point >= total_buffer_size)  current_point = 0;

    // sleep this thread until next capture time
    rtos::ThisThread::sleep_for(capture_ms);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  // start capute thread
  thread.start(capture);
  Serial.print(String(capture_point) + "_" + String(extra_point));
}


void loop() {
  int class_id, save;
  String s;
  
  // read class id from serial
  while(Serial.available() <= 0);
  s = Serial.readString();
  class_id = s.toInt();

  // sleep this thread for wait capture capture_point + extra_point data 
  rtos::ThisThread::sleep_for((capture_point + extra_point) * capture_ms);

  // start point idx = current_point - total_capture + 1, if res < 0 => + total_buffer_size 
  int start_point = (current_point - total_capture + 1) < 0 ? total_buffer_size + (current_point - total_capture + 1) : (current_point - total_capture + 1);
  Serial.print(class_id);
  Serial.print(",");
  // print data to serial
  for(int i=0, cur = start_point; i<total_capture; i++, cur = (cur+1) % total_buffer_size){
    Serial.print(aX[cur], 3);
    Serial.print(',');
    Serial.print(aY[cur], 3);
    Serial.print(',');
    Serial.print(aZ[cur], 3);
    Serial.print(',');
    Serial.print(gX[cur], 3);
    Serial.print(',');
    Serial.print(gY[cur], 3);
    Serial.print(',');
    Serial.print(gZ[cur], 3);
    Serial.println();
  }
  Serial.println();

}