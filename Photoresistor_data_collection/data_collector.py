import os
import time
from pymongo.common import NONSPEC_OPTIONS_VALIDATOR_MAP
import serial
import datetime
import tempfile
from dateutil import parser
from pymongo import MongoClient, errors

from serial.tools import list_ports

key = b'f'
request = b'r'
pinboard = None

def get_pinboard():
    ports = list_ports.comports()
    print(len(ports), ' ports found')
    for port in ports:
        pinboard = serial.Serial(port.device, 115200, timeout=1)
        pinboard.write(key)
        try:
            line = pinboard.readline().decode().strip()
            if (line == "etoprostojest"):
                print("good! Your port is ", port.device)
                return pinboard
        except ValueError:
            pass
    return None


def collect_data():
    global pinboard
    try:
        pinboard.write(request)
        time.sleep(0.1)
        analog_photresistor_answer = int(pinboard.readline().decode().strip())
        return analog_photresistor_answer
    except ValueError:
        return None
    except serial.SerialException:
        pinboard = None
        return None


def send_data(dt, value):
    storage_file = os.path.join(tempfile.gettempdir(), "serebrennikov")
    os.makedirs(storage_file, exist_ok=True)
    storage_file = storage_file + "\\file.txt"

    with open(storage_file, 'a') as storage:
        storage.write(str(dt) + " " + str(value) + '\n')
        storage.close()

    measures = []
    with open(storage_file, 'r') as storage:
        lines = storage.readlines()
        for line in lines:
            *storage_dt, storage_value = line.split(" ")
            storage_dt = parser.parse(" ".join(storage_dt))
            storage_value = int(storage_value)
            measures.append({"datetime": storage_dt, "value": storage_value})
        storage.close()

    try:
        coll.insert_many(measures)
        with open(storage_file, 'w') as storage:
            storage.writelines("")
            storage.close()
    except:
        pass

client = MongoClient("roboforge.ru", username="admin",
                     password="pinboard123", authSource="admin",
                     serverSelectionTimeoutMS=5000,
                     socketTimeoutMS=2000)

db = client["serebrennikov"]
coll = db["photoresistor"]
pinboard = get_pinboard()
while True:
    if pinboard:
        dt = datetime.datetime.now()
        if dt.second == 0:
            value = collect_data()
            if value:
                send_data(dt, value)
                print("Photoresistor value: ", value)
            time.sleep(1)
    else:
        print("Waiting pinboard...")
        pinboard = get_pinboard()
        time.sleep(5)