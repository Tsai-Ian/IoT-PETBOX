#include <WiFiClientSecure.h>
String Linetoken = "YOUR LINE TOKEN"; // LINE權杖
const char* host = "notify-api.line.me"; //LINE Notify API網址

WiFiClientSecure esp32Client;

void reconnect_Line()
{
   while (!esp32Client.connect(host, 443)) // 持續迴圈直到連線
  {
    Serial.println("Attempting Line connection...");
    delay(5000);
  }
}

void loop_Line()
{
  esp32Client.setInsecure();
  
  String message = "寵物箱發生異常，目前狀態："; // 傳送LINE的Line訊息內容
  message += "\n溫度=" + String((temperature)) + " °C ";
  message += "\n濕度=" + String((humidity)) + " H ";
  message += "\n剩餘飼料量=" + String((intFeedLevel)) + " 次 ";
  message += "\n水位=" + String((waterlevelper)) + " H ";

  Serial.println(message);
  if (!esp32Client.connect(host, 443))
  {
   reconnect_Line();
  }
  int LEN = message.length();
  
  String url = "/api/notify";// 傳遞POST表頭
  esp32Client.println("POST " + url + " HTTP/1.1");
  esp32Client.print("Host: ");
  esp32Client.println(host);
  esp32Client.print("Authorization: Bearer "); 
  esp32Client.println(Linetoken);
  esp32Client.println("Content-Type: application/x-www-form-urlencoded");
  esp32Client.print("Content-Length: "); 
  esp32Client.println( String((LEN + 8)) );
  esp32Client.println();      
  esp32Client.print("message=");
  esp32Client.println(message);
  esp32Client.println();
  String response = esp32Client.readString();
  Serial.println(response); // 顯示傳遞結果
  esp32Client.stop(); // 斷線
  Serial.println("Line_msgsent");
  
   }
