#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
//#include <SPIFFS.h>
#define _ESPASYNC_WIFIMGR_LOGLEVEL_    3
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
/****************************************************************************************************************************
  Async_AutoConnect_ESP32_minimal.ino
  For ESP8266 / ESP32 boards
  Built by Khoi Hoang https://github.com/khoih-prog/ESPAsync_WiFiManager
  Licensed under MIT license
 *****************************************************************************************************************************/
#if !(defined(ESP32) )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif
#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager



//AsyncWebServer webServer(80);
#if !( USING_ESP32_S2 || USING_ESP32_C3 )
DNSServer dnsServer;
#endif
//const char *ssid = "";
//const char *password = "";

const int led = 02;
const int resetButton =18; //D13 UNO
const int capteurPression = 35;
const int capteurTop = 26;   

int valeurDelayLed = 1000;
bool etatLed = 0;
bool etatLedVoulu = 0;
int previousMillis = 0;

AsyncWebServer server(80);
//WiFiManager wm;


//----------------------------------------------------ISR
unsigned long CountInput = 0;
void IRAM_ATTR isr() {
 CountInput = CountInput+1; // Formule de calcul:F(Hz)= 109 * débit (Q)(l/min)
}



void setup()
{
   
   
  //----------------------------------------------------SERVER
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/w3.css", "text/css"); });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/script.js", "text/javascript"); });

  server.on("/jquery-3.4.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/jquery-3.4.1.min.js", "text/javascript"); });

  server.on("/param.xml", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              request->send(SPIFFS, "/param.xml", "text/xml");});

  server.on("/wparamxml", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
                Serial.println("wparam");
                File file = SPIFFS.open("/param.xml","w");
                String message;
                message = request->getParam("wparamxml", true)->value();
                Serial.println(message);
                file.print(message);

              request->send(204);});

  server.on("/lectureValeur1", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              float sensorValue = float (map(float (analogRead(capteurPression)), 0.0,7500.0 ,0.0, 500.0))/100.0;
              //int val = analogRead(capteurPression);
              String Value = String(sensorValue);
              request->send(200, "text/plain", Value);
              //Serial.println(Value);
            });
 server.on("/lectureValeur2", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String Value = String(CountInput);
              request->send(200, "text/plain", Value);
              //Serial.println(Value);
            });
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              etatLedVoulu = 1;
              request->send(204);
             // Serial.println("On");
            });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              etatLedVoulu = 0;
              digitalWrite(led, LOW);
              etatLed = 0;
              request->send(204);
              // Serial.println("off");
            });

  server.on("/delayLed", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("valeurDelayLed", true))
              {
                String message;
                message = request->getParam("valeurDelayLed", true)->value();
                if(message.toInt()!=0)
                valeurDelayLed = message.toInt();
                Serial.println("valeurDelayLed");
              }
              request->send(204);
            });
  server.on("/valeurSaisi1", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("valeurSaisi1", true))
              {
                String message;
                message = request->getParam("valeurSaisi1", true)->value();
                if(message.toInt()!=0)
                valeurDelayLed = message.toInt();
                 Serial.println("valeurSaisi1");
              }
              request->send(204);
            });

    server.on("/valeurSaisi2", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("valeurSaisi2", true))
              {
                String message;
                message = request->getParam("valeurSaisi2", true)->value();
                if(message.toInt()!=0)
                valeurDelayLed = message.toInt();
                 //Serial.println("valeurSaisi2");
              }
              request->send(204);
            });
   server.on("/valeurSaisi3", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("valeurSaisi3", true))
              {
                String message;
                message = request->getParam("valeurSaisi3", true)->value();
                if(message.toInt()!=0)
                valeurDelayLed = message.toInt();
                // Serial.println("valeurSaisi3");
              }
              request->send(204);
            });
   server.on("/valeurSaisi4", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("valeurSaisi4", true))
              {
                String message;
                message = request->getParam("valeurSaisi4", true)->value();
                if(message.toInt()!=0)
                valeurDelayLed = message.toInt();
                // Serial.println("valeurSaisi4");
              }
              request->send(204);
            });
  //----------------------------------------------------OTA
  Serial.println("Starting OTA...");
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("Serveur actif!");
}

void loop()
{
 
 ArduinoOTA.handle();
   if(etatLedVoulu)
  {
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= valeurDelayLed)
    {
      previousMillis = currentMillis;

      etatLed = !etatLed;
      digitalWrite(led, etatLed);
    }

 /* if(digitalRead(resetButton))
	{
		Serial.println("Suppression des reglages et redemarrage...");
		wm.resetSettings();
		ESP.restart();
	}*/

  }

/*
Gestion de la pression.
ETAPE : 0 "Purge manuel de la boueille" #DMD => OUVRIR GAZ PURGE
T0 : "Détection préssion dans la bouteille P0 (mémorisation)" et Delta P,  stable après temps"
ETAPE : 10 "Purge gaz C02" #CMD => OUVERTURE VANNE PURGE
T10 : "X secondes"
ETAPE : 20 "Attentente retour Pression Purge P0"
T20 : "Pression >= P0 (mémorisation)"
ETAPE : 30 "Passage manuel Ligquide " #DMD => OUVRIR LIQUIDE  #CMD=> OUVERTURE VANNE LIQUIDE #CMD =>RAZ COMPTEUR LIQUIDE
T30 : "Pression > P0 (mémorisation) et Delta P,  stable après temps"
ETAPE : 40 "Ouverture Vanne Purge "
T40 : "Compteur Liquide >=Seuil Compteurs"
ETAPE : 50 "Fermeture Vanne Liquide "
T50 : "Pression < proche 0,5 bar et Delta P,  stable après temps"
ETAPE : 50 "Fermeture Vanne Purge " 
T50 : "true"

RETOUR a l'étape"0"

*/

}

 