#include "SIM900.h"
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>
#include "inetGSM.h"
#include <SPI.h>
#include <SD.h>
#define doorSwitch 20
#include <SimpleTimer.h>
#include "DHT.h"
#include "sms.h"
#define signalReport A0
#define gprsReport A1
#define smsProcess A2
#define cameraProcess A3
#define encodeProcess A4
#define uploadProcess A5
#define weatherReport A6
#define doorReport A7
SMSGSM sms;
SimpleTimer timer;
SimpleTimer timerRoutine;
SimpleTimer timerAlert;
InetGSM net;
boolean started = false;

char inSerial[40];
//uuint i = 0;
String smtp_server, smtp_port, smtp_username, smtp_password, email_server, email_device, device_id, report_periode, 
sms_server, sms_sv1, sms_sv2, sms_sv3, tresUpTemp, tresDownTemp, tresUpRH, tresDownRH, apnName;
String eventStatus, timeStamp, currentTemp, currentRH;
char end_c[2];

#define chipSelect 4
char character;
String settingValue;

//char buff[100];
//int j=0;

char aux_string[30];
int totalLine=0;
long totalChara=0;
int line=0;
boolean endOfData=false;

Adafruit_VC0706 cam = Adafruit_VC0706(&Serial1);
#define DHTPIN 21     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

boolean doorStateChange = false;
boolean pictureTaken = false;

String pictureOneName, pictureTwoName, pictureThreeName;
String b64OneName, b64TwoName, b64ThreeName;

boolean gprsDisconnect = true;
int doorState=1;
boolean cameraEncodeUploadProses = false;
boolean tempRHReport = false;
boolean sendTresReportSms = false;

unsigned long prevMillis = 0;

