#include <Firebase.h>
#include <FirebaseArduino.h>
#include <FirebaseCloudMessaging.h>
#include <FirebaseError.h>
#include <FirebaseHttpClient.h>
#include <FirebaseObject.h>

    #include <ArduinoJson.h>
    #include <ESP8266WiFi.h>
    #include <FirebaseArduino.h>
    #include "DHT.h"
    /******************************************************** define ******************************************************************************************************************/
    #define S0 D0                             /* Assign Multiplexer pin S0 connect to pin D0 of NodeMCU */
    #define S1 D1                             /* Assign Multiplexer pin S1 connect to pin D1 of NodeMCU */
    #define S2 D2                             /* Assign Multiplexer pin S2 connect to pin D2 of NodeMCU */
    #define S3 D3                             /* Assign Multiplexer pin S3 connect to pin D3 of NodeMCU */
    #define SIG A0                            /* Assign SIG pin as Analog output for all 16 channels of Multiplexer to pin A0 of NodeMCU */

    #define DHTPIN 2 /*pin 2 digital- D2*/
    #define DHTTYPE DHT11 
    
    #define FIREBASE_HOST "si-project-18bb2-default-rtdb.firebaseio.com"
    #define WIFI_SSID "AndroidAP"
    #define WIFI_PASSWORD "beank123"
    float RS_gas = 0;
    float ratio = 0;
    float sensorValue = 0;
    float sensor_volt = 0;
    float R0 =7200.0 ; /*Pt MQ7 trebuie determinata R0*/
    /******************************************************** define ******************************************************************************************************************/
     DHT dht(DHTPIN, DHTTYPE); /*Creare obiect pt senzor umiditate-temperatura*/
     StaticJsonBuffer<200> doc; /*Buffer*/
     JsonObject& obj = doc.createObject(); /*Obiect Json de care avem nevoie pentru a stoca si trimte informatiile */
    
    int decimal = 2;                         
    float airQuality;                            
    float CO;                            
    float Flame;                            
    float humidity;
    float temperature;
    float ppm;
/************************************************************* Setup functions *************************************************************************************************************/
void setupMultiplexer()
{
    pinMode(S0,OUTPUT);                       /* Define digital signal pin as output to the Multiplexer pin SO */   
    pinMode(S1,OUTPUT);                       /* Define digital signal pin as output to the Multiplexer pin S1 */  
    pinMode(S2,OUTPUT);                       /* Define digital signal pin as output to the Multiplexer pin S2 */  
    pinMode(S3,OUTPUT);                       /* Define digital signal pin as output to the Multiplexer pin S3 */  
    pinMode(SIG, INPUT);                      /*E legat la singurul pin analog de pe ESP8266 */             
} 
void setupDHT11()
{
  dht.begin();
}
void setupWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}
void setupFirebase()
{
 Firebase.begin(FIREBASE_HOST); 
}
void setupTime()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) 
  {
    delay(20000);
  }  
}

float readMQ135sensorValue()
{
  // Channel 0 (C0 pin - binary output 0,0,0,0)
    digitalWrite(S0,LOW); digitalWrite(S1,LOW); digitalWrite(S2,LOW); digitalWrite(S3,LOW);
    airQuality = analogRead(SIG);
}
float readMQ7sensorValue()
{
  // Channel 0 (C0 pin - binary output 1,0,0,0)
    digitalWrite(S0,HIGH); digitalWrite(S1,LOW); digitalWrite(S2,LOW); digitalWrite(S3,LOW);
    sensorValue = analogRead(SIG); 
    sensor_volt = sensorValue/1024*5.0;
    RS_gas = (5.0-sensor_volt)/sensor_volt;
    ratio = RS_gas/R0; //Replace R0 with the value found using the sketch above
    float x = 1538.46 * ratio;
    ppm = pow(x,-1.709);
}
float readFlamesensorValue()
{ 
    Flame = digitalRead(D6); /*Citesc valori digitale de pe un pin digita al ESP8266*/ /*1-daca nu-i flacara , 0 altfel*/
}


float readDHT11sensorValue()
{
   humidity = dht.readHumidity(); 
   temperature = dht.readTemperature();
}
void  sendDataToFirebase(float a,float t,float h,float p,float f,time_t now)
{
  obj["Timp"] = ctime(&now);
  obj["AirQ"]=a;
  obj["Flacara"] =  f;
  obj["Temperatura"]=t;
  obj["Umiditate"]=h;
  obj["CO"]=p;
  obj.printTo(Serial);
  Serial.println();
  Firebase.set("data",obj);
    delay(3000);
  
  if (Firebase.failed()) 
  {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
}
void setup()
{     
  Serial.begin(9600);                                        
   setupMultiplexer();
   setupDHT11();
   setupWiFi();
   setupFirebase();
   setupTime();                      
}

void loop() {                                 
  
    readMQ135sensorValue();
    Serial.print("Calitate aer : ");Serial.println(airQuality);         
    readDHT11sensorValue();
    Serial.print("Temperatura : ");Serial.println(temperature); 
    Serial.print("Umiditate : ");Serial.println(humidity); 
    delay(1000);
    readMQ7sensorValue();
    Serial.print("CO : ");Serial.println(ppm); 
    readFlamesensorValue();
    Serial.print("Flame : ");Serial.println(Flame); 
     time_t now = time(nullptr);
    sendDataToFirebase(airQuality,temperature,humidity,ppm,Flame,now);

  
    delay(25000);                                                 

}
