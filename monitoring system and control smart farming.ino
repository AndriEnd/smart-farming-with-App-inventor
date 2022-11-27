//#include <Firebase_ESP_Client.h> 
#include <FirebaseESP32.h>
//library sensor dht 11 
#include <DHT.h> 
// seting pin 5
DHT dht(5, DHT11);

//lcd
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);// sda scl pin
//status relay dan seting pin relay
String status_relay1 = "";
#define pinRELAY1 15 // seting pin relay 15
//token
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
//wifi
#define WIFI_SSID "Suhendro" //ssid wifi
#define WIFI_PASSWORD "12345677" // password
// deklarasi firebase
#define FIREBASE_HOST "database-pestisida-default-rtdb.firebaseio.com"   // host akun                   
#define FIREBASE_AUTH "z********BEVMI4GSgQOeJeggvaRC" //autentifikasi
#define API_KEY "AIzaSyCyEM9*********H3A6_4-cNyUrpI9jFws"  // api KEY
#define DATABASE_URL "https://database-pestisida-default-rtdb.firebaseio.com/"  //URL frebase
//data firebase
FirebaseData fbdo;  //firebase data relay
FirebaseData firebaseData; //firebase data dht 11
//FirebaseData firebasedata; <<opsi  jika Board ESP 8266
FirebaseAuth auth; // run auth
FirebaseConfig config;// set configurasi
FirebaseJson json;  // run json library

//time send data
unsigned long sendDataPrevMillis = 0; // set time delay firebase
int count = 0;
bool signupOK = false; // bolean true or false

void setup() {
Serial.begin(115200);// print serial
dht.begin(); // run dht
lcd.begin(); // run lcd
//pin Relay
pinMode(pinRELAY1, OUTPUT); //seting pin relay output
//pinMode(relay2, OUTPUT);

//wifi begin
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(300);
    }   
Serial.println();
Serial.print("Connected with IP: ");
Serial.println(WiFi.localIP());
Serial.println();
// cal firebase auth reconnected
config.api_key = API_KEY;
config.database_url = DATABASE_URL;
    //is jika selesai
    if (Firebase.signUp(&config, &auth, "", "")) {
      Serial.println("ok");
          signupOK = true;
        }
        else {
              Serial.printf("%s\n", config.signer.signupError.message.c_str());}
    
config.token_status_callback = tokenStatusCallback; 
Firebase.begin(&config, &auth);
Firebase.reconnectWiFi(true);
//set lcd
  lcd.setCursor(0,0);
  lcd.print("   Temperature");
  lcd.setCursor(0,1);
  lcd.print("     Monitor");
  delay(2000);
  lcd.clear();
}

void RLY1() {
    if (Firebase.getString(firebaseData, "relay1")) {  
    if (firebaseData.dataType() == "string") {
    status_relay1 = firebaseData.stringData();
    Serial.print("Stat Relay 1:"); Serial.print(status_relay1); Serial.print("  ");  //<<<<<<<<<<<<<<< relay 1
       if (status_relay1 == "1") {
         digitalWrite (pinRELAY1, 1);
         Serial.println("RELAY ON");      
         } 
           else if (status_relay1 == "0") {
           digitalWrite (pinRELAY1, 0);
           Serial.println("RELAY OFF");
      }
    }
  }
}
void loop() {
RLY1();
int t = dht.readTemperature(); //<<<<<< baca suhu
const int h = dht.readHumidity(); //<<<<<< baca kelembapan 

//MoistureValue = analogRead(sensorMoisture); /// <<<<<<<<< jika tambah sensor tanah
String temp = (String)t + " *C";  
String humi = (String)h + " %"; 
//String moist =(String)MoistureValue;  // set string

//to lcd
  lcd.setCursor(0,0);
  lcd.print ("Suhu :"); 
  lcd.print(temp);
 //<<<<<<<<<<<<<<<<<<<<<<<<<<<<< lopping lcd 
  lcd.setCursor(0,1);
  lcd.print ("Lembap:");   
  lcd.print(humi);

  
if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    // menulis data suhu dari DHT11 ke firebase
    if (Firebase.RTDB.setString(&fbdo, "/temperature", temp)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      delay(2000);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // menulis data kelembaban dari DHT11 ke firebase
    if (Firebase.RTDB.setString(&fbdo, "/humidity", humi)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      delay(2000);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
        count++;
}
//float kondisi_normal=(dht.readTemperature(28));  <<<
//float kondisi_dingin=(dht.readTemperature(24));<<<

float kondisi_normal= (t>=27);
float kondisi_dingin=(t<=26); //<<<<<<<<<<<<<<<<<<<<<<< kondisi (notifikasi kondisi suhu)
String normal= (String)kondisi_normal;
String dingin=(String)kondisi_dingin;

    if (t>=30){
      (Firebase.RTDB.setString(&fbdo, "/Kondisi","Cuaca Normal")); ///<<<<<<<<<<<< jika normal 
        digitalWrite (pinRELAY1, HIGH);
        delay(60000); // delay pompa 1 menit
        digitalWrite (pinRELAY1, LOW); //low
        }
        
        else{
          (Firebase.RTDB.setString(&fbdo, "/Kondisi", "Cuaca dingin")); //<<<<<<<<<<<<<<<<<<< jika dingin dan lembab
           digitalWrite (pinRELAY1, LOW); // pompa off
        }
}
