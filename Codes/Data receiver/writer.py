import firebase_admin
from firebase_admin import credentials, firestore
import sys
import csv
from dateutil.parser import parse


if __name__ == "__main__":
    # Connect to firestore database
    cred = credentials.Certificate('')
    firebase_admin.initialize_app(cred, {
        '' : ''
    })
    db = firestore.client()

    ADC_SENSORS = ['Time', 'P0', 'P1', 'P2', 'P3', 'P4', 'P5', 'P6', 'P7', 'P8', 'P9', 'P10',
    'P11', 'P12', 'P13', 'P14', 'P15', 'P16', 'P17', 'P18', 'P19', 'P20',
    'P21', 'P22', 'P23', 'P24', 'P25', 'P26', 'P27', 'P28', 'P29', 'P30', 'P31']
    BNO_SENSORS = ['ECG', 'AX', 'AY', 'AZ', 'QW', 'QX', 'QY', 'QZ', 'GX', 'GY', 'GZ', 'CS', 'CA', 'CG', 'CM']

    file_name = sys.argv[1] if len(sys.argv) > 1 else print('Error: No file name provided')
    user_email  = sys.argv[2] if len(sys.argv) > 2 else 'test@-----.com'
    device_id   = 'A-B-C-D-E'
    movement_id = '12345678'
    session_id  = '87654321'

    # Tranpose the csv file and put its contents into data_by_row
    data_by_col = []
    with open(file_name, newline='') as csvfile:
        data_reader = csv.reader(csvfile)
        for row in data_reader:
            data_by_col.append(row)
    data_by_row = [list(i) for i in zip(*data_by_col)]

    # Using batch we atomically add all the sensor data from the csv file
    batch = db.batch()
    user_ref = db.collection('users').where('email', '==', user_email).get()[0].reference
    batch.update(user_ref, {'sessions': firestore.ArrayUnion([session_id])})

    # Populate relevant fields in db
    for row in data_by_row:
        sensor_id = row[0]
        data_doc = {
            'deviceId': device_id,
            'movementId': movement_id,
            'sensorId': sensor_id,
            'sessionId': session_id
        }
        # Different data format for different type of sensors
        if sensor_id == 'Time':
            data_doc['values'] = [parse(timestamp) for timestamp in row[1:]]
        elif sensor_id in ADC_SENSORS:
            data_doc['values'] = [float(elem) for elem in row[1:]]
        elif sensor_id in BNO_SENSORS:
            data_doc['values'] = [int(elem) for elem in row[1:]]
        data_ref = db.collection('data_by_sensor').document(session_id + sensor_id)
        if 'values' in data_doc:
            batch.set(data_ref, data_doc)

    batch.commit()
    print(f'Successfully added data from csv with sessionId:{session_id} deviceId:{device_id} movementId:{movement_id} to user with email:{user_email}')
