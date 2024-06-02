#include "model_data_quant.h"

#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"


class tf_model{
    public:
        tf_model(int input, int output, int arena) {
            input_shape = input;
            output_shape = output;
            kTensorArenaSize = arena;
            model_init();
        }

        int infer(float *data){
            for(int i=0; i<input_shape; i++){
                interpreter->input(0)->data.int8[i] = (int8_t)(data[i] / input_scale + input_zero_point);
            }

            Serial.println("Invoke Start");

            if (interpreter->Invoke() != kTfLiteOk) {
                Serial.println("Invoke failed");
                return -1;
            }
            
            Serial.println("Invoke End");

            int max_value = -1e9, cls = -1;

            for(int i=0; i<output_shape; i++){
                if(interpreter->output(0)->data.int8[i] > max_value)
                    max_value = interpreter->output(0)->data.int8[i], cls = i;
            }

            return cls;
        }
    private:
        const tflite::Model* m; 
        int kTensorArenaSize;
        int input_shape;
        int output_shape;
        // uint8_t tensor_arena[4000];
        uint8_t *tensor_arena;
        tflite::MicroInterpreter *interpreter;
        float output_scale, input_scale;
        int output_zero_point, input_zero_point;

        void model_init(){
            
            m = tflite::GetModel(model);

            if (m->version() != TFLITE_SCHEMA_VERSION) {
                Serial.println("Model provided is schema version not equal to supported version");
            }

            static tflite::MicroMutableOpResolver<4> op;
            op.AddFullyConnected();
            op.AddCast();
            op.AddQuantize();
            op.AddSoftmax();

            tensor_arena = new uint8_t[kTensorArenaSize];

            interpreter = new tflite::MicroInterpreter(m, op, tensor_arena, kTensorArenaSize);

            if (interpreter->AllocateTensors() != kTfLiteOk) {
                Serial.println("AllocateTensors() failed");
                return;
            }
            

            TfLiteTensor* input = interpreter->input(0);
            TfLiteTensor* output = interpreter->output(0);

            output_scale = output->params.scale , output_zero_point = output->params.zero_point;
            input_scale = input->params.scale , input_zero_point = input->params.zero_point;

            for(int i=0; i<input_shape; i++){
                interpreter->input(0)->data.int8[i] = 1;
            }

            Serial.println("Invoke Test Start");

            if (interpreter->Invoke() != kTfLiteOk) {
                Serial.println("Invoke failed");
                return;
            }

            Serial.println("Invoke Test End");

            // for(int i=0; i<output_shape; i++){
            //     Serial.print(interpreter->output(0)->data.int8[i]);
            //     Serial.print(" ");
            // }

            Serial.println("Init Success!");
        }

};