void reportTempAndRH(){
  tempRHReport = true;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  int t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  //char tempData[50];
  //sprintf(tempData, "Humidity: %d Temperature: %d *C ",h,t);
  Serial.println("Humidity: "+String(h)+" %\t Temperature: "+String(t)+" *C ");
  //Serial.println(String(tempData));
  currentTemp = String(t);
  currentRH = String(h);

  if(readClockData() == 1){
    //send New message to sms server
    eventStatus = "S";
    char buff_v0[sms_server.length()+1];
    sms_server.toCharArray(buff_v0, sms_server.length()+1);
    String inputDigitalState = "111111111111111";
    String newMessage = "$ " +device_id +" " +eventStatus +" " +String(doorState) +inputDigitalState +" " +currentTemp 
                             +" " +currentRH +" " +timeStamp +"#";
    char buffMessage[newMessage.length()+1];
    newMessage.toCharArray(buffMessage, newMessage.length()+1);
    ulang:
    digitalWrite(smsProcess, LOW);
     if(sendNewMessage(buff_v0, buffMessage) == 1){
        Serial.println("new message sended to: " +String(buff_v0));
        tempRHReport = false;
        digitalWrite(smsProcess, HIGH);
     }
     else if(sendNewMessage(buff_v0, buffMessage) == 0){
        goto ulang;
     }
  }
  return;
}
void readTempAndRH(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  digitalWrite(weatherReport, LOW);
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  int t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.println("Humidity: "+String(h)+" %\t Temperature: "+String(t)+" *C ");
  currentTemp = String(t);
  currentRH = String(h);
  digitalWrite(weatherReport, HIGH);
  return;
}
void doorDetection(){
  doorStateChange = true;
}
int readDeviceSettings(){
  char(temp);
  String tempData[18];
  int counter = 0;
  String message="";
  File settingsFile = SD.open("config.txt", FILE_READ);
  if(settingsFile){
    Serial.println("file settings opened");
    while(settingsFile.available()){
      temp = settingsFile.read();
      if(temp != '\n'){
        tempData[counter] += temp;
      }  
      else{
        //Serial.println(tempData[counter]);
        int first = tempData[counter].indexOf('"');
        int second = tempData[counter].indexOf('"', first+1);
        if(counter == 0){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // device_id = bb;
              device_id = message;
              Serial.println("device id: " +String(device_id));
              message="";
        }
        else if(counter == 1){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // smtp_server = bb;
              smtp_server = message;
              Serial.println("smtp server: " +String(smtp_server));
              message="";
        }
        else if(counter == 2){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // smtp_port = bb;
              smtp_port = message;
              Serial.println("smtp port: " +String(smtp_port));
              message="";
        }
        else if(counter == 3){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // smtp_username = bb;
              smtp_username = message;
              Serial.println("smtp username: " +String(smtp_username));
              message="";
        }
        else if(counter == 4){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // smtp_password = bb;
              smtp_password = message;
              Serial.println("smtp password: " +String(smtp_password));
              message="";
        }
        else if(counter == 5){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_server = bb;
              email_server = message;
              Serial.println("email server: " +String(email_server));
              message="";
        }
        else if(counter == 6){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              email_device = message;
              Serial.println("email device: " +String(email_device));
              message="";
        }
        else if(counter == 7){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              report_periode = message;
              Serial.println("report periode: " +String(report_periode));
              message="";
        }
        else if(counter == 8){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              sms_server = message;
              Serial.println("sms server number: " +String(sms_server));
              message="";
        }
        else if(counter == 9){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              sms_sv1 = message;
              Serial.println("sms supervisor-1: " +String(sms_sv1));
              message="";
        }
        else if(counter == 10){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              sms_sv2 = message;
              Serial.println("sms supervisor-2: " +String(sms_sv2));
              message="";
        }
        else if(counter == 11){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              sms_sv3 = message;
              Serial.println("sms supervisor-3: " +String(sms_sv3));
              message="";
        }
        else if(counter == 12){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              tresUpTemp = message;
              Serial.println("treshold atas temperatur: " +String(tresUpTemp));
              message="";
        }
        else if(counter == 13){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              tresDownTemp = message;
              Serial.println("treshold bawah temperatur: " +String(tresDownTemp));
              message="";
        }
        else if(counter == 14){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              tresUpRH = message;
              Serial.println("treshold atas rh: " +String(tresUpRH));
              message="";
        }
        else if(counter == 15){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              tresDownTemp = message;
              Serial.println("treshold bawah rh: " +String(tresDownTemp));
              message="";
        }
        else if(counter == 16){
              char aa[tempData[counter].length()];
              tempData[counter].toCharArray(aa, tempData[counter].length());
              for(int i=first+1;i<second;i++){
                message += aa[i];
              }
              // char bb[message.length()];
              // message.toCharArray(bb, message.length());
              // email_device = bb;
              apnName = message;
              Serial.println("apn: " +apnName);
              message="";
        }
        tempData[counter] = "";
        counter++;
      } 
    }
    Serial.println("file settings done");
    settingsFile.close();
    return 1;
  }
  else{
    Serial.println("file settings not opened");
    return 0;
  }
}
void setup() {
  pinMode(doorSwitch,INPUT_PULLUP);
  pinMode(signalReport, OUTPUT);
  pinMode(gprsReport, OUTPUT);
  pinMode(smsProcess, OUTPUT);
  pinMode(cameraProcess, OUTPUT);
  pinMode(encodeProcess, OUTPUT);
  pinMode(uploadProcess, OUTPUT);
  pinMode(weatherReport, OUTPUT);
  pinMode(doorReport, OUTPUT);
    
  digitalWrite(signalReport, HIGH);
  digitalWrite(gprsReport, HIGH);
  digitalWrite(smsProcess, HIGH);
  digitalWrite(cameraProcess, HIGH);
  digitalWrite(encodeProcess, HIGH);
  digitalWrite(uploadProcess, HIGH);
  digitalWrite(weatherReport, HIGH);
  digitalWrite(doorReport, HIGH);

  timerAlert.setTimeout(1000, sendAlertMessage);
  timerRoutine.setInterval(10000, readTempAndRH);

  dht.begin();
  attachInterrupt(digitalPinToInterrupt(doorSwitch), doorDetection, RISING);
  end_c[0] = 0x1a;
  end_c[1] = '\0';
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization SD Card done.");
  
  // readImage = SD.open("IMAGE00.b64", FILE_READ);
  // readImage.close();

  Serial.println("Start GSM Shield");
  if (gsm.begin(9600)) {
    Serial.println("status=READY");
    started = true;
  }
  else
    Serial.println("status=IDLE");


  digitalWrite(signalReport, LOW);

  if (started) {
    if(getListTextFile() == 1){
      if(readDeviceSettings() == 1){
        //setting timer suhu dan rh in miliseconds
        char buff[50];
        report_periode.toCharArray(buff, 50);
        long d = atol(buff);
        //long d = 5000;
        timer.setInterval(d, reportTempAndRH);
        Serial.println("periode " +String(d));
        settingClock();
        // reportTempAndRH();
        // digitalWrite(signalReport, HIGH);
        //sendImage("IMAGE107.B64", "IMAGE107.JPG");
      }
    }
  }
 
 
}

