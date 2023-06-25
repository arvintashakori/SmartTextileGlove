import torch
import os
import math
import torch.nn as nn
import pandas as pd
import numpy as np
import seaborn as sns
from sklearn.preprocessing import MinMaxScaler
from sklearn.metrics import mean_squared_error
from sklearn.metrics import r2_score
import matplotlib.pyplot as plt
from itertools import combinations
import random

class EarlyStopping:
    """Early stops the training if validation loss doesn't improve after a given patience."""

    def __init__(self, patience=7, verbose=False, delta=0, path='checkpoint.pt', trace_func=print):
        """
        Args:
            patience (int): How long to wait after last time validation loss improved.
                            Default: 7
            verbose (bool): If True, prints a message for each validation loss improvement.
                            Default: False
            delta (float): Minimum change in the monitored quantity to qualify as an improvement.
                            Default: 0
            path (str): Path for the checkpoint to be saved to.
                            Default: 'checkpoint.pt'
            trace_func (function): trace print function.
                            Default: print
        """
        self.patience = patience
        self.verbose = verbose
        self.counter = 0
        self.best_score = None
        self.early_stop = False
        self.val_loss_min = np.Inf
        self.delta = delta
        self.path = path
        self.trace_func = trace_func

    def __call__(self, val_loss, model):

        score = -val_loss

        if self.best_score is None:
            self.best_score = score
            self.save_checkpoint(val_loss, model)
        elif score < self.best_score + self.delta and score < 0:
            self.counter += 1
            self.trace_func(
                f'EarlyStopping counter: {self.counter} out of {self.patience}')
            if self.counter >= self.patience:
                self.early_stop = True
        elif score < 0:
            self.best_score = score
            self.save_checkpoint(val_loss, model)
            self.counter = 0
        else:
            self.counter += 1
            self.trace_func(
                f'EarlyStopping counter: {self.counter} out of {self.patience}')
            if self.counter >= self.patience:
                self.early_stop = True

    def save_checkpoint(self, val_loss, model):
        '''Saves model when validation loss decrease.'''
        if self.verbose:
            self.trace_func(
                f'Validation loss decreased ({self.val_loss_min:.6f} --> {val_loss:.6f}).  Saving model ...')
        torch.save(model.state_dict(), self.path)
        self.val_loss_min = val_loss

class parameters:
    def __init__(self):
        self.output = ['PinkyMCPY', 'PinkyMCPZ',
                       'PinkyPIPZ',
                       'PinkyDIPZ',
                       'RingMCPY', 'RingMCPZ',
                       'RingPIPZ',
                       'RingDIPZ',
                       'MiddleMCPY', 'MiddleMCPZ',
                       'MiddlePIPZ',
                       'MiddleDIPZ',
                       'IndexMCPY', 'IndexMCPZ',
                       'IndexPIPZ',
                       'IndexDIPZ',
                       'ThumbsCMCY', 'ThumbsCMCZ',
                       'ThumbsMCPY', 'ThumbsMCPZ',
                       'ThumbsIPZ',
                       'HandX', 'HandY', 'HandZ', 'HandW']
        self.input = ['Sensor1', 'Sensor2', 'Sensor3', 'Sensor4', 'Sensor5',
                      'Sensor6', 'Sensor7', 'Sensor8', 'Sensor9', 'Sensor10',
                      'Sensor11', 'Sensor12', 'Sensor13', 'Sensor14', 'Sensor15',
                      'Sensor16', 'Sensor17', 'Sensor18', 'Sensor19', 'Sensor20',
                      'Sensor21', 'Sensor22', 'Sensor23', 'Sensor24', 'Sensor25',
                      'IMU1', 'IMU2', 'IMU3', 'IMU4']
        self.outputnames = []
        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        self.list_of_excersices = [[[os.path.join(os.pardir,"Datasets/subject1/random1_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random2_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random3_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random4_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random5_l.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject2/random1_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random2_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random3_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random4_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random5_l.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject3/random1_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random2_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random3_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random4_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random5_l.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject4/random1_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random2_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random3_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random4_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random5_l.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject5/random1_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random2_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random3_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random4_l.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random5_l.csv")]],
                                  [[os.path.join(os.pardir,"Datasets/subject1/random1_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random2_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random3_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random4_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject1/random5_r.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject2/random1_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random2_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random3_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random4_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject2/random5_r.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject3/random1_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random2_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random3_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random4_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject3/random5_r.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject4/random1_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random2_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random3_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random4_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject4/random5_r.csv")],
                                   [os.path.join(os.pardir,"Datasets/subject5/random1_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random2_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random3_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random4_r.csv"),
                                    os.path.join(os.pardir,"Datasets/subject5/random5_r.csv")]]]
        self.randomseed = 42
        self.number_of_output = len(self.output)
        self.number_of_input = len(self.input)
        self.train_window = 100
        self.learning_rate = 0.0001
        self.number_of_hidden_layer = 20
        self.lstm_layer = 1
        self.patience = 10
        self.epochs = 100
        self.plot = True
        self.mu = 0
        self.sigma = 10  # noise parameters
        self.minSelectedSensors = 1
        self.maxSelectedSensors = 10
        self.minSensorScale = 0.5
        self.maxSensorScale = 2.5

