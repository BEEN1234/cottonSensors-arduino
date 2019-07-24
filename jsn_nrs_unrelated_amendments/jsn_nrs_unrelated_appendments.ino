 /* TEST: Fucks sake. sendSMS code is stupid. for some reason it misreads and an argument seams to only take 1 thing from a string. I'm just going to write SMS start and send codes maybe?
 * TEST Change if to else if
 * TEST need to cut out EEPROM for now
 * it seemed to be working normally, just not receiving texts and obviously taking the 1s delay I told it to each loop. how to pass that... only check for SMS every 5 s. then I'll be through the cycle way quicker than it can read.
to do: 
  ->***thingspeak AND low power library. fuk low power. it feels like too much work.
  ->***shutDown() which will... do wat? take a read every 10 minutes then check it against a small string to see if the levels have increased 30 cm in the last... 10 mins or something. then send an update text. must execute this program in response to a shutdown text
    ->somehow should have an hours command. shut down for sure for __ hours and continue to be shut down until it wakes up. 
      ->i think i'll just have it shut down for at least 24 hours. And have a button to wake it up. or maybe until 10 am the next day or something?
    ->I'll add an input pin for the button to read from during shutDown() for a break condition. super freaking easy wit shutdown code
  ->*** fix the if statements. I thought they got checked 1 by 1, but if 1 is sated it skips the rest. 
    ->how does if and else if work?. i think maybe to get skipped else if needs to be used? and else terminates the loop.
  ->*** fix millis() variables. I think in milliseconds they'll only hold some 20 days. whereas millis*() will run for about 50 days. . . grrr. could just have a lea
//i think all the stupid shit like hexadecimal,  base 10, octa, and whatever is all the same shit to the microcontroller? so if it reads something in quotes it converts it to binary equivalent and whatever reads it is supposed to make sense of it. So you could take <LF> + <LF> to an integer. What does it matter if it gets converted to a different datatype?
  ->TEST***fix prevMss value.
  ->TEST***maybeNIMP because it should always be on with shutdown program***eeprom nol, era, ara, emp phoneNumbers and isNumbers (or use sizeof(phoneNumber1) which returns number of bytes occupied.)
    ->I guess I'll call the values in void Setup and write them each time a cell command is issued. write initials in the pre-program
  ->TEST -- NIMP a code to put infront of add (maybe year and month of purchase -> 1712), and program to check commands are only sent from certain messages  
  ->TESTsmooth - append it so it doesn't skew the data so much. wrong reads are rare. or possibly have a shorter full read time. like 20 seconds and 40 reads or something. 
  ->TEST(all done i think, just need to start using isNumbers in add program as well)***change SMS code to SMSToo with another variable for replyTo type idea. just need if statements with for loops within. Easy peasy.
  ->TEST***finish programming add del code. include response sendSMSTo codes 
  ->TEST sendUpdateSMS - a variable that can be update, emergency, or alert byte strings
  ->TEST***I did write in response texts*** (or a code to update phonebook and code to check that other commands are from a recognized number: just trying to avoid fluke texts changing nol or something) reply verifications texts . . . new program that'll send a byte string in the body and check if it's received then try again for up to 1 minute or something. 
    ->ugh. I think write code that checks if (replyBuffer[i] == phoneNumber1[0] && rB[i+1] == pN[1])... || (rB[i] == pN[0] ...)
      -> maybe this can be the leading condition?
    ->maybe reply to add should be inherently searching for at least 6 digits immediately following it. 
      -> and it's response should be sent to the old numbers... maybe move it up in the code before the phonebook gets updated.
    ->hrm. or add should have a code infront and only add the number it's received from? then you have to write code for iterating phonebooks and a system to tell what number is where (easy) and what to replace (in the text?)
  ->TEST(how logic control treats 2 ands and an or, and brackets)***remove case sensitivity of the first byte in the receive SMS block in void loop
  ->TEST***human words distance from sensor vs height above empty. empty - ave. for nol, ave, and prev.
  ->TEST***phonebook... update all three. I think they need to be strings. hmph can you use commas........f e q. i could. but what about idfferen lenght numbers. 
    ->got numbers and program to write them. to do:
      ->send text with them
      ->send only to those with numbers? - possibly write isNumber variable to 1 if buffer value in the given range is a number

  ->probablyNIMP*** check if SMS is sent//FERGET IT. could do it, but I think i'll leave it up to the farmers to make sure. I've already bent over backwards for this.
    -> while(thingo == thingo)
       ->sendSMS
      ->if watchdog (10s) passes power cycle the module and break; MAYBE THE WHILE LOOP IS USELESS. SET TIMEOUT TO 10s AND USE IF (SERIAL.FIND()) {DONE}. but if it doesn't work it shoudl try again. All find does is wait for a response. 
        -> maybe try to send an error text too. possible rx is just broken and tx is fine. 
      ->if (SErial.find("+CMGS "NEED TO KNOW WHAT IT RESPONDS WITH)... break;
    -> maybe instead of delay inside sendSMS check for replies using SIM.find("whatever it shjould say");
  ->NIMP 'help' text?. maybe do that through twilio instead. 

  */
