
/*
  ESP32 Cheap Yellow Display (CYD) Pinout (ESP32-2432S028R)
  https://randomnerdtutorials.com/esp32-cheap-yellow-display-cyd-pinout-esp32-2432s028r/
  Here’s the RGB LED pinout:
  RGB LED	GPIO
  Red LED	GPIO 4
  Green LED	GPIO 16
  Blue LED	GPIO 17

  ESP32 Pinout Reference: Which GPIO pins should you use?
  https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
  Available Pins to Connect Peripherals
  So, in summary, you have three pins available to connect peripherals:
  GPIO 35 — on the P3 connector
  GPIO 22 — on the P3 and CN1 connector
  GPIO 27 — on the CN1 connector
*/

/*
  Using Git with VS Code and PlatformIO
  https://www.youtube.com/watch?v=NdgMuZBpyo8

  VS Code Git status labels:
  A - Added (This is a new file that has been added to the repository)
  M - Modified (An existing file has been changed)
  D - Deleted (a file has been deleted)
  U - Untracked (The file is new or has been changed but has not been added to the repository yet)
  C - Conflict (There is a conflict in the file)
  R - Renamed (The file has been renamed)

  Setting up VSCode to push to a GitHub repository
  https://www.youtube.com/watch?v=mrGMxZkkIzg
*/

/*
  Error: The program size (1529481 bytes) is greater than maximum allowed (1310720 bytes)
  https://community.platformio.org/t/greater-than-maximum-allowed-1310720-bytes-ram-18-1-used-59404-bytes-from-327680-bytes-checkprogsize-explicit-exit-status-1-flash-103-0/32008/3

  board_build.partitions = huge_app.csv
  to your platformio.ini.

  or

i solved it the main issue was the enable
build_flags =
-DBOARD_HAS_PSRAM
so after lil research i modify it to

build_flags =
-DBOARD_HAS_PSRAM
-mfix-esp32-psram-cache-issue
-DCONFIG_MBEDTLS_DYNAMIC_BUFFER=1
-DCONFIG_BT_ALLOCATION_FROM_SPIRAM_FIRST=1
-DCONFIG_SPIRAM_CACHE_WORKAROUND=1

board_build.partitions = huge_app.csv
;no_ota.csv
extra_scripts = pre:build_script_versioning.py

so it work smoth and fixed my issue

or
so by changing my platform.ini fix this issue i dont know how

; PlatformIO Project Configuration File
;
; Build options: build flags, source filter
; Upload options: custom upload port, speed and extra flags
; Library options: dependencies, extra library storages
; Advanced options: extra scripting
;
; Please visit documentation for the other options and examples
; Redirecting... 24

[env:esp-wrover-kit]
platform = espressif32
board = esp-wrover-kit
framework = arduino
lib_deps = GitHub - gilmaimon/ArduinoWebsockets: A library for writing modern websockets applications with Arduino (ESP8266 and ESP32) 23

build_flags =
-DBOARD_HAS_PSRAM
-mfix-esp32-psram-cache-issue
-DCONFIG_MBEDTLS_DYNAMIC_BUFFER=1
-DCONFIG_BT_ALLOCATION_FROM_SPIRAM_FIRST=1
-DCONFIG_SPIRAM_CACHE_WORKAROUND=1

board_build.partitions = huge_app.csv
;no_ota.csv
extra_scripts = pre:build_script_versioning.py

monitor_speed = 115200
upload_port = COM5

*/

#include <Arduino.h>

#include <esp_task_wdt.h>
// ********************************************************************
// bool serialPrintLoopCoreStatus = false;
bool startCore0Thread = true;
// Create a Multi Core Task to display the Image onto the ILI9341 TFT Display using the <showimage()> function
TaskHandle_t ledBlinkingTaskByCore0;

int sucessYellowLedBlinkPeriod = 500;
unsigned long time_now = 0;

void sucessYellowLedBlinking(){
  if(millis() - time_now > sucessYellowLedBlinkPeriod){
        time_now = millis();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        // tft.drawString(WiFi.localIP().toString(), 20, 10);
        // Serial.println("Hello");
    }
}


