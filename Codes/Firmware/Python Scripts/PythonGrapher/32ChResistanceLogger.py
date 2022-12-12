from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time
import serial
import atexit
import time
import serial.tools.list_ports
import os

# *** Which sensor block do you want to plot? ***
# Set sensor_block_used = 0 for sensors @ pin 4 to pin 15
# Set sensor_block_used = 1 for sensors @ pin 16 to pin 27
# Set sensor_block_used = 2 for sensors @ pin 28 to pin 33
sensor_block_used = 0
com_port_string = 'COM6'
ports = list(serial.tools.list_ports.comports())
for p in ports:
    print(p)
    if "JLink" in p.description:
        print ("This is a JLink!")
        print (p.device)
        com_port_string = p.device
iscorrect_maping = True
#com_port_string = '/dev/cu.usbmodem1411'
baud_rate = 1000000
com_timeout = 3.0
# Change this file path to your directory! Prints the csv file to this directory too
file_string = os.path.dirname(os.path.realpath('32ChResistanceLogger.py'))
#file_string = 'C:\\Documents\\GitHub\\texavie\\Python Scripts\\PythonGrapher\\'
#file_string = 'C:\\Users\\Admin\\Documents\\GitHub\\texavie\\Python Scripts\\PythonGrapher'
#file_string = '/Users/reendael/Documents/PythonVisual/'

isFirstInit = True

# MAPPING PCB SENSORS TO SERIAL OUTPUT INDECES (0 means there's currently no sensor at the PCB pin):
# Sensor block 0 (Mux D)
if iscorrect_maping == True:
    sensorPin0 = 3
    sensorPin1 = 7
    sensorPin2 = 11
    sensorPin3 = 15
    sensorPin4 = 31   # F7
    sensorPin5 = 27   # F6
    sensorPin6 = 23   # F5
    sensorPin7 = 19
    #sensorPin8 = 0   # GND pin
    # Sensor block 1 (Mux C)
    sensorPin9 =  2
    sensorPin10 = 6   # E7
    sensorPin11 = 10  # E6
    sensorPin12 = 14  # E5
    sensorPin13 = 30
    sensorPin14 = 26  # D7
    sensorPin15 = 22  # D6
    sensorPin16 = 18  # D5
    # Sensor block 2 (Mux B)
    sensorPin17 = 16  # G3
    sensorPin18 = 20  # Not connected to a sensor
    sensorPin19 = 24  # F4
    sensorPin20 = 28  # F3
    sensorPin21 = 12  # F2
    sensorPin22 =  8
    sensorPin23 =  4   # D3
    sensorPin24 =  0   # D2
    #sensorPin25 = 0  # GND pin
    # Sensor block 3 (Mux A)
    sensorPin26 = 17
    sensorPin27 = 21  # C4
    sensorPin28 = 25  # C3
    sensorPin29 = 29  # C2
    sensorPin30 = 13  # C1
    sensorPin31 = 9   # Not connected to a sensor
    sensorPin32 = 5   # A4
    sensorPin33 = 1   # A3
else:
    sensorPin0 = 4#
    sensorPin1 = 8#
    sensorPin2 = 12
    sensorPin3 = 16
    sensorPin4 = 0#   # F7
    sensorPin5 = 28   # F6
    sensorPin6 = 24   # F5
    sensorPin7 = 20
    #sensorPin8 = 0   # GND pin
    # Sensor block 1 (Mux C)
    sensorPin9 = 3#2
    sensorPin10 = 7 #6   # E7
    sensorPin11 = 11#10  # E6
    sensorPin12 = 15#14  # E5
    sensorPin13 = 31#30
    sensorPin14 = 27#26  # D7
    sensorPin15 = 23#22  # D6
    sensorPin16 = 19#18  # D5
    # Sensor block 2 (Mux B)
    sensorPin17 = 17#16  # G3
    sensorPin18 = 21#20  # Not connected to a sensor
    sensorPin19 = 25#24  # F4
    sensorPin20 = 29#28  # F3
    sensorPin21 = 13#12  # F2
    sensorPin22 = 9#8
    sensorPin23 = 5#4   # D3
    sensorPin24 = 1#0   # D2
    #sensorPin25 = 0  # GND pin
    # Sensor block 3 (Mux A)
    sensorPin26 = 18#17
    sensorPin27 = 22#21  # C4
    sensorPin28 = 26#25  # C3
    sensorPin29 = 30#29  # C2
    sensorPin30 = 14#13  # C1
    sensorPin31 = 10#9   # Not connected to a sensor
    sensorPin32 = 6#5   # A4
    sensorPin33 = 2#1   # A3


