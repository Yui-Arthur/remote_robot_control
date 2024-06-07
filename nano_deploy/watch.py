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
    
    
    while True:
        
        while not ser.in_waiting: pass
        
                 
        data_raw = ser.read_all()  
        data = data_raw.decode()
        print(data, end="")


except KeyboardInterrupt:
    ser.close()
    print('end')