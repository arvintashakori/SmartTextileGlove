from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time
import serial
import time

# *** Which sensor block do you want to plot? ***
# Set SENSOR_BLOCK_USED = 1 for sensors @ pin 4 to pin 15
# Set SENSOR_BLOCK_USED = 2 for sensors @ pin 16 to pin 27
# Set SENSOR_BLOCK_USED = 3 for sensors @ pin 28 to pin 33
SENSOR_BLOCK_USED = 1

# MAPPING PCB SENSORS TO SERIAL OUTPUT INDECES (0 means there's currently no sensor at the PCB pin):
#sensorPin0 = 0
#sensorPin1 = 0
#sensorPin2 = 0
#sensorPin3 = 0
sensorPin4 = 5    # F7
sensorPin5 = 8    # F6
sensorPin6 = 17   # F5
#sensorPin7 = 0
#sensorPin8 = 0
#sensorPin9 = 0
sensorPin10 = 16  # E7
sensorPin11 = 22  # E6
sensorPin12 = 19  # E5
#sensorPin13 = 0
sensorPin14 = 13  # D7
sensorPin15 = 11  # D6
sensorPin16 = 2   # D5
sensorPin17 = 21  # G3
#sensorPin18 = 0
sensorPin19 = 15  # F4
sensorPin20 = 6   # F3
sensorPin21 = 3   # F2
#sensorPin22 = 0
sensorPin23 = 0   # D3
sensorPin24 = 9   # D2
#sensorPin25 = 0
#sensorPin26 = 0
sensorPin27 = 7   # C4
sensorPin28 = 4   # C3
sensorPin29 = 1   # C2
sensorPin30 = 10  # C1
#sensorPin31 = 0
sensorPin32 = 12  # A4
sensorPin33 = 18  # A3

# Change this file path to your directory!!!!!!!! make sure the six pictures are in the same directory
#Prints the csv file to this directory too
#file_string = 'C:\\Users\\Admin\\Documents\\PythonVisual\\'
file_string = '/Users/reendael/Documents/PythonVisual/'
file_name = str(input("Please Enter the File Name: "))
print("\n")

app = QtGui.QApplication([])
pg.setConfigOptions(antialias=True, background='w', foreground='k')

win = pg.GraphicsWindow(title="Resistance Plotting")
win.resize(1600, 1000)
win.setWindowTitle('pyqtgraph example: Plotting')

pg.setConfigOptions(antialias=True)

if (SENSOR_BLOCK_USED == 1): # SENSORS @ PINS 4-15
    p1 = win.addPlot(title="Sensor @ pin 4 (F7)")
    p2 = win.addPlot(title="Sensor @ pin 5 (F6)")
    p3 = win.addPlot(title="Sensor @ pin 6 (F5)")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 10 (E7)")
    p5 = win.addPlot(title="Sensor @ pin 11 (E6)")
    p6 = win.addPlot(title="Sensor @ pin 12 (E5)")
    win.nextRow()
    p7 = win.addPlot(title="Sensor @ pin 14 (D7)")
    p8 = win.addPlot(title="Sensor @ pin 15 (D6)")

elif (SENSOR_BLOCK_USED == 2): # SENSORS @ PINS 16-27
    p1 = win.addPlot(title="Sensor @ pin 16 (D5)")
    p2 = win.addPlot(title="Sensor @ pin 17 (F6)")
    p3 = win.addPlot(title="Sensor @ pin 19 (F5)")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 20 (F3)")
    p5 = win.addPlot(title="Sensor @ pin 21 (F2)")
    p6 = win.addPlot(title="Sensor @ pin 23 (D3)")
    win.nextRow()
    p7 = win.addPlot(title="Sensor @ pin 24 (D2)")
    p8 = win.addPlot(title="Sensor @ pin 27 (C4)")

elif (SENSOR_BLOCK_USED == 3): # SENSORS @ PINS 28-33
    p1 = win.addPlot(title="Sensor @ pin 28 (C3)")
    p2 = win.addPlot(title="Sensor @ pin 29 (C2)")
    p3 = win.addPlot(title="Sensor @ pin 30 (C1)")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 32 (A4)")
    p5 = win.addPlot(title="Sensor @ pin 33 (A3)")



#--------------------------------------------------Gesture Recognition Window---------------------------------------------#

        
'''window = QtGui.QMainWindow()
#x,y,width, height
window.setGeometry(200, 200, 900, 700)

pic = QtGui.QLabel(window)
pic.setGeometry(50, 50, 800, 600)
#use full ABSOLUTE path to the image, not relative
pic.setPixmap(QtGui.QPixmap(file_string+'piczero.jpg'))
#Default Picture
window.show()'''

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
p1.setRange(yRange=[0, 320])
p1.setRange(xRange=[0, 100])
p1.showGrid(x=True, y=True)

