/*
   "Sending recieved data (from XBee) from coordinator to NodeMCU"

   === MATERIAL ===

   - Arduino UNO
   - XBee shield for Arduino UNO
   - Sensors
      - DHT22 (AM2302)
         - Measure        : Humidity and Temperature
         - Input Volatage : 5V
         - Signal         : Digital
      - 10HS
         - Measure        : Humidity and Temperature
         - Input Volatage : 5V - 12V
         - Signal         : Analog
      - Davis 6450
         - Measure        : Humidity and Temperature
         - Input Volatage : 3.3V
         - Signal         : Analoge

    === PINOUT ===

   +-------+---------------+
   |  UNO  | XBee (Shield) |
   +-------+---------------+
   | 0(RX) |     0 TX      |
   | 1(TX) |     1 RX      |
   |  GND  |      GND      |
   |  5V   |      5V       |
   +-------+---------------+

   +-------+-------+------+-----------+
   |  UNO  | DHT22 | 10HS | Davis6450 |
   +-------+-------+------+-----------+
   |   4   |  OUT  |      |           |
   |  5V   |  5V   |      |           |
   |  GND  |  GND  |      |           |
   |  A0   |       | DATA |           |
   |   -   |       |  12V |           |
   |  GND  |       |  GND |           |
   |  A1   |       |      |   DATA    |
   |  3.3V |       |      |   3.3V    |
   |  GND  |       |      |    GND    |
   +-------+-------+------+-----------+


   By Khmaoch
*/

#include <XBee.h>
#include <SoftwareSerial.h>
#include <DHT.h>

/*
    === APDU STRUCTURE ===
*/

//Define package structure
/*
    +--------+---------+----------+-------------+----------+-----------+------------+
    | header | node_id | humidity | temperature | moisture | radiation |            |
    +--------+---------+----------+-------------+----------+-----------+            |
    |   1    |    1    |     4    |      4      |     4    |     4     | = 18 bytes |
    +--------+---------+----------+-------------+----------+-----------+------------+
*/

typedef struct
{
  uint8_t header;
  uint8_t node_id;

  float humidity;
  float temperature;
  float moisture;
  float radiation;

} data_struct_t;

typedef union
{
  data_struct_t data_struct;
  uint8_t data_byte[18];
} packet_t;

/*
    === XBEE ===
*/

//Define XBee
//HardwareSerial &XBee = Serial;
SoftwareSerial XBee(2, 3);

/*
    === SENSORS ===
*/

//Define DHT22 sensor
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

float hum = 0.0;
float temp = 0.0;

//Define 10HS sensor
#define hsPin A0

float rawHsValue = 0;
float voltageHsValue = 0;
float mois = 0.0;

//Define Davis6450 sensor
#define davisPin A1

float rawDavisValue = 0;
float voltageDavisValue = 0;
float rad = 0.0;

/*
    === OTHER ===
*/

//Define timer
unsigned long pre_time;
unsigned long cur_time;


void setup()
{
  Serial.begin(9600);
  XBee.begin(9600);
}

void loop()
{
  /*
      === READ SENSORS ===
  */

  //Read data from DHT22
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  //Read data from 10HS
  rawHsValue = analogRead(hsPin);
  voltageHsValue = map (rawHsValue, 0 , 1023, 0, 5000);
  mois = map (voltageHsValue, 300, 1250, 0, 57);

  //Collect data into package
  packet_t node;
  node.data_struct.header = 0x55;
  node.data_struct.node_id = 1;

  node.data_struct.humidity = hum;
  node.data_struct.temperature = temp;
  node.data_struct.moisture = mois;
  node.data_struct.radiation = rad;

  //Start timer
  cur_time = millis();
  if (cur_time - pre_time >= 120000)
  {
    //Sending data though XBee
    for (int i = 0; i < 18; i++)
    {
      XBee.write(node.data_byte[i]);
    }

    pre_time = cur_time;
  }
}