int settingClock(){
  if(sapbrFunction() == 1){
    Serial.println("gprs activated");
    gprsDisconnect = false;
    char aa[apnName.length()+1];
    apnName.toCharArray(aa, apnName.length()+1);
    Serial.println(String(aa));
    if (net.attachGPRS(aa, "", "") == 1) {

        Serial.println("apn configured");
        if(readClockData() == 1){
          Serial.println("clock configured");
          return 1;
        }
        else{
          Serial.println("clock not configured");
          return 0;
        }
    }
    else{
        Serial.println("apn not configured");
    }
  }
  else{
    Serial.println("gprs not activated");
    return 0;
  }
}

int getListTextFile(){
  File root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("list done!");
  return 1;
}
void printDirectory(File dir, int numTabs) {
  int count=0;
  int totalFile=0;
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    totalFile++;
    String temp = String(entry.name());
    String num;
    for(int m=0; m<100; m++){
      if(m<10)
      num = "0"+String(m)+".TXT";
      else
      num = String(m)+".TXT";
      
      if(temp == num){
        count++;
      }
    }

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
  Serial.println("there are " +String(count) +" File");
  Serial.println("total " +String(totalFile) +" File");
  count=0; totalFile=0;
}
int deleteAllFiles(){
  int nr = 1;
  char filename[16];
  while(nr <= 300){
    sprintf(filename, "IMAGE%03d.JPG", nr);
    if(SD.remove(filename)){
      Serial.print(filename);
      Serial.println(" removed");
    }
    nr++;
  }
}
int snapPicture(int count){
  digitalWrite(cameraProcess, LOW);
  Serial.println("VC0706 Camera snapshot test");
   // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    digitalWrite(cameraProcess, HIGH);
    return 0;
  }
  
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
    digitalWrite(cameraProcess, HIGH);
    return 0;
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  
  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

  // Serial.println("Snap in 3 secs...");
  // delay(3000);

  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Create an image with the name IMAGExxx.JPG
  char filename[16];
  //strcpy(filename, "IMAGE");
  int nr = 1;  
  
  while (nr != 0)
  {
  //for (int i = 0; i < 1000; i++) {
    sprintf(filename, "IMAGE%03d.JPG", nr);
    if (SD.exists(filename) == false) break;
    Serial.print(filename);
    Serial.println(" exists.");
    if(nr >= 300){
        deleteAllFiles();
        nr = 1;
    }
    else{
      nr++;
    }
    //strcpy(filename, "IMAGE%d.JPG", i);
    // filename[5] = '0' + i%100;
    // filename[6] = '0' + i/10;
    // filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    // if (! SD.exists(filename)) {
    //   break;
    // }
  }
  Serial.println("Save as " +String(filename));
  char buff[30];int first;
  switch (count) {
      case 1:
        pictureOneName = String(filename);
        pictureOneName.toCharArray(buff, 30);
        first = pictureOneName.indexOf('.');
        for(int m=0;m<first;m++){
          b64OneName += buff[m];
        }
         b64OneName += ".b64";
        Serial.println(pictureOneName +" : " +b64OneName);
        break;
      case 2:
        pictureTwoName = String(filename);
        pictureTwoName.toCharArray(buff, 30);
        first = pictureTwoName.indexOf('.');
        for(int m=0;m<first;m++){
          b64TwoName += buff[m];
        }
        b64TwoName += ".b64";
        Serial.println(pictureTwoName +" : " +b64TwoName);
        break;
      case 3:
        pictureThreeName = String(filename);
        pictureThreeName.toCharArray(buff, 30);
        first = pictureThreeName.indexOf('.');
        for(int m=0;m<first;m++){
          b64ThreeName += buff[m];
        }
        b64ThreeName += ".b64";
        Serial.println(pictureThreeName +" : " +b64ThreeName);
        break;
  }
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);
  
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(96, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    // if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
    //   Serial.print('.');
    //   wCount = 0;
    // }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();

  time = millis() - time;
  time = time/1000;
  Serial.println("done!");
  Serial.print(time); Serial.println(" s elapsed");

  // // Create a text with the name xx.txt to save image name that haven't upload to mail
  // char filenameText[13];
  // strcpy(filenameText, "00.txt");
  // for (int i = 0; i < 100; i++) {
  //   filenameText[0] = '0' + i/10;
  //   filenameText[1] = '0' + i%10;
  //   // create if does not exist, do not open existing, write, sync after write
  //   if (! SD.exists(filenameText)) {
  //     break;
  //   }
  // }
  // File imgFileList = SD.open(filenameText, FILE_WRITE);
  // Serial.println("Save as " +String(filenameText));
  // if(imgFileList){
  //   imgFileList.print(String(filename));
  //   Serial.println("inside " +String(filename));
  // }
  // imgFileList.close();  
  digitalWrite(cameraProcess, HIGH);
  return 1;
}
int readClockData(){
    // gsm.SimpleWriteln("AT+CLTS=1");
    // switch (gsm.WaitResp(500, 100, "OK")) {
    //   case RX_FINISHED_STR_RECV:
    //   Serial.println("local time received");
    //   break;
    // }

    gsm.SimpleWriteln("AT+CNTPCID=1");
    switch (gsm.WaitResp(500, 100, "OK")) {
      case RX_FINISHED_STR_RECV:
      Serial.println("ntp id received");
      break;
    }

    gsm.SimpleWriteln("AT+CNTP=time.apple.com,28");
    switch (gsm.WaitResp(500, 100, "OK")) {
      case RX_FINISHED_STR_RECV:
      Serial.println("ntp route received");
      break;
    }

    gsm.SimpleWriteln("AT+CNTP");
    switch (gsm.WaitResp(500, 100, "OK")) {
      case RX_FINISHED_STR_RECV:
      Serial.println("ntp sync received");
      break;
    }
    delay(500);
    switch (gsm.WaitResp(1000, 100, "+CNTP: 1")) {
      case RX_FINISHED_STR_RECV:
      Serial.println("ntp time syncronized");
      break;
    }

    gsm.SimpleWriteln("AT+CCLK?");
    switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          String aa = (char*)gsm.comm_buf;
          int firstQuotation = aa.indexOf('"');
          int secondQuotation = aa.indexOf('"', firstQuotation+1);
          char buff[aa.length()];
          aa.toCharArray(buff,aa.length());
          String message="";
          for(int i=firstQuotation+1;i<secondQuotation;i++){
            message += buff[i];
          }
          Serial.println("date: " +message);
          timeStamp = message;
          return 1;
          break;
    }
  
}

