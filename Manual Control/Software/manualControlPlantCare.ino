/*   
     Manually Controlled Plant Care Operating System MCU Codes 
     for Arduino Uno
     
     HMISerial (10 (RX), 11 (TX)), Connect TX of Nextion to Pin 10, RX to Pin 11
     
     DHT11 Sensor Pin Digital 2
     
     20.12.2016
     by M.Murat Akcayigit
 */
                                               

#include <dht11.h>
#include <Nextion.h>
#include <NexNumber.h>
SoftwareSerial HMISerial(10,11);

/* Define PIN Sensor Enable Numbers */ 
#define LIGHT 13   /* Led Enable Pin 13 */
#define PUMP 9    /* Pump Enable Pin 12 */
#define FAN 8      /* Fan Enable Pin 8 */

dht11 DHT11;  /* DHT11 Temp */
char buffer[20] = {0};  /* Uses a string to feed Nextion Screen of DHT11 Sensor Value */
uint32_t number;  /* Keeps value of the set picture page numbers */
unsigned long update = 0; /* For delay purposes in order to updated temperature values */

volatile float t;
volatile float hum;
volatile short int pumpUp=0;
volatile short int lightUp=0;
volatile short int fanUp=0;

/* NEXTION */
NexText t0 = NexText(0, 1, "t0"); /* Temperature Text Transmiting information*/
NexText t1 = NexText(0, 2, "t1"); /* Air Moist Text Transmitting information*/
NexText t2 = NexText(0, 3, "t2"); /* Soil Moist Text Transmitting information*/

/* Nextion Control Units For Picture Buttons */
NexPicture pumpPic = NexPicture(2, 1, "p1");    /* Nextion Picture information for DC Pump */
NexPicture lightPic = NexPicture(4, 1, "p1");   /* Nextion Picture information for LED Light */
NexPicture fanPic = NexPicture(5, 1, "p1");     /* Nextion Picture information for DC Fan*/
NexButton homeButton = NexButton(1, 1, "b0");

/* Page Set Components */
NexButton setPump = NexButton(1, 2, "b1");
NexButton setLight = NexButton(1, 3, "b2");
NexButton setFan = NexButton(1, 4, "b3");

volatile unsigned short int i=0;

void nextionTemp(float t, float hum)
{
  uint16_t valCelc;
  uint16_t valHum;
  
  memset(buffer, 0, sizeof(buffer));
  valCelc = t;
  itoa(valCelc, buffer, 10);
  t0.setText(buffer);
  
  delay(20);
  
  valHum = hum;
  itoa(valHum, buffer, 10);
  t1.setText(buffer);

  unsigned int moist= analogRead(A0); /*reads the soil moist sensor value */  
  
  if (moist >= 700)
  {
    t2.setText("Dry");    
    sendCommand("t2.pco=33280");
    sendCommand("ref t2");
  }else{
    t2.setText("Wet");
    sendCommand("t2.pco=1055");
    sendCommand("ref t2");
  }
  
}

void pumpPicPopCallback(void *ptr) /* Pump Function : On-Off & Picture Set */
{
   pumpPic.getPic(&number);
    
    if (number == 3)    /* Pump On Icon */
    {
        number = 6;     /* Pump Off Icon */
        digitalWrite(PUMP, LOW);  /* PUMP CONNECTED PIN 11 */
        pumpUp = 0;
        delay(10);
    }
    else
    {
        number = 3;
        digitalWrite(PUMP,HIGH);
        pumpUp = 1;                  
        delay(10);
    }
    
    pumpPic.setPic(number);
}

void setPumpPopCallback(void *ptr)    // When we return to main menu, Pump icon is set to not Pumping, In order to keep icon updated we use the function
{
    if (pumpUp == true)
  {
    pumpPic.setPic(3);
  }else{
    pumpPic.setPic(6);
  }
}

/* End Of PUMP */


void lightPicPopCallback(void *ptr) /* Light Function : On-Off & Picture Set */
{
    lightPic.getPic(&number);
    
    if (number == 2)  /* LED ON Icon */
    {
        number = 5;   /* LED Off Icon */
        digitalWrite(LIGHT, LOW);  /* LEDs CONNECTED PIN 13 */
        lightUp = 0;
        delay(10);
    }
    else
    {
        number = 2;
        digitalWrite(LIGHT,HIGH);
        lightUp = 1;
        delay(10);
    }
       lightPic.setPic(number);
}

void setLightPopCallback(void *ptr) // When we return to main menu, Light icon is set to not Lighting, In order to keep icon updated we use the function 
{

  if (lightUp == 1)
  {
    delay(10);
    lightPic.setPic(2);
  }else{
    delay(10);
    lightPic.setPic(5);
  }
}

/* End Of LIGHT */


void fanPicPopCallback(void *ptr) /* Fan Function : On-Off & Picture Set */
{
   fanPic.getPic(&number);  
   
    if (number == true)    /* Fan On Icon */
    {
        number = 4;     /* Fan Off Icon */
        digitalWrite(FAN, LOW);  /* Fan CONNECTED PIN 10 */
        fanUp=0;
        delay(10);
    }
    else
    {
        number = 1;
        digitalWrite(FAN, HIGH);            
        fanUp=1;      
        delay(10);
    }

    
    fanPic.setPic(number);
}


void setFanPopCallback(void *ptr) // When we return to main menu, Fan icon is set to not cooling, In order to keep icon updated we use the function
{
    if (fanUp == 1)
  {
    fanPic.setPic(1);
  }else{
    fanPic.setPic(4);
  }
}

/* End Of FAN */

void homeButtonPopCallback(void *ptr)
{
  nextionTemp(t, hum);
}

NexTouch *nexListenList[] = 
{ 
  &pumpPic,
  &lightPic,
  &fanPic,
  &homeButton,
  &setPump, 
  &setLight, 
  &setFan,
  NULL
};

void setup()
{

  DHT11.attach(2);  /* DHT11 Connection Pin (SPI) Digital 2 */
  
  nexInit();
  
  pumpPic.attachPop(pumpPicPopCallback, &pumpPic);
  lightPic.attachPop(lightPicPopCallback, &lightPic); 
  fanPic.attachPop(fanPicPopCallback, &fanPic);
  homeButton.attachPop(homeButtonPopCallback, &homeButton);
  setPump.attachPop(setPumpPopCallback, &setPump);
  setLight.attachPop(setLightPopCallback, &setLight);
  setFan.attachPop(setFanPopCallback, &setFan);
  
  pinMode(LIGHT, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(FAN, OUTPUT);

  digitalWrite(LIGHT, LOW);
  digitalWrite(PUMP, LOW);
  digitalWrite(FAN, LOW);
  
  delay(10); 
}

void loop()   
{

  int chk = DHT11.read(2); // SENSOR
  t = (float)DHT11.temperature;
  hum = (float)DHT11.humidity;
  unsigned long nowtime = millis(); 
  
  nexLoop(nexListenList);
 
  if (nowtime > update) // Here is a delay for the temperature reads not to make system busy
  {
    nextionTemp(t, hum);
    update+=5000;
  }
}
