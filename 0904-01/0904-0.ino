/*
  blue test: 
  http://www.kccistc.net/
  작성일 : 2022.12.19
  작성자 : IoT 임베디드 KSH
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MsTimer2.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DEBUG
#define CDS_PIN  1
#define BUTTON_PIN 2
//#define LED_BUILTIN_PIN 13
#define Water A0
#define INSIDE 3
#define OUTSIDE 5
#define DHTPIN A4 
#define ARR_CNT A5
#define Buzzer1 6
#define GREENLED 12
#define REDLED 13

#define Total 20
#define DHTTYPE DHT11
#define CMD_SIZE 60


char sendBuf[CMD_SIZE];
char recvBuf[CMD_SIZE];
char lcdLine1[17] = "Smart IoT By KSH";
char lcdLine2[17] = "";
char recvId[10] = "KSH_SQL";  // SQL 저장 클라이이언트 ID
bool lastButton = HIGH;       // 버튼의 이전 상태 저장
bool currentButton = HIGH;    // 버튼의 현재 상태 저장
bool ledOn = false;      // LED의 현재 상태 (on/off)
bool timerIsrFlag = false;
unsigned int secCount;
int cds=0;
float humi;
float temp;
bool cdsFlag = false;
int getSensorTime;
int n=0;
int i=0,o=0;
int IN=0,OUT=0;
int val = 0;
int R;
int j=0;
unsigned long INTime = 0; 
unsigned long OUTTime = 0;
unsigned long checkTime = 0;
unsigned long setTime = 0;
SoftwareSerial BTSerial(10, 11); // RX ==>BT:TXD, TX ==> BT:RXD

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("setup() start!");
#endif
  lcd.init();
  lcd.backlight();
  sprintf(lcdLine1, "IN : %d", i);
  sprintf(lcdLine2, " OUT : %d", o);
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pinMode(INSIDE, INPUT);
  pinMode(OUTSIDE, INPUT);
  pinMode(Water,INPUT);
  pinMode(Buzzer1,OUTPUT);
  BTSerial.begin(9600); // set the data rate for the SoftwareSerial port
  //MsTimer2::set(1000, timerIsr); // 1000ms period
  //MsTimer2::start();
}

void loop()
{
  IN = digitalRead(INSIDE);
  OUT = digitalRead(OUTSIDE);
  val = analogRead(Water);
  //IN = analogRead(INSIDE);
 // OUT = analogRead(OUTSIDE);
 // Serial.println(IN);
  //Serial.println(OUT);
  setTime = millis();

  Serial.println(n);

  CP(); //재실인원함수(Count Person)

  if(n > 0) //침수시 경고 알림
  {
    if(val>300)
    {
      emergency();
      delay(10);
      digitalWrite(GREENLED, LOW);
    }
  }

  sprintf(lcdLine1, "IN : %d", i);
  sprintf(lcdLine2, " OUT : %d", o);
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);

  delay(500);

  if (BTSerial.available())
    bluetoothEvent();
  
  if (timerIsrFlag)
  {
    timerIsrFlag = false;
    cds = map(analogRead(CDS_PIN), 0, 1023, 0, 100);

    //sprintf(lcdLine2,"C:%d T:%d H:%d",cds,(int)temp,(int)humi);
#ifdef DEBUG
    //Serial.println(lcdLine2);
#endif
    
    lcdDisplay(0, 1, lcdLine2);

    if(getSensorTime != 0 && !(secCount % getSensorTime)) {
    //  sprintf(sendBuf,"[%s]SENSOR@%d@%d@%d\n",recvId,cds,(int)temp,(int)humi);
    //  BTSerial.write(sendBuf);   
    }    
    
    if ((cds >= 50) && cdsFlag)
    {
      cdsFlag = false;
      //sprintf(sendBuf, "[%s]CDS@%d\n", recvId, cds);
    //  BTSerial.write(sendBuf, strlen(sendBuf));
    } 
    else if ((cds < 50) && !cdsFlag)
    {
      cdsFlag = true;
    //  sprintf(sendBuf, "[%s]CDS@%d\n", recvId, cds);
     // BTSerial.write(sendBuf, strlen(sendBuf));
    }
  }

  currentButton = debounce(lastButton);   // 디바운싱된 버튼 상태 읽기

 if (lastButton == HIGH && currentButton == LOW)  // 버튼을 누르면...
 {
    ledOn = !ledOn;       // LED 상태 값 반전
   // sprintf(sendBuf, "[%s]BUTTON@%s\n", recvId, ledOn ? "ON" : "OFF");
   // BTSerial.write(sendBuf);
#ifdef DEBUG
    //Serial.println(sendBuf);
#endif
  }
  lastButton = currentButton;     // 이전 버튼 상태를 현재 버튼 상태로 설정

#ifdef DEBUG
  if (Serial.available())
    BTSerial.write(Serial.read());
#endif
}

void bluetoothEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);

#ifdef DEBUG
  Serial.print("Recv : ");
  Serial.println(recvBuf);
#endif

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }

  if ((strlen(pArray[1]) + strlen(pArray[2])) < 16)
  {
    sprintf(lcdLine2, "%s %s", pArray[1], pArray[2]);
    lcdDisplay(0, 1, lcdLine2);
  }
  
  if (!strcmp(pArray[1], "LED")) {
    if (!strcmp(pArray[2], "ON")) {
     // digitalWrite(LED_BUILTIN_PIN, HIGH);
    }
    else if (!strcmp(pArray[2], "OFF")) {
    //  digitalWrite(LED_BUILTIN_PIN, LOW);
    }
    sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]);
  }
  else if(!strcmp(pArray[1],"GETSENSOR"))
  {
    if(pArray[2] == NULL){
      getSensorTime = 0;
    }else {
      getSensorTime = atoi(pArray[2]);
      strcpy(recvId,pArray[0]);
    }
    
  }
  else if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
    return ;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
    return ;
  }

#ifdef DEBUG
  Serial.print("Send : ");
  Serial.print(sendBuf);
#endif
  BTSerial.write(sendBuf);
}

void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
}

void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
}

boolean debounce(boolean last)
{
  boolean current = digitalRead(BUTTON_PIN);  // 버튼 상태 읽기
  if (last != current)      // 이전 상태와 현재 상태가 다르면...
  {
    delay(5);         // 5ms 대기
    current = digitalRead(BUTTON_PIN);  // 버튼 상태 다시 읽기
  }
  return current;       // 버튼의 현재 상태 반환
}

void emergency()
{
  int k;

  Serial.begin(115200); // 부저 출력을 위한 변경
  for(k=0;k<n;k++) //부저 이벤트
  {
    tone(Buzzer1, 261);// send pulse
    delay (1) ;// delay 1ms
    noTone(Buzzer1);
    delay (1) ;
  }

  Serial.begin(9600);

  sprintf(lcdLine1, "emergency : %d", i);
  sprintf(lcdLine2, " emergency : %d", o);
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);

  sprintf(sendBuf, "[%s]emergency@%d\n", recvId, n);
  BTSerial.write(sendBuf);

  delay(5);

  sprintf(sendBuf, "[%s]emergency@%d\n", "KSH_STM", n);
  BTSerial.write(sendBuf);

  val = digitalRead(Water); // 워터센서 읽기
  digitalWrite(GREENLED, HIGH);

  delay(500);
}

// 인원체크(재실측정) 함수
void CP()
{
  if(IN==LOW) //입구센서감지
  {
    i++;
    delay(20);
    INTime = millis();
    Serial.println("i++");
  }
  if(OUT==LOW) //출구센서감지
  {
    o++;
    delay(20);
    OUTTime = millis();
    Serial.println("o++");
  }

  if(i!=o) // 센서값 불일치할 때
  {
    checkTime = millis();
    j=1; //측정시간 오차를 수정하기 위한 변수
   // 만약 측정이 잘못된 이후 사람의 출입이 있었다면 나오는 오류 수정 필요 08/31
    if(i<o) // 들어가려다 나옴 = 들어가지 않음
    {
     if((checkTime-OUTTime)>5000)
        {
            if(INTime>OUTTime) //측정오류 이후 사람이 나가(OUTTime sensor->INTime sensor) 마지막으로 기록된 시간이 INTime sensor의 시간인 경
            {
              n--; //사람이 나갔기 때문에 보정
              i++; // 나간 사람에 대한 보정
              j=0;
            }
            else
            {
              o--;
              j=0;
            }
        }
    }
    else if(i>o) // 나가려다 들어옴 = 나가지 않음
    {
      if((checkTime-INTime)>5000)
        {
          if(OUTTime>INTime) //측정오류 이후 사람이 들어와(INTime sensor->OUTTime sensor) 마지막으로 기록된 시간이 OUTTime sensor의 시간인 경우
          {
            n++; //사람이 들어갔기 때문에 보정
            o++; // 들어간 사람에 대한 보정
            j=0;
          }
          else
          {
            i--;
            j=0;
          }
        }
    }
  }
  else if(i==o)  // 센서값 일치할 때
  {
    if(OUTTime<INTime)
    {
      if(j=1)
      {
        //
      }
      INTime=0;
      OUTTime=0;

      digitalWrite(GREENLED, HIGH);
      digitalWrite(REDLED, LOW);
      n--;

      if(n < 0) // 인원 수 보정
        n = 0;
    }
    else if(OUTTime>INTime)
    { 
      INTime=0;
      OUTTime=0;

      digitalWrite(GREENLED, LOW);
      digitalWrite(REDLED, HIGH);
      
      n++;
    }

    Serial.println(n);

    R=(((float)n/(float)Total)*100); // 혼잡도 계산 공식

    // 데이터 전송
    if(setTime % 100)
    {
      Serial.print("val= ");
      Serial.println(val);

      if(val!=HIGH)
      {
        sprintf(sendBuf, "[%s]AAA@%d\n", recvId, R);
        BTSerial.write(sendBuf);
        delay(5);
        sprintf(sendBuf, "[%s]AAA@%d\n", "KSH_STM", R);
        BTSerial.write(sendBuf);
      }
    }
  }
}
