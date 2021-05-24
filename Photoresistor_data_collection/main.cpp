#include "mbed.h"
#include "ESP8266Interface.h"
#include "TCPSocket.h"
#include "MQTTmbed.h"
#include "MQTTClientMbedOs.h"
#include <cstdio>
#include <vector>

#include <cstdint>
#include <ctime>
#include <string.h>


#define ANG PA_6
#define LED PB_5
#define TX  PA_9
#define RX  PA_10
#define TX2 PA_2
#define RX2 PA_3

#define RS PB_11
#define E  PB_10
#define D4 PB_12
#define D5 PB_13
#define D6 PB_14
#define D7 PB_15 

#define BAUDRATE 115200
#define EBAUDRATE 115200
#define SSID "Mi 9T Pro"//"iPhone (Dmitry)"//"Mi 9T Pro"
#define PASSWORD "2281488kekes"//"midas2021"//"2281488kekes"
#define MQTT_MAX_PACKET_SIZE 400
#define MQTT_MAX_PAYLOAD_SIZE 300

using namespace std::chrono_literals;

const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}
static DigitalOut rs(RS);
static DigitalOut e(E);
static DigitalOut d4(D4);
static DigitalOut d5(D5);
static DigitalOut d6(D6);
static DigitalOut d7(D7); 

static AnalogIn analog(ANG);
static DigitalOut led(LED);
static UnbufferedSerial pc(TX, RX, BAUDRATE);
static ESP8266Interface wifi(TX2, RX2);
static BufferedSerial uart(TX, RX, BAUDRATE);

FileHandle *mbed::mbed_override_console(int fd){
    return &uart;
}

void send(bool isCommand, uint8_t data) {
    rs.write(!isCommand);
    ThisThread::sleep_for(5ms);
    
    d7.write((data >> 7) & 1);
    d6.write((data >> 6) & 1);
    d5.write((data >> 5) & 1);
    d4.write((data >> 4) & 1);

    e.write(1);
    ThisThread::sleep_for(5ms);
    e.write(0);
    ThisThread::sleep_for(5ms);

    d7.write((data >> 3) & 1);
    d6.write((data >> 2) & 1);
    d5.write((data >> 1) & 1);
    d4.write((data >> 0) & 1);
    ThisThread::sleep_for(5ms);

    e.write(1);
    ThisThread::sleep_for(5ms);
    e.write(0);
    ThisThread::sleep_for(5ms);
}

void sendCommand(uint8_t cmd) {
    send(true, cmd);
}

void sendChar(const char chr) {
    send(false, chr);
}

void sendString(const char * str) {
    while (*str != '\0') {
        sendChar(*str);
        str++;
    }
}

void scan(WiFiInterface *wifi) {
    WiFiAccessPoint *ap;

    printf("Scan:\r\n");
    int count = wifi->scan(NULL, 0);
    count = count < 15 ? count : 15;
    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count); 
    for (int i = 0; i < count; i++){
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d: networks available.\r\n ", count);
    delete[] ap;
}

bool connect(WiFiInterface *wifi) 
{
    SocketAddress a;

    printf("\r\nConnecting...\r\n");
    int ret = wifi->connect(SSID, PASSWORD, NSAPI_SECURITY_WPA);
    if (ret != 0){
        printf("\r\nConnection error\r\n");
        return false;
    }
    else {
        printf("Success\r\n\r\n");
        printf("MAC: %s\r\n", wifi->get_mac_address());
        wifi->get_ip_address(&a);
        printf("IP: %s\r\n", a.get_ip_address());
        wifi->get_netmask(&a);
        printf("Netmask: %s\r\n", a.get_ip_address());
        wifi->get_gateway(&a);
        printf("Gateway: %s\r\n", a.get_ip_address());
        printf("RSSI: %d\r\n\r\n", wifi->get_rssi());
        return true;
    }
}

void http_demo(NetworkInterface *net) 
{
    // Open a socket on the network interface, and create a TCP connection to mbed.org
    TCPSocket socket;
    socket.open(net);

    SocketAddress a;
    net->gethostbyname("ifconfig.io", &a);
    a.set_port(80);
    socket.connect(a);
    // Send a simple http request
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: ifconfig.io\r\n\r\n";
    int scount = socket.send(sbuffer, sizeof sbuffer);
    printf("sent %d [%.*s]\n", scount, strstr(sbuffer, "\r\n") - sbuffer, sbuffer);

    // Recieve a simple http response and print out the response line
    char rbuffer[64];
    int rcount = socket.recv(rbuffer, sizeof rbuffer);
    printf("recv %d [%.*s]\n", rcount, strstr(rbuffer, "\r\n") - rbuffer, rbuffer);

    // Close the socket to return its memory and bring down the network interface
    socket.close();
}
int main()
{
    char symbol;
    bool connected = false;
    TCPSocket socket;
    MQTTClient client(&socket);
    SocketAddress a;
    char* hostname = "roboforge.ru";
    int port = 1883;
    a.set_port(port);
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 4;
    data.clientID.cstring = "serebrennikov";
    data.username.cstring = "guest";
    data.password.cstring = "mqtt123";
    MQTT::Message message;

    char buf[MQTT_MAX_PAYLOAD_SIZE];
    char* topic = "pinboards/serebrennikov/photoresistor";
    
    sendCommand(0b00110000);
    sendCommand(0b00000010);
    sendCommand(0b00001111);
    sendCommand(0b00000001);

    sendCommand(0b10000000);
    /*
    sendString("lovushka Dockera");
    */
    char photo_data[10] = {};
    
    while (true) {
        if (uart.readable()) {
            uart.read(&symbol, 1);

            if (symbol == 'f') {
                printf("etoprostojest\r\n");
            }
            if (symbol == 'r'){
                long long int res = analog.read_u16();
                printf("%d\r\n", int(res));  

                sendCommand(0b10000000);
                sprintf(photo_data, "%d", int(res)); 
                sendString(photo_data);      
            }
        }
        /*
        analog.read_u16();

        */
        
        //float res = analog.read_u16();
        ////long long int res = analog.read_u16();
        ////printf("%d\r\n", int(res));
        //printf("res\n\r", %d);
        /*
        if (connected) {
            sprintf(buf, 
                "%d", 1123581321);
            message.qos = MQTT::QOS0;
            message.retained = false;
            message.dup = false;
            message.payload = (void*)buf;
            message.payloadlen = strlen(buf);
            int rc = client.publish(topic, message);
            if (rc != 0) {
                printf("Something wrong %d\r\n", rc);
            }
        }
        if (uart.readable()) {
            uart.read(&symbol, 1);
            if (symbol == 'c') {
                connect(&wifi);
                wifi.gethostbyname(hostname, &a);
                printf("Connecting to %s:%d\r\n", hostname, port);
                socket.open(&wifi);
                printf("Opened socket \r\n");
                int rc = socket.connect(a);
                if (rc != 0) {
                    printf("rc from TCP connect is %d\r\n", rc);
                }
                printf("Connected socket\r\n");
                if ((rc = client.connect(data)) != 0) {
                    printf("rc from MQTT connect is %d\r\n", rc);
                }
                connected = true;
            }
        }
        */
        /*
        sendCommand(0b10000000);
        sprintf(photo_data, "%d", int(res)); 
        sendString(photo_data);
        */
        led = !led;
        ThisThread::sleep_for(500ms);
    }
}

//db = client['family']
// coll = db['photoresistor']
// coll.insert_one({"datetime": dt, "value": value})
//print(client.list_database_name())

//degree = 70
//  plt.xticks