p2.setLabel('left', "Resistance", units='Ohm')
p2.setRange(yRange=[0, 320])
p2.setRange(xRange=[0, 100])
p2.showGrid(x=True, y=True)

p3.setLabel('left', "Resistance", units='Ohm')
p3.setRange(yRange=[0, 320])
p3.setRange(xRange=[0, 100])
p3.showGrid(x=True, y=True)

p4.setLabel('left', "Resistance", units='Ohm')
p4.setRange(yRange=[0,320])
p4.setRange(xRange=[0,100])
p4.showGrid(x=True, y=True)

p5.setLabel('left', "Resistance", units='Ohm')
p5.setRange(yRange=[0,320])
p5.setRange(xRange=[0,100])
p5.showGrid(x=True, y=True)

p6.setLabel('left', "Resistance", units='Ohm')
p6.setRange(yRange=[0,320])
p6.setRange(xRange=[0,100])
p6.showGrid(x=True, y=True)

p7.setLabel('left', "Resistance", units='Ohm')
p7.setRange(yRange=[0,320])
p7.setRange(xRange=[0,100])
p7.showGrid(x=True, y=True)

p8.setLabel('left', "Resistance", units='Ohm')
p8.setRange(yRange=[0, 320])
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

sensor1 = [0]
sensor2 = [0]
sensor4 = [0]
sensor6 = [0]
sensor9 = [0]
sensor10 = [0]
sensor11 = [0]
sensor12 = [0]

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

#raw = serial.Serial('COM4', 1000000)
raw=serial.Serial('/dev/cu.usbmodem1411', baudrate=1000000, timeout = 3.0)

ptr = 0

#-------------------------------------Plotting Function-----------------------------------
def update1():
    global curve1, curve2, curve3, curve4, curve5, curve6, curve7, curve8, \
           curvePoint1, curvePoint2, curvePoint3, curvePoint4, curvePoint5, curvePoint6, \
           curvePoint7, curvePoint8, \
           ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7,\
           ptr
    serialLine = raw.readline()
    bytesToString = serialLine.decode()
    voltageAndLog = bytesToString.split(',')

if (SENSOR_BLOCK_USED == 1): # SENSORS @ PINS 4-15
    ch0.append(float(voltageAndLog[sensorPin4]))
    ch1.append(float(voltageAndLog[sensorPin5]))
    ch2.append(float(voltageAndLog[sensorPin6]))
    ch3.append(float(voltageAndLog[sensorPin10]))
    ch4.append(float(voltageAndLog[sensorPin11]))
    ch5.append(float(voltageAndLog[sensorPin12]))
    ch6.append(float(voltageAndLog[sensorPin14]))
    ch7.append(float(voltageAndLog[sensorPin15]))

elif (SENSOR_BLOCK_USED == 2): # SENSORS @ PINS 16-27
    ch0.append(float(voltageAndLog[sensorPin16]))
    ch1.append(float(voltageAndLog[sensorPin17]))
    ch2.append(float(voltageAndLog[sensorPin19]))
    ch3.append(float(voltageAndLog[sensorPin20]))
    ch4.append(float(voltageAndLog[sensorPin21]))
    ch5.append(float(voltageAndLog[sensorPin23]))
    ch6.append(float(voltageAndLog[sensorPin24]))
    ch7.append(float(voltageAndLog[sensorPin27]))

