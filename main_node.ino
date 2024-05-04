#include <HardwareSerial.h>

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


String str;
unsigned long currTime;
unsigned long startmillis;
const unsigned long period = 30000;

String sync_value = "1000";                       //sync value
String ack = "10";                                //ack value
String WLS_ID = "101";                            //WATER LEVEL SENSOR ID
String TS_ID = "102";                             //TEMPERATURE SENSOR ID
String tx_frame = sync_value + WLS_ID + TS_ID;    //SYNC FRAME

void setup() {
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);  // Resetting RN2483 by pulling RST pin low in 200 ms
  delay(200);
  digitalWrite(RST, HIGH);
  led_off();

  // Open serial communications and wait for port to open:

  Serial.begin(57600);

  mySerial.begin(57600, SERIAL_8N1, RXD2, TXD2);  //baudrate, 8bits+1parity, RX, TX
  mySerial.setTimeout(1000);
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

  //  mySerial.println("radio set bt 0.5");
  //  wait_for_ok();

  mySerial.println("radio set mod lora");  // lora modulation
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

  mySerial.println("radio set rxbw 20.8");  // Receiver bandwidth can be adjusted here. Lower BW equals better link budget / SNR (less noise).
  str = mySerial.readStringUntil('\n');     // However, the system becomes more sensitive to frequency drift (due to temp) and PPM crystal inaccuracy.
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

  mySerial.println("radio set cr 4/5");  // Maximum reliability is 4/8 ~ overhead ratio of 2.0
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
}
//Main operational loop
void loop() {
  lora_send_sync();                             //Broadcasting sync frame to the sensor nodes
  lora_receive();                               //Receiving ack from WL sensor
  delay(500);
  lora_receive();                               //Receving ack from T sensor
  delay(500);
  lora_receive_ack();                           //Receivng data from WL sensor + sending ack
  delay(1000);
  lora_receive_ack();                           //Receivng data from T sensor + sending ack
  Serial.println("Entered sleeping mode");
  mySerial.println("sys sleep 60000");          //Entering sleep mode for 1 minute (demonstration purposes)
  delay(60000);                                 //Wait for sleep mode before starting next cycle
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
 * Function for sending sync frame 
 */

void lora_send_sync() {
  Serial.println("Broadcasting sync");
  mySerial.print("radio tx ");
  mySerial.println(tx_frame);
  str = mySerial.readStringUntil('\n');
  //Serial.println(str);
  str = mySerial.readStringUntil('\n');
  Serial.println(str);
  Serial.println("waiting for ack...");

}
/*
 *Receiving over LoRa, with ack
*/
void lora_receive_ack() {
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
      toggle_led();
      Serial.println(str);  //printing received data
    } else {
      Serial.println("Received nothing");
    }
  } else {
    Serial.println("radio not going into receive mode");
    delay(1000);
  }

  delay(1000);

  Serial.println("sending ack");
  mySerial.print("radio tx ");
  mySerial.println(ack);  //ack to transmit
  str = mySerial.readStringUntil('\n');
  //Serial.println(str);
  str = mySerial.readStringUntil('\n');
  Serial.println(str);
}

/*
 *Reciving over LoRa, no ack
*/

void lora_receive() {
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
      toggle_led();
      Serial.println(str);  //printing received data
    } else {
      Serial.println("Received nothing");
    }
  } else {
    Serial.println("radio not going into receive mode");
    delay(1000);
  }

  delay(1000);
}

void toggle_led() {
  digitalWrite(13, 1);
  delay(100);
  digitalWrite(13, 0);
}

void led_on() {
  digitalWrite(13, 1);
}

void led_off() {
  digitalWrite(13, 0);
}
