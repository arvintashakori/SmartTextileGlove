import asyncio
from bleak import BleakClient
from vpython import *
import numpy as np
import math

# Usage: connecting to peripheral device to visualize quaternion
# will pop-out a web browser with the visualizations

address = "EC:4D:A4:2A:DC:7D"
ADC_CHAR_UUID = "d3091401-b0f3-11ea-b3de-0242ac130004"
BNO_CHAR_UUID = "d3091403-b0f3-11ea-b3de-0242ac130004"

def adc_callback(sender: int, data: bytearray):
    pass

def bno_callback(sender: int, data: bytearray):
    int16 = [data[i*2:(i+1)*2] for i in range((len(data) + 2 - 1) // 2)]
    bno = [int.from_bytes(x, byteorder='little', signed=True) for x in int16]
    quad0 = bno[3:7]
    quad1 = bno[13:17]
    plot(quad0)

async def run(address):
    async with BleakClient(address) as client:
        await client.start_notify(ADC_CHAR_UUID, adc_callback)
        await client.start_notify(BNO_CHAR_UUID, bno_callback)
        while True:
            pass

def plot(quaternion):
    q0 = quaternion[0] / pow(2, 14)
    q1 = quaternion[1] / pow(2, 14)
    q2 = quaternion[2] / pow(2, 14)
    q3 = quaternion[3] / pow(2, 14)
    print(q0, q1, q2, q3)
    roll=-math.atan2(2*(q0*q1+q2*q3),1-2*(q1*q1+q2*q2))
    pitch=math.asin(2*(q0*q2-q3*q1))
    yaw=-math.atan2(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3))-np.pi/2

    rate(50)
    k=vector(cos(yaw)*cos(pitch), sin(pitch),sin(yaw)*cos(pitch))
    y=vector(0,1,0)
    s=cross(k,y)
    v=cross(s,k)
    vrot=v*cos(roll)+cross(k,v)*sin(roll)

    frontArrow.axis=k
    sideArrow.axis=cross(k,vrot)
    upArrow.axis=vrot
    myObj.axis=k
    myObj.up=vrot
    sideArrow.length=2
    frontArrow.length=4
    upArrow.length=1      

scene.range=5
scene.background=color.yellow
toRad=2*np.pi/360
toDeg=1/toRad
scene.forward=vector(-1,-1,-1)
 
scene.width=1200
scene.height=1080
 
xarrow=arrow(lenght=2, shaftwidth=.1, color=color.red,axis=vector(1,0,0))
yarrow=arrow(lenght=2, shaftwidth=.1, color=color.green,axis=vector(0,1,0))
zarrow=arrow(lenght=4, shaftwidth=.1, color=color.blue,axis=vector(0,0,1))
 
frontArrow=arrow(length=4,shaftwidth=.1,color=color.purple,axis=vector(1,0,0))
upArrow=arrow(length=1,shaftwidth=.1,color=color.magenta,axis=vector(0,1,0))
sideArrow=arrow(length=2,shaftwidth=.1,color=color.orange,axis=vector(0,0,1))
 
bBoard=box(length=6,width=2,height=.2,opacity=.8,pos=vector(0,0,0,))
bn=box(length=1,width=.75,height=.1, pos=vector(-.5,.1+.05,0),color=color.blue)
nano=box(lenght=1.75,width=.6,height=.1,pos=vector(-2,.1+.05,0),color=color.green)
myObj=compound([bBoard,bn,nano])

loop = asyncio.get_event_loop()
loop.run_until_complete(run(address))
