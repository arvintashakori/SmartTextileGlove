from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import pyqtgraph as pg
from pyqtgraph.ptime import time
import serial
import atexit
import time

# *** Which sensor block do you want to plot? ***
# Set SENSOR_BLOCK_USED = 0 for sensors @ pin 4 to pin 15
# Set SENSOR_BLOCK_USED = 1 for sensors @ pin 16 to pin 27
# Set SENSOR_BLOCK_USED = 2 for sensors @ pin 28 to pin 33
SENSOR_BLOCK_USED = 0

isFirstInit = True


# MAPPING PCB SENSORS TO SERIAL OUTPUT INDECES (0 means there's currently no sensor at the PCB pin):
sensorPin0 = 12 
sensorPin1 = 21
sensorPin2 = 18
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
sensorPin17 = 23  # G3  remapped for conn 6
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
sensorPin32 = 14  # A4     remapped for conn 6
sensorPin33 = 20  # A3     remapped for conn 6


# Change this file path to your directory!!!!!!!! make sure the six pictures are in the same directory
#Prints the csv file to this directory too
#file_string = 'C:\\Documents\\GitHub\\texavie\\Python Scripts\\PythonGrapher\\'
file_string = 'C:\\Users\\Admin\\Documents\\GitHub\\texavie\\Python Scripts\\PythonGrapher'
#file_string = '/Users/reendael/Documents/PythonVisual/'
file_name = str(input("Please Enter the File Name: "))
print("\n")





#app = QtGui.QApplication([])
pg.setConfigOptions(antialias=True, background='w', foreground='k')

win = pg.GraphicsWindow(title="Resistance Plotting")
win.resize(1600, 1000)
win.setWindowTitle('pyqtgraph example: Plotting')

pg.setConfigOptions(antialias=True)

if (SENSOR_BLOCK_USED == 0): # SENSORS @ PINS 4-15
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
    #p9 = win.addLayout(row=3, col=2)

elif (SENSOR_BLOCK_USED == 1): # SENSORS @ PINS 16-27
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

elif (SENSOR_BLOCK_USED == 2): # SENSORS @ PINS 28-33
    p1 = win.addPlot(title="Sensor @ pin 28 (C3)")
    p2 = win.addPlot(title="Sensor @ pin 29 (C2)")
    p3 = win.addPlot(title="Sensor @ pin 30 (C1)")
    win.nextRow()
    p4 = win.addPlot(title="Sensor @ pin 32 (A4)")
    p5 = win.addPlot(title="Sensor @ pin 33 (A3)")

def selectionchange (i):
    global SENSOR_BLOCK_USED, ptr, curve1, curve2, curve3, curve4, curve5, curve6, curve7, \
           ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7, isFirstInit
    SENSOR_BLOCK_USED = i
    if SENSOR_BLOCK_USED == 0:
        p1.setTitle('Conn4 Pin1')
        p2.setTitle('Conn4 Pin2')
        p3.setTitle('Conn4 Pin3')
        p4.setTitle('Conn4 Pin4')
        p5.setTitle('Conn5 Pin1')
        p6.setTitle('Conn5 Pin2')
        p7.setTitle('Conn5 Pin3')
        p8.setTitle('Conn5 Pin4')
    elif SENSOR_BLOCK_USED == 1:
        p1.setTitle('Conn6 Pin1')
        p2.setTitle('Conn6 Pin2')
        p3.setTitle('Conn6 Pin3')
        p4.setTitle('Conn6 Pin4')
        p5.setTitle('NA')
        p6.setTitle('NA')
        p7.setTitle('NA')
        p8.setTitle('NA')
    elif SENSOR_BLOCK_USED == 2:
        p1.setTitle('Sensor @ pin 28 (C3)')
        p2.setTitle('Sensor @ pin 29 (C2)')
        p3.setTitle('Sensor @ pin 30 (C1)')
        p4.setTitle('Sensor @ pin 32 (A4)')
        p5.setTitle('Sensor @ pin 33 (A3)')
        p6.setTitle('NOT USED')
        p7.setTitle('NOT USED')
        p8.setTitle('NOT USED')
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
comboBox.addItem('Sensors @ pin 4 to pin 15')
comboBox.addItem('Sensors @ pin 16 to pin 27')
comboBox.addItem('Sensors @ pin 28 to pin 33')
proxy.setWidget(comboBox)
p9 = win.addLayout()
p9.addItem(proxy,row=3,col=1)

curve1 = p1.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve2 = p2.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve3 = p3.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve4 = p4.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
curve5 = p5.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))

if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
    curve6 = p6.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
    curve7 = p7.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))
    curve8 = p8.plot(np.random.normal(size=750), pen=pg.mkPen('b', width=3))

# rewrite this for an iterative init?
p1.setLabel('left', "Resistance", units='Ohm')
p1.setRange(yRange=[0, 550])
p1.setRange(xRange=[0, 100])
p1.showGrid(x=True, y=True)

p2.setLabel('left', "Resistance", units='Ohm')
p2.setRange(yRange=[0, 550])
p2.setRange(xRange=[0, 100])
p2.showGrid(x=True, y=True)

p3.setLabel('left', "Resistance", units='Ohm')
p3.setRange(yRange=[0, 550])
p3.setRange(xRange=[0, 100])
p3.showGrid(x=True, y=True)

p4.setLabel('left', "Resistance", units='Ohm')
p4.setRange(yRange=[0,550])
p4.setRange(xRange=[0,100])
p4.showGrid(x=True, y=True)

p5.setLabel('left', "Resistance", units='Ohm')
p5.setRange(yRange=[0,550])
p5.setRange(xRange=[0,100])
p5.showGrid(x=True, y=True)

