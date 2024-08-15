#include <Adafruit_GFX.h>

#include <SPI.h>
#include <Wire.h>
#include "BluetoothSerial.h"

//config afficheur
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define USE_NAME

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS \
  0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306_EMULATOR display(SCREEN_WIDTH,
                                  SCREEN_HEIGHT,
                                  &Wire,
                                  OLED_RESET);

BluetoothSerial SerialBT;

#ifdef USE_NAME
String device = "arm";  // Change this to reflect the real name of your slave BT device
#endif

String myName = "gentlet";




//config gyro
const int MPU = 0x68;
int16_t GyX, GyY;
String direction;


int button = 23;
int pince = 0;
int Gled = 17;
int Rled = 4;
volatile byte mode = LOW;

void setup() {
  //connexion bluetooth
  bool connected;
  Serial.begin(115200);
  SerialBT.begin(myName, true);
  Serial.printf("The device \"%s\" started in master mode, make sure slave BT device is on!\n", myName.c_str());

// #ifndef USE_NAME
//   SerialBT.setPin(pin);
//   Serial.println("Using PIN");
// #endif

// connect(address) is fast (up to 10 secs max), connect(slaveName) is slow (up to 30 secs max) as it needs
// to resolve slaveName to address first, but it allows to connect to different devices with the same name.
// Set CoreDebugLevel to Info to view devices Bluetooth address and device names
#ifdef USE_NAME
  connected = SerialBT.connect(device);
  Serial.printf("Connecting to slave BT device named \"%s\"\n", device.c_str());
#endif

  #if connected 
    Serial.println("Connected Successfully!");
  #else
    while (!SerialBT.connected(10000)) {
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 10);
      display.println("   error  ");
      display.display();
    }
#endif

// if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
//     Serial.println(F("SSD1306 allocation failed"));
//     for (;;)
//       ;
//   }
  pinMode(pince, INPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(Gled, OUTPUT);
  pinMode(Rled, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(button), switchmode, CHANGE);

  
  display.clearDisplay();
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

void switchmode() {
  mode = !mode;
}

void loop() {

  if (mode == 0) {
    //reset position
    direction = "n";

    //sensor code
    digitalWrite(Gled, 1);
    digitalWrite(Rled, 0);
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 12, true);

    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();

    //conversion des coordonnées pour une meilleurs compréhension
    int X = map(GyX, -16000, 16000, -100, 100);
    int Y = map(GyY, -16000, 16000, -100, 100);


    //affichage des informations
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("      Gentlet      ");
    display.print(X);
    display.println(" X");
    display.print(Y);
    display.println(" Y");
    if (!digitalRead(pince)) {
      display.setCursor(0, 24);
      display.print("pince");
      direction = "p";
    }
    if (X > 50) {
      display.print("backward");
      direction = "b";
    } else if (X < -50) {
      display.print("forward");
      direction = "f";
    }
    if (Y > 50) {
      display.print("-right");
      direction = "r";
    } else if (Y < -50) {
      display.print("-left");
      direction = "l";
    }

    display.display();
    delay(500);
    display.clearDisplay();

    //bluetooth send
    SerialBT.print(direction);

  } else if (mode == 1) {
    digitalWrite(Gled, 0);
    digitalWrite(Rled, 1);

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("   phone  ");
    display.display();
    display.clearDisplay();
  }
}