void sucessYellowLedBlinkingInfinitelyByCore0(void * pvParameters){
  bool sucessYellowLedBlinkingInfinitelyByCore0Status = true;
  for(;;){
    /*
    if(millis() - time_now > sucessYellowLedBlinkPeriod){
          time_now = millis();
          digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
          // Serial.println("Hello");
      }
      */
     if(sucessYellowLedBlinkingInfinitelyByCore0Status == true){
      Serial.println("sucessYellowLedBlinkingInfinitelyByCore0");
      sucessYellowLedBlinkingInfinitelyByCore0Status = false;
     }
     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
     delay(1000);
  }
}
// ********************************************************************

#include <WiFi.h>

// Libraries for Displaying the video from the camera to th elocal connected ILI9341 TFT Display
#include <TFT_eSPI.h>
// #include <TJpg_Decoder.h>
TFT_eSPI tft = TFT_eSPI();
int tft_led = 1;

#include <Preferences.h>

Preferences preferences;

const int ledRedPin = 4; // Use the appropriate GPIO pin for your setup
const int ledGreenPin = 16; 
const int ledBluePin = 17; 
int ledStatus = 0;
String ledArray[] = {"NONE", "RED", "GREEN", "BLUE"};

/* ***************************************************
  WIFI variable declaration
*/
// std::string bluetooth_name = "ESP32-BLE-ROVER";
std::string bluetooth_name = "";
// String bluetooth_name = "";
String ssids_array[50];
String network_string;
String connected_string;

const char* pref_ssid = "";
const char* pref_pass = "";

String apply_SSID = "";
String apply_Password = "";

String client_wifi_ssid;
String client_wifi_password;

long start_wifi_millis;
long wifi_timeout = 10000; // Wifi Authenticate connection time out duration. 10000 means 10 sec.

enum wifi_setup_stages { NONE, GET_SSID, GET_PASSWORD, WIFI_START_SCAN, WIFI_SCAN_COMPLETE, WIFI_SELECT, AUTHENTICATE_WIFI, WIFI_CONNECT_FAIL, WIFI_CONNECT_SUCESS};
enum wifi_setup_stages wifi_stage = GET_SSID;
bool wifiConnectedSuccessfully = false;

bool bleIniConnected = true;
bool enterSSIDstatus = true;
// ***************************************************

/* ***************************************************
  BLE variable declaration   
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Create the BLE Server
// BLEServer *server = BLEDevice::createServer();
BLEServer* server = NULL;

BLECharacteristic *characteristicTX = NULL; // characteristicTX means BLE Client Transmit/Write to this BLE Server
BLECharacteristic *characteristicRX = NULL; // characteristicRX means BLE Client Receive/Read from this BLE Server
BLECharacteristic *characteristicNOTIFY = NULL; // This BLE Server will sent Notification message to connected client

bool deviceConnected = false; //BLE Client connection status
bool oldDeviceConnected = false;
const int LED = 2; // Optional LED connection to GPIO for ESP32 CAM Board
// const int BUZZER = x; // Optional Buzzer connection to GPIO for ESP32 CAM Board

#define SERVICE_UUID   "00000000-198e-4351-b779-901fa0e0371e"
#define CHARACTERISTIC_UUID_RX  "11111111-9736-4e5d-932b-e9b31405049c"
#define CHARACTERISTIC_UUID_TX  "22222222-198e-4351-b779-901fa0e0371e"
#define CHARACTERISTIC_UUID_NOTIFY  "33333333-198e-4351-b779-901fa0e0371e"


BLEDescriptor *pDescr_TX;                          // Pointer to Descriptor of Characteristic 1
BLEDescriptor *pDescr_NOTIFY;
BLEDescriptor *pDescr_RX;

// BLE2902 object is require for all Notification property
BLE2902 *pBLE2902_TX;                              // Pointer to BLE2902 of Characteristic 1
BLE2902 *pBLE2902_NOTIFY;                          // Pointer to BLE2902 of Characteristic 2
BLE2902 *pBLE2902_RX;
/* ***************************************************

/* ***************************************************
  BLE Function and Process Declaration   
*/
// BLE Callback function for BLE connect and disconnect
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      wifi_stage = GET_SSID;             
      }                

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// WiFi network scanning function
void scan_wifi_networks()
{
  // SerialBT.println("@@@ In scan_wifi_networks() function @@@@@@@@@@@@");
  Serial.println("@@@@@@@@@@@@@@ In scan_wifi_networks() function @@@@@@@@@@@@");
  
  WiFi.mode(WIFI_STA);
  // WiFi.scanNetworks will return the number of networks found
  int n =  WiFi.scanNetworks();
  if (n == 0) {
    // SerialBT.println("no networks found");
    std::string msg = "no networks found";
    characteristicNOTIFY->setValue(msg);

    // String msg = "no networks found";
    // characteristicNOTIFY->setValue(msg);
    characteristicNOTIFY->notify(); 
    Serial.println(msg.c_str()); 
  } else {
    delay(1000);
    for (int i = 0; i < n; ++i) {
      ssids_array[i + 1] = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(ssids_array[i + 1]);
      network_string = i + 1;
      network_string = network_string + ": " + WiFi.SSID(i) + " (Strength:" + WiFi.RSSI(i) + ")";
    }
    wifi_stage = WIFI_SCAN_COMPLETE;
  }
}

