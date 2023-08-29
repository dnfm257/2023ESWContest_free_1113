#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include <MsTimer2.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <DHT.h>

#define AP_SSID "iot0"
#define AP_PASS "iot00000"
#define SERVER_NAME "10.10.141.82"
#define SERVER_PORT 5000
#define LOGID "KSH_SQL"
#define PASSWD "PASSWD"

#define WIFIRX 6  //6:RX-->ESP8266 TX
#define WIFITX 7  //7:TX -->ESP8266 RX

#define CMD_SIZE 50
#define ARR_CNT 5

#define LED_PIN A0
#define DHTPIN A3 
#define LCD_SDA A4
#define LCD_SCL A5 
#define DHTTYPE DHT11

char sendBuf[CMD_SIZE];
char recvBuf[CMD_SIZE]; 
char lcdLine1[17];
char lcdLine2[17];
unsigned long INTime = 0; 
unsigned long OUTTime = 0; 
int n=0;
int i=0,o=0;
int IN=0,OUT=0;
int ms=0;
int percent = 0;
int flag = 0;
int led = 10;
int emerg = 0;

typedef struct{
  int standard;
  char* status;
} STDAD_OBJ;

STDAD_OBJ st[4];

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
WiFiEspClient client;
SoftwareSerial wifiSerial(WIFIRX, WIFITX);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);

  pinMode(LED_PIN, OUTPUT);

  wifi_Setup();

  st[0] = {0, "Sleep"};
  st[1] = {25, "Free"};
  st[2] = {23, "Normal"};
  st[3] = {21, "Full"};
}

void loop() 
{
  // put your main code here, to run repeatedly:
  if (client.available()) {
    socketEvent();
  }
  
  delay(300);

  if(!emerg){
    dht11_check();
  }
}

void dht11_check(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if(isnan(h) || isnan(t)){
    sprintf(lcdLine1, "Failed DHT sensor!");
    lcdDisplay(0, 0, lcdLine1);
    return;
  }
  
  float hic = dht.computeHeatIndex(t, h, false);

  sprintf(lcdLine1, "T: %dC, st: %dC", (int)hic, st[flag].standard);
  lcdDisplay(0, 0, lcdLine1);
}

void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
}

void controller(){
  if(percent < 40 && percent > 0){
    flag = 1;
    digitalWrite(LED_PIN, HIGH);
  }
  else if(percent >= 40 && percent < 80){
    flag = 2;
    digitalWrite(LED_PIN, HIGH);
  }
  else if(percent >= 80){
    flag = 3;
    digitalWrite(LED_PIN, HIGH);
  }
  else{
    flag = 0;
    digitalWrite(LED_PIN, LOW);
  }
}

void socketEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len;
  char* people_ch;

  sendBuf[0] = '\0';
  len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
  client.flush();
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.print(recvBuf);
#endif
  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  //[KSH_ARD]LED@ON : pArray[0] = "KSH_ARD", pArray[1] = "LED", pArray[2] = "ON"
  if ((strlen(pArray[1]) + strlen(pArray[2])) < 16)
  {
    
  }
  if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    //strcpy(lcdLine2, "Server Connected");
    //lcdDisplay(0, 1, lcdLine2);
    return;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    client.stop();
    server_Connect();
    return;
  }
  else if (!strcmp(pArray[1], "AAA")) {
    emerg = 0;

    people_ch = pArray[2];
    percent = atoi(people_ch);
    controller();

    sprintf(lcdLine2, "%s %s", st[flag].status, pArray[2]);
    lcdDisplay(0, 1, lcdLine2);
  }
  else if(!strcmp(pArray[1], "emergency")){
    emerg = 1;

    sprintf(lcdLine1, "%s", pArray[1]);
    if(atoi(pArray[2]) == 0){
      sprintf(lcdLine2, "%s", pArray[2]);
    }
    else{
      sprintf(lcdLine2, "%s PEOPLE LEFT!", pArray[2]);
    }
    
    lcdDisplay(0, 0, lcdLine1);
    lcdDisplay(0, 1, lcdLine2);
  }
  else
    return;

  client.write(sendBuf, strlen(sendBuf));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
}

void wifi_Setup() {
  wifiSerial.begin(19200);
  wifi_Init();
  server_Connect();
}

void wifi_Init()
{
  do 
  {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) 
    {
#ifdef DEBUG_WIFI
      Serial.println("WiFi shield not present");
#endif
    }
    else
      break;
  } while (1);

#ifdef DEBUG_WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif
  while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) 
  {
#ifdef DEBUG_WIFI
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);
#endif
  }
  sprintf(lcdLine1, "ID:%s", LOGID);
  lcdDisplay(0, 0, lcdLine1);
  sprintf(lcdLine2, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  lcdDisplay(0, 1, lcdLine2);

#ifdef DEBUG_WIFI
  Serial.println("You're connected to the network");
  printWifiStatus();
#endif
}

int server_Connect()
{
#ifdef DEBUG_WIFI
  Serial.println("Starting connection to server...");
#endif

  if (client.connect(SERVER_NAME, SERVER_PORT)) 
  {
#ifdef DEBUG_WIFI
    Serial.println("Connect to server");
#endif
    client.print("["LOGID":"PASSWD"]");
  }
  else
  {
#ifdef DEBUG_WIFI
    Serial.println("server connection failure");
#endif
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}