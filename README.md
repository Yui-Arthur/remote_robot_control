### make 
```bash
# {port} set connect port number, example COM3 => port=3 or /dev/ttyACM1 => port=1
# {win} set env is win or not 
make help
"""
usage  :
    build-p, complie the IMU_Capture sketch
    build-d, complie the nano_infer_with_BLE sketch
    run-p, upload the IMU_Capture sketch, and connect serial, watch the result
    run-d, upload the nano_infer_with_BLE sketch, and connect serial, watch the result
    watch-p connect serial, watch the result
    watch-d connect serial, watch the result

arg
    port, set connect port number
        ex. COM3 => port=3, /dev/ttyACM1 => port=1
    win set env is win or not 
        ex. win=1 denote env is windows 

"""
make build-d 
make run-d port={port} win={win}
make watch-p port={port} win={win}
```