elif (SENSOR_BLOCK_USED == 3): # SENSORS @ PINS 28-33
    ch0.append(float(voltageAndLog[sensorPin28]))
    ch1.append(float(voltageAndLog[sensorPin29]))
    ch2.append(float(voltageAndLog[sensorPin30]))
    ch3.append(float(voltageAndLog[sensorPin32]))
    ch4.append(float(voltageAndLog[sensorPin33]))


    if ptr != 0:
        ch0[:-1] = ch0[1:]
        ch1[:-1] = ch1[1:]
        ch2[:-1] = ch2[1:]
        ch3[:-1] = ch3[1:]
        ch4[:-1] = ch4[1:]  
        if (SENSOR_BLOCK_USED == 1 or SENSOR_BLOCK_USED == 2):
            ch5[:-1] = ch5[1:]  # COMMENT OUT IF SENSORS @ PINS 28-33 ARE DISPLAYED
            ch6[:-1] = ch6[1:]  # COMMENT OUT IF SENSORS @ PINS 28-33 ARE DISPLAYED
            ch7[:-1] = ch7[1:]  # COMMENT OUT IF SENSORS @ PINS 28-33 ARE DISPLAYED


    if ptr <= 100:
        curvePoint1.setPos(ptr)
        curvePoint2.setPos(ptr)
        curvePoint3.setPos(ptr)
        curvePoint4.setPos(ptr)
        curvePoint5.setPos(ptr)
        if (SENSOR_BLOCK_USED == 1 or SENSOR_BLOCK_USED == 2):
            curvePoint6.setPos(ptr) # COMMENT OUT IF SENSORS @ PINS 28-33 ARE DISPLAYED
            curvePoint7.setPos(ptr) # COMMENT OUT IF SENSORS @ PINS 28-33 ARE DISPLAYED
            curvePoint8.setPos(ptr) # COMMENT OUT IF SENSORS @ PINS 28-33 ARE DISPLAYED
    else:
        curvePoint1.setPos(100)
        curvePoint2.setPos(100)
        curvePoint3.setPos(100)
        curvePoint4.setPos(100)
        curvePoint5.setPos(100)
        if (SENSOR_BLOCK_USED == 1 or SENSOR_BLOCK_USED == 2):
            curvePoint6.setPos(100)
            curvePoint7.setPos(100)
            curvePoint8.setPos(100)
        ch0.pop(0)
        ch1.pop(0)
        ch2.pop(0)
        ch3.pop(0)
        ch4.pop(0)
        if (SENSOR_BLOCK_USED == 1 or SENSOR_BLOCK_USED == 2):
            ch5.pop(0)
            ch6.pop(0)
            ch7.pop(0)

    text1.setText('[%0.1f]' % (ch0[-1]))
    text2.setText('[%0.1f]' % (ch1[-1]))
    text3.setText('[%0.1f]' % (ch2[-1]))
    text4.setText('[%0.1f]' % (ch3[-1]))
    text5.setText('[%0.1f]' % (ch4[-1]))
    if (SENSOR_BLOCK_USED == 1 or SENSOR_BLOCK_USED == 2):
        text6.setText('[%0.1f]' % (ch5[-1]))
        text7.setText('[%0.1f]' % (ch6[-1]))
        text8.setText('[%0.1f]' % (ch7[-1]))

    ptr += 1

    curve1.setData(ch0)
    curve2.setData(ch1)
    curve3.setData(ch2)
    curve4.setData(ch3)
    curve5.setData(ch4)
    if (SENSOR_BLOCK_USED == 1 or SENSOR_BLOCK_USED == 2):
        curve6.setData(ch5)
        curve7.setData(ch6)
        curve8.setData(ch7)
    #curve.setPos(ptr, 0)

    #app.processEvents()

#Opening of the CSV File	
resistance_file_name = file_string+file_name+'.csv'
resistance_file = open(resistance_file_name, 'w')
resistance_file.write("Time Stamp")
resistance_file.write(",")
# Displayed channels
resistance_file.write("sensorPin4")
resistance_file.write(",")
resistance_file.write("sensorPin5")
resistance_file.write(",")
resistance_file.write("sensorPin6")
resistance_file.write(",")
resistance_file.write("sensorPin10")
resistance_file.write(",")
resistance_file.write("sensorPin11")
resistance_file.write(",")
resistance_file.write("sensorPin12")
resistance_file.write(",")
resistance_file.write("sensorPin14")
resistance_file.write(",")
resistance_file.write("sensorPin15")
resistance_file.write(",")
# Remaining Channels which are not displayed
resistance_file.write("sensorPin16")
resistance_file.write(",")
resistance_file.write("sensorPin17")
resistance_file.write(",")
resistance_file.write("sensorPin19")
resistance_file.write(",")
resistance_file.write("sensorPin20")
resistance_file.write(",")
resistance_file.write("sensorPin21")
resistance_file.write(",")
resistance_file.write("sensorPin23")
resistance_file.write(",")
resistance_file.write("sensorPin24")
resistance_file.write(",")
resistance_file.write("sensorPin27")
resistance_file.write(",")
resistance_file.write("sensorPin28")
resistance_file.write(",")
resistance_file.write("sensorPin29")
resistance_file.write(",")
resistance_file.write("sensorPin30")
resistance_file.write(",")
resistance_file.write("sensorPin32")
resistance_file.write(",")
resistance_file.write("sensorPin33")

resistance_file.write("\n")

#iteration flag	
flag = 0
#LUT value
displayvalue = 0

#-------------------------------------Calculation and Logging Function-----------------------------------
def update2():
    global displayvalue, flag
    
    serialLine = raw.readline()

    bytesToString2 = serialLine.decode()
    resistanceValues = bytesToString2.split(',')

    now = time.strftime('%d-%m-%Y %H:%M:%S')
    resistance_file.write(now)
    resistance_file.write(",")
    # Displayed Channels
    resistance_file.write(resistanceValues[sensorPin4])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin5])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin6])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin10])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin11])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin12])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin14])
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceValues[sensorPin15])))
    resistance_file.write(",")
    # Remaining Channels which are not displayed
    resistance_file.write(resistanceValues[sensorPin15])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin16])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin17])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin19])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin20])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin21])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin23])
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceValues[sensorPin24])))
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin27])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin28])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin29])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin30])
    resistance_file.write(",")
    resistance_file.write(resistanceValues[sensorPin32])
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceValues[sensorPin33])))
        
    flag +=1 	
    
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