// std::string bluetooth_name = "ESP32-BLE-ROVER";

// BLE Callback function for BLE when BLE Client Transmit/Write to this BLE Server
class CharacteristicCallbacks: public BLECharacteristicCallbacks {
    // onWrite means Client send data to Server
     void onWrite(BLECharacteristic *characteristic) {
          std::string rxValue = characteristic->getValue(); 
        //   String rxValue = characteristic->getValue();
          String revStr = rxValue.c_str();
          Serial.print("Serial Received First Char: ");          
          Serial.println(revStr.substring(0,1)); 
          
          if (revStr.substring(0,1) == "*") {
            // std::string id = revStr.c_str();
            String id = revStr.c_str();
            // id = id.substring(1);  
            char str[id.length() + 1] = {};
            strcpy(str, id.c_str()); 
            String roverIdTemp1 = str; 
            String roverIdTemp2 = "ESP32_CYD_BLE_V1 " + roverIdTemp1.substring(1);
            bool doesExist = preferences.isKey("roverName");
            if (doesExist == true) {
              preferences.remove("roverName");
              delay(500);
            }
            // preferences.begin("RoverDeviceName", false);
            preferences.putString("roverName", roverIdTemp2);
            
            // std::string roverId = roverIdTemp2.substring(1).c_str();  
            // ************************ 
            // std::string newRoverId = roverIdTemp2.c_str();  
            // bluetooth_name = newRoverId;  
            // ************************    
            // id = revStr.c_str().substring(1);
            // bluetooth_name = "ESP32-BLE-ROVER" + " " + rxValue.substring(1);
            // bluetooth_name = roverId;
            // rxValue = "";
            Serial.print("Rover Cam New ID Updated to :"); 
            Serial.println(roverIdTemp2.c_str()); 
            delay(2000); // give the bluetooth stack the chance to get things ready
            // server->startAdvertising(); // restart advertising
            // Serial.println("start advertising");
            ESP.restart();
          }
          // Check whether is there any received data
          else if (rxValue.length() > 0 && wifi_stage != AUTHENTICATE_WIFI) {
            //    for (int i = 0; i < rxValue.length(); i++) {
            //  Serial.print(rxValue[i]);
            //    }
                Serial.print(rxValue.c_str());
                String temp = rxValue.c_str(); 
                String data = temp;    
                Serial.println(data);
              
              if(wifi_stage == GET_SSID && apply_SSID == "") {
                std::string msg = "**Enter SSID"; 
                // String msg = "**Enter SSID"; 

                characteristicNOTIFY->setValue(msg);
                characteristicNOTIFY->notify(); 
                Serial.println(msg.c_str());                
              }
              if (wifi_stage == GET_SSID) {
                apply_SSID = data;
                wifi_stage = GET_PASSWORD; 
                String msgStr = "SSID: " + data;
                Serial.println(msgStr);   
                std::string msg = "Enter Password";      
                // String msg = "Enter Password";     
                characteristicNOTIFY->setValue(msg); 
                characteristicNOTIFY->notify();  
                Serial.println(msg.c_str());                  
              } 
              else if (wifi_stage == GET_PASSWORD && data != apply_SSID) {
                if(apply_Password != apply_SSID){
                  apply_Password = data;                  
                  // wifi_stage = AUTHENTICATE_WIFI;
                  wifi_stage = WIFI_START_SCAN;
                  String msgStr = "Password: " + data;
                  Serial.println(msgStr); 
                  std::string msg = "Connecting WIFI";      
                  characteristicNOTIFY->setValue(msg); 
                  // characteristicNOTIFY->notify(); 
                }
                else {
                  wifi_stage = GET_PASSWORD; 
                  apply_Password = "";                
                }
                  
              } 
          }
                  
     }//onWrite
};
// ****************************************************

