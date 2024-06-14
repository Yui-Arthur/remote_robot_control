import asyncio
import contextlib
from bleak import BleakClient, BleakScanner
from threading import Thread
import time

async def connect_and_recv(name, lock, address, characteristic_uuid, class_name, opearate):
    
    while 1:
        try:
            async with contextlib.AsyncExitStack() as stack:
                async with lock:
                    device = await BleakScanner.find_device_by_address(address, timeout=30)
                    print(device)
                    if device == None : continue
                    client = BleakClient(device, timeout=60)
                    await stack.enter_async_context(client)
                    print(f"connect {address} success !")
            
                while(1):
                    class_id = await client.read_gatt_char(characteristic_uuid)     
                    class_id = int.from_bytes(class_id, "big")
                    opearate[0] = class_id
                    print(f"Recv {name} Class:" , class_name[class_id])
                
        except Exception as e:
            print(e)

async def send_operation(left_operate, right_operate):
    async with contextlib.AsyncExitStack() as stack:
        while 1:
            """ TODO send request here """
            print(f"latest operation ({left_operate[0]}, {right_operate[0]})")
            await asyncio.sleep(10.0)

right_address = "50:13:05:5b:a0:8a"
right_class_name = ["front", "back", "up", "down", "left", "right", "stop", "continue"]
right_characteristic_uuid = "00000000-eeee-eeee-eeee-eeeeeeeeeeee"

left_address = "66:7B:B5:7D:84:C9"
left_class_name = ["front", "back", "up", "down", "left", "right", "stop", "continue"]
left_characteristic_uuid = "00000000-eeee-eeee-eeee-eeeeeeeeeeee"

right_operate = [0]
left_operate = [0]

async def main():
    lock = asyncio.Lock()
    letf_task = asyncio.create_task(connect_and_recv("left", lock, left_address, left_characteristic_uuid, left_class_name, left_operate))
    right_task = asyncio.create_task(connect_and_recv("right", lock, right_address, right_characteristic_uuid, right_class_name, right_operate))
    send_task = asyncio.create_task(send_operation(left_operate, right_operate))
    await right_task
    await letf_task
    await send_task

asyncio.run(main())
