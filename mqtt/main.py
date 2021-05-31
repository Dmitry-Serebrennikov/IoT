import paho.mqtt.client as mqtt
import time
import serial

client = mqtt.Client()
client.username_pw_set(username='guest', password='mqtt123')
client.connect('roboforge.ru', 1883, 60)

s = serial.Serial("COM5", baudrate=9600, timeout=0.1)

voltage = None

prev_sending = time.time()

while True:
    line = s.readline().decode()
    if line != '':
        if line.startswith("v="):
            voltage = float(line[2:])
            print('Voltage:', voltage)
    if (time.time() - prev_sending) > 1:
        client.publish('pinboards/serebrennikov/values', payload=voltage)
        prev_sending = time.time()
