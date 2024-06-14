
#include "Arduino_BMI270_BMM150.h"
// #include "Arduino_LSM9DS1.h"
#include "mbed.h"
#include "rtos.h"
#include "tf_model.h"
#include "config.h"

uint8_t pred_count[output_gesture + 1] = {0};
int current_point = 0;
uint64_t total_captured_point = 0;

BLEService gestureService(deviceServiceUuid); 
BLECharacteristic gestureCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite, 100);

float aX[total_buffer_size], aY[total_buffer_size], aZ[total_buffer_size]; 
float gX[total_buffer_size], gY[total_buffer_size], gZ[total_buffer_size];

rtos::Thread capture_thread;
rtos::Thread ble_thread;

// capture data thread
void capture(){
    while(1){

        if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
            IMU.readAcceleration(aX[current_point], aY[current_point], aZ[current_point]);
            IMU.readGyroscope(gX[current_point], gY[current_point], gZ[current_point]);
            current_point += 1;
            total_captured_point += 1;
        }

        if(current_point >= total_buffer_size)  current_point = 0;

        // sleep this thread until next capture time
        rtos::ThisThread::sleep_for(capture_ms);
    }
}

// ble connect and send
void BLE_service(){
    while(1){
        BLEDevice central = BLE.central();
        rtos::ThisThread::sleep_for(1500);
        if(central){

            while (central.connected()) {
                int final_pred = -1, max_value = -1;
                
                #if (SEDDING_TYPE == 1)
                    String s;
                    byte buf[300];
                    for(int i=0; i <= output_gesture; i++){
                        s += String(pred_count[i]) + " ";
                        if(pred_count[i] > max_value)
                            max_value = pred_count[i], final_pred = i;
                        pred_count[i] = 0;
                    }
                    s += String(final_pred);
                    s.getBytes(buf, 300);
                    gestureCharacteristic.writeValue(buf, s.length());
                #else
                    
                    for(int i=0; i <= output_gesture; i++){
                        if(pred_count[i] > max_value)
                            max_value = pred_count[i], final_pred = i;
                        pred_count[i] = 0;
                    }
                
                    gestureCharacteristic.writeValue(byte(final_pred));
                #endif
                rtos::ThisThread::sleep_for(BLE_send_ms);
            }
                
        }
        rtos::ThisThread::sleep_for(1000);

    }
    
}

tf_model *t;
void setup() {
    Serial.begin(9600);
    while (!Serial);

    // IMU init
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }
    // BLE init
    if (!BLE.begin()) {
        Serial.println("- Starting Bluetooth® Low Energy module failed!");
        while (1);
    }
    BLE.setLocalName("Arduino Nano 33 BLE (Peripheral)");
    BLE.setAdvertisedService(gestureService);
    gestureService.addCharacteristic(gestureCharacteristic);
    BLE.addService(gestureService);
    BLE.advertise();

    Serial.print("Local address is: ");
    Serial.println(BLE.address());

    // tflm init
    tflite::InitializeTarget();
    t = new tf_model(capture_point*6, output_gesture, arena_size);

    // start capute thread
    capture_thread.start(capture);
    ble_thread.start(BLE_service);   
    
}

float data[capture_point*6];
uint64_t last_point;
uint64_t cur_point;
void loop() {
    cur_point = total_captured_point;
    int start_point = (cur_point - capture_point) % total_buffer_size;

    mbed::Timer timer;
    timer.start();
    for(int i=0, cur = start_point; i<capture_point; i++, cur = (cur+1) % total_buffer_size){
        data[6*i + 0] = (aX[cur] + 4.0) / 8.0;
        data[6*i + 1] = (aY[cur] + 4.0) / 8.0;
        data[6*i + 2] = (aZ[cur] + 4.0) / 8.0;

        data[6*i + 3] = (gX[cur] + 2000.0) / 4000.0;
        data[6*i + 4] = (gY[cur] + 2000.0) / 4000.0;
        data[6*i + 5] = (gZ[cur] + 2000.0) / 4000.0;
    }
    int pred_class = t->infer(data, threshold);
    pred_count[pred_class]++;
    timer.stop();
    #ifdef DEBUG
        Serial.println("There are " + String((unsigned long)(cur_point - last_point)) +  " new point since last inference");
        Serial.print("pred class : ");
        Serial.println(pred_class);
        Serial.print("infer time : ");
        Serial.print(std::chrono::duration_cast<std::chrono::microseconds>(timer.elapsed_time()).count());
        Serial.println(" (ns) \n");
        rtos::ThisThread::sleep_for(infer_sleep_ms);
        last_point = cur_point;
    #endif
     

}
