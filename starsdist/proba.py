#created on 12/04/2021
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

def subZero(image): # add zeros-border # subZero # subMatrixZero
    #newImage = np.zeros((imageX + 2, imageY + 2), dtype=int, order='C')
    #newImage = np.zeros((image.shape[0] + 2, image.shape[1]+ 2), dtype=int, order='C')
    newImage = np.zeros((image.shape[0] + 2, image.shape[1] + 2), dtype=int)
    newImage[1:-1, 1:-1] = image
    return newImage

def obnulit(image, position): # remove star collapse
    radiusStep = 0
    while image[position[0]][position[1] - radiusStep] != 0:
        radiusStep += 1  # radiusStep -=- 1

    for x in range(position[1] - radiusStep, position[1] + radiusStep + 1):
        for y in range(position[0] - radiusStep, position[0] + radiusStep + 1):
            # if im1[y][x] == im1[pos1[0]][pos1[1]]:
            #    continue
            image[y][x] = 0
    return image

def findProxima(image):
    num = np.argmax(image)
    pos = np.unravel_index(num, image.shape)
    value = image[pos[0]][pos[1]]

    newImage = subZero(image, IMAGE_SIZE_X, IMAGE_SIZE_Y)

    image = obnulit(newImage, pos)
    print("position: ", pos)
    return image, pos
#def findMaximus def findRadius def supernova outbreak def Zvezda po imeni def obnulenie

#играй же песнь, хвались и пой, пускайся calculateDistance, да будет TSOI
# привет IOT -  TOI
def calculateDistance(image):
    image, pos1 = findProxima(image)
    image, pos2 = findProxima(image)

    res = np.abs(np.array(pos1) - np.array((pos2)))
    distance = round((res[0] ** 2 + res[1] ** 2) ** 0.5, 1)

    return distance

host = "84.237.21.36"
port = 5152

plt.ion()
plt.figure()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect((host, port))

    beat = b"nope"
    while beat != b"yep":
        sock.send(b"get")
        bts = recvall(sock, 40002)
        print(len(bts))
        im1 = np.frombuffer(bts[2:40002],
                            dtype="uint8").reshape(bts[0], bts[1])
        IMAGE_SIZE_X = 200
        IMAGE_SIZE_Y = 200

        #im1, pos = findProxima(im1)
        #im1, pos2 = findProxima(im1)
        #res = np.abs(np.array(pos) - np.array(pos2))
        #distance = round((res[0] ** 2 + res[1] ** 2) ** 0.5, 1)

        #print(pos1)
        #print(pos2_3)

        #print("distance:", round((res[0] ** 2 + res[1] ** 2) ** 0.5, 1))
        #print("position1: ", pos1)
        print("distance: ", calculateDistance(im1))
        plt.clf()

        plt.imshow(im1)
        plt.pause(2)

        sock.send(b"beat")
        beat = sock.recv(20)
        print(beat)

print("Done!")