#define trig 12 //start sensor
#define echo 13
#define vcc 11

#include <SoftwareSerial.h>
#include <EEPROM.h>

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
byte password[4] = "1712";
byte phoneNumber1[15];
int isNumber1 = 0; // to check so as to not send a wrong number
byte phoneNumber2[15];
int isNumber2 = 0;
byte phoneNumber3[15];
int isNumber3 = 0;
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
  sensorReadMss = millis();
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
//stop smoothing
void sendUpdateSMSTo(int n) {
  int Continue = 0;
  if (n == 1 && isNumber1 == 1)
    Continue = 1;
  else if (n == 2 && isNumber2 == 1)
    Continue = 1;
  else if (n == 3 && isNumber3 == 1)
    Continue = 1;
  else if (n == 4 && isDigit(bufferPhoneNumber[0]))
    Continue = 1;
  if (Continue == 1) { //doesn't work because when other if statements are sated it'll skip er
    SIM.write("AT+CMGS=\"+");
    for (int i = 0; i < 15; i++) {
      if (n == 1) {
        SIM.write(phoneNumber1[i]);
      }
      if (n == 2) {
        SIM.write(phoneNumber2[i]);
      }
      if (n == 3) {
        SIM.write(phoneNumber3[i]);
      }
      if (n == 4) {
        SIM.write(bufferPhoneNumber[i]);
      }
    }
    SIM.write("\"");
    SIM.write( 0x0D );
    delay(500);
    SIM.write("The level is ");
    SIM.print(emp - ave);
    SIM.write(". it was ");
    SIM.print(emp - previous);
    SIM.write(". The normal operating level is ");
    SIM.print(emp - nol);
    SIM.write(". ara and era equal ");
    SIM.print(ara);
    SIM.write(" and ");
    SIM.print(era);
    SIM.write(".");
    SIM.write( 0x1A );
    delay(500);
    Continue = 0;
  }
}
/*
void sendSMSTo(byte Write, int Print, byte Write2, int n) {
  SIM.write("AT+CMGS=\"+");
  for (int i = 0; i < 15; i++) {
    if (n == 1) {
      SIM.write(phoneNumber1[i]);
    }
    else if (n == 2) {
      SIM.write(phoneNumber2[i]);
    }
    else if (n == 3) {
      SIM.write(phoneNumber3[i]);
    }
    else if (n == 4) {
      SIM.write(bufferPhoneNumber[i]);
    }
  }
  SIM.write("\"");
  SIM.write( 0x0D );
  delay(500);
  SIM.write(Write);
  if (Print > -40000) {
    SIM.print(Print);
  }
  SIM.write(Write2);
  SIM.write( 0x1A );
  delay(500);
}
  */
