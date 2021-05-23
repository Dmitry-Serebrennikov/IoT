#created on 12/04/2021
#В честь ясности и правильности пришлось отказаться от звучных наименований функций, но память о них осталась в хештегах
import socket
import numpy as np
import matplotlib.pyplot as plt

def recvall(sock, n):
    data = bytearray()
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data.extend(packet)
    return data

def addZerosBorder(image): #def subZero
    newImage = np.zeros((image.shape[0] + 2, image.shape[1] + 2), dtype=int)
    newImage[1:-1, 1:-1] = image
    return newImage

def removeStarNoise(image, position): #def obnulit' #def starCollapse 
    radiusStep = 0
    while image[position[0]][position[1] - radiusStep] != 0:
        radiusStep += 1

    for x in range(position[1] - radiusStep, position[1] + radiusStep + 1):
        for y in range(position[0] - radiusStep, position[0] + radiusStep + 1):
            image[y][x] = 0
    return image

def findStarCenter(image): #quaerereProxima #performApproximation 
    num = np.argmax(image)
    pos = np.unravel_index(num, image.shape)
    newImage = addZerosBorder(image)

    image = removeStarNoise(newImage, pos)
    print("position: ", pos)
    return image, pos

def calculateDistance(image): 
    image, pos1 = findStarCenter(image)
    image, pos2 = findStarCenter(image)

    res = np.abs(np.array(pos1) - np.array((pos2)))
    distance = round((res[0] ** 2 + res[1] ** 2) ** 0.5, 1)

    return distance

host = "..."
port = ...

IMAGE_SIZE_X = 200
IMAGE_SIZE_Y = 200

plt.ion()
plt.figure()
n = 0
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect((host, port))

    beat = b"nope"
    #while beat != b"yep":
    while n != 10:
        n += 1
        sock.send(b"get")
        bts = recvall(sock, 40002)
        print(len(bts))
        im1 = np.frombuffer(bts[2:40002],
                            dtype="uint8").reshape(bts[0], bts[1])


        print("distance: ", calculateDistance(im1))
        plt.clf()

        plt.imshow(im1)
        plt.pause(2)

        sock.send(b"beat")
        beat = sock.recv(20)
        print(beat)

print("Done!")
