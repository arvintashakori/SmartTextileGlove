import firebase_admin
from firebase_admin import credentials, firestore

# Usage: adding devices to uses for testing purposes
# Note: endianness of mac address in the datagrapher app/main board testing doc and in the ios app/nrf connect is reversed



db = firestore.client()

knee_braces = {"D785906065CE": "Left", "EC4DA42ADC7D": "Right"} # 9 20
test_boards = {"D48151545096": "Left", "DD4CBE2A8942": "Right"} # 1 11
test_test_boards = {"ECD88583253E": "Left", "EDE21D862714": "Right"} # 18 7
arvin = {"F887AA9A0BE6": "Left"} # 17 F887AA9A0BE6
sixtwelve = {"EADC89431BB9": "Left", "F639DA0C882C": "Right"} # 6 12
eighteen = {"ECD88583253E": "Right"}

# cli@texavie.com  tpBlXmZqDDWcgg8Cr7ZRp6WNQhz2
# test@texavie.com QgBMmY3gTTYthWoNF5SRYhwr7722
# 

db.collection('users').document('QgBMmY3gTTYthWoNF5SRYhwr7722').set({
    'devices': sixtwelve
}, merge=True)
