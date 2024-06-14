#ifndef _TF_MODEL_H_
#define _TF_MODEL_H_

#include "model_data_quant.h"
// #include "config.h"
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

        int infer(float *data, float _threshold){
            for(int i=0; i<input_shape; i++){
                interpreter->input(0)->data.int8[i] = (data[i] / input_scale + input_zero_point);
            }

            #ifdef DEBUG
                Serial.println("Invoke Start");
            #endif
            if (interpreter->Invoke() != kTfLiteOk) {
                Serial.println("Invoke failed");
                return -1;
            }
            
            #ifdef DEBUG
                Serial.println("Invoke End");
            #endif

            float max_value = -1e9, cls = output_shape;

            for(int i=0; i<output_shape; i++){
                float out = (interpreter->output(0)->data.int8[i] - output_zero_point) * output_scale;
                #ifdef DEBUG
                    Serial.print(out);
                    Serial.print(" ");
                #endif
                if(out > _threshold && out > max_value)
                    max_value = out, cls = i;
            }
            #ifdef DEBUG
                Serial.println();
            #endif

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
            
            test_model();
    
            Serial.println("Init Success!");
        }

        void test_model(){
            TfLiteTensor* input = interpreter->input(0);
            TfLiteTensor* output = interpreter->output(0);
            float example_data[] = {-0.157,-0.554,0.881,2.258,-10.620,-10.132,-0.115,-0.545,0.871,-3.662,-7.263,-8.362,-0.101,-0.547,0.844,-7.629,-4.272,-3.113,-0.097,-0.558,0.820,-9.888,-3.113,-2.197,-0.090,-0.555,0.805,-8.240,-4.700,-3.784,-0.075,-0.547,0.793,-7.507,-4.578,-3.723,-0.074,-0.542,0.786,-8.362,-3.967,-2.869,-0.070,-0.547,0.783,-7.568,-3.784,-1.709,-0.071,-0.545,0.780,-3.723,-4.333,1.221,-0.086,-0.536,0.794,-3.052,-4.150,2.686,-0.095,-0.538,0.800,-2.869,-3.601,3.479,-0.100,-0.541,0.802,-2.380,-3.662,3.113,-0.104,-0.548,0.810,-1.831,-2.869,3.357,-0.108,-0.548,0.814,-1.038,-2.380,3.906,-0.101,-0.545,0.827,-2.319,-1.892,3.113,-0.096,-0.552,0.819,-2.563,-0.854,3.540,-0.107,-0.551,0.825,-2.808,1.282,4.944,-0.115,-0.552,0.825,-3.967,-0.977,6.165,-0.130,-0.552,0.810,-4.333,-1.831,6.042,-0.128,-0.545,0.813,-6.287,-2.014,3.418,-0.123,-0.544,0.799,-7.141,-1.282,2.686,-0.119,-0.551,0.798,-4.761,-2.197,3.601,-0.123,-0.546,0.805,-4.028,-1.038,3.662,-0.117,-0.548,0.810,-4.028,-2.014,3.052,-0.113,-0.548,0.817,-5.005,-2.014,2.380,-0.108,-0.549,0.813,-5.615,-2.319,2.319,-0.107,-0.543,0.814,-5.737,-2.258,1.953,-0.110,-0.544,0.803,-5.432,-1.892,2.625,-0.115,-0.547,0.804,-5.005,-1.587,3.235,-0.109,-0.547,0.807,-5.371,-0.793,1.526,-0.101,-0.542,0.810,-4.944,-1.770,1.404,-0.095,-0.541,0.816,-5.310,-1.099,1.770,-0.102,-0.540,0.812,-5.249,-1.465,2.869,-0.108,-0.546,0.804,-4.456,-1.099,3.540,-0.115,-0.551,0.808,-3.723,-1.160,4.150,-0.117,-0.549,0.816,-4.028,-1.648,3.357,-0.114,-0.545,0.809,-5.981,-2.380,2.075,-0.112,-0.544,0.798,-4.944,-3.052,1.587,-0.114,-0.540,0.798,-3.479,-1.343,2.258,-0.116,-0.545,0.810,-3.601,-1.038,2.563};
            int data_point = (30+10)*6;
            output_scale = output->params.scale , output_zero_point = output->params.zero_point;
            input_scale = input->params.scale , input_zero_point = input->params.zero_point;

            Serial.println("Invoke Test Start");
            int pred_cnt[output_shape] = {0};

            for(int i=0; i < data_point ; i+=6){
                example_data[i + 0] = ((example_data[i + 0] + 4.0) / 8.0) / input_scale + input_zero_point;
                example_data[i + 1] = ((example_data[i + 1] + 4.0) / 8.0) / input_scale + input_zero_point;
                example_data[i + 2] = ((example_data[i + 2] + 4.0) / 8.0) / input_scale + input_zero_point;
                example_data[i + 3] = ((example_data[i + 3] + 2000.0) / 4000.0) / input_scale + input_zero_point;
                example_data[i + 4] = ((example_data[i + 4] + 2000.0) / 4000.0) / input_scale + input_zero_point;
                example_data[i + 5] = ((example_data[i + 5] + 2000.0) / 4000.0) / input_scale + input_zero_point;
            }
            
            for(int i=0; i < data_point - input_shape; i+=6){

                for(int j=0; j<input_shape; j++){

                    interpreter->input(0)->data.int8[j] = example_data[i+j];
                    // Serial.print(example_data[i+j]);
                    // Serial.print(" ");
                }
                // Serial.println();


                if (interpreter->Invoke() != kTfLiteOk) {
                    Serial.println("Invoke failed");
                    return;
                }

                int max_value = -1e9, pred = 0;
                for(int j=0; j<output_shape; j++){
                    int out = interpreter->output(0)->data.int8[j];
                    // Serial.print(out);
                    // Serial.print(" ");
                    if(out > max_value)
                        max_value = out, pred = j;
                }
                // Serial.println();
                pred_cnt[pred]++;
            }

            int max_value = -1e9, pred = 0;
            for(int j=0; j<output_shape; j++){
                int out = pred_cnt[j];
                if(out > max_value)
                    max_value = out, pred = j;
                Serial.print(out);
                Serial.print(" ");
            }
            Serial.println();
            Serial.print("pred result = ");
            Serial.println(pred);
            Serial.println("Invoke Test End");
        }

};

#endif