from os import times
import firebase_admin
from firebase_admin import credentials, firestore

# Usage: updating the static data in our database. 

class Exercise:
    def __init__(self, name, category, hasLeftAndRight, hasAngle, usesCalf, usesHamstring, usesQuad, angleThreshold = 180, times = [], states = []):
        self.name            = name
        self.category        = category
        self.hasLeftAndRight = hasLeftAndRight
        self.hasAngle        = hasAngle
        self.usesCalf        = usesCalf
        self.usesHamstring   = usesHamstring
        self.usesQuad        = usesQuad
        self.angleThreshold  = angleThreshold
        self.times           = times
        self.states          = states

    def to_dict(self):
        dict = {}
        dict["category"]        = self.category
        dict["hasLeftAndRight"] = self.hasLeftAndRight
        dict["hasAngle"]        = self.hasAngle
        dict["usesCalf"]        = self.usesCalf
        dict["usesHamstring"]   = self.usesHamstring
        dict["usesQuad"]        = self.usesQuad
        dict["angleThreshold"]  = self.angleThreshold
        dict["times"]           = self.times
        dict["states"]          = self.states
        return dict

if __name__ == "__main__":
    STRETCH  = "Stretching"
    STRENGTH = "Strengthening"
    T = True
    F = False
    R = "Rest"
    U = "Up"
    H = "Hold"
    D = "Down"
    DOWN_FIRST = [R, D, H, U]
    UP_FIRST   = [R, U, H, D]
    exercises = [
        #Exercise("Standing Quadriceps Stretch", STRETCH,  F, T, F, F, T, 90),
        #Exercise("Supine Hamstring Stretch",    STRETCH,  F, T, F, T, F, 90),
        Exercise("Half Squats",                 STRENGTH, T, T, T, T, T, 60,  [3, 2, 3, 2], DOWN_FIRST),
        Exercise("Squats",                      STRENGTH, T, T, T, T, T, 100, [3, 2, 3, 2], DOWN_FIRST),
        Exercise("Slow Squats",                 STRENGTH, T, T, T, T, T, 90,  [5, 5, 5, 5], DOWN_FIRST),
        Exercise("Staggered Squat",             STRENGTH, T, T, T, T, T, 80,  [3, 2, 3, 2], DOWN_FIRST),
        Exercise("Half Staggered Squat",        STRENGTH, T, T, T, T, T, 80,  [3, 2, 3, 2], DOWN_FIRST),
        Exercise("Hamstring Curls",             STRENGTH, F, T, F, T, T, 90,  [3, 2, 2, 2], UP_FIRST),
        Exercise("Calf Raises",                 STRENGTH, F, F, T, F, F, 180, [3, 2, 4, 2], UP_FIRST),
        #Exercise("Leg Extensions",              STRENGTH, F, T, F, T, T),
        #Exercise("Straight Leg Raises",         STRENGTH, F, T, F, T, T),
        #Exercise("Prone Straight Leg Raises",   STRENGTH, F, T, F, T, T),
        #Exercise("Hip Abduction",               STRENGTH, F, F, F, F, F),
        #Exercise("Hip Adduction",               STRENGTH, F, F, F, F, F),
        #Exercise("Leg Presses",                 STRENGTH, F, T, F, T, T),
        Exercise("Lateral Lunges",              STRENGTH, T, F, T, T, T, 180, [3, 2, 4, 2], DOWN_FIRST),
        Exercise("Forward Lunges",              STRENGTH, T, T, T, T, T, 90,  [3, 2, 4, 2], DOWN_FIRST),
        Exercise("Reverse Lunges",              STRENGTH, T, T, T, T, T, 90,  [3, 2, 4, 2], DOWN_FIRST)
    ]
    device_names = ["Glove", "Knee Brace", "Tights"]
    device_sensors = {
        "sensors": [
            "Time", 
            "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P9", 
            "P10", "P11", "P12", "P13", "P14", "P15", "P16", "P17", "P18", "P19", 
            "P20", "P21", "P22", "P23", "P24", "P25", "P26", "P27", "P28", "P29", 
            "P30", "P31", 
            "AX0", "AY0", "AZ0", "QW0", "QX0", "QY0", "QZ0", "GX0", "GY0", "GZ0", 
            "AX1", "AY1", "AZ1", "QW1", "QX1", "QY1", "QZ1", "GX1", "GY1", "GZ1",
            "ECG"
        ]
    }

    cred = 
    
    # Delete all current exercises
    for exercise in db.collection('exercises').stream():
        exercise.reference.delete()

    batch = db.batch()
    # Add exercises defined above
    for exercise in exercises:
        doc_ref = db.collection('exercises').document(exercise.name)
        batch.set(doc_ref, exercise.to_dict())
    # Add device types to database
    for device_name in device_names:
        doc_ref = db.collection('deviceTypes').document(device_name)
        batch.set(doc_ref, device_sensors)
    batch.commit()

