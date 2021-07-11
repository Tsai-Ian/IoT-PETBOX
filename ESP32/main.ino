#include <WiFi.h> // 連WiFi用
#include <PubSubClient.h> // mqtt用
#include <Wire.h> // i2c 和 spi 通讯用
#include <ArduinoJson.h> // json格式用

char tempString[10]="";
char humString[10]="";
char distanceString[10]="";
String bulbString="";
String BulbStart = "17:18";
String BulbStop = "17:18";
String feedString = "00:00";
String feedLevel = "0";
String jsonmsg;
char stmsg[50];
const char* SR04_topic = "petbox001/SR04";
String FeedTime1 = "12:13";
String FeedTime2 = "18:18";
String FeedTime3 = "17:18";
String taddsec = ":00"; // 設定時間新增秒數:00
float duration, distance;
const int ledPin = 27;
const int feedPin = 25;
float temperature = 0;
float humidity = 0;
int intFeedLevel = 0; 
float waterlevelper = 0;
StaticJsonDocument<500> json_doc; // 向系統要一個「json變數」的空間存放json，靜態庫
char json_output[500];

const char* ssid = "YOUR_SSID";//輸入你的wifi帳號
const char* password = "PASSWORD";//輸入你的wifi密碼

const char* mqtt_server = "YOUR_MQTT BORKER HOST";
const char* mqtt_username = ""; // MQTT username
const char* mqtt_password = ""; // MQTT password
const char* clientID = "petbox001"; // MQTT client ID


//訂閱的主題
const char* callback_topic = "petbox001/switch";


long lastMsg = 0;
long timerA = 0;
long timerB = 0;
long timerR1 = 0;
char msg[50];

WiFiClient espClient; // 初始化 espClient
PubSubClient client(espClient);

// 原型宣告
void setup_DHT11();
void setup_SR04();
void setup_FEED();
void loop_DHT11();
void loop_LED();
void loop_SR04();
void loop_FEED();
void loop_sendmail();
void loop_Line();
void switch_bulb(String bulb_state);
void bulbStatus();
void msgActive();
void handleControll(String device, String state);
void reconnect_Line();
void feedStatus();
void feed();
void setFeedTime(String setFeedTime, String Timevalue);
void setBulbTime(String setBulb, String Timevalue);
void resetFeedLevel();

// Handle MQTT_sub msg
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  jsonmsg = messageTemp; // 將獲得的暫存訊息儲存到jsonmsg，以便之後轉換json格式
  Serial.println();
  
  msgActive();
  client.publish("petbox/status", json_output);
  
  Serial.println();
}
void msgActive() // 印出訂閱
{
  DynamicJsonDocument doc(1024); // 向系統要一個「json變數」的空間存放json，動態庫
  deserializeJson(doc, jsonmsg); // 將json反序列化為物件
  JsonObject object = doc.as<JsonObject>(); // extract the data

  String controllDevice = object["switch"]; // Get sensor type value
  String changeStatus = object["status"];
  
  handleControll(controllDevice, changeStatus);
  
    
  Serial.print("controllDevice: ");
  Serial.println(controllDevice);
  Serial.print("changeStatus: ");
  Serial.println(changeStatus);
  Serial.println();
}

void handleControll(String device, String value){
  if(device == "bulb"){
    switch_bulb(value);
    bulbStatus();
  }
  if(device == "feed"){
    feed();
    feedStatus();   
  }
  if(device == "setFeedTime1"){
    setFeedTime("FeedTime1" , value); 
    FeedTime1 += taddsec;      
  }
  if (device == "setFeedTime2"){
    setFeedTime("FeedTime2" , value);
    FeedTime2 += taddsec;    
  }
  if (device == "setFeedTime3"){
    setFeedTime("FeedTime3" , value);
    FeedTime3 += taddsec;     
  }
  if (device == "setBulbStart"){
    setBulbTime("BulbStart" , value); 
    BulbStart += taddsec;     
  }
  if (device == "setBulbStop"){
    setBulbTime("BulbStop" , value);  
    BulbStop += taddsec;      
  }
  if (device == "resetFeedLevel"){
    resetFeedLevel();
  }
}

void reconnect() 
{
  while (!client.connected()) // 持續迴圈直到連線
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientID, mqtt_username, mqtt_password)) 
    {
      Serial.println("connected");// 連線成功
      client.subscribe(callback_topic);// 訂閱一個主題，可以設定多個主題
    } 
    else 
    {
      Serial.print("failed, rc="); // 印出連線錯誤代碼
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // 初始化連接WiFi
  while (WiFi.status() != WL_CONNECTED) //WiFi準備連線中，連續印出"."表示
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());// 印出已連線的WiFi名稱和WiFiIP

  client.setServer(mqtt_server, 1883);// 設定 mqtt server 及連接 port
  client.setCallback(callback); // 設定 mqtt broker 並設定 callback function
  setup_DHT11();
  setup_SR04();
  setup_FEED();
  pinMode(ledPin, OUTPUT);
}

void loop() 
{
  if (!client.connected()) // 設定自動斷線重連
  {
    reconnect();
  }
  if(!client.loop()) // 設定重連時的ID、帳號密碼驗證
  {
    client.connect(clientID, mqtt_username, mqtt_password);
  }
  //設定延遲
  long now = millis(); // 設定loop迴圈延遲時間
      if (now - timerB > 1000) {
        timerB = now; 
        Serial.println("Sensor Reading Vaule");
        loop_FEED();
        loop_LED();
        loop_SR04();  
        loop_DHT11();
        Serial.println();
      
        json_doc["deviceId"] = "petbox001"; // 傳送json資料格式
        json_doc["temperature"] = tempString;
        json_doc["humidity"] = humString;
        json_doc["waterlevel"] = distanceString;
        json_doc["feedlevel"] = feedLevel;
        json_doc["bulb"] = bulbString;
        json_doc["feed"] = feedString;
        json_doc["FeedTime1"] = FeedTime1;
        json_doc["FeedTime2"] = FeedTime2;
        json_doc["FeedTime3"] = FeedTime3;
        json_doc["bulbTime1"] = BulbStart;
        json_doc["bulbTime2"] = BulbStop;
        
        serializeJson(json_doc, json_output); // 將物件與Dataset序列化為json
        client.publish("petbox001/status", json_output);
      }

  
    // 每 15 分鐘 寫入數據庫
    if (now - timerA > 90000) {
          timerA = now; 
          
          client.publish("petbox/status", json_output);
    }
  
    if (now - timerR1 > 1800000) // 每半小時檢測一次1000*60*30=1800000
    {
      timerR1 = now;
      AlertReminder();
    }
}
