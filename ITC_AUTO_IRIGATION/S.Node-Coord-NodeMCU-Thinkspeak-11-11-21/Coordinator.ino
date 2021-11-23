/*
   "Recieving data from XBee and sending to NodeMCU"

   +-------+---------------+
   |  UNO  | XBee (Shield) |
   +-------+---------------+
   | 0(RX) |     0 TX      |
   | 1(TX) |     1 RX      |
   |  GND  |      GND      |
   |  5V   |      5V       |
   +-------+---------------+

   +-------+---------+
   |  UNO  | NodeMCU |
   +-------+---------+
   | 8(RX) |   TX    |
   | 9(TX) |   RX    |
   |  GND  |   GND   |
   +-------+---------+

   By Khmaoch
*/

#include <SoftwareSerial.h>
#include <XBee.h>

//Define XBee serial pin
HardwareSerial &XBee = Serial;

//Define package structure
/*  +--------+---------+----------+-------------+----------+-----------+------------+
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

//Predefine Parameters
//mois1 and 2 should more than 50 in order to prevent relay activate in case one node is not available
float temp1, hum1, mois1 = 50.0, rad1;
float temp2, hum2, mois2 = 50.0, rad2;

//Define NodeMCU serial pin
SoftwareSerial esp(8, 9);

//Define relay pin
#define relay1Pin 5

void setup()
{
  Serial.begin(9600);
  XBee.begin(9600);
  esp.begin(115200);
}

void loop() {
  //Initialize and start recieved data from XBee
  if (XBee.available() > 0)
  {
    packet_t coord;

    for (int i = 0; i < 18; i++)
    {
      coord.data_byte[i] = XBee.read();
    }

    //Send data to NodeMCU
    esp.write(coord.data_byte, 18);

    //Verify if packet belong to the system
    if (coord.data_struct.header == 0x55)
    {
      //Store data from SENSOE NODE 1
      if (coord.data_struct.node_id == 1)
      {
        temp1 = coord.data_struct.temperature;
        hum1 = coord.data_struct.humidity;
        mois1 = coord.data_struct.moisture;
        rad1 = coord.data_struct.radiation;
      }

      //Store data from SENSOE NODE 2
      if (coord.data_struct.node_id == 2)
      {
        temp2 = coord.data_struct.temperature;
        hum2 = coord.data_struct.humidity;
        mois2 = coord.data_struct.moisture;
        rad2 = coord.data_struct.radiation;
      }
    }
  }
}

void irrigation()
{
  //Define when to start the irrigation
  if (mois1 <= 32.00 or mois2 <= 32.00)
  {
    digitalWrite(relay1Pin, HIGH);
  }

  //Define when to stop the irrigation
  if (mois1 >= 34.00 and mois2 >= 34.00)
  {
    digitalWrite(relay1Pin, LOW);
  }
}
