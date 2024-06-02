
#include "Arduino_BMI270_BMM150.h"
#include "Arduino_LSM9DS1.h"
#include "mbed.h"
#include "rtos.h"
#include "tf_model.h"
// #include "model_data_quant.h"

// model input
const int capture_point = 15;
const int output_gesture = 10;

const int buf_scale = 2;
// buffer size = total capture data * scale
const int total_buffer_size = capture_point * buf_scale; 

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

tf_model *t;
void setup() {
    Serial.begin(9600);
    while (!Serial);

    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }
    
    // start capute thread
    thread.start(capture);   
    tflite::InitializeTarget();
    t = new tf_model(capture_point, output_gesture, 5000);

}

float data[capture_point*6];
void loop() {

    int start_point = (current_point - capture_point + 1) < 0 ? total_buffer_size + (current_point - capture_point + 1) : (current_point - capture_point + 1);

    for(int i=0, cur = start_point; i<capture_point; i++, cur = (cur+1) % total_buffer_size){
        data[6*i + 0] = aX[cur];
        data[6*i + 1] = aY[cur];
        data[6*i + 2] = aZ[cur];
        data[6*i + 3] = gX[cur];
        data[6*i + 4] = gY[cur];
        data[6*i + 5] = gZ[cur];
    }
    Serial.println(t->infer(data));

    rtos::ThisThread::sleep_for(1000);


}
