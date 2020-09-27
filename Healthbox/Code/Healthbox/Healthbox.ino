/*-----------------------------------------------------------------------------------------------------------------
 * 
 * University of Macedonia: Department of Applied Informatics 
 * Academic year: 2016-2017
 * 
 * This code is part of a completed academic project called: Healthbox(R)
 * Its purpose is to dispsay the capabilities of the IoT platform while making a unique and functional product that can be usefull to the user
 * This code runs on a Arduino Uno Board and it manages all of the devices functions.
 * The whole project was orchestrated by professor Konstantinos E. Psannis (kpsannis@uom.gr) 
 * For questions about the code for future developers you can contact the lead software engineer -> it1456@uom.edu.gr
 * 
 * -----------------------------------------------------------------------------------------------------------------
 */


// !Note: This code contains an constant to char compiler error that does NOT affect its functionality.

#include <LiquidCrystal.h>                                                                     //LCD screen library import
#include <SoftwareSerial.h>                                                                    //Serial port library import
#include <DHT.h>                                                                               //Temp/Hum library import

SoftwareSerial espSerial =  SoftwareSerial(7,8);                                               //Wifi shield pin declaration RX -> pin 7 , TX -> pin 8   
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);                                                         //LCD screen pin declaration
#define DHTPIN 9                                                                               //Temp/Hum sensor pin/type decleration
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
const int gasPin = A0;                                                                         //GAS sensor analog pin declaration
int pirPin = 6;                                                                                //Motion sensor pin declaration 


String apiKey = "******DND19S";                                                            //Channel's thingspeak WRITE API key

String ssid=" name ";                                                                          // Wifi network SSID
String password =" password ";                                                                 // Wifi network password

boolean DEBUG=true;

//======================================================================== Wifi Shield responce function

void showResponce(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (espSerial.available()){
        c=espSerial.read();
        if (DEBUG) Serial.print(c);
      }
    }
                   
}

//======================================================================== Function that sends sensor data to thingSpeaks database

boolean thingSpeakWrite(float value1, float value2, int value3, int value4){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";
  espSerial.println(cmd);
  if (DEBUG) Serial.println(cmd);
  if(espSerial.find("Error")){
    if (DEBUG) Serial.println("AT+CIPSTART error");
    return false;
  }
  
  
  String getStr = "GET /update?api_key=";   // prepare GET string
  getStr += apiKey;
  
  getStr +="&field1=";
  getStr += String(value1);
  getStr +="&field2=";
  getStr += String(value2);
  getStr +="&field3=";
  getStr += String(value3);
  getStr +="&field4=";
  getStr += String(value4);
  
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);
  if (DEBUG)  Serial.println(cmd);
  
  delay(100);
  if(espSerial.find(">")){
    espSerial.print(getStr);
    if (DEBUG)  Serial.print(getStr);
  }
  else{
    espSerial.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}

//================================================================================ Setup function

void setup() {   

  lcd.begin(16, 2);
  
  lcd.setCursor(0,0);                                                                                                 //LCD screen permanent data initialization
  lcd.print("T=");
  lcd.setCursor(9,0);
  lcd.print("H=");


  lcd.setCursor(6,0);
  lcd.print("C");
  lcd.setCursor(15,0);
  lcd.print("%");

  lcd.setCursor(0,1);
  lcd.print("G=");
  lcd.setCursor(5,1);
  lcd.print("ppm");
            
  DEBUG=true; 
  
  Serial.begin(9600); 
  
  dht.begin();                                                                  
  
  espSerial.begin(9600);                                                                                              //enable software serial
                          
  espSerial.println("AT+UART_CUR=9600,8,1,0,0");   
  showResponce(1000);
  
  espSerial.println("AT+RST");                                                                                        //reset esp8266
  showResponce(1000);

  espSerial.println("AT+CWMODE=1");                                                                                   //set esp8266 as client
  showResponce(1000);

  espSerial.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  
  showResponce(5000);

   if (DEBUG)  Serial.println("Setup completed");
}


// ====================================================================== Looping ---> Collects sensor data, prints them on screen, sends data to thingSpeak

void loop() {

                                                                                       // Read sensor values
   float t = dht.readTemperature();
   float h = dht.readHumidity();
   int   g = analogRead(gasPin);
   int   m = digitalRead(pirPin);


   lcd.setCursor(2,0);
   lcd.print(t, 1);
   lcd.setCursor(11,0);
   lcd.print(h, 1);
   lcd.setCursor(2,1);
   lcd.print(g);

   if(m == 1)
   {
    lcd.setCursor(10, 1);
    lcd.print("MOTION");
   }else{
     lcd.setCursor(10, 1);
     lcd.print("      ");
        }
    
   
        if (isnan(t) || isnan(h)) {
        if (DEBUG) Serial.println("Failed to read from DHT");
      }
      else {
          if (DEBUG) Serial.println("Temp="+String(t)+" *C");
          if (DEBUG) Serial.println("Humidity="+String(h)+" %");
          if (DEBUG) Serial.println("Gas="+String(g)+" ppm");
          if (DEBUG) Serial.println("Motion="+String(m));
           thingSpeakWrite(t,h,g,m);                                                      // Write values to thingspeak
      }
  
    
     
  delay(5000);                             
}