if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
    p6.setLabel('left', "Resistance", units='Ohm')
    p6.setRange(yRange=[0,550])
    p6.setRange(xRange=[0,100])
    p6.showGrid(x=True, y=True)

    p7.setLabel('left', "Resistance", units='Ohm')
    p7.setRange(yRange=[0,550])
    p7.setRange(xRange=[0,100])
    p7.showGrid(x=True, y=True)

    p8.setLabel('left', "Resistance", units='Ohm')
    p8.setRange(yRange=[0, 550])
    p8.setRange(xRange=[0, 100])
    p8.showGrid(x=True, y=True)

#data = [0]
ch0 = [0]
ch1 = [0]
ch2 = [0]
ch3 = [0]
ch4 = [0]
if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
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

if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
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

raw = serial.Serial('COM4', 1000000)
#raw=serial.Serial('/dev/cu.usbmodem1411', baudrate=1000000, timeout = 3.0)

ptr = 0

#Opening of the CSV File	
resistance_file_name = file_string+file_name+'.csv'
resistance_file = open(resistance_file_name, 'w')
resistance_file.write("Time Stamp")
resistance_file.write(",")
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
           ptr, SENSOR_BLOCK_USED

    try:
    	serialLine = raw.readline()
    except serial.SerialException as e:
	    print(str(e))

    bytesToString = serialLine.decode()
    resistanceArray = bytesToString.split(',')

    if SENSOR_BLOCK_USED == 0: # SENSORS @ PINS 4-15
        ch0.append(float(resistanceArray[sensorPin32]))
        ch1.append(float(resistanceArray[sensorPin33]))
        ch2.append(float(resistanceArray[sensorPin17]))
        ch3.append(float(resistanceArray[sensorPin6]))
        ch4.append(float(resistanceArray[sensorPin24]))
        ch5.append(float(resistanceArray[sensorPin20]))#[6]
        ch6.append(float(resistanceArray[sensorPin21]))
        ch7.append(float(resistanceArray[sensorPin0]))#[12]

    elif SENSOR_BLOCK_USED == 1: # SENSORS @ PINS 16-27
        ch0.append(float(resistanceArray[sensorPin19]))
        ch1.append(float(resistanceArray[sensorPin1]))
        ch2.append(float(resistanceArray[sensorPin23]))
        ch3.append(float(resistanceArray[sensorPin2])) # *****
        ch4.append(float(resistanceArray[sensorPin17]))
        ch5.append(float(resistanceArray[sensorPin4]))
        ch6.append(float(resistanceArray[sensorPin24]))
        ch7.append(float(resistanceArray[sensorPin27]))

    elif SENSOR_BLOCK_USED == 2: # SENSORS @ PINS 28-33
        ch0.append(float(resistanceArray[sensorPin28]))
        ch1.append(float(resistanceArray[sensorPin29]))
        ch2.append(float(resistanceArray[sensorPin30]))
        ch3.append(float(resistanceArray[sensorPin32]))
        ch4.append(float(resistanceArray[sensorPin33]))


    if ptr != 0:
        ch0[:-1] = ch0[1:]
        ch1[:-1] = ch1[1:]
        ch2[:-1] = ch2[1:]
        ch3[:-1] = ch3[1:]
        ch4[:-1] = ch4[1:]  
        if SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1:
            ch5[:-1] = ch5[1:]
            ch6[:-1] = ch6[1:]
            ch7[:-1] = ch7[1:]


    if ptr <= 100:
        curvePoint1.setPos(ptr)
        curvePoint2.setPos(ptr)
        curvePoint3.setPos(ptr)
        curvePoint4.setPos(ptr)
        curvePoint5.setPos(ptr)
        if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
            curvePoint6.setPos(ptr)
            curvePoint7.setPos(ptr)
            curvePoint8.setPos(ptr)
    else:
        curvePoint1.setPos(100)
        curvePoint2.setPos(100)
        curvePoint3.setPos(100)
        curvePoint4.setPos(100)
        curvePoint5.setPos(100)
        if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
            curvePoint6.setPos(100)
            curvePoint7.setPos(100)
            curvePoint8.setPos(100)
        ch0.pop(0)
        ch1.pop(0)
        ch2.pop(0)
        ch3.pop(0)
        ch4.pop(0)
        if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
            ch5.pop(0)
            ch6.pop(0)
            ch7.pop(0)

    text1.setText('[%0.1f]' % (ch0[-1]))
    text2.setText('[%0.1f]' % (ch1[-1]))
    text3.setText('[%0.1f]' % (ch2[-1]))
    text4.setText('[%0.1f]' % (ch3[-1]))
    text5.setText('[%0.1f]' % (ch4[-1]))
    if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
        text6.setText('[%0.1f]' % (ch5[-1]))
        text7.setText('[%0.1f]' % (ch6[-1]))
        text8.setText('[%0.1f]' % (ch7[-1]))

    ptr += 1

    curve1.setData(ch0)
    curve2.setData(ch1)
    curve3.setData(ch2)
    curve4.setData(ch3)
    curve5.setData(ch4)
    if (SENSOR_BLOCK_USED == 0 or SENSOR_BLOCK_USED == 1):
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
    resistance_file.write(resistanceArrayLog[sensorPin4])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin5])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin6])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin10])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin11])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin12])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin14])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin15])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin16])
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin17])))
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin19])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin20])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin21])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin23])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin24])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin27])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin28])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin29])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin30])
    resistance_file.write(",")
    resistance_file.write(resistanceArrayLog[sensorPin32])
    resistance_file.write(",")
    resistance_file.write(str(float(resistanceArrayLog[sensorPin33])))
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
