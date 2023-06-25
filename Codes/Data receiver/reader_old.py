import firebase_admin
from firebase_admin import credentials, firestore
import sys
import csv
import re
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
import pandas as pd

# This version of firestore reader does not consider ordering the data by bin IDs!!

if __name__ == "__main__":
    # Connect to firestore database
    cred =

    # device must be specified by the user. Example: knee_brace
    device_type = sys.argv[1] if len(sys.argv) > 1 else ""
    if not device_type:
        print("Error: No device provided")
        sys.exit()

    # user_email must be specified by the user Example: 
    user_email = sys.argv[2] if len(sys.argv) > 2 else ""
    if not user_email:
        print("Error: No email provided")
        sys.exit()

    # Perform look up for given user
    user_doc = db.collection("users").where("email", "==", user_email).get()
    if len(user_doc) == 0:
        print("Error: No user with given email")
        sys.exit()

    # Perform look up for sessionIds of given user
    sessions = user_doc[0].to_dict()["sessions"]
    if len(sessions) == 0:
        print("Error: No sessions associated with given account")
        sys.exit()

    user_devices = user_doc[0].to_dict()["devices"]
    if len(user_devices) == 0:
        print("Error: No devices registered with given user")
        sys.exit()

    # Show sessions to pick from
    i = 0
    print("Select session to export:")
    for session in sessions:
        print(f"{i}: {session['sessionId']} Timestamp: {session['timestamp'].astimezone()}")
        i += 1

    # User selects a session or sessions to export
    input_valid = False
    selections = []
    while not input_valid:
        selection = input()
        # Range based selection Ex: 0-2
        # Note: inclusive with starting and ending values
        if "-" in selection:
            indices = selection.replace(" ", "").split("-")
            indices = list(map(int, indices))
            if indices[1] < indices[0]:
                print("Error: Enter a valid range")
            selections = []
            for i in range(int(indices[0]), int(indices[1]) + 1):
                selections.append(str(i))
        # Comma seperated selection Ex: 0, 1, 2
        elif "," in selection:
            selections = selection.replace(" ", "").split(",")
        # Single value selection Ex: 0
        else:
            selections = [selection.replace(" ", "")]

        valid_inputs = 0
        for selection in selections:
            if selection.isdigit() and int(selection) in range(len(sessions)):
                valid_inputs += 1
        if len(selections) == 0:
            print("Enter a valid range")
        elif valid_inputs == len(selections):
            input_valid = True
        else:
            print("Error: Enter a valid session index")

    device_side_mapping = user_doc[0].to_dict()["devices"]
    if len(device_side_mapping) == 0:
        print("Error: No devices registered to user")

    # Regex for data error in db
    # TODO: remove this after fixing database
    bad_sensor_id_re = re.compile("P\w*O")


    """
    Start reading the data for each session and store inside a dictionary where key = sensorID, value = data from each sensor.
    This dictionary is created for each device (left and right) and inserted into the array data_by_devices.
    A dictionary ensures that we can append the data from subsequent bins to the same sensor rather than overwriting it.
    
    Below is an example of this array:
    [
        { "Time": [], "P1": [], ... },
        { "Time": [], "P1": [], ... }
    ]
    
    The device and movement IDs are also collected in their own arrays for generating the csv file name later on.
    """

    device_ids = []
    movement_ids = []
    data_by_devices = []
    for selection in selections:
        for document in db.collection("data_by_sensor").where("sessionId", "==", sessions[int(selection)]["sessionId"]).stream():
            doc_dict = document.to_dict()

            # If this is a new device, add the id to the device_ids array and append a new dictionary to the data_by_devices array
            device_id = doc_dict["deviceId"]
            if device_id not in device_ids:
                device_ids.append(device_id)
                data_by_devices.append(dict())
                movement_ids.append("")

            device_index = device_ids.index(device_id)
            
            # If this is a new movement, add the id to the movement_ids array
            movement_id = doc_dict["movementId"]
            if movement_id not in movement_ids[device_index]:
                movement_ids[device_index] = movement_id

            # replaces specifics occurences of capital O with 0
            # TODO: remove this after fixing database
            sensor_id = doc_dict["sensorId"]
            if bad_sensor_id_re.match(sensor_id):
                sensor_id = sensor_id.replace("O", "0")
            
            # Select the dictionary for the current device
            data_by_sensor = data_by_devices[device_index]

            if sensor_id == "Time":
                data_by_sensor[sensor_id] = [time.astimezone() for time in doc_dict["values"]]

            if sensor_id in data_by_sensor.keys():
                data_by_sensor[sensor_id].extend(doc_dict["values"])
            else:
                data_by_sensor[sensor_id] = doc_dict["values"]

        if data_by_devices == []:
            print("No data for this session" + " Session Index: " + selection)

        for i in range(len(data_by_devices)):
            # Get the necessary ids and names and generate the file name for the csv file
            device_id = device_ids[i]
            movement_id = movement_ids[i]
            current_session = sessions[int(selection)]
            session_id = current_session["sessionId"]

            file_name = data_by_devices[0]["Time"][0].strftime("%m-%d-%Y-%I-%M-%p") + "-"

            if device_id in device_side_mapping:
                file_name += device_side_mapping[device_id] + "-"
                side = device_side_mapping[device_id]

            movement_name = db.collection("sessions").document(session_id).get().to_dict()["movements"][0]["name"]

            device = db.collection("devices").document(device_id).get()
            # device_type = device.to_dict()["deviceType"] if device.exists else ""

            file_name += device_type + "-" + movement_name + "-" + device_id + "-" + session_id + ".csv"
            file_name = file_name.replace(" ", "")


            """
            Now iterate through the entries in the dictionary for the current device and create a 2D array.
            To ensure the correct order, we first get the list of sensors under each device.
            Then for each sensor in this ordered list, we get the corresponding value inside the dictionary and append it to the 2D array.

            Below is an example of this 2D array:
            [
                [<entry 1 from Time>, <entry 2 from Time>, ...]
                [<entry 1 from P1>, <entry 2 from P1>, ...]
                [<entry 1 from P2>, <entry 2 from P2>, ...]
            ]
            
            This matrix can then be transposed using the zip() function to create the csv file.
            """

            sensors = device.to_dict()["sensors"] if device.exists else []
            device_data = data_by_devices[i]
            sensor_data_mat = []

            for sensor in sensors:
                sensor_data_mat.append(device_data[sensor])

            rows = zip(*sensor_data_mat)

            with open(file_name, "w", newline="") as f:
                writer = csv.writer(f)
                writer.writerow(sensors)
                writer.writerows(rows)

            print(f"File successfully created for index:{selection} deviceId:{device_id} deviceType:{device_type} sessionId:{session_id} movementId:{movement_id}")

            # Create the plot
            figure(figsize=(20, 10), dpi=80)
            data = pd.read_csv(file_name)
            labels = pd.read_csv(f'labels/labels_{device_type}.csv')

            plot_sensor = labels['plot_sensor'].tolist()
            label_name = labels['label_name'].tolist()
            color = labels['color'].tolist()
            marker = labels['marker'].tolist()
            linestyle = labels['line_style'].tolist()

            for i in range(len(plot_sensor)):
                plt.plot(data[plot_sensor[i]], label=label_name[i], color=color[i], marker=marker[i], linestyle=linestyle[i])

            plt.title(f"Session_{selection}_{device_type}_{side}", fontsize=18)
            plt.legend(bbox_to_anchor=(1.02,1.15),fontsize=18)
            plt.xlabel('time (20 unit = 1 second)', fontsize=18)
            plt.ylabel('Actual sensor value', fontsize=18)
            plt.savefig(f"{user_email}_Session_{selection}_{device_type}_{side}.png")
            plt.legend()
            plt.show()
            plt.close()

        device_ids = []
        movement_ids = []
        data_by_devices = []
