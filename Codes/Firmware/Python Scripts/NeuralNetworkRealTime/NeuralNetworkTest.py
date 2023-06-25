import serial
import numpy as np

# IF THE SCRIPT COMPLAINS ABOUT THE COM PORT, YOU CAN CHANGE IT HERE (go to Device Manager->Ports->find J-Link and use associated port)
raw = serial.Serial('COM4', 1000000)
# raw=serial.Serial('\dev\cu.usbmodem1421', 1000000)
# ------------------------------------------------------Sensor Variables--------------------------------------------------#
# Row 1
sensor1a = 0.0
sensor1b = 0.0
sensor1c = 0.0
sensor1d = 0.0

# Row 2
sensor2a = 0.0
sensor2b = 0.0
sensor2c = 0.0
sensor2d = 0.0

# Row 3
sensor3a = 0.0
sensor3b = 0.0
sensor3c = 0.0
sensor3d = 0.0
sensor3e = 0.0

# Row 4
sensor4a = 0.0
sensor4b = 0.0
sensor4c = 0.0

# Row 5
sensor5a = 0.0
sensor5b = 0.0
sensor5c = 0.0
sensor5d = 0.0

# Row 6
sensor6a = 0.0
sensor6b = 0.0
sensor6c = 0.0
sensor6d = 0.0


#variables for the linear equations
x = zeros(1,24)
y = zeros(1,6)
W1 = zeros(24,10)
W2 = zeros(10,10)
b1 = 1/10*(np.ones(1,10))
b2 = 1/10*(np.ones(1,10))
a1 = zeros(1,10)
a2 = zeros(1,10)
z1 = zeros(1,10)
z2 = zeros(1,10)

def neuralNetwork ():
    serialLine = raw.readline()
    resistanceValues = serialLine.split(',')
    global sensor1a, sensor1b, sensor1c, sensor1d, sensor2a, sensor2b, sensor2c, sensor2d, \
        sensor3a, sensor3b, sensor3c, sensor3d, sensor3e, sensor4a, sensor4b, sensor4c, \
        sensor5a, sensor5b, sensor5c, sensor5d, sensor6a, sensor6b, sensor6c, sensor6d

    # THIS IS WHERE THE SENSORS ARE MAPPED ACCORDING TO THE PDF GUIDE
    # Row 1
    sensor1a = float(resistanceValues[8])
    sensor1b = float(resistanceValues[5])
    sensor1c = float(resistanceValues[2])
    sensor1d = float(resistanceValues[11])
    # Row 2
    sensor2a = float(resistanceValues[14])
    sensor2b = float(resistanceValues[20])
    sensor2c = float(resistanceValues[23])
    sensor2d = float(resistanceValues[17])
    # Row 3
    sensor3a = float(resistanceValues[7])
    sensor3b = float(resistanceValues[4])
    sensor3c = float(resistanceValues[1])
    sensor3d = float(resistanceValues[10])
    sensor3e = float(resistanceValues[16])
    # Row 4
    sensor4a = float(resistanceValues[13])
    sensor4b = float(resistanceValues[19])
    sensor4c = float(resistanceValues[22])
    # Row 5
    sensor5a = float(resistanceValues[6])
    sensor5b = float(resistanceValues[3])
    sensor5c = float(resistanceValues[0])
    sensor5d = float(resistanceValues[9])
    # Row 6
    sensor6a = float(resistanceValues[12])
    sensor6b = float(resistanceValues[18])
    sensor6c = float(resistanceValues[21])
    sensor6d = float(resistanceValues[15])

    #Inputs
    x = [sensor1a, sensor1b, sensor1c, sensor1d, sensor2a, sensor2b, sensor2c,sensor2d,sensor3a,sensor3b,sensor3c,sensor3d, sensor3e,sensor4a,sensor4b,sensor4c,sensor5a,sensor5b,sensor5c,sensor5d,sensor6a,sensor6b,sensor6c,sensor6d]
    #Outputs for the new thresholds
    #TO DO populate with the new thresholds and teh assiociated outputs for all possible maps for the gestures 0,1s only in y
    #for example:
    if (sensor1d > 179.8 and sensor3c > 171.247 and sensor4 > 79.418 and sensor > 157.3):
        y = [ , , , , , , , , , ,]
    #Continue this for however many output combinations there are....    
    elif():
        y =  [ , , , , , , , , , ,]
    elif():
        y =  [ , , , , , , , , , ,]
    elif():
        y =  [ , , , , , , , , , ,]
    elif():
        y =  [ , , , , , , , , , ,]    
    else: 
        y = [0,0,0,0,0,0,0,0,0,0]    

    #Now that we have our training data we can use the back propagation algorithm to find the weights of the hidden layers
    
    #W1 is a 24x10 matrix
    #TO DO FILL IN THESE VALUES FROM THE BACK PROPAGATION ALGORITHM
    #Use #np.random.randn(y, x) for each value in the matrix
    W1 = [, , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,;
        , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,;
        , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,]
    #W2 is a 10x10 matrix
    #TO DO FILL IN THESE VALUES FROM THE BACK PROPAGATION ALGORITHM
    W2 = [, , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,; , , , , , , , , , ,]

    #Now that we have our inputs and our experimental weights we can find the true output of the experimental data
    z1 = np.dot(x,W1)+b1
    a1 = np.tanh(z1)
    z2 = np.dot(a1,W2)+b2
    a2 = e ^ z2 / sum(e ^ z2) #softmax(z2)

    print(a2)


    #Here we check that our experimental map of outputs matches (test data) our training data 
    #TO DO FIND THE MAP FROM EXPERIMENTAL DATA AND USE THESE AS COMPARISON TO THE OUTPUT FROM THE READER
    y1 = [, , , , , , , , , ,] #for fist
    y2 = [, , , , , , , , , ,] #for point
    y3 = [, , , , , , , , , ,] #for pinch
    y4 = [, , , , , , , , , ,] #for touch
    y5 = [, , , , , , , , , ,] #for thumbs up
    if (a2 == y1)
        print("The Gesture is a Fist")
    elif (a2 == y2)
        print("The Gesture is a Point")
    elif (a2 == y3)
        print("The Gesture is a Pinch")
    elif (a2 == y4):
        print ("The Gesture is a Touch")
    elif (a2 == y5)
        print("The Gesture is a Thumbs Up")    
    else:
        print("There is no gesture occuring at this time")    