void sendSMSTo(int n) {
  if (n == 1 && isNumber1 == 1)
    Continue = 1;
  else if (n == 2 && isNumber2 == 1)
    Continue = 1;
  else if (n == 3 && isNumber3 == 1)
    Continue = 1;
  else if (n == 4 && isDigit(bufferPhoneNumber[0]))
    Continue = 1;
  if (Continue == 1) { //doesn't work because when other if statements are sated it'll skip er
    SIM.write("AT+CMGS=\"+");
    for (int i = 0; i < 15; i++) {
      if (n == 1) {
        SIM.write(phoneNumber1[i]);
      }
      if (n == 2) {
        SIM.write(phoneNumber2[i]);
      }
      if (n == 3) {
        SIM.write(phoneNumber3[i]);
      }
      if (n == 4) {
        SIM.write(bufferPhoneNumber[i]);
      }
    }
    SIM.write("\"");
    SIM.write( 0x0D );
    delay(500);
  }
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

void Shutdown() {
  int thingo = 1;
  unsigned long previousRead = millis() + 60000*60*24;
  while (thingo == 1) {
    int wrongReads = 0;
    delay(10000);
    //power down module
    //low power mode for arduino
    //Wake up and take tests once per 10 min and make sure it's within 300 of emp. following a high read, read again twice more over 2 minutes and then start up and send a text.
    if ((previousRead + 600000) < millis()) {
      previousRead = millis(); 
      if (sensorRead() > (emp + 300)) {
        for(int i = 0; i < 5; i++) {
          if (sensorRead() > (emp + 300))
            wrongReads++;
            delay(3000);
        }
      }
      if (wrongReads > 4) {
        thingo = 0;
        //wakeup
      }
      wrongReads = 0;
    }
  }
}
  //This oould just be in a broken while loop that has  text set the number to 1, and a read above 300 followed by 2 consecuteive reads over 300 as well
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

  //eeprom initializations. fuck. how do the first time? in the part prior to setup () can we write eeprom values? who cares. there msut be a way. here I'll read the first set. 2 bytes per thing and 15 per phone number
  //nol, ara, era, emp, pN1, pN2, pN3, iN1, iN2, and iN3// have pN1, iN1. who cares. iN is a dumb thing to include, but whatever. I already did. a simple "isDigit(pN[]) would've worked
  //2, 2, 2, 2, 15, 1, 15, 1, 15, 1. that's a total of 56. 
  //looks like val read, number read, and iN read are all things I'll need.
  nol = EEPROM.read(0)*256 + EEPROM.read(1);
  ara = EEPROM.read(2)*256 + EEPROM.read(3);
  era = EEPROM.read(4)*256 + EEPROM.read(5);
  emp = EEPROM.read(6)*256 + EEPROM.read(7);
  for (int i = 0; i < 15; i++) {
    phoneNumber1[i] = EEPROM.read(8 + i);
  }
  isNumber1 = EEPROM.read(23);
  for (int i = 0; i < 15; i++) {
    phoneNumber2[i] = EEPROM.read(24 + i);
  }
  isNumber2 = EEPROM.read(39);
  for (int i = 0; i < 15; i++) {
    phoneNumber3[i] = EEPROM.read(40 + i);
  }
  isNumber3 = EEPROM.read(55);
}
void loop() {
  if (sensorReadMss < (millis() - ((1 + countSensorRead) * 500))) {
    sensorReadArray[countSensorRead]=sensorRead();
    countSensorRead++;
  }
  if (countSensorRead >= 39) {
    smooth();
    countSensorRead = 0;
    Serial.println("nol, ara and era");
    Serial.println(nol);
    Serial.println(ara);
    Serial.println(era);
  }
  //if statement: needs to consider if it's out of range, 8 minutes since the last, and the ditch isn't filling (if we just start should wait until it's 10 cm into the nol range) or emptying (once it gets 50 cm out of nol... or maybe just shut it down..)
  if ((ave > (nol + ara) || ave < (nol - ara)) && (millis() - previousMss) > 420000 && slp < millis()) {
    sendUpdateSMSTo(1);
    sendUpdateSMSTo(2);
    sendUpdateSMSTo(3);
    Serial.println("sent alarms");
    previousMss = millis();
  }
  if ((ave > (nol + era) || ave < (nol - era)) && (millis() - previousMss) > 120000 && slp < millis()) {
    sendUpdateSMSTo(1);
    sendUpdateSMSTo(2);
    sendUpdateSMSTo(3);
    previousMss = millis();
  }
  if (checkSMSMss < millis()) {
    checkSMSMss = millis() + 5000;
    memset(replyBuffer, 0, 150);
    watchDog = millis() + 2000; // should wait 2 seconds for a response... probs only need a few milliseconds.
    while (SIM.available()) {
      byte z = SIM.read(); //clear the serial buffer prior to my making a command so i can recieve a text msg
    }//***what about if something is being written to the serial buffer whilst I'm making my command and it's response get's printed? THen I guess i better hope it doesn't happen when i have sent a text and the erronous reads include del1,2, or 3, or nol... grrr
    SIM.write("AT+CMGRD=0");
    SIM.write( 0x0D );
    while (SIM.available() == 0) {
      if (watchDog < millis()) {
        break;}
      }
    if (SIM.available()) { //apparently readBytes does exactly this :'( and the stuff above too
      int count = 0;
      while (SIM.available()) {
        replyBuffer[count] = SIM.read();
        watchDog = millis() + 1000;
        count++;
        while (SIM.available() == 0) {
          if (watchDog < millis()) {
            break;
          }
        }
        if (count >= 149)
          break;
      }
    }
  }
  for (int i = 0; i < 100; i++) {
    if (replyBuffer[i] == '+') {
      for (int j = 0; j < 15; j++) {
        if (replyBuffer[i+1+j] == phoneNumber1[j]) {
          if (replyBuffer[i+2+j] == '"') {
            replyTo = 1; 
            break;// need to reset this to 0 after it is read. i guess just at the end of the void loop. 
          }
        }
        if (!replyBuffer[i+1+j] == phoneNumber1[j]) {
          break;
        }
      }
      for (int j = 0; j < 15; j++) {
        if (replyBuffer[i+1+j] == phoneNumber2[j]) {
          if (replyBuffer[i+2+j] == '"') {
            replyTo = 2;
            break;// need to reset this to 0 after it is read. i guess just at the end of the void loop. 
          } //it still will be set every time. need to do it under the '"'
        }
        if (!replyBuffer[i+1+j] == phoneNumber1[j]) {
          break;
        }
      }
      for (int j = 0; j < 15; j++) {
        if (replyBuffer[i+1+j] == phoneNumber3[j]) {
          if (replyBuffer[i+2+j] == '"') {
            replyTo = 3;
            break;// need to reset this to 0 after it is read. i guess just at the end of the void loop. 
          }
        }
        if (!replyBuffer[i+1+j] == phoneNumber1[j]) {
          break;
        }
      }
      for (int j = 0; j < 15; j++) {
        if (replyBuffer[i+1+j] == bufferPhoneNumber[j]) {
          if (replyBuffer[i+2+j] == '"') {
            replyTo = 4;
            break;// need to reset this to 0 after it is read. i guess just at the end of the void loop. 
          }
        }
        if (!replyBuffer[i+1+j] == bufferPhoneNumber[j]) {
          break;
        }
      }
    }
  }
  if (replyTo > 0) {
    for (int i = 0; i < 150; i++) {
      //intuitively I really really wish that this would search for only 1 or 2 things before pulling up other parts of the array. It seams like accessing 6 consecutive chunks of the array and comparing them to 6 consecutive numbers of another array could be taxing. although they are pretty small values . . . say a byte for each datattype, location, and value
      if ((replyBuffer[i] == 'u' || replyBuffer[i] == 'U') && replyBuffer[i+1] == 'p' && replyBuffer[i+2] == 'd') {
        sendUpdateSMSTo(replyTo);
        Serial.println("Recieved SMS \"upd\"");
      }
      if ((replyBuffer[i] == 'a' || replyBuffer[i] == 'A') && replyBuffer[i+1] == 'r' && replyBuffer[i+2] == 'a') {
        ara = 1000*getNumber(replyBuffer[i+3]) + 100*getNumber(replyBuffer[i+4]) + 10*getNumber(replyBuffer[i+5]) + getNumber(replyBuffer[i+6]);
        EEPROM.write(1, (ara%256));
        EEPROM.write(0, ((ara - (ara%256))/256));
        for (int j = 1; j <=3; j++) {
          sendSMSTo(j);
          if (Continue == 1) {
            SIM.write("ara is ");
            SIM.print(ara);
            SIM.write(" mm.");
            SIM.write(0x1A);
            delay(500);
            Continue = 0;
          }
        }
      }
      if ((replyBuffer[i] == 'e' || replyBuffer[i] == 'E') && replyBuffer[i+1] == 'r' && replyBuffer[i+2] == 'a') {
        era = 1000*getNumber(replyBuffer[i+3]) + 100*getNumber(replyBuffer[i+4]) + 10*getNumber(replyBuffer[i+5]) + getNumber(replyBuffer[i+6]);
        EEPROM.write(4, ((era - (era%256))/256));
        EEPROM.write(5, era%256);
        for (int j = 1; j <=3; j++) {
          sendSMSTo(j);
          if (Continue == 1) {
            SIM.write("era is ");
            SIM.print(era);
            SIM.write(" mm.");
            SIM.write(0x1A);
            delay(500);
            Continue = 0;
          }
        }
      }
      if ((replyBuffer[i] == 'e' || replyBuffer[i] == 'E') && replyBuffer[i+1] == 'm' && replyBuffer[i+2] == 'p') {
        emp = ave;
        EEPROM.write(6, ((emp - (emp%256))/256));
        EEPROM.write(7, emp%256);
        for (int j = 1; j <=3; j++) {
          sendSMSTo(j);
          if (Continue == 1) {
            SIM.write("emp is ");
            SIM.print(emp);
            SIM.write(" mm from the sensor.");
            SIM.write(0x1A);
            delay(500);
            Continue = 0;
          }
        }
      }
      if ((replyBuffer[i] == 's' || replyBuffer[i] == 'S') && replyBuffer[i+1] == 'l' && replyBuffer[i+2] == 'p') {
        slp = millis() + 60000*(10*getNumber(replyBuffer[i+3]) + getNumber(replyBuffer[i+4]));
        for (int j = 1; j <=3; j++) {
          sendSMSTo(j);
          if (Continue == 1) {
            SIM.write("Sleep for ");
            SIM.print(slp/60000);
            SIM.write(" minutes.");
            SIM.write(0x1A);
            delay(500);
            Continue = 0;
          }
        }
      }
      if ((replyBuffer[i] == 'n' || replyBuffer[i] == 'N') && replyBuffer[i+1] == 'o' && replyBuffer[i+2] == 'l') {
        nol = ave;
        EEPROM.write(0, ((nol - (nol%256))/256));
        EEPROM.write(1, nol%256);
        for (int j = 1; j <=3; j++) {
          sendSMSTo(j);
          if (Continue == 1) {
            SIM.write("The normal operating level is ");
            SIM.print(emp - nol);
            SIM.write(" mm.");
            SIM.write(0x1A);
            delay(500);
            Continue = 0;
          }
        }
      }
            /* 
        *  nol = EEPROM.read(0)*255 + EEPROM.read(1);
  ara = EEPROM.read(2)*255 + EEPROM.read(3);
  era = EEPROM.read(4)*255 + EEPROM.read(5);
  emp = EEPROM.read(6)*255 + EEPROM.read(7);
  for (int i = 0; i < 15; i++) {
    phoneNumber1[i] = EEPROM.read(8 + i);
  }
  isNumber1 = EEPROM.read(23);
  for (int i = 0; i < 15; i++) {
    phoneNumber2[i] = EEPROM.read(24 + i);
  }
  isNumber2 = EEPROM.read(39);
  for (int i = 0; i < 15; i++) {
    phoneNumber3[i] = EEPROM.read(40 + i);
  }
  isNumber3 = EEPROM.read(55);
}
        */
      if ((replyBuffer[i] == 'd' || replyBuffer[i] == 'D') && replyBuffer[i+1] == 'e' && replyBuffer[i+2] == 'l') {
        //need to hash this out with a reply text like the one in add and then give er another run... wish i could use addReply = 1
        //OR(done it already) *** prompt them to respond with del# to both this and ####add (obviously only if phonebook is full)
        //need to combine with below function. if i+3 isDigit and addReply == 1 and replyTo == 4 replace. if addReply == 0 just memset. if i+3 isn't a digit, respond with phonebook text I wrote for add.
        //*** whichever
        //need to have code that searches text for the number... maybe use add# format to make this possible
        //need to memset bufferPhoneNumber, reset addReply, and maybe have an expiration... or just wait until the next add command. after storing it in whichever thing is to be replaced.
        if (replyBuffer[i+3] == '1') { //isnumber
          if (replyTo < 4) {
            sendSMSTo(replyTo);
            if (Continue == 1) {
              SIM.write("Phone number 1 deleted");
              SIM.write(0x1A);
              delay(500);
              Continue = 0; 
            }
            
          }
          memset(phoneNumber1, 0, 15);
          isNumber1 = 0;
          for (int j = 0; j < 15; j++) {
            EEPROM.write(8 + j, phoneNumber1[j]);
          }
          EEPROM.write(23, 0);
          
          //if addReply == 1 && replyTo ==4 also reset addReply
          if (replyTo == 4) {
            for (int j = 0; j < 15; j++) {
              phoneNumber1[j] = bufferPhoneNumber[j];
              EEPROM.write(8 + j, bufferPhoneNumber[j]);
            }
            isNumber1 = 1;
            EEPROM.write(23, 1);
            memset(bufferPhoneNumber, 0, 15);
            addReply = 0;
            sendSMSTo(1);
            if (Continue == 1) {
              SIM.write("Your number is now phone number 1!");
              SIM.write(0x1A);
              delay(500);
              Continue = 0;
            }
            //reset the  bufferPN? if you don't replyTo will be 4 if they text
          }
        }
        else if (replyBuffer[i+3] == '2') {
          if (replyTo < 4) {
            sendSMSTo(replyTo);
            if (Continue == 1) {
              SIM.write("Phone number 2 deleted");
              SIM.write(0x1A);
              delay(500);
              Continue = 0;
            }
          }
          memset(phoneNumber2, 0, 15);
          isNumber2 = 0;
          for (int j = 0; j < 15; j++) {
            EEPROM.write(24 + j, phoneNumber2[j]);
          }
          EEPROM.write(39, 0);
          
          if (replyTo == 4) {
            for (int j = 0; j < 15; j++) {
              phoneNumber2[j] = bufferPhoneNumber[j];
              EEPROM.write(24 + j, bufferPhoneNumber[j]);
            }
            isNumber2 = 1;
            EEPROM.write(39, 1);
            memset(bufferPhoneNumber, 0, 15);
            addReply = 0;
            sendSMSTo(2);
            if (Continue == 1) {
              SIM.write("Your number is now ");
              SIM.write("phone number 2");
              SIM.write(0x1A);
              delay(500);
              Continue = 0;
            }
          }
        }
        else if (replyBuffer[i+3] == '3') {
          if (replyTo < 4) {
            sendSMSTo(replyTo);
            if (Continue == 1) {
              SIM.write("Phone number");
              SIM.write("3 deleted");
              SIM.write(0x1A);
              delay(500);
              Continue = 0;
            }
          }
          memset(phoneNumber3, 0, 15);
          isNumber3 = 0;
          for (int j = 0; j < 15; j++) {
            EEPROM.write(40 + j, phoneNumber3[j]);
          }
          EEPROM.write(55, 0);
          
          if (replyTo == 4) {
            for (int j = 0; j < 15; j++) {
              EEPROM.write(40 + j, bufferPhoneNumber[j]);
              phoneNumber3[j] = bufferPhoneNumber[j];
            }
            isNumber3 = 1;
            EEPROM.write(55, 1);
            memset(bufferPhoneNumber, 0, 15);
            addReply = 0;
            sendSMSTo(1);
            if (Continue == 1) {
              SIM.write("Your number is now");
              SIM.write("phone number 3");
              SIM.write( 0x1A );
              delay(500);
              Continue = 0;
            }
          }
        }
        else {
          if (addReply == 1) {
            sendSMSTo(4);
            if (Continue == 1) {
              SIM.write("Error, please respond with \"del1\", \"del2\", or \"del3\".");
              SIM.write( 0x1A );
              delay(500);
              Continue = 0;
            }
          }
          else if (addReply == 0) {
            SIM.write("AT+CMGS=\"+");
            if (replyTo == 1) {
              for (int m = 0; m < 15; m++) {
                SIM.write(phoneNumber1[m]);
              }
            }
            else if (replyTo == 2) {
              for (int m = 0; m < 15; m++) {
                SIM.write(phoneNumber1[m]);
              }
            }
            else if (replyTo == 3) {
              for (int m = 0; m < 15; m++) {
                SIM.write(phoneNumber1[m]);
              }
            }
            SIM.write("\"");
            SIM.write( 0x0D );
            delay(500);
            SIM.write("In the phonebook Position \"1\", is ");
            for (int m = 0; m < 15; m++) { //isNumber
              SIM.write(phoneNumber1[m]);
            }
            SIM.write(". \"2\" is ");
            for (int m = 0; m < 15; m++) {
              SIM.write(phoneNumber2[m]);
            }
            SIM.write(". And \"3\" is ");
            for (int m = 0; m < 15; m++) {
              SIM.write(phoneNumber3[m]);
            }
            SIM.write(". Choose which you would like to delete by replying with \"del1\", \"del2\", or \"del3\"");                 
            SIM.write( 0x1A );
            delay(500);           
          }
        }
        
      }//should i add a break at the end of every sated command.       
    }
  }
  for (int i = 0; i < 150; i++) { //needs serious work. looks pretty alright i'd say. just defo need EEPROM work and 
    //NEED to read the specfic phonenumber (more than 1 +)
    Serial.write(replyBuffer[i]);
    if (replyBuffer[i] == password[0] && replyBuffer[i+1] == password[1] && replyBuffer[i+2] == password[2] && replyBuffer[i+3] == password[3] && replyBuffer[i+4] == 'a' && replyBuffer[i+5] == 'd' && replyBuffer[i+6] == 'd'){
      //need to cycle through buffer again for + and replace a null # with what follows until '"' which will cause a break. 
      memset(bufferPhoneNumber, 0, 15);
      for (int n = 0; n < 100; n++) {
        if (replyBuffer[n] == '+' && isDigit(replyBuffer[n+1])) {
          for (int mm = 0; mm < 15; mm++) {
            if (isDigit(replyBuffer[n+1+mm])) {
              bufferPhoneNumber[mm] = replyBuffer[n+1+mm];
              if (replyBuffer[n+2+mm] == '"')
                break;
            }
          }
          Serial.println("add code initiated");
          if (isNumber1 == 0) {
            for (int m = 0; m < 15; m++) {
              EEPROM.write(8 + m, bufferPhoneNumber[m]);
              phoneNumber1[m] = bufferPhoneNumber[m];
            }
            memset(bufferPhoneNumber, 0, 15);
            isNumber1 = 1;
            EEPROM.write(23, 1);
            sendSMSTo(1);
            if (Continue == 1) {
              SIM.write("Your number is now phone number 1"); 
              SIM.write( 0x1A );
              delay(500);
              Continue = 0;
            }
            break; // possibly need to cut out of this program before other values are written?
          }
          else if (isNumber2 == 0) {
            for (int m = 0; m < 15; m++) {
              EEPROM.write(24 + m, bufferPhoneNumber[m]);
              phoneNumber1[m] = bufferPhoneNumber[m];
            }   
            memset(bufferPhoneNumber, 0, 15);         
            isNumber2 = 1;
            EEPROM.write(39, 1);
            sendSMSTo(2);
            if (Continue == 1) {
              SIM.write("Your number is now phone number 2");
              SIM.write( 0x1A );
              delay(500);
              Continue = 0;
            }
            break;
          }
          else if (isNumber3 == 0) {
            for (int m = 0; m < 15; m++) {
              EEPROM.write(40 + m, bufferPhoneNumber[m]);
              phoneNumber1[m] = bufferPhoneNumber[m];
            }
            memset(bufferPhoneNumber, 0, 15);            
            isNumber3 = 1;
            EEPROM.write(55, 1);
            sendSMSTo(3);
            if (Continue == 1) {
              SIM.write("Your number is now phone number 3"); 
              SIM.write( 0x1A );
              delay(500);
              Continue = 0;
            }
            break;
          }
          else {
            //give er reply text //quite a bit of coding. also need a del function...
            //not gonna process the re-reply here. set a variable to 1 and run an if statement up above.
            SIM.write("AT+CMGS=\"+");
            for (int m = 0; m < 15; m++) {
              SIM.write(bufferPhoneNumber[m]);
            }
            SIM.write("\"");
            SIM.write( 0x0D );
            delay(500);
            SIM.write("The phonebook is full. Position \"1\", is ");
            for (int m = 0; m < 15; m++) {
              SIM.write(phoneNumber1[m]);
            }
            SIM.write(". \"2\" is ");
            for (int m = 0; m < 15; m++) {
              SIM.write(phoneNumber2[m]);
            }
            SIM.write(". And \"3\" is ");
            for (int m = 0; m < 15; m++) {
              SIM.write(phoneNumber3[m]);
            }
            SIM.write(". Choose which you would like to replace by replying with \"del1\", \"del2\", or \"del3\"");                 
            SIM.write( 0x1A );
            delay(500);
            //*** make addReply a variable that also get's executed with del and then combine streams!
            addReply = 1;
          }
        }
      }
    }
  }
      /*
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
      for (int n = 0; n < 3; n++) {
        SIM.write("AT+CMGS=\"+"
        for (int i = 0; i < 15; i++) {
          if (n == 0) {  
            SIM.write(phoneNumber1[i]);}
          if (n == 1) {
            SIM.write(phoneNumber2[i]);
          }
          if (n == 2) {
            SIM.write(phoneNumber3[i]);
          }
        }
        SIM.write("\""); //sendsms
        SIM.write( 0x0D );
        delay(500);
        SIM.write("Phonebook updated");
        SIM.write( 0x1A );
      }*/
  memset(replyBuffer, 0, 150);
  replyTo = 0;
}