def rmse_loss(output, target):
    loss_fn = nn.MSELoss()
    RMSE_loss = torch.sqrt(loss_fn(output, target))
    return RMSE_loss

def create_inout_sequences(data, label, seq_length):
    inout_seq = []
    for i in range(len(data)-seq_length-1):
        train_seq = data[i:(i+seq_length)]
        train_label = label[i+seq_length-1:i+seq_length]
        inout_seq.append((train_seq, train_label))
    return inout_seq

def r2_loss(output, target):
    target_mean = torch.mean(target)
    ss_tot = torch.sum((target - target_mean) ** 2)
    ss_res = torch.sum((target - output) ** 2)
    r2 = 1 - ss_res / ss_tot
    return r2

def rmse(actual, predicted):
    params = parameters()
    results = []
    for i in range(params.number_of_output):
        MSE = mean_squared_error(actual[:, i], predicted[:, i])
        results.append(math.sqrt(MSE))
    MSE = mean_squared_error(actual, predicted)
    results.append(math.sqrt(MSE))
    return results

def r2(actual, predicted):
    params = parameters()
    results = []
    for i in range(params.number_of_output):
        R2 = r2_score(actual[:, i], predicted[:, i])
        results.append(R2)
    R2 = r2_score(actual, predicted, multioutput='variance_weighted')
    #R2= r2_loss(torch.FloatTensor(predicted), torch.FloatTensor(actual))
    results.append(R2)
    return results