file_name = str(input("Please Enter the File Name: "))
print("\n")

#app = QtGui.QApplication([])
pg.setConfigOptions(antialias=True, background='w', foreground='k')

win = pg.GraphicsWindow(title="Resistance Plotting")
win.resize(1600, 1000)
win.setWindowTitle('pyqtgraph example: Plotting')

pg.setConfigOptions(antialias=True)

if (sensor_block_used == 0): # SENSORS @ PINS 0-7
    p1 = win.addPlot(title="Sensor @ pin 0")
    p2 = win.addPlot(title="Sensor @ pin 1")
    p3 = win.addPlot(title="Sensor @ pin 2")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 3")
    p5 = win.addPlot(title="Sensor @ pin 4")
    p6 = win.addPlot(title="Sensor @ pin 5")
    win.nextRow()
    p7 = win.addPlot(title="Sensor @ pin 6")
    p8 = win.addPlot(title="Sensor @ pin 7")
    #p9 = win.addLayout(row=3, col=2)

elif (sensor_block_used == 1): # SENSORS @ PINS 9-16
    p1 = win.addPlot(title="Sensor @ pin 9")
    p2 = win.addPlot(title="Sensor @ pin 10")
    p3 = win.addPlot(title="Sensor @ pin 11")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 12")
    p5 = win.addPlot(title="Sensor @ pin 13")
    p6 = win.addPlot(title="Sensor @ pin 14")
    win.nextRow()
    p7 = win.addPlot(title="Sensor @ pin 15")
    p8 = win.addPlot(title="Sensor @ pin 16")

elif (sensor_block_used == 2): # SENSORS @ PINS 17-24
    p1 = win.addPlot(title="Sensor @ pin 17")
    p2 = win.addPlot(title="Sensor @ pin 18")
    p3 = win.addPlot(title="Sensor @ pin 19")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 20")
    p5 = win.addPlot(title="Sensor @ pin 21")
    p6 = win.addPlot(title="Sensor @ pin 22")
    win.nextRow()
    p7 = win.addPlot(title="Sensor @ pin 23")
    p8 = win.addPlot(title="Sensor @ pin 24")

elif (sensor_block_used == 3): # SENSORS @ PINS 26-33
    p1 = win.addPlot(title="Sensor @ pin 26")
    p2 = win.addPlot(title="Sensor @ pin 27")
    p3 = win.addPlot(title="Sensor @ pin 28")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 29")
    p5 = win.addPlot(title="Sensor @ pin 30")
    p6 = win.addPlot(title="Sensor @ pin 31")
    win.nextRow()
    p7 = win.addPlot(title="Sensor @ pin 32")
    p8 = win.addPlot(title="Sensor @ pin 33")