int readFile(long location, long destination, File readImage){
    char clientBuf;
    char temp[100];
    int clientCount = 0;
    long pos=0; int currentPos=0;
    readImage.seek(location);
    if(location != 0)
    gsm.SimpleWrite("\n");
        while(readImage.available()>0){
          clientBuf = readImage.read();
          pos = readImage.position();
          //Serial.println(pos);
        if(pos<=destination)
        {
          
          if(clientBuf != '\n' && pos==destination){
            String ss;
            for(int i=0;i<clientCount;i++)
              ss+=temp[i];
            gsm.SimpleWriteln(ss);
            //Serial.print(".");
            ss="";
            
            clientCount=0;            
          }
          else if(clientBuf != '\n'){
            temp[clientCount] = clientBuf;
            clientCount++;
            currentPos = readImage.position();
           // Serial.println(currentPos);
          }
          else{
            String ss;
            for(int i=0;i<clientCount;i++){
              //gsm.SimpleWrite(temp[i]);
              ss+=temp[i];
              //Serial.print(temp[i]);
            }
            gsm.SimpleWriteln(ss);
            Serial.print(".");
            ss="";
            
            clientCount=0;  
          }
         }
          
        }       
return 1;
}
int sapbrFunction(){
      gsm.SimpleWriteln("AT+SAPBR=3,1,Contype,GPRS");
      gsm.WaitResp(2000, 100, "OK");
      if(!gsm.IsStringReceived("OK"))
          return 0;

      gsm.SimpleWriteln("AT+SAPBR=3,1,APN,"+apnName);
      gsm.WaitResp(2000, 100, "OK");
      if(!gsm.IsStringReceived("OK"))
          return 0;

      gsm.SimpleWriteln("AT+SAPBR=1,1");
      gsm.WaitResp(2000, 100, "OK");
 
      return 1;
}
int sendImage(String filename, String imageName){
      digitalWrite(uploadProcess, LOW);
      char aa[apnName.length()+1];
      apnName.toCharArray(aa, apnName.length()+1);
    
    if(sapbrFunction() == 1){
    Serial.println("gprs activated");
    gprsDisconnect = false;
      if (net.attachGPRS(aa, "", "") == 1){
        Serial.println("apn configured");

        gsm.SimpleWriteln("AT+SMTPSRV="+smtp_server+","+smtp_port);
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("smtp server timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("smtp server no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("smtp server configured");
          break;
        }

        gsm.SimpleWriteln("AT+SMTPAUTH=1,"+smtp_username+","+smtp_password);
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("smtp auth timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("smtp auth no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("smtp auth configured");
          break;
        }

        gsm.SimpleWriteln("AT+SMTPFROM="+email_device+",yogo");
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("address device timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("address device no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("address device configured");
          break;
        }

        gsm.SimpleWriteln("AT+SMTPRCPT=0,0,"+email_server+",fajar");
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("address server timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("address server no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("address server configured");
          break;
        }
        gsm.SimpleWriteln("AT+SMTPSUB=report " +device_id);
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("subject timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("subject no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("subject configured");
          break;
        }
        gsm.SimpleWriteln("AT+SMTPBODY=32");
        switch (gsm.WaitResp(500, 100, "DOWNLOAD")) {
        case RX_TMOUT_ERR:
          Serial.println("body mail length timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("body mail length no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("body mail length configured");
          break;
        }
        gsm.SimpleWriteln("test report " +timeStamp);
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("body mail timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("body mail no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("body mail configured");
          break;
        }
        gsm.SimpleWriteln("AT+SMTPFILE=2," +imageName +",1");
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("attachment timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("attachment no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("attachment configured");
          break;
        }
        
        gsm.SimpleWriteln("AT+SMTPSEND");
        switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          Serial.println("smtp send timeout");
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          Serial.println("smtp send no response");
          return 0;
          break;
        case RX_FINISHED_STR_RECV:
          Serial.println("smtp send configured");
          break;
        }
        // gsm.WaitResp(3000, 100, "+SMTPFT: 1,1360");
        // delay(500);
        // gsm.WaitResp(5000, 100);
        switch (gsm.WaitResp(3000, 100, "+SMTPFT: 1,1360")){
          case RX_TMOUT_ERR:
          Serial.println("prepared timeout");
          return 0;
          break;
          case RX_FINISHED_STR_NOT_RECV:
          Serial.println("prepared no response");
          return 0;
          break;
          case RX_FINISHED_STR_RECV:
          Serial.println("prepare to send..");
          break;
        }
        // delay(500);
        // switch (gsm.WaitResp(5000, 100)){
        // case RX_FINISHED:
        //   Serial.println("start send email");
        //   break;
        // }
        
       

        File readImage = SD.open(filename, FILE_READ);
        int32_t time = millis();
        
        for(int i=0;i<line;i++){
          long awalData = i * 1326;
          long akhirData = (i+1) * 1326;
          int dataLength = 1326+15;
          if(i==(line-1)){
            akhirData = totalChara;
            dataLength = (totalChara - (awalData)) + (totalLine - (i*17));
            endOfData = true;
          }
          transferData(awalData, dataLength, akhirData, readImage);
        }

        time = millis() - time;
        time = time/1000;
        Serial.println("done!");
        Serial.print(time); Serial.println(" s uploaded");
        readImage.close();
        digitalWrite(uploadProcess, HIGH);
        return 1;
      }
      // else{
      //   Serial.println("apn not configured");
      //   digitalWrite(uploadProcess, HIGH);
      //   return 0;
      // }
    } 
}
int transferData(long location,int dataLength, long destination, File readImage){
  sprintf(aux_string,"AT+SMTPFT=%d", dataLength);
  gsm.SimpleWriteln(aux_string);
  sprintf(aux_string,"+SMTPFT: 2,%d", dataLength);
  gsm.WaitResp(500, 100, aux_string);
  readFile(location,destination, readImage);
  gsm.WaitResp(500, 100, "OK");
  switch (gsm.WaitResp(500, 100, "+SMTPFT: 1,1360")){
    case RX_FINISHED_STR_RECV:
          Serial.print(".");
          break;
  }
  if(endOfData){
     gsm.SimpleWriteln("AT+SMTPFT=0");
     gsm.WaitResp(500, 100, "OK");
     endOfData = false;
  }
}

