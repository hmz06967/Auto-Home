#include "Adafruit_CC3000.h"
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"
  
//Pin baglantilari
#define ADAFRUIT_CC3000_IRQ   3  
#define ADAFRUIT_CC3000_VBAT  5 
#define ADAFRUIT_CC3000_CS    10 
  
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, 
                                         ADAFRUIT_CC3000_VBAT,SPI_CLOCK_DIVIDER); 
  
#define WLAN_SSID       "Sifre_WiFi"   // Ag baglanti adi
#define WLAN_PASS       "123456789"     //Ag baglanti sifresi
//Guvenlik turleri : WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2  // Guvenlik turu
  
#define LISTEN_PORT           80     
#define MAX_ACTION            10     
#define MAX_PATH              64     
#define BUFFER_SIZE           MAX_ACTION + MAX_PATH + 20 
#define TIMEOUT_MS            500   
  
Adafruit_CC3000_Server httpServer(LISTEN_PORT);
  
uint8_t buffer[BUFFER_SIZE+1];
int bufindex = 0;
char action[MAX_ACTION+1];
char path[MAX_PATH+1];
  
void setup(void)
{
  pinMode(A2, INPUT); //Potansiyometre
  Serial.begin(115200);
  
  Serial.println(F("n CC3000 Baslatiliyor.."));
  if (!cc3000.begin())
  {
    Serial.println(F("Baglantilarinizi kontrol ediniz.Baglanti kurulamiyor !"));
    while(1);
  }
    
  Serial.print(F("n Aga baglanmaya calisiliyor ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) 
  {
    Serial.println(F("Baglanti basarisiz !"));
    while(1);
  }
  Serial.println(F("Baglandi !"));
    
  Serial.println(F("DHCP baglanti istegi"));
  while (!cc3000.checkDHCP())
  {
    delay(100); 
  }  
  //Bilgi baglantisini goruntuler
  while (! displayConnectionDetails()) 
  {
    delay(1000);
  }
  //Web browsere baglanıyor
  httpServer.begin();
    
  Serial.println(F(" Baglanti bekleniyor..."));
}
  
void loop(void)
{
  //potansiyometre degerini adcden oku ve degiskene ata
  int analog_value = analogRead(A2);
  
  Adafruit_CC3000_ClientRef client = httpServer.available();
  if (client) 
  {
    Serial.println(F("Baglanti kuruldu."));
    bufindex = 0;
    memset(&buffer, 0, sizeof(buffer));
    memset(&action, 0, sizeof(action));
    memset(&path,   0, sizeof(path));
    unsigned long endtime = millis() + TIMEOUT_MS;
      
    bool parsed = false;
    while (!parsed && (millis() < endtime) && (bufindex < BUFFER_SIZE)) 
    {
      if (client.available()) {
      buffer[bufindex++] = client.read();
    }
      parsed = parseRequest(buffer, bufindex, action, path);
   }
     
   if (parsed) 
   {
     Serial.println(F("islem yapma istegi "));
     Serial.print(F("Action: ")); Serial.println(action);
     Serial.print(F("Path: ")); Serial.println(path);
      if (strcmp(action, "GET") == 0) {
     client.fastrprintln(F("HTTP/1.1 200 OK"));
     client.fastrprintln(F("Content-Type: text/plain"));
     client.fastrprintln(F("Connection: close"));
     client.fastrprintln(F("Refresh: 0"));
     client.fastrprintln(F("Server: Adafruit CC3000"));
     client.fastrprintln(F(""));
     //Web browsere verileri gonder
     client.fastrprintln(F("-----------------------------"));
     client.fastrprintln(F("         KontrolArge         "));
     client.fastrprintln(F("-----------------------------"));
     client.fastrprintln(F(" "));
     client.fastrprint(F("Potansiyometreden okunan analog deger : "));
     client.println(analog_value);
     client.fastrprintln(F(""));
   }
   else
   {
     client.fastrprintln(F("HTTP/1.1 405 Method Not Allowed"));
     client.fastrprintln(F(""));
   }
 }
  
    //Delay antes de desconectar
    delay(1000);
    Serial.println(F("Client disconnected"));
    client.close();
  }
  delay(500);
}
  
bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path) {
  //Check if the request ends with rn to signal end of first line.
  if (bufSize < 2)
    return false;
  if (buf[bufSize-2] == 'r' && buf[bufSize-1] == 'n') {
    parseFirstLine((char*)buf, action, path);
    return true;
  }
  return false;
}
  
void parseFirstLine(char* line, char* action, char* path) {
  //Parse first word up to whitespace as action.
  char* lineaction = strtok(line, " ");
  if (lineaction != NULL)
    strncpy(action, lineaction, MAX_ACTION);
  //Parse second word up to whitespace as path.
  char* linepath = strtok(NULL, " ");
  if (linepath != NULL)
    strncpy(path, linepath, MAX_PATH);
}
  
//Baglanti iP adres bilgileri okunuyor
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
    
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("IP adresi okunmadi!rn"));
    return false;
  }
  else
  {
    Serial.print(F("nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
