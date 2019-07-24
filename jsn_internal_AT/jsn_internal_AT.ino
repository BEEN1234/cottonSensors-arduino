/*
to do: 
  ->***thingspeak
  ->make the loop go through one sensor read then check for a message
  ->NIMP 'help' text?. maybe do that through twilio instead. 
  ->NIMP a code to put infront of add, and program to check commands are only sent from certain messages
  ->
  ->human words distance from sensor vs height above empty. empty - ave.
  ->***phonebook... update all three. I think they need to be strings. hmph can you use commas........f e q. i could. but what about idfferen lenght numbers. 
    ->got numbers and program to write them. to do:
      ->send text with them
      ->send only to those with numbers? - possibly write isNumber variable to 1 if buffer value in the given range is a number
  ->***eeprom nol, era, ara
  ->NIMP set nol if i+3 is a number? ferrrget it
  ->figure out shut down code and push buttons. 
*/
#define trig 12 //start sensor
#define echo 13
#define vcc 11

#include <SoftwareSerial.h>

SoftwareSerial SIM(2, 3);

//stop sensor
int sensorReadArray[60]; //start smoothing
int ave;
int emp = 3500; //all rates will pertain to sensor reads. updates will be done iin human language
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
int error; //a percentage of how much higher the read is when you don't drop outliers. could tell if a sensro is faulty
byte replyBuffer[255];
int sensorRead() {
  int ping;
  int distance;
  delay(950);                 //start sensor read
  digitalWrite(vcc, HIGH);
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
  for(int i = 0; i <=59; i++) {
    if(sensorReadArray[i] > 3500) {
      sensorReadArray[i] = 0;
    }
    if(sensorReadArray[i] < 200) {
      sensorReadArray[i] = 0;
    }
  }
}
void top() {
  int max_v = 0;
  int max_i = 0;
  for(int i=0; i<=59; i++) {
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
  for(int i=0; i<=59; i++) {
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
  for(int i=0; i<=59; i++) {
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
  aboveAndBelow();
  preave = average();
  for(int i=1; i<=5; i++) {
    top();
    bottom();
  }
  ave = average();
  error = (100*(preave - ave))/ave;
  Serial.print("the average is ");
  Serial.println(ave);
  Serial.println(millis());
  Serial.println();  
}
//stop smoothing
void SendUpdateSMS() {
  SIM.write("AT+CMGS=\"+61457254178\"");
  SIM.write( 0x0D );
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
  SIM.write( 0x1A );
  previousMss = millis();
}
int getNumber(byte bite) {
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
  else return 0;
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
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SIM.begin(4800);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(vcc, OUTPUT);
}

void loop() {
  for(int i = 0;i < 60;i++) {
    sensorRead();
    sensorReadArray[i]=sensorRead();};
  smooth();
  //if statement: needs to consider if it's out of range, 8 minutes since the last, and the ditch isn't filling (if we just start should wait until it's 10 cm into the nol range) or emptying (once it gets 50 cm out of nol... or maybe just shut it down..)
  if ((ave > (nol + ara) || ave < (nol + ara)) && (millis() - previousMss) > 420000 && slp < millis()) {
    SendUpdateSMS();
  }
  if ((ave > (nol + era) || ave < (nol + era)) && (millis() - previousMss) > 120000 && slp < millis()) {
    SendUpdateSMS();
  }
  memset(replyBuffer, 0, 255);
  SIM.write("AT+CMGRD=0");
  SIM.write( 0x0D );
  if (SIM.available()) {
    while (SIM.available()) {
      replyBuffer[count] = SIM.read();
      count++;
      if (count == 254)
        break;
    }
  }
  count = 0;
//add, del, upd, ara, era, emp, slp, nol grr. add del i think will just be all numbers back to back
  for (int i = 0; i < 255; i++) {
    Serial.write(replyBuffer[i]);
    if (replyBuffer[i] == 'u' && replyBuffer[i+1] == 'p' && replyBuffer[i+2] == 'd') {
      SendUpdateSMS();
      Serial.println("Recieved SMS \"upd\"");
    }
    if (replyBuffer[i] == 'a' && replyBuffer[i+1] == 'r' && replyBuffer[i+2] == 'a') {
      ara = 1000*getNumber(replyBuffer[i+3]) + 100*getNumber(replyBuffer[i+4]) + 10*getNumber(replyBuffer[i+5]) + getNumber(replyBuffer[i+6]);
    }
    if (replyBuffer[i] == 'e' && replyBuffer[i+1] == 'r' && replyBuffer[i+2] == 'a') {
      era = 1000*getNumber(replyBuffer[i+3]) + 100*getNumber(replyBuffer[i+4]) + 10*getNumber(replyBuffer[i+5]) + getNumber(replyBuffer[i+6]);
    }
    if (replyBuffer[i] == 'e' && replyBuffer[i+1] == 'm' && replyBuffer[i+2] == 'p') {
      emp = ave;}
    if (replyBuffer[i] == 's' && replyBuffer[i+1] == 'l' && replyBuffer[i+2] == 'p') {
      slp = millis() + 60000*(10*getNumber(replyBuffer[i+3]) + getNumber(replyBuffer[i+4]));
    }
    if (replyBuffer[i] == 'n' && replyBuffer[i+1] == 'o' && replyBuffer[i+2] == 'l') {
      nol = ave;
    }
    if (replyBuffer[i] == 'a' && replyBuffer[i+1] == 'd' && replyBuffer[i+2] == 'd'){
      int firstN;
      int secondN;
      memset(phoneNumber1, 0, 15);
      isNumber1 = 0;
      memset(phoneNumber2, 0, 15);
      isNumber2 = 0;
      memset(phoneNumber3, 0, 15);
      isNumber3 = 0;
      //geeeeez somehow need ###... , ####... ,#### to be turned into a phone book
      //kin of leaning towards a phonebook of about 45 characters and using commas as a command for the computer. ..  . mehhhhhhhhhhh
      //feeeq. can you stor the numb in an int? nope
      //write three seperate arrays. if buffer[i+x] == ',' break? else = phone[1].. think'd be nice to use a for loop. then switch to the next buffer. writes null? init w/ memset
      for (int n = 0; n < 15; n++) {
        if (replyBuffer[i+3+n] == ',') {
          firstN = n;
          break;
      }
        if (isDigit(replyBuffer[i+3+n])) {
          phoneNumber1[n] = replyBuffer[i+3+n];
          if (n > 6) {
            isNumber1 = 1;
          }
        }
      }
      for (int n = 0; n < 15; n++) { 
        if (replyBuffer[i+3+n+firstN+1] == ',') {
          secondN = n;
          break;
        }
        if (isDigit(replyBuffer[i+3+n+1+firstN])) {
          phoneNumber2[n] = replyBuffer[i+3+n+1+firstN];
          if (n > 6) {
            isNumber2 = 1;         
          }
        //going to have to store the break number of the first and pick up off it +2
        }
      }
      for (int n = 0; n < 15; n++) {
        if (isDigit(replyBuffer[i+3+n+1+firstN+secondN])) {
          phoneNumber3[n] = replyBuffer[i+3+n+1+firstN+secondN];
          if (n > 6) {
            isNumber3 = 1;
        }
        }
      }
    }
  }
  memset(replyBuffer, 0, 255);
}