char position;
char phone_number[20]; // array for the phone number string
char sms_text[100];
void checkNewMessage(){
  position = sms.IsSMSPresent(SMS_UNREAD);
    if (position) {
      sms.GetSMS(position, phone_number, sms_text, 100);
      Serial.println("New sms in storge location: " +String(position));
      Serial.println("phone number: " +String(phone_number));
      Serial.println("message: " +String(sms_text));

      char buff_v0[sms_server.length()+1];
      sms_server.toCharArray(buff_v0, sms_server.length()+1);
      char buff_v1[sms_sv1.length()+1];
      sms_sv1.toCharArray(buff_v1, sms_sv1.length()+1);
      char buff_v2[sms_sv2.length()+1];
      sms_sv2.toCharArray(buff_v2, sms_sv2.length()+1);
      char buff_v3[sms_sv3.length()+1];
      sms_sv3.toCharArray(buff_v3, sms_sv3.length()+1);

      eventStatus = "R";

      //Serial.println("buff num: " +String(buff_v1));
      if(String(phone_number) == String(buff_v0)){
        if(String(sms_text) == "status device"){
          String inputDigitalState = "111111111111111";
          String newMessage = "$ " +device_id +" " +eventStatus +" " +String(doorState) +inputDigitalState +" " +currentTemp 
                              +" " +currentRH +" " +timeStamp +"#";
          char buffMessage[newMessage.length()+1];
          newMessage.toCharArray(buffMessage, newMessage.length()+1);
          if(sendNewMessage(buff_v0, buffMessage) == 1){
            Serial.println("new message sended to: " +String(buff_v0));
          }
        }
      }

      else if(String(phone_number) == String(buff_v1)){
        if(String(sms_text) == "status device"){
          String inputDigitalState = "111111111111111";
          String newMessage = "$ " +device_id +" " +eventStatus +" " +String(doorState) +inputDigitalState +" " +currentTemp 
                              +" " +currentRH +" " +timeStamp +"#";
          char buffMessage[newMessage.length()+1];
          newMessage.toCharArray(buffMessage, newMessage.length()+1);
          if(sendNewMessage(buff_v1, buffMessage) == 1){
            Serial.println("new message sended to: " +String(buff_v1));
          }
        }
      }
      else if(String(phone_number) == String(buff_v2)){
        if(String(sms_text) == "status device"){
          String inputDigitalState = "111111111111111";
          String newMessage = "$ " +device_id +" " +eventStatus +" " +String(doorState) +inputDigitalState +" " +currentTemp 
                              +" " +currentRH +" " +timeStamp +"#";
          char buffMessage[newMessage.length()+1];
          newMessage.toCharArray(buffMessage, newMessage.length()+1);
          if(sendNewMessage(buff_v2, buffMessage) == 1){
            Serial.println("new message sended to: " +String(buff_v2));
          } 
        }
      }
      else if(String(phone_number) == String(buff_v3)){
        if(String(sms_text) == "status device"){
          String inputDigitalState = "111111111111111";
          String newMessage = "$ " +device_id +" " +eventStatus +" " +String(doorState) +inputDigitalState +" " +currentTemp 
                              +" " +currentRH +" " +timeStamp +"#";
          char buffMessage[newMessage.length()+1];
          newMessage.toCharArray(buffMessage, newMessage.length()+1);
          if(sendNewMessage(buff_v3, buffMessage) == 1){
            Serial.println("new message sended to: " +String(buff_v3));
          }
        }
      }

    }
}