def selectionchange (i):
    global sensor_block_used, ptr, curve1, curve2, curve3, curve4, curve5, curve6, curve7, \
           ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7, isFirstInit
    sensor_block_used = i
    if sensor_block_used == 0:
        p1.setTitle('Sensor @ pin 0')
        p2.setTitle('Sensor @ pin 1')
        p3.setTitle('Sensor @ pin 2')
        p4.setTitle('Sensor @ pin 3')
        p5.setTitle('Sensor @ pin 4')
        p6.setTitle('Sensor @ pin 5')
        p7.setTitle('Sensor @ pin 6')
        p8.setTitle('Sensor @ pin 7')
    elif sensor_block_used == 1:
        p1.setTitle('Sensor @ pin 9')
        p2.setTitle('Sensor @ pin 10')
        p3.setTitle('Sensor @ pin 11')
        p4.setTitle('Sensor @ pin 12')
        p5.setTitle('Sensor @ pin 13')
        p6.setTitle('Sensor @ pin 14')
        p7.setTitle('Sensor @ pin 15')
        p8.setTitle('Sensor @ pin 16')
    elif sensor_block_used == 2:
        p1.setTitle('Sensor @ pin 17')
        p2.setTitle('Sensor @ pin 18')
        p3.setTitle('Sensor @ pin 19')
        p4.setTitle('Sensor @ pin 20')
        p5.setTitle('Sensor @ pin 21')
        p6.setTitle('Sensor @ pin 22')
        p7.setTitle('Sensor @ pin 23')
        p8.setTitle('Sensor @ pin 24')
    elif sensor_block_used == 3:
        p1.setTitle('Sensor @ pin 26')
        p2.setTitle('Sensor @ pin 27')
        p3.setTitle('Sensor @ pin 28')
        p4.setTitle('Sensor @ pin 29')
        p5.setTitle('Sensor @ pin 30')
        p6.setTitle('Sensor @ pin 31')
        p7.setTitle('Sensor @ pin 32')
        p8.setTitle('Sensor @ pin 33')
    if isFirstInit == False:
        del ch0[:]
        del ch1[:]
        del ch2[:]
        del ch3[:]
        del ch4[:]
        del ch5[:]
        del ch6[:]
        del ch7[:]
        ptr = 0
        curve1.setPos(ptr, 0)
        curve2.setPos(ptr, 0)
        curve3.setPos(ptr, 0)
        curve4.setPos(ptr, 0)
        curve5.setPos(ptr, 0)
        curve6.setPos(ptr, 0)
        curve7.setPos(ptr, 0)
        curve8.setPos(ptr, 0)

proxy = QtGui.QGraphicsProxyWidget()
#button = QtGui.QPushButton('button')
comboBox = QtGui.QComboBox()
comboBox.SizeAdjustPolicy = comboBox.AdjustToContents
comboBox.currentIndexChanged.connect(selectionchange)
comboBox.addItem('Sensors @ pin 0 to pin 7')
comboBox.addItem('Sensors @ pin 9 to pin 16')
comboBox.addItem('Sensors @ pin 17 to pin 24')
comboBox.addItem('Sensors @ pin 26 to pin 33')
proxy.setWidget(comboBox)
p9 = win.addLayout()
p9.addItem(proxy,row=3,col=1)

curve1 = p1.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve2 = p2.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve3 = p3.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve4 = p4.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve5 = p5.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve6 = p6.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve7 = p7.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve8 = p8.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))

# rewrite this for an iterative init?
p1.setLabel('left', "Resistance", units='Ohm')
p1.setRange(yRange=[0, 1550])
p1.setRange(xRange=[0, 100])
p1.showGrid(x=True, y=True)

p2.setLabel('left', "Resistance", units='Ohm')
p2.setRange(yRange=[0, 1550])
p2.setRange(xRange=[0, 100])
p2.showGrid(x=True, y=True)

p3.setLabel('left', "Resistance", units='Ohm')
p3.setRange(yRange=[0, 1550])
p3.setRange(xRange=[0, 100])
p3.showGrid(x=True, y=True)

p4.setLabel('left', "Resistance", units='Ohm')
p4.setRange(yRange=[0,1550])
p4.setRange(xRange=[0,100])
p4.showGrid(x=True, y=True)

p5.setLabel('left', "Resistance", units='Ohm')
p5.setRange(yRange=[0,1550])
p5.setRange(xRange=[0,100])
p5.showGrid(x=True, y=True)

p6.setLabel('left', "Resistance", units='Ohm')
p6.setRange(yRange=[0,1550])
p6.setRange(xRange=[0,100])
p6.showGrid(x=True, y=True)

p7.setLabel('left', "Resistance", units='Ohm')
p7.setRange(yRange=[0,1550])
p7.setRange(xRange=[0,100])
p7.showGrid(x=True, y=True)

