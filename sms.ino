/*
 */
#define trig 12 // start sensor
#define echo 13
#define vcc 11

#include <SoftwareSerial.h>

SoftwareSerial SIM(2, 3);

// stop sensor
int sensorReadArray[60]; // start smoothing
int ave;
int emp = 3500; // all rates will pertain to sensor reads. updates will be done iin human language
int ara = 50;
int era = 300;
int nol = 1000;
int count;
int previous;
byte phoneNumber1[15];
int isNumber1 = 0; // to check so as to not send a wrong number
byte phoneNumber2[15];
int isNumber2 = 0;
byte phoneNumber3[15];
int isNumber3 = 0;
unsigned long previousMss = 0;
unsigned long slp;
int error; // a percentage of how much higher the read is when you don't drop outliers. could tell if a sensro is faulty
byte replyBuffer[255];
int sensorRead()
{
  int ping;
  int distance;
  delay(950); // start sensor read
  digitalWrite(vcc, HIGH);
  //^just to supply power to the module. always on
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);
  //^^^trigger read
  ping = pulseIn(echo, HIGH);
  distance = ((ping * 0.34) / 2);
  Serial.print("individual read is ");
  Serial.print(distance);
  Serial.println();
  return distance;
}
void aboveAndBelow()
{
  for (int i = 0; i <= 59; i++)
  {
    if (sensorReadArray[i] > 3500)
    {
      sensorReadArray[i] = 0;
    }
    if (sensorReadArray[i] < 200)
    {
      sensorReadArray[i] = 0;
    }
  }
}
void top()
{
  int max_v = 0;
  int max_i = 0;
  for (int i = 0; i <= 59; i++)
  {
    if (max_v > sensorReadArray[i])
    {
      max_v = sensorReadArray[i];
      max_i = i;
    }
  }
  sensorReadArray[max_i] = 0;
}
void bottom()
{
  int min_v = 9999;
  int min_i = 0;
  for (int i = 0; i <= 59; i++)
  {
    if (sensorReadArray[i] < min_v && sensorReadArray[i] > 0)
    {
      min_v = sensorReadArray[i];
      min_i = i;
    }
  }
  sensorReadArray[min_i] = 0;
}
int average()
{
  int countAverage = 0; // count for this local average
  long totalAverage = 0;
  int aveAverage;
  for (int i = 0; i <= 59; i++)
  {
    if (sensorReadArray[i] > 0)
    {
      countAverage = countAverage + 1;
      totalAverage = totalAverage + sensorReadArray[i];
    }
  }
  aveAverage = totalAverage / countAverage;
  return aveAverage;
}
void smooth()
{
  int preave;
  aboveAndBelow();
  preave = average();
  for (int i = 1; i <= 5; i++)
  {
    top();
    bottom();
  }
  ave = average();
  error = (100 * (preave - ave)) / ave;
  Serial.print("the average is ");
  Serial.println(ave);
  Serial.println(millis());
  Serial.println();
}
// stop smoothing
void SendUpdateSMS()
{
  SIM.write("AT+CMGS=\"+61457254178\"");
  SIM.write(0x0D);
  delay(500);
  SIM.write("The level is ");
  SIM.print(ave);
  SIM.write(". it was ");
  SIM.print(previous);
  SIM.write(". The normal operating level is ");
  SIM.print(nol);
  SIM.write(". ara and era equal ");
  SIM.print(ara);
  SIM.write(" and ");
  SIM.print(era);
  SIM.write(".");
  SIM.write(0x1A);
  previousMss = millis();
}
int getNumber(byte bite)
{
  if (bite == '0')
    return 0;
  if (bite == '1')
    return 1;
  if (bite == '2')
    return 2;
  if (bite == '3')
    return 3;
  if (bite == '4')
    return 4;
  if (bite == '5')
    return 5;
  if (bite == '6')
    return 6;
  if (bite == '7')
    return 7;
  if (bite == '8')
    return 8;
  if (bite == '9')
    return 9;
  else
    return 0;
}
/*feb 16th to do:
-> alert
-> update series, text messages to set values:
  ->change ara from a text :: alert range. the plus or minus value from nor
  ->change nol :: normal operating range
  ->add :: add a phone number
  ->del :: delete a phone number
  ->upd :: update
    ->add a function that tells it whether levels are increasing or decreasing over the last (' ' mins)
->update http or something... defs need to test this and texts stepwise
  ->maybe delay updates if reads are the same (+/- 10 mm)
  */
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  SIM.begin(4800);
}

void loop()
{
  
}
