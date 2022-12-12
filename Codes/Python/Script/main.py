import torch
import pandas as pd
from utils import LSTM, parameters, preparedata, eval, plot, train, rmse, r2
import torch.nn as nn
import numpy as np
import os

if __name__ == '__main__':
    if not os.path.exists('Results'):
        os.makedirs('Results')
    params = parameters()
    device = params.device
    for randSeed in range(10): # runing 10 times with different seed values
        params.randomseed = randSeed
        RANDOM_SEED = params.randomseed
        np.random.seed(RANDOM_SEED)
        torch.manual_seed(RANDOM_SEED)
        for hand in range(2):  # 0: for left hand 1: for right hand

            # Intra-subject Cross Validation
            for subjectID in range(5):
                data_trial = []
                for trialID in range(5):
                    data_trial.append(preparedata(params.list_of_excersices[hand][subjectID][trialID], params))
                for trialID in range(5):
                    model = LSTM(input_size=params.number_of_input, hidden_layer_size=params.number_of_hidden_layer,
                                 output_size=params.number_of_output, lstm_layer=params.lstm_layer)
                    model = model.to(device)
                    loss_function = nn.SmoothL1Loss(beta=0.5)
                    optimizer = torch.optim.Adam(model.parameters(), lr=params.learning_rate)

                    RMSE_list = []
                    R2_list = []
                    init_flag = 0
                    for trainingID in range(5):
                        if trainingID != trialID:
                            if init_flag == 0:
                                training_data = data_trial[trainingID]
                                init_flag = 1
                            else:
                                training_data = np.concatenate((training_data, data_trial[trainingID]), axis=0)
                    print("Training intra-subject CV model_" + str(trialID) + "_subject_" + str(subjectID) + "_hand_" + str(
                        hand)+ "_randSeed_" + str(randSeed))
                    model = train(training_data, model, device, params, optimizer, loss_function, data_trial[trialID],
                                  params.patience)
                    actual, predicted = eval(data_trial[trialID], model, device,
                                             "intra-subject CV model_" + str(trialID) + "_subject_" + str(
                                                 subjectID) + "_hand_" + str(hand) + "_randSeed_" + str(randSeed))
                    if params.plot == True:
                        plot(actual, predicted, params, 'scatter',
                             "Results/Subject" + str(subjectID) + "_hand_" + str(hand)+ "_randSeed_" + str(randSeed))
                        plot(actual, predicted, params, 'time', "Results/Subject" + str(subjectID) + "_hand_" + str(hand)+ "_randSeed_" + str(randSeed))
                    RMSE_list.append(rmse(actual, predicted))
                    R2_list.append(r2(actual, predicted))

                    # resutls
                    df_r2 = pd.DataFrame(R2_list)
                    df_r2.to_csv("Results/r2_intra_subject" + str(subjectID) + "_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")
                    df_r2.describe().to_csv(
                        "Results/r2_intra_subject" + str(subjectID) + "_summary_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")

                    df_rmse = pd.DataFrame(RMSE_list)
                    df_rmse.to_csv("Results/rmse_intra_subject" + str(subjectID) + "_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")
                    df_rmse.describe().to_csv(
                        "Results/rmse_intra_subject" + str(subjectID) + "_summary_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")

            # Inter-subject Cross Validation
            user_data = []
            for subjectID in range(5):
                for trialID in range(1):
                    if trialID == 0:
                        data_trial = preparedata(params.list_of_excersices[hand][subjectID][trialID], params)
                    else:
                        data_trial = np.concatenate(
                            (data_trial, preparedata(params.list_of_excersices[hand][subjectID][trialID], params)), axis=0)
                user_data.append(data_trial)

            RMSE_list = []
            R2_list = []

            for subjectID in range(5):
                model = LSTM(input_size=params.number_of_input, hidden_layer_size=params.number_of_hidden_layer,
                             output_size=params.number_of_output, lstm_layer=params.lstm_layer)
                model = model.to(device)
                loss_function = nn.SmoothL1Loss(beta=0.5)
                optimizer = torch.optim.Adam(model.parameters(), lr=params.learning_rate)

                init_flag = 0
                for trainingID in range(5):
                    if trainingID != subjectID:
                        if init_flag == 0:
                            training_data = user_data[trainingID]
                            init_flag = 1
                        else:
                            training_data = np.concatenate((training_data, user_data[trainingID]), axis=0)
                print("Training inter-subject CV model subject_" + str(subjectID) + "_hand_" + str(hand)+ "_randSeed_" + str(randSeed))
                model = train(training_data, model, device, params, optimizer, loss_function, user_data[subjectID],
                              params.patience)
                actual, predicted = eval(user_data[subjectID], model, device,
                                         "inter-subject CV model_" + "subject_" + str(subjectID) + "_hand_" + str(hand)+ "_randSeed_" + str(randSeed))
                if params.plot == True:
                    plot(actual, predicted, params, 'scatter', "Results/Subject" + str(subjectID) + "_hand_" + str(hand)+ "_randSeed_" + str(randSeed))
                    plot(actual, predicted, params, 'time', "Results/Subject" + str(subjectID) + "_hand_" + str(hand)+ "_randSeed_" + str(randSeed))
                RMSE_list.append(rmse(actual, predicted))
                R2_list.append(r2(actual, predicted))

                # Saving inter-subject cross validation resutls
                df_r2 = pd.DataFrame(R2_list)
                df_r2.to_csv("Results/r2_inter_subject_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")
                df_r2.describe().to_csv("Results/r2_inter_subject_summary_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")

                df_rmse = pd.DataFrame(RMSE_list)
                df_rmse.to_csv("Results/rmse_inter_subject_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")
                df_rmse.describe().to_csv("Results/rmse_inter_subject_summary_hand_" + str(hand) + "_randSeed_" + str(randSeed) + ".csv")
                
        # Self-supervised pretraining
        for hand in range(2):
            for subjectID in range(5):
                data_trial_normal = []
                data_trial_augmented = []
                data_trial_masked = []
                data_trial_scaled = []
                data_trial_noisy = []

                print("Loading datasets")
                for trialID in range(2):
                    print("Trial"+str(trialID))
                    print("Normal data")
                    data_trial_normal.append(preparedata(params.list_of_excersices[hand][subjectID][trialID], params))
                    print("Augmented data")
                    data_trial_augmented.append(preparedata(params.list_of_excersices[hand][subjectID][trialID], params, augmented = True))
                    print("Masked data")
                    data_trial_masked.append(preparedata(params.list_of_excersices[hand][subjectID][trialID], params, masked = True))
                    print("Scaled data")
                    data_trial_scaled.append(preparedata(params.list_of_excersices[hand][subjectID][trialID], params, scaled = True))
                    print("Noisy data")
                    data_trial_noisy.append(preparedata(params.list_of_excersices[hand][subjectID][trialID], params, noisy=True))

                model_normal = LSTM(input_size=params.number_of_input, hidden_layer_size=params.number_of_hidden_layer, output_size=params.number_of_output, lstm_layer=params.lstm_layer)
                model_normal = model_normal.to(device)

                model_augmented = LSTM(input_size=params.number_of_input, hidden_layer_size=params.number_of_hidden_layer, output_size=params.number_of_output, lstm_layer=params.lstm_layer)
                model_augmented = model_augmented.to(device)

                loss_function_normal = nn.SmoothL1Loss(beta=0.5)
                optimizer_normal = torch.optim.Adam(model_normal.parameters(), lr=params.learning_rate)

                loss_function_augmented = nn.SmoothL1Loss(beta=0.5)
                optimizer_augmented = torch.optim.Adam(model_augmented.parameters(), lr=params.learning_rate)

                print("Training normal model")
                model_normal = train(data_trial_normal[0], model_normal, device, params, optimizer_normal, loss_function_normal, data_trial_normal[1], params.patience)

                print("Training augmented model")
                model_augmented = train(data_trial_augmented[0], model_augmented, device, params, optimizer_augmented, loss_function_augmented, data_trial_augmented[1], params.patience)

                print("Fine-tuning augmented model on normal dataset")
                model_augmented = train(data_trial_normal[0], model_augmented, device, params, optimizer_augmented, loss_function_augmented, data_trial_augmented[1], params.patience)
                
                R2_normal = []
                RMSE_normal = []

                #evaluate normal model
                actual, predicted = eval(data_trial_normal[1], model_normal, device, "normal_model_normal_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_normal.append(rmse(actual, predicted))
                R2_normal.append(r2(actual, predicted))

                actual, predicted = eval(data_trial_masked[1], model_normal, device, "normal_model_masked_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_normal.append(rmse(actual, predicted))
                R2_normal.append(r2(actual, predicted))

                actual, predicted = eval(data_trial_scaled[1], model_normal, device, "normal_model_scaled_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_normal.append(rmse(actual, predicted))
                R2_normal.append(r2(actual, predicted))

                actual, predicted = eval(data_trial_noisy[1], model_normal, device, "normal_model_noisy_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_normal.append(rmse(actual, predicted))
                R2_normal.append(r2(actual, predicted))

                R2_augmented = []
                RMSE_augmented = []
                # evaluate augmented model
                actual, predicted = eval(data_trial_normal[1], model_augmented, device, "augmented_model_normal_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_augmented.append(rmse(actual, predicted))
                R2_augmented.append(r2(actual, predicted))

                actual, predicted = eval(data_trial_masked[1], model_augmented, device, "augmented_model_masked_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_augmented.append(rmse(actual, predicted))
                R2_augmented.append(r2(actual, predicted))

                actual, predicted = eval(data_trial_scaled[1], model_augmented, device, "augmented_model_scaled_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_augmented.append(rmse(actual, predicted))
                R2_augmented.append(r2(actual, predicted))

                actual, predicted = eval(data_trial_noisy[1], model_augmented, device, "augmented_model_noisy_data_subject"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed))
                RMSE_augmented.append(rmse(actual, predicted))
                R2_augmented.append(r2(actual, predicted))

                df_r2_normal = pd.DataFrame(R2_normal)
                df_rmse_normal = pd.DataFrame(RMSE_normal)
                df_r2_augmented = pd.DataFrame(R2_augmented)
                df_rmse_augmented = pd.DataFrame(RMSE_augmented)

                df_r2_normal.to_csv("Results/r2_normal_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")
                df_r2_normal.describe().to_csv("Results/r2_normal_summary_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")

                df_rmse_normal.to_csv("Results/rmse_normal_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")
                df_rmse_normal.describe().to_csv("Results/rmse_normal_summary_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")

                df_r2_augmented.to_csv("Results/r2_augmented_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")
                df_r2_augmented.describe().to_csv("Results/r2_augmented_summary_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")

                df_rmse_augmented.to_csv("Results/rmse_augmented_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")
                df_rmse_augmented.describe().to_csv("Results/rmse_augmented_summary_subject_"+str(subjectID)+"_hand_"+str(hand)+"_seed_"+str(randSeed)+".csv")