p8.setLabel('left', "Resistance", units='Ohm')
p8.setRange(yRange=[0, 1550])
p8.setRange(xRange=[0, 100])
p8.showGrid(x=True, y=True)

#data = [0]
ch0 = [0]
ch1 = [0]
ch2 = [0]
ch3 = [0]
ch4 = [0]
ch5 = [0]
ch6 = [0]
ch7 = [0]

## Set up an animated arrow and text that track the curve
curvePoint1 = pg.CurvePoint(curve1)
p1.addItem(curvePoint1)
text1 = pg.TextItem("text1", anchor=(0.75, 0.0), color='k')
text1.setParentItem(curvePoint1)
arrow1 = pg.ArrowItem(angle=270)
arrow1.setParentItem(curvePoint1)

curvePoint2 = pg.CurvePoint(curve2)
p2.addItem(curvePoint2)
text2 = pg.TextItem("text2", anchor=(0.75, 0.0), color='k')
text2.setParentItem(curvePoint2)
arrow2 = pg.ArrowItem(angle=270)
arrow2.setParentItem(curvePoint2)

curvePoint3 = pg.CurvePoint(curve3)
p3.addItem(curvePoint3)
text3 = pg.TextItem("text3", anchor=(0.75, 0.0), color='k')
text3.setParentItem(curvePoint3)
arrow3 = pg.ArrowItem(angle=270)
arrow3.setParentItem(curvePoint3)

curvePoint4 = pg.CurvePoint(curve4)
p4.addItem(curvePoint4)
text4 = pg.TextItem("text4", anchor=(0.75, 0.0), color='k')
text4.setParentItem(curvePoint4)
arrow4 = pg.ArrowItem(angle=270)
arrow4.setParentItem(curvePoint4)

curvePoint5 = pg.CurvePoint(curve5)
p5.addItem(curvePoint5)
text5 = pg.TextItem("text5", anchor=(0.75, 0.0), color='k')
text5.setParentItem(curvePoint5)
arrow5 = pg.ArrowItem(angle=270)
arrow5.setParentItem(curvePoint5)

if (sensor_block_used == 0 or sensor_block_used == 1):
    curvePoint6 = pg.CurvePoint(curve6)
    p6.addItem(curvePoint6)
    text6 = pg.TextItem("text6", anchor=(0.75, 0.0), color='k')
    text6.setParentItem(curvePoint6)
    arrow6 = pg.ArrowItem(angle=270)
    arrow6.setParentItem(curvePoint6)

    curvePoint7 = pg.CurvePoint(curve7)
    p7.addItem(curvePoint7)
    text7 = pg.TextItem("text7", anchor=(0.75, 0.0), color='k')
    text7.setParentItem(curvePoint7)
    arrow7 = pg.ArrowItem(angle=270)
    arrow7.setParentItem(curvePoint7)

    curvePoint8 = pg.CurvePoint(curve8)
    p8.addItem(curvePoint8)
    text8 = pg.TextItem("text2", anchor=(0.75, 0.0), color='k')
    text8.setParentItem(curvePoint8)
    arrow8 = pg.ArrowItem(angle=270)
    arrow8.setParentItem(curvePoint8)

isFirstInit = False

raw = serial.Serial(com_port_string, baud_rate, timeout = com_timeout)
#raw=serial.Serial('/dev/cu.usbmodem1411', baudrate=1000000, timeout = 3.0)

ptr = 0

