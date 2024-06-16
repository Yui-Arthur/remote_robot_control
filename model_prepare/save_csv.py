import serial
import time
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", nargs='?', type=int, default=0, help="device port")
parser.add_argument("-win", action="store_true", help="windows env")
args = parser.parse_args()
args = vars(args)

port = f"/dev/ttyACM{args['port']}" if not args['win'] else f"COM{args['port']}" 
print("Connect Port :",port)

# device com port and baud rate 

baud_rate = 9600
# connect serial
ser = serial.Serial(port, baud_rate)
try:
    # read capture setting
    while not ser.in_waiting : pass
    data_raw = ser.read_all()  
    data = data_raw.decode()
    capture_num, extra, capture_ms = data.strip('\n').split('_')
    print(capture_num, extra, capture_ms)
    
    while True:
        # read class id and send to nano
        class_id = input("class id : ")
        ser.write(bytes(class_id, encoding='utf8'))
        # wait data reply  
        while not ser.in_waiting: pass
        # sleep a while for all data transmit finish
        time.sleep(1)
                 
        data_raw = ser.read_all()  
        data = data_raw.decode()
        data = data.replace('\r\n', ',').strip(',')
        print('data : \n', data)

        # save data or not
        save = input("save to csv : ")
        if int(save) == 1:
            # write file 
            with open(f"c{capture_num}_e{extra}_m{capture_ms}_data.csv", 'a') as f:
                f.write(data + '\n')
                print("Write Data Success")

except KeyboardInterrupt:
    ser.close()
    print('end')