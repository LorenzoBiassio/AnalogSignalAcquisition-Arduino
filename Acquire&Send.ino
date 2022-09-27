#include "BluetoothSerial.h"
#include <ArduinoJson.h>

// BT based of BT examples library
// packaging based of multiprocessing code of Daniel C. and Thiago D.

/*definition of BT, and error prevention*/
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT; //define BT funcions header
/*---------------------------------------------*/

#define signal 15      //define the input pin for esp32
#define bufferSize 64  //define the amount of data stored inside de buffer

/*Global declaration for blinking led withoud delay*/
int ledState = LOW;             // ledState used to set the LED
unsigned long LEDpreviouMicros = 0;        // will store last time LED was updated
const long LEDinterval = 1000000;         //set the interval for blinking
/*---------------------------------------------*/

/*Global declaration for setting transfer speed*/
unsigned long BUFFERpreviousMicros = 0;       
const long BUFFERinterval = 1000;       //sets time interval (in microsec) between each data read
/*---------------------------------------------*/

int iteracoes = 0;  //counter for mesuring loop cicles in 1 second 

/*Global declaration for packaging data   (NEEDS REWORK)*/
int buffer [bufferSize]; //create buffer
const size_t CAPACITY = JSON_ARRAY_SIZE(bufferSize);
int bufferIndex = 0; //index for counting buffer readyness
/*---------------------------------------------*/


String DataPrep(){
  StaticJsonDocument<CAPACITY> doc; // allocate the memory for the document

  JsonArray msg = doc.to<JsonArray>(); // create an empty array
  for (int i = 0; i < bufferSize; i++){
      msg.add(buffer[i]);
  }
  // serialize the array and sed the result to Serial
  String json;
  serializeJson(msg, json);
  return json;
}

bool bufferBuild(float valueRead, unsigned long currentMicros){
  if(currentMicros - BUFFERpreviousMicros >= BUFFERinterval){
    //Serial.println(currentMicros - BUFFERpreviousMicros);      //*debug*
    BUFFERpreviousMicros = currentMicros;  

    buffer[bufferIndex] = valueRead;
    //Serial.print(bufferIndex); Serial.print(" - "); Serial.print(buffer[bufferIndex]); Serial.print(" - "); Serial.println(valueRead);     //*debug*
    bufferIndex++;

    if (bufferIndex >= bufferSize){
      //Serial.println(bufferIndex);    //*debug*
      bufferIndex = 0;
      return 1;
    }
  }
  return 0;
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT); //setup builtin led
  pinMode(signal, OUTPUT); //setup input pin

  Serial.begin(115200);
  SerialBT.begin("ESP32_BT"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }

  unsigned long currentMicros = micros(); //using microsec precision reduces the code velocity to ~=10380 cicles per sec

  //this if section is only for blinking the onboard LED
  if (currentMicros - LEDpreviouMicros >= LEDinterval) {
    Serial.print(iteracoes);  Serial.print(" --- "); Serial.println(currentMicros - LEDpreviouMicros); //*debug*    //print elapsed loop time in milliseconds and the count of iterations

    LEDpreviouMicros = currentMicros;        //saves the last time you blinked the LED
    iteracoes = 0;

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(LED_BUILTIN, ledState);
  }

  bool bufferFlag = 0;

  bufferFlag = bufferBuild(analogRead(signal), currentMicros);   //call the buffer builder function every loop cicle, now using raw analog input and microsec precision
  //Serial.print("------------"); Serial.println(bufferFlag);     //*debug*
  
  if(bufferFlag == 1){
    SerialBT.println(DataPrep());           //print data to bluetooth serial
    //Serial.print("DATA- ");Serial.println(DataPrep());         //*debug*
  }
  
  iteracoes++;         //increases the loop cicles counter
    
} 