#Opening of the CSV File	
resistance_file_name = file_string+file_name+'.csv'
resistance_file = open(resistance_file_name, 'w')
resistance_file.write("Time Stamp")
resistance_file.write(",")
#resistance_file.write("sensorPin0")
#resistance_file.write(",")
#resistance_file.write("sensorPin1")
#resistance_file.write(",")
resistance_file.write("sensorPin2")
resistance_file.write(",")
resistance_file.write("sensorPin3")
resistance_file.write(",")
resistance_file.write("sensorPin4")
resistance_file.write(",")
resistance_file.write("sensorPin6")
resistance_file.write(",")
resistance_file.write("sensorPin7")
resistance_file.write(",")
resistance_file.write("sensorPin9")
resistance_file.write(",")
resistance_file.write("sensorPin10")
resistance_file.write(",")
resistance_file.write("sensorPin11")
resistance_file.write(",")
resistance_file.write("sensorPin12")
resistance_file.write(",")
resistance_file.write("sensorPin13")
resistance_file.write(",")
#resistance_file.write("sensorPin14")
#resistance_file.write(",")
#resistance_file.write("sensorPin15")
#resistance_file.write(",")
#resistance_file.write("sensorPin16")
#resistance_file.write(",")
#resistance_file.write("sensorPin17")
#resistance_file.write(",")
#resistance_file.write("sensorPin18")
#resistance_file.write(",")
resistance_file.write("sensorPin19")
resistance_file.write(",")
resistance_file.write("sensorPin20")
resistance_file.write(",")
resistance_file.write("sensorPin21")
resistance_file.write(",")
resistance_file.write("sensorPin22")
resistance_file.write(",")
resistance_file.write("sensorPin23")
resistance_file.write(",")
resistance_file.write("sensorPin24")
resistance_file.write(",")
resistance_file.write("sensorPin26")
resistance_file.write(",")
resistance_file.write("sensorPin27")
resistance_file.write(",")
resistance_file.write("sensorPin28")
resistance_file.write(",")
resistance_file.write("sensorPin29")
resistance_file.write(",")
resistance_file.write("sensorPin30")
resistance_file.write(",")
resistance_file.write("sensorPin31")
#resistance_file.write(",")
#resistance_file.write("sensorPin32")
#resistance_file.write(",")
#resistance_file.write("sensorPin33")
resistance_file.write("\n")

def exit_handler():
    global resistance_file
    resistance_file.close()
    print("closed")
    #print 'My application is ending!'