int sendNewMessage(char *number, char *message){
  // char buff[number.length()];
  // char buffMessage[message.length()];
  // number.toCharArray(buff, number.length());
  // message.toCharArray(buffMessage, message.length());
  sms.SendSMS(number, message);
  return 1;
}
int state = 0;

void checkGPRSConnection(){
  if(gsm.IsStringReceived("+PDP: DEACT")){
    gprsDisconnect = true;
    digitalWrite(GSM_RESET, LOW);
    delay(105);
    digitalWrite(GSM_RESET, HIGH);

    if(sapbrFunction() == 1){
      Serial.println("gprs activated");
      gprsDisconnect = false;
    }
  }  
  if(!gprsDisconnect){
    digitalWrite(gprsReport, LOW);
  }
  else if(gprsDisconnect){
    digitalWrite(gprsReport, HIGH); 
  }
}

void checkSignal(){
  gsm.SimpleWriteln("AT+CSQ");
   switch (gsm.WaitResp(500, 100, "OK")) {
        case RX_TMOUT_ERR:
          break;
        case RX_FINISHED_STR_NOT_RECV:
          break;
        case RX_FINISHED_STR_RECV:
          String aa = (char*)gsm.comm_buf;
          int firstQuotation = aa.indexOf(':');
          firstQuotation += 1;
          int secondQuotation = aa.indexOf(',');
          char buff[aa.length()];
          aa.toCharArray(buff,aa.length());
          String message="";
          for(int i=firstQuotation+1;i<secondQuotation;i++){
            message += buff[i];
          }
          Serial.println("signal: " +message);
          boolean inRange = false;
          int ab = message.toInt();
          for(int m=2; m<=30; m++){
            
            if(ab == m){
              inRange = true;
            }
          }
          if(inRange){
            Serial.println("Signal is in range");
          }
          else if(!inRange){
            Serial.println("Signal is not in range");
          }
          break;
    }
}
void sendAlertMessage(){
  if(readClockData() == 1){
    //send New message to sms server
    eventStatus = "A";
    char buff_v0[sms_server.length()+1];
    sms_server.toCharArray(buff_v0, sms_server.length()+1);
    String inputDigitalState = "111111111111111";
    String newMessage = "$ " +device_id +" " +eventStatus +" " +String(doorState) +inputDigitalState +" " +currentTemp 
                             +" " +currentRH +" " +timeStamp +"#";
    char buffMessage[newMessage.length()+1];
    newMessage.toCharArray(buffMessage, newMessage.length()+1);
    digitalWrite(smsProcess, LOW);
    ulang:
     if(sendNewMessage(buff_v0, buffMessage) == 1){
        Serial.println("new message sended to: " +String(buff_v0));
        tempRHReport = false;
        digitalWrite(smsProcess, HIGH);
     }
     else if(sendNewMessage(buff_v0, buffMessage) == 0){
        goto ulang;
     }
  }  
}

