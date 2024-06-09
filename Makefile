compiler = arduino-cli
device = arduino:mbed_nano:nano33ble
prepare = model_prepare/IMU_Capture
deploy = nano_deploy/nano_infer_with_BLE
compile_output = nano_deploy/nano_infer_with_BLE/compiled

port ?= 0

python_arg = 
compile_arg =
win ?= 0
debug ?= 0
ifeq ($(win), 1)
	python_arg = -win
endif

ifeq ($(debug), 1)
	compile_arg =--build-property build.extra_flags=-DDEBUG 
endif

build-p : 
	$(compiler) compile --fqbn $(device) $(prepare)

build-d : 
	$(compiler) compile $(compile_arg) --fqbn $(device) --output-dir $(compile_output) $(deploy)

run-p :
	$(compiler) upload -p /dev/ttyACM$(port) --fqbn $(device) $(prepare)
	make watch-p port=$(port) win=$(win)

run-d :
	$(compiler) upload -p /dev/ttyACM$(port) --fqbn $(device) $(deploy)
	make watch-d port=$(port) win=$(win)

watch-p :
	python ./model_prepare/save_csv.py -p ${port} ${python_arg}

watch-d:
	python ./nano_deploy/watch.py -p ${port} ${python_arg}

help:
	
	@echo "usage  :"
	@echo "	build-p, complie the IMU_Capture sketch"
	@echo "	build-d, complie the nano_infer_with_BLE sketch"
	@echo "	run-p, upload the IMU_Capture sketch, and connect serial, watch the result"
	@echo "	run-d, upload the nano_infer_with_BLE sketch, and connect serial, watch the result"
	@echo "	watch-p connect serial, watch the result"
	@echo "	watch-d connect serial, watch the result"
	@echo 
	@echo "arg :"
	@echo "	port, set connect port number"
	@echo "		ex. COM3 => port=3, /dev/ttyACM1 => port=1"
	@echo "	win set env is win or not "
	@echo "		ex. win=1 denote env is windows "
	@echo 
	
