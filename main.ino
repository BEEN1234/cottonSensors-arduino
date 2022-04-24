#define trig 12 //start sensor
#define echo 13
#define vcc 11

#include <SoftwareSerial.h>

SoftwareSerial SIM(4, 5);

//stop sensor
int sensorReadArray[40]; //start smoothing
int ave;
int countSensorRead;
unsigned long sensorReadMss = 0;
int emp = 3500; //all rates will pertain to sensor reads. updates will be done iin human language
int ara = 50; 
int era = 300;
int nol = 1000;
int previous;
int Continue = 0;
int replyTo = 0;
byte bufferPhoneNumber[15]; //for add command. it'll be 4 in my phonebook impromptu indexing system. The BJPPIIS as i call it. 
byte addReply; // for add command
unsigned long previousMss = 0;
unsigned long slp;
unsigned long watchDog = 0;
unsigned long checkSMSMss;
int error; //a percentage of how much higher the read is when you don't drop outliers. could tell if a sensro is faulty
byte replyBuffer[150];
int sensorRead() {
  //need global usigned long sensorReadMss = millis() upon each smooth. and each read should be taken if sRM > (millis() + ((1+count)*500))//gthe one is for the first read to be half a second after smooth is done. assign it's value in smooth and check the if statement in loop
  int ping;
  int distance;
  digitalWrite(vcc, HIGH); //start sensor read
  //^just to supply power to the module. always on
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);
  //^^^trigger read
  ping = pulseIn(echo, HIGH);
  distance = ((ping*0.34)/2);
  Serial.print("individual read is ");
  Serial.print(distance);
  Serial.println();
  return distance;
}
void aboveAndBelow() {
  for(int i = 0; i <40; i++) {
    if(sensorReadArray[i] > 3500) {
      sensorReadArray[i] = 0;
    }
    if(sensorReadArray[i] < 100) {
      sensorReadArray[i] = 0;
    }
  }
}
void top() {
  int max_v = 0;
  int max_i = 0;
  for(int i=0; i<40; i++) {
    if(max_v>sensorReadArray[i]) {
      max_v = sensorReadArray[i];
      max_i = i;
    }
  }
  sensorReadArray[max_i] = 0;
}
void bottom() {
  int min_v = 9999;
  int min_i = 0;
  for(int i=0; i<40; i++) {
    if(sensorReadArray[i] < min_v && sensorReadArray[i] > 0) {
      min_v = sensorReadArray[i];
      min_i = i;
    }
  }
  sensorReadArray[min_i] = 0;
}
int average() {
  int countAverage = 0; //count for this local average
  long totalAverage = 0;
  int aveAverage;
  for(int i=0; i<40; i++) {
    if(sensorReadArray[i] > 0) {
      countAverage = countAverage + 1;
      totalAverage = totalAverage + sensorReadArray[i];
    }
  }
  aveAverage = totalAverage/countAverage;
  return aveAverage;
} 
void smooth() {
  int preave;
  previous = ave;
  aboveAndBelow();
  preave = average();
  top();
  bottom();
  ave = average();
  error = (100*(preave - ave))/ave;
  Serial.print("the average is ");
  Serial.println(ave);
  Serial.println(millis());
  Serial.println();  
}

void setup() {
  Serial.begin(115200);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(vcc, OUTPUT);
}
void loop() {
  Serial.println("LOOOOOPP YAYA");
  if (sensorReadMss < (millis() - ((1 + countSensorRead) * 500))) {
    sensorReadArray[countSensorRead]=sensorRead();
    countSensorRead++;
  }
  if (countSensorRead >= 39) {
    smooth();
    countSensorRead = 0;
  }
}