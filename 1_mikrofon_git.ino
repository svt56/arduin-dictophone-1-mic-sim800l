/*
   Hardware Pinout Connection
   Arduino Nano        SD Pin
        5v ------------ VCC krasniy
        GND ----------- GND 4erniy
        D10 ----------- CS   jeltiy
        D11 ----------- MOSI  zeleniy
        D12 ----------- MISO  4stiy
        D13 ----------- SCK beliy
  ________________________________________
   Arduino Nano         MAX9814
        3.3v ----------- VDD krasniy
        GND ------------ GND ekran
        A0 -------------  Out beliy 
        AR  ------------ GND
        GAIN ------------ 
  ______________________________
    Микрофон 1
    A1 -------------  Out__________
   Arduino Nano D2 pin user for Led to notify that record is in process.
   _____________________________________________
   config file
   8000 bitrate
   380;  //пороговое значение
   1500;            //продолжительнось аудио дорожки присрабатывании в 100 милисекундах, т.е.  при 100 единицах продолжительсность записи 10 секунд, около 78 кБайт
   _______________________________________________________________________
   SIM800L RED - Arduino
   VCC - 5V
   GND - GND
   TX - D2
   RX - D3 через делитель напряжения 2 резистора по 10 кОм, две ножки спаяанные идут на RX sim 800, один резистор идет на GND второй на D3
*/
/*
   use Link: https://www.arduino.cc/reference/en/libraries/tmrpcm/ TMRpcm library for recording audio using MAX9814
   Recording a WAV file to an SD card is an advanced feature of the TMRpcm library so you must edit the library configuration file in order to use it.
   It simply searches the file "pcmConfig.h" using File Explorer and disables a few lines of code (then saves it).
    1. On Uno or non-mega boards uncomment the line #define buffSize 128
    2. Also uncomment #define ENABLE_RECORDING and #define BLOCK_COUNT 10000UL
*/
#include <TMRpcm.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
File logfile;
SoftwareSerial sim800(2, 3); // RX, TX
TMRpcm audio;
int file_number = EEPROM.read(0);
char filePrefixname1[50] = "1mik";
char exten[10] = ".wav";
String rr;
const int mic_pin1 = A6;            //пин к которому подключен микрофон 1
//const int sample_rate = 8000;       //16000
int sample_rate;
//const int MaxAnalogPinValue = 380;  //пороговое значение
int MaxAnalogPinValue;
//const int log_wav = 35000;            //продолжительнось аудио дорожки присрабатывании
int log_wav;
#define SD_CSPin 10
int config[3];
void setup() {

  Serial.begin(9600);
  //задаем режимы работы используемых контактов
  sim800.begin(9600);  // Скорость обмена данными с GSM модулем
  delay(500);
  pinMode(mic_pin1, INPUT);
  if (!SD.begin(SD_CSPin)) {
    Serial.println("An Error has occurred while mounting SD");
  }
  while (!SD.begin(SD_CSPin)) {
    Serial.print(".");
    delay(500);
  }
  audio.CSPin = SD_CSPin ;
  configSD();
  sample_rate = config[0];
  MaxAnalogPinValue = config[1];
  log_wav = config[2];
  //SendSMS(String(MaxAnalogPinValue), " " + String(file_number));

}
void loop() {
  //формируем имя для микрофофна
  char fileSlNum[20] = "";
  itoa(file_number, fileSlNum, 10);
  char file_name1[70] = "";
  strcat(file_name1, filePrefixname1);
  strcat(file_name1, fileSlNum);
  strcat(file_name1, exten);
  //Serial.println(file_name1);
  int AnR1 = analogRead(mic_pin1);
  Serial.println(AnR1);
  if (AnR1 >= MaxAnalogPinValue) {
    while (!SD.begin(SD_CSPin)) {
    Serial.print(".");
    delay(500);
  }
    Serial.println(file_name1);
    SendSMS("start ", file_name1);
    logfile = SD.open("log.csv", FILE_WRITE);
    logfile.println(String(AnR1) + ";" + String(millis()));
    logfile.close();
    audio.startRecording(file_name1, sample_rate, mic_pin1);
    wait_min(log_wav);
    audio.stopRecording(file_name1);
    SendSMS("stop ", file_name1);
    Serial.println("stopRecording");
    file_number++;
    if (file_number > 32766) file_number = 0;
    EEPROM.write(0, file_number);
  }
}
void configSD() { // функции чтения конфигурационного файла с SD
File file = SD.open("config", FILE_READ);
  if (file) {
    int line_count = 0;
    while (file.available()) {
      String line = file.readStringUntil('\n');  // \n character is discarded from buffer
            config[line_count] = line.toInt();
            line_count++;
    }
    file.close();
    } else {
    Serial.print(F("SD Card: error on opening file"));
  }
}
void SendSMS(String textt, String name)
{
  Serial.println("Sending SMS... " + textt + name);                // Печать текста
  sim800.print("AT+CMGF=1\r");                   // Выбирает формат SMS
  delay(100);
  //String nuber = "+792100000000";
  //String nuber1 = "AT+CMGS=\" + number + ""\"\r";
  sim800.print("AT+CMGS=\"+792100000000\"\r");  // Отправка СМС на указанный номер +792100000000"
  //Serial.println(nuber1);
  delay(300);
  sim800.print(textt + name);       // Тест сообщения
  delay(300);
  sim800.print((char)26);// (требуется в соответствии с таблицей данных)
  delay(300);
  sim800.println();
  Serial.println("Sendt" + textt + " " + name);
}
void wait_min(int mins) {
  int count = 0;
  int secs = mins * 1;
  while (1) {
    Serial.print('.');
            delay(100);
            count++;
            if (count == secs) {
            count = 0;
            break;
            }
  }
  Serial.println();
  return ;
}
