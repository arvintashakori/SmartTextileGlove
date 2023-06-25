import socket
import pandas as pd
import numpy as np
import time
import scipy.signal as sps

class parameters:
    def __init__(self):
        self.file_name = 'Dataset/March19_2022/optitrack_for_unity_random12 (1).csv'
        self.frameRate = 20
        self.delay = 1
        self.ipAddress = '127.0.0.1'
        self.port = 8910
        self.output = ["Pinky MCP:X","Pinky MCP:Y","Pinky MCP:Z","Pinky MCP:W","Pinky PIP:X","Pinky PIP:Y","Pinky PIP:Z","Pinky PIP:W","Pinky DIP:X","Pinky DIP:Y","Pinky DIP:Z","Pinky DIP:W","Ring MCP:X","Ring MCP:Y","Ring MCP:Z","Ring MCP:W","Ring PIP:X","Ring PIP:Y","Ring PIP:Z","Ring PIP:W","Ring DIP:X","Ring DIP:Y","Ring DIP:Z","Ring DIP:W","Middle MCP:X","Middle MCP:Y","Middle MCP:Z","Middle MCP:W","Middle PIP:X","Middle PIP:Y","Middle PIP:Z","Middle PIP:W","Middle DIP:X","Middle DIP:Y","Middle DIP:Z","Middle DIP:W","Index MCP:X","Index MCP:Y","Index MCP:Z","Index MCP:W","Index PIP:X","Index PIP:Y","Index PIP:Z","Index PIP:W","Index DIP:X","Index DIP:Y","Index DIP:Z","Index DIP:W","Thumbs CMC:X","Thumbs CMC:Y","Thumbs CMC:Z","Thumbs CMC:W","Thumbs MCP:X","Thumbs MCP:Y","Thumbs MCP:Z","Thumbs MCP:W","Thumbs IP:X","Thumbs IP:Y","Thumbs IP:Z","Thumbs IP:W","Hand:X","Hand:Y","Hand:Z","Hand:W"]



def simulate():
    # loading parameters
    params = parameters()
    data = pd.read_csv(params.file_name)
    labels = data[params.output]
    nan_value = float("NaN")
    labels.replace("", nan_value, inplace=True)
    labels.dropna( inplace=True)
    labels = pd.DataFrame(labels, columns=params.output)

    time.sleep(params.delay)

    # socket programming
    s = socket.socket()
    s.connect((params.ipAddress, params.port))

    # tell server we are ready
    s.send(bytes("r", "utf-8"))
    
    # transmit pose parameters
    for k in labels.index.values.tolist():
        message=""
        for i in range(len(params.output)):
            message = message+str(labels.loc[k][i]) + ","
        message = message+"frame:\n"
        s.send(bytes(message, "utf-8"))
        time.sleep(1/params.frameRate)
        
        
if __name__ == '__main__':
    simulate()
    
    
    