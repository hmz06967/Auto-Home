#include "Adafruit_CC3000.h"
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

#define ADAFRUIT_CC3000_IRQ   3  
#define ADAFRUIT_CC3000_VBAT  5  
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,SPI_CLOCK_DIVIDER); 

#define WLAN_SSID       "wlan_adi"   //wifi erişim noktası adını giriyoruz.
#define WLAN_PASS       "wlan_sifresi" //wifi şifresini giriyoruz
#define WLAN_SECURITY WLAN_SEC_WPA2 //wpa2 güvenlik belirtiyoruz.
#define LISTEN_PORT 80
#define MAX_ACTION 10 
#define MAX_PATH 64 
#define BUFFER_SIZE MAX_ACTION + MAX_PATH + 20 
#define TIMEOUT_MS 500 
#define SERVER_SCRIPT   "<script src='https://www.you-mp3.net/test/arduino/load.arduino.js'></script>" //html kodlarını çağıran javascripti sayfaya dahil ediyoruz.

Adafruit_CC3000_Server httpServer(LISTEN_PORT);

uint8_t buffer[BUFFER_SIZE+1];
int bufindex = 0,
    lm35pin = 4,
    potpin = 3,
    led1 = 2,
    led2 = 3,
    sensor_bilgisi,
    pot_bilgisi; //gerekli değişkenleri ve pinleri dahil ettik
    
char action[MAX_ACTION+1];
char path[MAX_PATH+1];
String inString = String(35);
String inString2 = String(35);
String Led, tamam = "ok";

void setup(void)
{

Serial.begin(115200);
pinMode(led1, OUTPUT);
pinMode(lm35pin, INPUT);
pinMode(potpin, INPUT);
pinMode(A2, OUTPUT);

Serial.print("Free RAM: "); 
Serial.println(getFreeRam(), DEC);

Serial.println(F("\nBaslatiliyor."));
if (!cc3000.begin())
{
Serial.println(F("Baslatilamadi. bir sorun var"));
while(1);
}
Serial.print(F("\nWifi cihazina baglaniyor.. ")); Serial.println(WLAN_SSID);
if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
Serial.println(F("Hata!"));
while(1);
}
analogWrite(A2,1023);
Serial.println(F("Baglandi!\nDHCP bilgileri aliniyor."));

while (!cc3000.checkDHCP())

{
delay(100); 
}

while (! displayConnectionDetails()) {
delay(1000);
}
httpServer.begin();
Serial.println(F("Ziyaretci bekleniyor..."));
}
void loop(void){
  
  Adafruit_CC3000_ClientRef client = httpServer.available();
  if (client)
  { 
    boolean current_line_is_blank = true;
    Serial.println(F("Ziyaretci Baglandi."));
    
    while (client.connected()) {
      if (client.available())
      {
        char c = client.read();
    
    if (inString.length() < 35)
    {
      inString.concat(c);
      }
    if (inString2.length() < 35){
      inString2.concat(c);
      }

      if (c == '\n' && current_line_is_blank)
      {
        sensor_bilgisi = analogRead(lm35pin);
        pot_bilgisi = analogRead(potpin); 
        
        client.fastrprintln(F("HTTP/1.1 200 OK"));
        client.fastrprintln(F("Content-Type: text/html;charset=utf-8"));
        client.println();
        
        if(inString.indexOf("sensor_bilgisi_al=on")>0 ){
          client.print(sensor_bilgisi);client.print("__"); client.print(pot_bilgisi); break;   }
                  
        else if(inString.indexOf("led1durum=acik")>0){
          client.print(tamam);
          digitalWrite(led1, HIGH);  break;
        }
        else if(inString.indexOf("led1durum=kapali")>0){
          client.print(tamam);
          digitalWrite(led1, LOW);  break;
        }
        else if(inString.indexOf("led2durum=acik")>0){
          client.print(tamam);
          digitalWrite(led2, HIGH); break;
        }
        else if(inString.indexOf("led2durum=kapali")>0){
          client.print(tamam);
          digitalWrite(led2, LOW);  break;
        }  
        else{
          client.println("<html id='page'>");
          client.println("Yükleniyor..");
          client.println("<script src='https://ajax.googleapis.com/ajax/libs/jquery/2.2.3/jquery.min.js'></script>");              
          client.println(SERVER_SCRIPT);                    
          break;
      }
      if (c == '\n') {
       current_line_is_blank = true; 
       }
      else if (c != '\r'){
       current_line_is_blank = false; 
       }
     }
  } 
} 
  delay(100);
    inString2 = "";
    inString = "";
  client.close();
  Serial.println("baglanti kesildi");
  }
}


bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path) {
if (bufSize < 2)
return false;
if (buf[bufSize-2] == '\r' && buf[bufSize-1] == '\n') {
parseFirstLine((char*)buf, action, path);
return true;
}
return false;
}

void parseFirstLine(char* line, char* action, char* path) {
char* lineaction = strtok(line, " ");
if (lineaction != NULL)
strncpy(action, lineaction, MAX_ACTION);
char* linepath = strtok(NULL, " ");
if (linepath != NULL)
strncpy(path, linepath, MAX_PATH);
}
bool displayConnectionDetails(void)
{
uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
{
Serial.println(F("Ip adresi alinamadi!\r\n"));analogWrite(A2,500);
return false;
}
else
{
Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
Serial.println();
return true;
}
}