atexit.register(exit_handler)
#-------------------------------------Plotting Function-----------------------------------
def update1():
    global curve1, curve2, curve3, curve4, curve5, curve6, curve7, curve8, \
           curvePoint1, curvePoint2, curvePoint3, curvePoint4, curvePoint5, curvePoint6, \
           curvePoint7, curvePoint8, \
           ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7,\
           ptr, sensor_block_used

    try:
    	serialLine = raw.readline()
    except serial.SerialException as e:
	    print(str(e))

    bytesToString = serialLine.decode()
    resistanceArray = bytesToString.split(',')

    if sensor_block_used == 0: # SENSORS @ PINS 4-15
        ch0.append(float(resistanceArray[sensorPin0]))
        ch1.append(float(resistanceArray[sensorPin1]))
        ch2.append(float(resistanceArray[sensorPin2]))
        ch3.append(float(resistanceArray[sensorPin3]))
        ch4.append(float(resistanceArray[sensorPin4]))
        ch5.append(float(resistanceArray[sensorPin5]))
        ch6.append(float(resistanceArray[sensorPin6]))
        ch7.append(float(resistanceArray[sensorPin7]))

    elif sensor_block_used == 1: # SENSORS @ PINS 16-27
        ch0.append(float(resistanceArray[sensorPin9]))
        ch1.append(float(resistanceArray[sensorPin10]))
        ch2.append(float(resistanceArray[sensorPin11]))
        ch3.append(float(resistanceArray[sensorPin12])) 
        ch4.append(float(resistanceArray[sensorPin13]))
        ch5.append(float(resistanceArray[sensorPin14]))
        ch6.append(float(resistanceArray[sensorPin15]))
        ch7.append(float(resistanceArray[sensorPin16]))

    elif sensor_block_used == 2: # SENSORS @ PINS 28-33
        ch0.append(float(resistanceArray[sensorPin17]))
        ch1.append(float(resistanceArray[sensorPin18]))
        ch2.append(float(resistanceArray[sensorPin19]))
        ch3.append(float(resistanceArray[sensorPin20]))
        ch4.append(float(resistanceArray[sensorPin21]))
        ch5.append(float(resistanceArray[sensorPin22]))
        ch6.append(float(resistanceArray[sensorPin23]))
        ch7.append(float(resistanceArray[sensorPin24]))

    elif sensor_block_used == 3: # SENSORS @ PINS 28-33
        ch0.append(float(resistanceArray[sensorPin26]))
        ch1.append(float(resistanceArray[sensorPin27]))
        ch2.append(float(resistanceArray[sensorPin28]))
        ch3.append(float(resistanceArray[sensorPin29]))
        ch4.append(float(resistanceArray[sensorPin30]))
        ch5.append(float(resistanceArray[sensorPin31]))
        ch6.append(float(resistanceArray[sensorPin32]))
        ch7.append(float(resistanceArray[sensorPin33]))


    if ptr != 0:
        ch0[:-1] = ch0[1:]
        ch1[:-1] = ch1[1:]
        ch2[:-1] = ch2[1:]
        ch3[:-1] = ch3[1:]
        ch4[:-1] = ch4[1:]  
        ch5[:-1] = ch5[1:]
        ch6[:-1] = ch6[1:]
        ch7[:-1] = ch7[1:]


    if ptr <= 100:
        curvePoint1.setPos(ptr)
        curvePoint2.setPos(ptr)
        curvePoint3.setPos(ptr)
        curvePoint4.setPos(ptr)
        curvePoint5.setPos(ptr)
        curvePoint6.setPos(ptr)
        curvePoint7.setPos(ptr)
        curvePoint8.setPos(ptr)
    else:
        curvePoint1.setPos(100)
        curvePoint2.setPos(100)
        curvePoint3.setPos(100)
        curvePoint4.setPos(100)
        curvePoint5.setPos(100)
        curvePoint6.setPos(100)
        curvePoint7.setPos(100)
        curvePoint8.setPos(100)

        ch0.pop(0)
        ch1.pop(0)
        ch2.pop(0)
        ch3.pop(0)
        ch4.pop(0)
        ch5.pop(0)
        ch6.pop(0)
        ch7.pop(0)

    text1.setText('[%0.1f]' % (ch0[-1]))
    text2.setText('[%0.1f]' % (ch1[-1]))
    text3.setText('[%0.1f]' % (ch2[-1]))
    text4.setText('[%0.1f]' % (ch3[-1]))
    text5.setText('[%0.1f]' % (ch4[-1]))
    text6.setText('[%0.1f]' % (ch5[-1]))
    text7.setText('[%0.1f]' % (ch6[-1]))
    text8.setText('[%0.1f]' % (ch7[-1]))

    ptr += 1

    curve1.setData(ch0)
    curve2.setData(ch1)
    curve3.setData(ch2)
    curve4.setData(ch3)
    curve5.setData(ch4)
    curve6.setData(ch5)
    curve7.setData(ch6)
    curve8.setData(ch7)
    #curve.setPos(ptr, 0)

    #app.processEvents()


#iteration flag	
flag = 0
#LUT value
displayvalue = 0

#-------------------------------------Calculation and Logging Function-----------------------------------
def update2():
    global displayvalue, flag
    
    serialLine = raw.readline()

    bytesToString2 = serialLine.decode()
    resistanceArrayLog = bytesToString2.split(',')

    now = time.strftime('%d-%m-%Y %H:%M:%S')
    resistance_file.write(now)
    resistance_file.write(",")
    # Displayed Channels
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin0])))
    #resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin1])))
    #resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin2])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin3])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin4])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin5])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin6])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin7])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin9])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin10])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin11])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin12])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin13])))
    resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin14])))
    #resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin15])))
    #resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin16])))
    #resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin17])))
    #resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin18])))
    #resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin19])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin20])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin21])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin22])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin23])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin24])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin26])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin27])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin28])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin29])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin30])))
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin31])))
    #resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin32])))
    #resistance_file.write(",")
    #resistance_file.write(str(float(resistanceArrayLog[sensorPin33])))
    resistance_file.write("\n")

def update():
    update1()
    update2()
    
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0)


if __name__ == '__main__':
    import sys
    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        QtGui.QApplication.instance().exec_()