void displayInit(){
  tft.begin();
//   tft.setRotation(1);// value 1 and 3 are in landscape mode where value of 0 and 2 are in portrait mode
  tft.setRotation(3);
  tft.fillScreen(TFT_BLUE);

  // TJpge decoding and display the JPEG onto the ILI9341 TFT display by calling the Callback Function => <tft_output>
  // TJpgDec.setJpgScale(1);
  // TJpgDec.setSwapBytes(true);
  // TJpgDec.setCallback(tft_output);
} 

// ===========================
// Stand routine for ESP32 CAM
// ===========================
const char* ssid = "";
const char* password = "";


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // declaring LED pin as output
  // pinMode(ledRedPin , OUTPUT);
  // declaring LED pin as output
  pinMode(LED_BUILTIN , OUTPUT);


  displayInit();
  pinMode(tft_led , OUTPUT);
  digitalWrite(tft_led, HIGH);

  preferences.begin("RoverDeviceName", false);
  bool doesExist = preferences.isKey("roverName");
  if (doesExist == true) {
    // preferences.putString("roverName", "ESP32-BLE-ROVER");
    String rover = preferences.getString("roverName");
    bluetooth_name = rover.c_str();
    Serial.print("*************** Rover Name Key exist: "); 
    Serial.println(rover); 
  }
  else {
    bluetooth_name = "ESP32-BLE-ROVER";
    Serial.print("?????????? Rover Name Key does not exist!!!");
  }  