def preparedata(which, params, augmented = False, masked = False, scaled = False, noisy = False):
    all_data = pd.read_csv(which)
    all_data = all_data.fillna(all_data.mean())
    labels = all_data[params.output]
    data = all_data[params.input]

    if augmented == True:
        for k in range(params.minSelectedSensors, params.maxSelectedSensors):  # select number of sensors to be transformed
            selected = list(combinations(params.input, k + 1))
            total_init = 0
            total = []
            for sel in selected:
                    # masked sensors
                masked_dataset = data.copy()
                masked_dataset[np.array(sel)] *= 0
                if total_init == 0:
                    total_init = 1
                    total = masked_dataset.copy()
                else:
                    total.append(masked_dataset)

                # scaled sensors
                scaled_dataset = data.copy()
                scaled_dataset[np.array(sel)] *= random.uniform(params.minSensorScale, params.maxSensorScale)
                total.append(scaled_dataset)

                # noisy sensors
                noisy_dataset = data.copy()
                noise = np.random.normal(params.mu, params.sigma, [data.shape[0], k + 1])
                noisy_dataset[np.array(sel)] += noise
                total.append(noisy_dataset)
        data = total.copy()

    if masked == True:
        for k in range(params.minSelectedSensors, params.maxSelectedSensors):  # select number of sensors to be transformed
            selected = list(combinations(params.input, k + 1))
            total = []
            total_init = 0
            for sel in selected:
                # masked sensors
                masked_dataset = data.copy()
                masked_dataset[np.array(sel)] = 0
                if total_init == 0:
                    total_init =1
                    total = masked_dataset.copy()
                else:
                    total.append(masked_dataset)
        data = total.copy()

    if scaled == True:
        for k in range(params.minSelectedSensors, params.maxSelectedSensors):  # select number of sensors to be transformed
            selected = list(combinations(params.input, k + 1))
            total = []
            total_init = 0
            for sel in selected:
                # scaled sensors
                scaled_dataset = data.copy()
                scaled_dataset[np.array(sel)] *= random.uniform(params.minSensorScale, params.maxSensorScale)
                if total_init == 0:
                    total_init = 1
                    total = scaled_dataset.copy()
                total.append(scaled_dataset)
        data = total.copy()

    if noisy == True:
        for k in range(params.minSelectedSensors, params.maxSelectedSensors):  # select number of sensors to be transformed
            selected = list(combinations(params.input, k + 1))
            total = []
            total_init = 0
            for sel in selected:
                # noisy sensors
                noisy_dataset = data.copy()
                noise = np.random.normal(params.mu, params.sigma, [data.shape[0], k + 1])
                noisy_dataset[np.array(sel)] += noise
                if total_init == 0:
                    total_init = 1
                    total = noisy_dataset.copy()
                else:
                    total.append(noisy_dataset)
        data = total.copy()

    data = data.values.astype(float)

    scaler_data = MinMaxScaler(feature_range=(-1, 1))
    data_normalized = scaler_data.fit_transform(data)
    data_normalized = torch.FloatTensor(data_normalized)

    scaler_label = MinMaxScaler(feature_range=(-1, 1))
    labels_normalized = scaler_label.fit_transform(labels)
    labels = torch.FloatTensor(labels_normalized)

    data_inout_seq = create_inout_sequences(
        data_normalized, labels, params.train_window)

    return data_inout_seq

def plot(actual, predicted, params, type, name):
    if type == 'scatter':
        for i in range(params.number_of_output):
            fig = plt.figure(figsize=(20, 10))
            p = sns.relplot(x=actual[:, i], y=predicted[:, i])
            plt.title(params.output[i])
            p.set(xlabel='Actual',
                  ylabel='Predicted')
            plt.savefig(name + "_" + params.output[i] + "_" + type + ".pdf")
            plt.show(block=False)
    if type == 'time':
        for i in range(params.number_of_output):

            fig = plt.figure(figsize=(20, 10))
            sns.lineplot(data=actual[:, i])
            sns.lineplot(data=predicted[:, i])
            fig.legend(labels=['Actual', 'Predicted'])
            plt.title(params.output[i])
            plt.savefig(name+"_"+params.output[i]+"_"+type+".pdf")
            plt.show(block=False)
    


