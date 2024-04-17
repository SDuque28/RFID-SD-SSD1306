#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Pines de la RFID
#define RFID_SS_PIN 5 // (SDA)
#define RFID_RST_PIN 0
// #define RFID_MISO 19
// #define RFID_MOSI 23
// #define RFID_SCK  18 

//Pines de la tarjeta SD
#define SD_SS 15 //(CS)
// #define SD_SCK 14
// #define SD_MISO 12
// #define SD_MOSI 13

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
 
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

File myFile;
SPIClass SPI2(HSPI);

void escribir(String Palabra){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("The UID tag is:");
  display.println(Palabra);
  display.display();
}

void WriteFile(String path, String message){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(path, FILE_APPEND);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.printf("Writing to %s ", path);
    while (Serial.available() == 0) {}
    String name = Serial.readString();
    myFile.println(message + "," + name);
    myFile.close(); // close the file:
    Serial.println("completed.");
  } 
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening file ");
    Serial.println(path);
  }
}

void ReadFile(const char * path, String uid){
  // open the file for reading:
  myFile = SD.open(path);
  if (myFile) {
     Serial.printf("Reading file from %s\n", path);
     // read from the file until there's nothing else in it:
    while (myFile.available()) {
      String line = myFile.readStringUntil('\n'); // Lee una línea completa del archivo
      if(line.startsWith(uid)){
        int commaIndex = line.indexOf(','); // Busca la posición de la coma
        if(commaIndex != -1){
          String owner = line.substring(commaIndex + 1); // Extrae el dueño de la tarjeta
          Serial.print("El dueño de la tarjeta es: ");
          Serial.println(owner);
          myFile.close(); // Cierra el archivo antes de salir de la función
          return; // Sale de la función después de encontrar la UID
        }
      }
    }
    myFile.close(); // close the file:
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  if (!SD.begin(SD_SS,SPI2)) {
    Serial.println("Error al inciar el Modulo MicroSD");
  }
  Serial.println(F("This code scan the MIFARE Classsic UID."));
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
  }
}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  Serial.println(F("A new card has been detected."));

  String content = "";
  // Store UID into content String
  for (byte i = 0; i < 4; i++) {
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX));
  }
    
  Serial.print(F("The UID tag is:"));
  Serial.println(content);
  escribir(content);
  //WriteFile("/test.txt", content);
  ReadFile("/test.txt",content);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}