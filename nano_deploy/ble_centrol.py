import asyncio
from bleak import BleakClient, BleakScanner

address = "66:7B:B5:7D:84:C9"
MODEL_NBR_UUID = "00000000-eeee-eeee-eeee-eeeeeeeeeeee"
class_name = ["front", "back", "up", "down", "left", "right", "stop"]
# 66:7B:B5:7D:84:C9: 66-7B-B5-7D-84-C9
async def main(address):
    async with BleakClient(address, timeout=30) as client:
        print("connect success !")
        while(1):
            class_id = await client.read_gatt_char(MODEL_NBR_UUID)        
            print("Recv Class:" , class_name[int.from_bytes(class_id, "big")])
            

async def dis():
    while(1):
        devices = await BleakScanner.discover()
        for d in devices:
            if d.address == address:
                print(d.name)
                print(d.details)
                print("find !")
                return
                

# asyncio.run(main(address))
# asyncio.run(dis())
asyncio.run(main(address))