/* ###################################################
  BLE will be setup and start here
*/
  // Create the BLE Device
  // BLEDevice::init("ESP32-BLE-ROVER"); // nome do dispositivo bluetooth
  BLEDevice::init(bluetooth_name);
  
  // Create the BLE Server
  BLEServer *server = BLEDevice::createServer();
  
  server->setCallbacks(new ServerCallbacks()); //set BLE Callback Function

  // Create the BLE Service
  BLEService *service = server->createService(SERVICE_UUID);
  
  // Create a BLE characteristicTX for this Service
  characteristicTX = service->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_WRITE
      // BLECharacteristic::PROPERTY_READ   |
      // BLECharacteristic::PROPERTY_WRITE  |
      // BLECharacteristic::PROPERTY_NOTIFY
      );
  // Create a description for this characteristicTX with the pDescr_TX object
  pDescr_TX = new BLEDescriptor((uint16_t)0x2901);
  pDescr_TX->setValue("SEND CMD TO BLE");
  // pBLE2902_TX = new BLE2902();
  // pBLE2902_TX->setNotifications(true); // This will enable the Notificatyion to start automatically without the client to initiate it
  characteristicTX->addDescriptor(pDescr_TX);
  // characteristicTX->addDescriptor(pBLE2902_TX);  
  // Since this pDescr_TX doesn't define PROPERTY_NOTIFY therefore no need to add BLE2902() object
  // characteristicTX->addDescriptor(new BLE2902());

  // Create a BLE characteristicRX for this Service
  characteristicRX = service->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_READ
      );
  pDescr_RX = new BLEDescriptor((uint16_t)0x2901);
  pDescr_RX->setValue("CLIENT READ DATA");
  pBLE2902_RX= new BLE2902();
  // pBLE2902_NOTIFY->setNotifications(true); // This will enable the Notificatyion to start automatically without the client to initiate it
  characteristicRX->addDescriptor(pDescr_RX);
  // characteristicNOTIFY->addDescriptor(pBLE2902_NOTIFY);   

  // Create a BLE characteristicNOTIFY for this Service
  characteristicNOTIFY = service->createCharacteristic(
      CHARACTERISTIC_UUID_NOTIFY,
      BLECharacteristic::PROPERTY_NOTIFY
      );
  pDescr_NOTIFY = new BLEDescriptor((uint16_t)0x2901);
  pDescr_NOTIFY->setValue("BLE NOTIFY CLIENT");
  pBLE2902_NOTIFY = new BLE2902();
  // This will enable the Notificatyion to start automatically without the client to initiate it
  pBLE2902_NOTIFY->setNotifications(true); 
  characteristicNOTIFY->addDescriptor(pDescr_NOTIFY);
  characteristicNOTIFY->addDescriptor(pBLE2902_NOTIFY);    
  
  // // Create a BLE Characteristic para recebimento de dados
  // BLECharacteristic *characteristic = service->createCharacteristic(
  //     CHARACTERISTIC_UUID_RX,
  //     BLECharacteristic::PROPERTY_WRITE
  //     );

  // // Create a BLE Characteristic para envio de dados
  // characteristicTX = service->createCharacteristic(
  //     CHARACTERISTIC_UUID_TX,
  //     BLECharacteristic::PROPERTY_READ   |
  //     BLECharacteristic::PROPERTY_NOTIFY
  //     );

  characteristicTX->setCallbacks(new CharacteristicCallbacks());
  
  // Start the service
  service->start();
  
  // Start advertising (descoberta do ESP32)
  // server->getAdvertising()->start();/////////////////////////////
  BLEDevice::startAdvertising();  
  Serial.print("**** ESP32_CYD_BLE: "); 
  Serial.print(bluetooth_name.c_str()); 
  Serial.println(" ***"); 
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


  if (WiFi.status() != WL_CONNECTED) {
        // if (bleIniConnected == true) {
        //   // std::string msg = "Ming BLE Connected"; 
        //   // characteristicNOTIFY->setValue(msg);
        //   // characteristicNOTIFY->notify();
        //   // Serial.println(msg.c_str()); 
        //   // delay(1000);
        //   bleIniConnected = false;
        // }    

        if (wifi_stage == GET_SSID && enterSSIDstatus == true) {
          std::string msg = "Enter SSID"; 
          // String msg = "Enter SSID";     
          characteristicNOTIFY->setValue(msg);
          characteristicNOTIFY->notify(); 
          Serial.println(msg.c_str());
          enterSSIDstatus = false;
        }    

        // *****************************************
        start_wifi_millis = millis();        
        // *****************************************************
        if (wifi_stage == WIFI_START_SCAN) {
            // WIFI_START_SCAN, WIFI_SCAN_COMPLETE, AUTHENTICATE_WIFI,  
            std::string msg = "WIFI SCAN START";      
          //   String msg = "WIFI SCAN START";  
            characteristicNOTIFY->setValue(msg); 
            characteristicNOTIFY->notify();                
            scan_wifi_networks();
            delay(500);
            wifi_stage = WIFI_SCAN_COMPLETE;                             
          }
          else if (wifi_stage == WIFI_SCAN_COMPLETE) {
            wifi_stage = AUTHENTICATE_WIFI;
            std::string msg = "Connecting WIFI";    
          //   String msg = "Connecting WIFI";      
            characteristicNOTIFY->setValue(msg); 
            characteristicNOTIFY->notify();
          }
        // *****************************************************
          else if(wifi_stage == AUTHENTICATE_WIFI) {
            // wifiConnectedSuccessfully = connectToWifi(String apply_SSID, String apply_Password);
            // **************************************************************
            // start_wifi_millis = millis();
            // WiFi.begin(pref_ssid, pref_pass);  
            WiFi.begin(apply_SSID, apply_Password);
            while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              Serial.print(".");
              if (millis() - start_wifi_millis > wifi_timeout) {
                WiFi.disconnect(true, true);
                wifi_stage == GET_SSID; 
                wifiConnectedSuccessfully = false;                     
                break;
              }
              // Serial.println("connectToWifi function => Fail to connect Wifi");     
              // wifiConnectedSuccessfully = false;  
            } 
            if(WiFi.status() == WL_CONNECTED) {
              // Will return True when Wifi is connected before timeout
              Serial.println("connectToWifi function => Wifi Connected Successfully"); 
              wifiConnectedSuccessfully = true; 
              std::string msg = "WIFI Success";      
              // String msg = "WIFI Success";      
              characteristicNOTIFY->setValue(msg); 
              characteristicNOTIFY->notify();        
              delay(2000);
              Serial.println(WiFi.localIP());  
              // String ipAddress = WiFi.localIP().c_str();
              connected_string = bluetooth_name.c_str();
              connected_string = connected_string + " IP: ";
              connected_string = connected_string + WiFi.localIP().toString();
              String ipConnect = WiFi.localIP().toString();    
              msg = "IP: ";        
              msg = msg + WiFi.localIP().toString().c_str();  
              std::string stdStr(ipConnect.c_str(), ipConnect.length());
              // characteristicNOTIFY->setValue(std::string stdStr(ipConnect.c_str(), ipConnect.length())); 
              characteristicNOTIFY->setValue(msg); 
              characteristicNOTIFY->notify();              
              Serial.println(connected_string); 

              // tft.loadFont("666", true); // Must load the font first
              tft.setTextColor(TFT_BLACK, TFT_WHITE);
              tft.drawString(WiFi.localIP().toString(), 50, 50, 7);
              // tft.drawCentreString(connected_string, 120, 160, 30);
              // delay(5000);


              /*
                Ming comment on 050624
                Trying to stream video from the camera to the locally connected ILI9341 TFT display               
              */
              // showingImage();                                          
            } 
            else {
              wifiConnectedSuccessfully = false; 
              std::string msg = "WIFI Fail";   
              // String msg = "WIFI Fail";   
              characteristicNOTIFY->setValue(msg); 
              characteristicNOTIFY->notify(); 
              wifi_stage == GET_SSID;  
              apply_Password = "";
              apply_SSID = "";                                 
            }

            // ***************************************************************

            
            if (wifiConnectedSuccessfully == true) {        
              Serial.println("WIFI Connected Succesful with BLE Provision");  
              apply_Password = "";
              apply_SSID = "";  
              // CAM Server will start here            
              // startCameraServer();
              Serial.print("Camera Ready! Use 'http://");
              Serial.print(WiFi.localIP());
              Serial.println("' to connect");  

              /*
                Ming comment on 050624
                Trying to stream video from the camera to the locally connected ILI9341 TFT display               
              */
              // showingImage();

              }
            else {
              Serial.println("BLE Provision fail to connect to Wifi");
              wifi_stage = GET_SSID;
              enterSSIDstatus = true;
              apply_Password = "";
              apply_SSID = ""; 
            }         
          }
        // *****************************************
        }

}

void loop() {
  // put your main code here, to run repeatedly:
  if(startCore0Thread == true){
    xTaskCreatePinnedToCore(
                sucessYellowLedBlinkingInfinitelyByCore0,   /* Task function. */
                "ledBlinkingTaskByCore0",     /* name of task. */
                10000,       /* Stack size of task */
                NULL,        /* parameter of the task */
                1,           /* priority of the task */
                &ledBlinkingTaskByCore0,      /* Task handle to keep track of created task */
                0);          /* pin task to core 0 */               
    delay(200);
    startCore0Thread = false;
  }

  Serial.println("************ Ming **************");

}
