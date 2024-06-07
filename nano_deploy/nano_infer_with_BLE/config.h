#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "ArduinoBLE.h"

// #define DEBUG

// model input and output
#define capture_point 30
#define output_gesture 7

#define buf_scale 2
// buffer size = total capture data * scale
#define total_buffer_size (capture_point * buf_scale)

// delay time between two capture
#define capture_ms 20
// delay time between two BLE send operate
#define BLE_send_ms 5000
// delay time between two inference
#define infer_sleep_ms 1000

// tflm setting
#define threshold 0.5
#define arena_size 3000

// BLE setting
const char* deviceServiceUuid = "00000000-eeee-eeee-eeee-eeeeeeeeeeee";
const char* deviceServiceCharacteristicUuid = "00000000-eeee-eeee-eeee-eeeeeeeeeeee";


#endif /* _CONFIG_H_ */