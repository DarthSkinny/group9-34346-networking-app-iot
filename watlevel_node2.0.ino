#include <HardwareSerial.h>
#include <Arduino.h>
////////////////main changes in relation to code for ESP8266 ////////////////
HardwareSerial mySerial(2);  //enabling UART2 on GPIO16/17

//////Connections///////
//RX2 on ESP <-> TX on Lora module
//TX2 on ESP <-> RX on Lora module
//3V3 on ESP <-> 3V3 on Lora module
//GND on ESP <-> GND on Lora module
//D23 on ESP <-> RST on Lora module

#define RXD2 16  //GPIO16 on ESP32
#define TXD2 17  //GPIO17 on ESP32
#define RST 23   //RESET pin for RN2483 module
////////////////////////////////////////////////////////////////////////////
//Defining additional pins for control purpose
#define POWER 2
#define SIGNAL 4

int value = 0;

int level = 0;


String str;
int bc_val;
int packageID = 0;
int ack = 10;   //acknowledgement value
String send_callback;

void setup() {

  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, LOW);

  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);  // Resetting RN2483 by pulling RST pin low in 200 ms
  delay(200);
  digitalWrite(RST, HIGH);

  led_off();

  // Open serial communications and wait for port to open:
  Serial.begin(57600);  // Serial communication to PC

  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);  //baudrate, 8bits+1parity, RX, TX
  mySerial.setTimeout(1000); //Timeout for serial communication
  //lora_autobaud();

  led_on();
  delay(1000);
  led_off();

  Serial.println("Initing LoRa");

  str = mySerial.readStringUntil('\n');
  Serial.println(str);
  mySerial.println("sys get ver");  //request version related info from RN2483
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("mac pause");  //disabling LoraWAN
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  //  mySerial.println("radio set bt 0.5");  // Uncomment if we want to use FSK
  //  wait_for_ok();

  mySerial.println("radio set mod lora");  // Comment if we want to use FSK
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set freq 864125000");  //frequency
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set pwr 5");  //tx power
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set sf sf10");  //spread factor 10 (modules were close by)
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set afcbw 41.7");  //auto-freq correction
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set rxbw 125");  //receiver bandwidth
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  //  mySerial.println("radio set bitrate 50000");
  //  wait_for_ok();

  //  mySerial.println("radio set fdev 25000");
  //  wait_for_ok();

  mySerial.println("radio set prlen 8");  //preamble length
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set crc on");  //CRC calculation on
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set iqi off");  // IQ invert state
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set cr 4/5");  //coding rate for fixing errors
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set wdt 60000");  //disable for continuous reception
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set sync 12");  //sync word
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  mySerial.println("radio set bw 125");  //operating radio bandwidth
  str = mySerial.readStringUntil('\n');
  Serial.println(str);

  Serial.println("starting loop");
}
//Main operational loop
void loop() {
  Serial.println("waiting for sync");           //Node listening for sync from master
  lora_receive_sync_ack();                      //Reception of sync value + ID
  delay(5000);                                  //Waiting to simulate processing time
  lora_send_wl();                               //Sending water level
  lora_receive();                               //Receiving ack
  delay(1000);
  Serial.println("Entering sleeping mode");
  mySerial.println("sys sleep 50000");          //Set LoRa to sleep for 50 sec (demonstration purposes)
  delay(50000);                                 //Wait for sleep mode before starting next cycle
  str = mySerial.readStringUntil('\n');
  Serial.println(str);
}

/*
 * This function blocks until the word "ok\n" is received on the UART,
 * or until a timeout of 3*5 seconds.
 */
int wait_for_ok() {
  str = mySerial.readStringUntil('\n');
  if (str.indexOf("ok") == 0) {
    return 1;
  } else return 0;
}

/*
 *Receiving sync over LoRa with ack
*/
void lora_receive_sync_ack() {
  mySerial.println("radio rx 0");
  str = mySerial.readStringUntil('\n');
  //Serial.println(str);
  delay(20);
  if (str.indexOf("ok") == 0) {
    str = String("");
    while (str == "") {
      str = mySerial.readStringUntil('\n');
    }
    if (str.indexOf("radio_rx") == 0)  //checking if data was received (equals radio_rx = <data>). indexOf returns position of "radio_rx"
    {
      Serial.println(str);
      //printing received data
    } else {
      Serial.println("Received nothing");//Error if no data recieved
      return;
    }
  }
    else {
      Serial.println("radio not going into receive mode");
      delay(1000);
    }

    delay(2000);

    if (str.length() == 21) {                       //checking if sync value is correct
      Serial.println("sending ack");
      mySerial.print("radio tx ");
      mySerial.println(ack);                        //ack to transmit
      send_callback = mySerial.readStringUntil('\n');
      //Serial.println(send_callback);
      send_callback = mySerial.readStringUntil('\n');
      Serial.println(send_callback);
    }
  }
/*
 *Sending water level over LoRa
*/
  void lora_send_wl() {
    level = waterSensor();// Read the water level sensor
    Serial.println("sending sensor data");
    mySerial.print("radio tx ");//Transmit sensor data
    mySerial.println(level);  //payload to transmit
    send_callback = mySerial.readStringUntil('\n');
    //Serial.println(send_callback);
    send_callback = mySerial.readStringUntil('\n');
    Serial.println(send_callback);
  }

/*
 *Receiving data over LoRa no ack
*/
  void lora_receive() {
    Serial.println("waiting for ack...");
    mySerial.println("radio rx 0");
    str = mySerial.readStringUntil('\n');
    //Serial.println(str);
    delay(20);
    if (str.indexOf("ok") == 0) {
      str = String("");
      while (str == "") {
        str = mySerial.readStringUntil('\n');
      }
      if (str.indexOf("radio_rx") == 0)  //checking if data was received (equals radio_rx = <data>). indexOf returns position of "radio_rx"
      {
        Serial.println(str);  //printing received data
      } else {
        Serial.println("Received nothing");
        return;
      }
    } else {
      Serial.println("radio not going into receive mode");
      delay(1000);
    }

    delay(1000);
  }

/*
 *Reading water level from sensor and returning
*/
  int waterSensor() {
    digitalWrite(POWER, HIGH);
    delay(10);
    value = analogRead(SIGNAL);
    delay(10);
    digitalWrite(POWER, LOW);
    return value;
  }
  
/*
 *Turning on a LED
*/
  void led_on() {
    digitalWrite(13, 1);
  }
/*
 *Turning off a LED
*/
  void led_off() {
    digitalWrite(13, 0);
  }