def train(data, model, device, params, optimizer, loss_function, valid, patience):
    # to track the training loss as the model trains
    train_losses = []
    # to track the validation loss as the model trains
    valid_losses = []
    # to track the average training loss per epoch as the model trains
    avg_train_losses = []
    # to track the average validation loss per epoch as the model trains
    avg_valid_losses = []

    # initialize the early_stopping object
    early_stopping = EarlyStopping(patience=patience, verbose=True)

    for epoch in range(1, params.epochs+1):
        model.train()
        for seq, labels in data:
            optimizer.zero_grad()

            model.hidden_cell = (torch.zeros(2*params.lstm_layer, 1, model.hidden_layer_size).to(device),
                                 torch.zeros(2*params.lstm_layer, 1, model.hidden_layer_size).to(device))

            y_pred = model(seq.to(device))

            y_pred = y_pred.view(1, -1)

            single_loss = loss_function(y_pred, labels.to(device))
            single_loss.backward()
            optimizer.step()
            train_losses.append(single_loss.item())

        ######################
        # validate the model #
        ######################
        model.eval()  # prep model for evaluation
        total_R2 = 0
        count = 0
        for seq, labels in valid:
            with torch.no_grad():
                model.hidden_cell = (torch.zeros(2*params.lstm_layer, 1, model.hidden_layer_size).to(device),
                                     torch.zeros(2*params.lstm_layer, 1, model.hidden_layer_size).to(device))
                pred = model(seq.to(device))
                y_pred = pred.view(1, -1)
                loss = loss_function(y_pred, labels.to(device))
                valid_losses.append(loss.item())

        # print training/validation statistics
        # calculate average loss over an epoch
        train_loss = np.average(train_losses)
        valid_loss = np.average(valid_losses)
        avg_train_losses.append(train_loss)
        avg_valid_losses.append(valid_loss)

        epoch_len = len(str(params.epochs))

        print_msg = (f'[{epoch:>{epoch_len}}/{params.epochs:>{epoch_len}}] '
                     + f'train_loss: {train_loss:.5f} '
                     + f'valid_loss: {valid_loss:.5f}')

        print(print_msg)

        # clear lists to track next epoch
        train_losses = []
        valid_losses = []

        # early_stopping needs the validation loss to check if it has decresed,
        # and if it has, it will make a checkpoint of the current model
        early_stopping(valid_loss, model)

        if early_stopping.early_stop:
            print("Early stopping")
            break

    # load the last checkpoint with the best model
    model.load_state_dict(torch.load('checkpoint.pt'))

    return model


def eval(data, model, device, name):
    model.eval()
    total_R2 = 0
    count = 0
    params = parameters()
    for seq, labels in data:
        with torch.no_grad():
            model.hidden_cell = (torch.zeros(2*params.lstm_layer, 1, model.hidden_layer_size).to(device),
                                 torch.zeros(2*params.lstm_layer, 1, model.hidden_layer_size).to(device))
            pred = model(seq.to(device))
            y_pred = pred.view(1, -1)
            #loss = r2_loss(y_pred, labels.to(device))
            #total_R2 = total_R2 + loss.item() * 100
            count = count + 1
            if count == 1:
                actual = labels.detach().cpu().numpy()
                predicted = y_pred.detach().cpu().numpy()
            else:
                actual = np.concatenate(
                    (actual, labels.detach().cpu().numpy()), axis=0)
                predicted = np.concatenate(
                    (predicted, y_pred.detach().cpu().numpy()), axis=0)

    print(f'R2 {name}:      {r2_loss(torch.FloatTensor(predicted),torch.FloatTensor(actual)):10.10f}%')
    print(f'RMSE {name}:      {rmse_loss(torch.FloatTensor(predicted),torch.FloatTensor(actual)):10.10f}')
    return actual, predicted


class LSTM(nn.Module):
    def __init__(self, input_size, hidden_layer_size, output_size, lstm_layer):
        super().__init__()
        self.params = parameters()

        self.hidden_layer_size = hidden_layer_size

        self.drp = nn.Dropout(p=0.2)

        self.lstm = nn.LSTM(input_size, hidden_layer_size,
                            num_layers=lstm_layer, bidirectional=True)

        self.linear1 = nn.Linear(2*hidden_layer_size, hidden_layer_size)

        self.linear2 = nn.Linear(hidden_layer_size, output_size)

        self.hidden_cell = (torch.zeros(2*self.params.lstm_layer, 1, self.hidden_layer_size),
                            torch.zeros(2*self.params.lstm_layer, 1, self.hidden_layer_size))

    def forward(self, input_seq):
        dropout = self.drp(input_seq)
        lstm_out, self.hidden_cell = self.lstm(
            dropout.view(len(input_seq), 1, -1), self.hidden_cell)
        out = self.drp(lstm_out)
        buffer = self.linear1(out.view(len(input_seq), -1))
        predictions = self.linear2(buffer)
        return predictions[-1]
