#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "BluetoothSerial.h"
#include <ArduinoWebsockets.h>
#include <WiFi.h>


Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
BluetoothSerial SerialBT;

String device_name = "arm";
String Log[50];

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#define SERVOMIN 150   // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 600   // This is the 'maximum' pulse length count (out of 4096)
#define USMIN 600      // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 2400     // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50  // Analog servos run at ~50 Hz updates

int button = 23;
int pince = 4;
byte ouvert = 0;
int lampe = 18;
volatile byte mode = LOW;
double Fext = 150;
double Sext = 150;
double pivopulse = 320;
double speed = 10;



void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Client Connected");
  }
  if (event == ESP_SPP_CLOSE_EVT ) {
    Serial.println("Client disconnected");
    //SerialBT.flush();
    //SerialBT.disconnect();
    //SerialBT.end();
    //SerialBT.begin("ESP32BT");
    ESP.restart(); // needed to be able to reconnect
  }
}


// void sendLogsToClient() {
//   String logsToSend = "";
//   for (int i = 0; i < 50; i++) {
//     if (Log[i] != "") {
//       logsToSend += Log[i] + "\n";  // add log to send
//     }
//   }
//   if (logsToSend != "") {
//     client.send(logsToSend); //send log to client
//   }
// }

// void addToLog(String entry) {
//   String timestamp = String(year()) + "-" + String(month()) + "-" + String(day()) +
//                      " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
//   String logEntry = "[" + timestamp + "] " + entry;
// s
//   for (int i = 49; i > 0; i--) {
//     Log[i] = Log[i - 1];
//   }
//   Log[0] = logEntry; 

// sendLogsToClient();

// }


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);


    //bluetooth connexion
  SerialBT.register_callback(callback);
  SerialBT.begin(device_name);  //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
for(int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++) {
      Serial.print(".");
      delay(1000);
  }
  
  pwm.begin();
  pinMode(pince, INPUT);
  attachInterrupt(digitalPinToInterrupt(button), stopmode, CHANGE);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates



  //default position
  //motor pivo
  pwm.setPWM(0, 0, pivopulse);
  //fix motor
  pwm.setPWM(14, 0, 500);
  pwm.setPWM(15, 0, 600);
  //extension motor
  pwm.setPWM(2, 0, Sext);
  delay(1000);
  pwm.setPWM(1, 0, Fext);
  //attention la pince peut surchauffer et envoyé un retour de force
  pwm.setPWM(3, 0, 360);
  delay(2000);
}


String message = "";

void stopmode() {
  mode = !mode;
}

void loop() {
  //motor pivo = 0
  //motor vertical1 = 1
  //motor vertical2 = 2
  //motor pince = 3
  //bluetooth receive

 if (SerialBT.available() && mode == 0) {
  digitalWrite(lampe,1);
    char recev = SerialBT.read();
    if (recev != 'n') {
      message += String(recev);
    } else {
      message = "";
    }
    if (message == "p" && ouvert == 0) { 
      Serial.println("pince");
      pwm.setPWM(3, 0, 600);
      ouvert = 1;
    } else if (message == "p" && ouvert == 1){
      ouvert = 0;
      pwm.setPWM(3, 0, 370);
    }

    // go rigth
    if (message == "r") {
      Serial.println("R");
      pivopulse = pivopulse + speed;
      pwm.setPWM(0, 0, pivopulse);
    }
    
    // // go left
    if (message == "l") {
      Serial.println("L");
      pivopulse = pivopulse - speed;
      pwm.setPWM(0, 0, pivopulse);
    }

    //front
    if (message == "f") {
      Serial.println("E");
      if (Fext >= 250 && Sext < 400) {
        Fext += speed;
        Sext += speed;
        pwm.setPWM(1, 0, Fext);
        pwm.setPWM(2, 0, Sext);
      } else if (Fext < 500) {
        Fext = Fext + speed;
        pwm.setPWM(1, 0, Fext);
      }
    }

    //back
    if (message == "b") {
      if (Fext <= 270 && Sext > 150) {
        Fext -= speed / 2;
        Sext -= speed;
        pwm.setPWM(1, 0, Fext);
        pwm.setPWM(2, 0, Sext);
      } else if (Fext > 150) {
        Serial.println("B");
        Serial.println(Fext);
        Fext -= speed;
        pwm.setPWM(1, 0, Fext);
      }
    }
  }else{
    digitalWrite(lampe,0);
  } 
}


    



