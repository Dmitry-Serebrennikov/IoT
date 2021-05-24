import time
import serial
import datetime
#import pymongo
from pymongo import MongoClient, errors
#pip install pyserial
from serial.tools import list_ports
# Mongo
##from pymongo import MongoClient
##client = mongoClient("roboforge.ru", username="admin", password="pinboard123", authSource="admin")
##print(client.list_database_names())
#key = '1123581321'
key = b'f'
request = b'r'

ports = list_ports.comports()

for port in ports:
    pinboard = serial.Serial(port.device, 115200, timeout=1)
    pinboard.write(key)
    port_answer = pinboard.readline()
    print(port_answer)
    if (port_answer == b"etoprostojest\r\n"):
        print("good! Your port is ", port.device)
        break
    #time.sleep(5)
    #print(port.device)
print(len(ports), ' ports found')

##photo_data_list = []
##photo_data_list.append(pinboard.readline())
## while через каждую минуту

client = MongoClient("roboforge.ru", username="admin",
                     password="pinboard123", authSource="admin",
                     serverSelectionTimeoutMS=5000,
                     socketTimeoutMS=2000)

db = client["serebrennikov"]
coll = db["photoresistor"]
#coll.insert_one({"datetime": dt, "value": value})
print(client.list_database_names())

while True:
    dt = datetime.datetime.now()
    if datetime.datetime.now().second == 0:
        pinboard.write(request)
        analog_photresistor_answer = pinboard.readline()
        #print("Photoresistor value: ", analog_photresistor_answer)
        time.sleep(1)
        try:
            value = int(analog_photresistor_answer.decode().strip())
        except ValueError:
            continue
        coll.insert_one({"datetime": dt, "value": value})
        print("Photoresistor value: ", value)
        #except:
#decode -> strip -> int
#try except
#col.drop
"""
while True:
    line = pinboard.readline()

    if datetime.datetime.now().second == 0:
        #line = pinboard.readline()
        #while line:
        #    photo_data_list.append(line)
        #    line = pinboard.readline()

        #print(photo_data_list)
        #analog_photresistor_answer = photo_data_list[-1]
        analog_photresistor_answer = line
        print("Photoresistor value: ", analog_photresistor_answer)
        time.sleep(1)
        #photo_data_list = []
"""