void checkTresholdTempRH(){
  char buffTempUp[tresUpTemp.length()+1];
  tresUpTemp.toCharArray(buffTempUp, tresUpTemp.length()+1);

  char buffTempDown[tresDownTemp.length()+1];
  tresDownTemp.toCharArray(buffTempDown, tresDownTemp.length()+1);

  char buffRHUp[tresUpRH.length()+1];
  tresUpRH.toCharArray(buffRHUp, tresUpRH.length()+1);

  char buffRHDown[tresDownRH.length()+1];
  tresDownRH.toCharArray(buffRHDown, tresDownRH.length()+1);

  if(currentTemp > buffTempUp || currentTemp < buffTempDown || currentRH > buffRHUp || currentRH > buffRHDown){
    timerAlert.run();
  }
  else{
    timerAlert.setTimeout(1000, sendAlertMessage);
  }

}
void loop() {
  
  //send data sensor temp&rh to sms server periodic
  timer.run();
  
  //just check sensor temp &rh every 10 s
  timerRoutine.run();
  
  
  
  //reconnect gprs if disconnect
  if(millis()-prevMillis >= 60000){
    prevMillis = millis();
    checkGPRSConnection();
    checkSignal();
    //check new message and handle sms request
    checkNewMessage();
  }

    serialhwread();
    serialswread();

  // state = digitalRead(doorSwitch);
  // if(state == HIGH){
  if(doorStateChange){
    doorState = digitalRead(doorSwitch);
    doorStateChange = false;
    cameraEncodeUploadProses = true;
    digitalWrite(doorReport, LOW);
    Serial.println("Door Opened");
    Serial.println("Take picture 3 times");
    boolean takePicture = false;
    
    for(int m=0;m<3;m++){
      if(snapPicture(m+1)==1){
        Serial.println("Picture 0" +String(m+1) +" Taken");
        takePicture = true;
      }
      else{
        Serial.println("Picture 0" +String(m+1) +" not Taken");
        takePicture = false;
      }
    }
    if(takePicture){
      for(int m=0;m<3;m++){
        switch (m) {
            case 0:
              if(encodeJPGToB64(pictureOneName, b64OneName) == 0)
              m=3;
              break;
            case 1:
              if(encodeJPGToB64(pictureTwoName, b64TwoName) == 0)
              m=3;
              break;
            case 2:
              if(encodeJPGToB64(pictureThreeName, b64ThreeName) == 0)
              m=3;
              break;
        }
      }
    }
    cameraEncodeUploadProses = true;
  }
  else if(!doorStateChange){
    digitalWrite(doorReport, HIGH);
  }
}

void serialhwread()
{
  int i = 0;
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      inSerial[i] = (Serial.read());
      delay(10);
      i++;
    }

    inSerial[i] = '\0';
    if (!strcmp(inSerial, "/END")) {
      Serial.println("_");
      inSerial[0] = 0x1a;
      inSerial[1] = '\0';
      gsm.SimpleWriteln(inSerial);
    }
    //Send a saved AT command using serial port.
    if (!strcmp(inSerial, "//")) {
      Serial.println("_");
      inSerial[0] = 0x1a;
      inSerial[1] = '\0';
      gsm.SimpleWriteln(inSerial);
      net.disconnectTCP();
    }
    else {
      Serial.println(inSerial);
      gsm.SimpleWriteln(inSerial);
    }
    inSerial[0] = '\0';
  }
}

void serialswread()
{
  gsm.SimpleRead();
}

String me = "";
long mm=0;
long chara=0;
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void encodeblock(unsigned char in[3],unsigned char out[4],int len) {
 out[0]=cb64[in[0]>>2]; out[1]=cb64[((in[0]&0x03)<<4)|((in[1]&0xF0)>>4)];
 out[2]=(unsigned char) (len>1 ? cb64[((in[1]&0x0F)<<2)|((in[2]&0xC0)>>6)] : '=');
 out[3]=(unsigned char) (len>2 ? cb64[in[2]&0x3F] : '=');
}
int encodeJPGToB64(String jpgFile, String b64File) {
 unsigned char in[3],out[4]; int i,len,blocksout=0;
 me = ""; mm=0; chara=0; 
 File picture = SD.open(jpgFile, FILE_READ);
 if(SD.exists(b64File)){
    SD.remove(b64File);
 }
 File encodeFile = SD.open(b64File, FILE_WRITE);

 digitalWrite(encodeProcess, LOW);
 int32_t time = millis();
 Serial.println("start encoding file " +jpgFile +" to " +b64File);
 while (picture.available()!=0) {
   len=0; 
   for (i=0;i<3;i++) {
    in[i]=(unsigned char) 
    picture.read(); 
    if (picture.available()!=0)
    len++; else in[i]=0; 
   }
   if (len) {
    encodeblock(in, out, len); 
    for (i = 0; i < 4; i++){ 
      chara++;
      encodeFile.write(out[i]); 
      //Serial.write(out[i]);
    } 
    blocksout++; 
   }
   if (blocksout >= 19 || picture.available() == 0) { 
    if (blocksout){ 
      mm++; chara+=2;
      encodeFile.print("\r\n"); 
      //Serial.print("\r\n");
      Serial.print(".");
    } 
    blocksout = 0;
   }
   //Serial.print(".");
 }
 Serial.println("encode done.");
 picture.close();
 encodeFile.close();

  time = millis() - time;
  time = time/1000;
  Serial.println("done!");
  Serial.print(time); Serial.println(" s encoded");
  Serial.println(String(mm)+" line");
  Serial.println(String(chara)+" character");

  totalLine=mm;
  totalChara=chara;
  if(totalLine % 17 == 0)
    line = totalLine / 17;
  if(totalLine % 17 != 0){
    line = totalLine / 17;
    line +=1;
  }
  Serial.println("Total Line devide:" +String(line));
  digitalWrite(encodeProcess, HIGH);
  if(sendImage(b64File, jpgFile)==0){
    return 0;
  }  
  else{
    return 1;
  }
}
