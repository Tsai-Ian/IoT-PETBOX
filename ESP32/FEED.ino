#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP; // Define NTP Client to get time
NTPClient timeClient(ntpUDP);

void feedStatus();

String formattedDate; // Variables to save date and time
String dayStamp;
String timeStamp;

int FeedTimes = 2;

void setup_FEED() 
{
  pinMode(feedPin, OUTPUT);
  digitalWrite(feedPin, HIGH); 
  timeClient.begin(); // 初始化NTPClient獲得時間
  timeClient.setTimeOffset(28800); // 時區的時差+8 = 28800秒
}
void loop_FEED() 
{
  while(!timeClient.update()) // 迴圈:更新時間
  {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format: 2018-05-28T16:00:13Z(印出的格式)
  formattedDate = timeClient.getFormattedDate(); // 取出時間日期
  //Serial.println(formattedDate); // 印出時間
  int splitT = formattedDate.indexOf("T");
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1); // 取出現在時間
  Serial.print("Current time: ");
  Serial.println(timeStamp);
  //hour:min:sec = h1:m1:s1
  Serial.print("feed time: ");

  
  if (timeStamp == FeedTime1 or timeStamp == FeedTime2 or timeStamp == FeedTime3) 
  {
    if (intFeedLevel > 0){
      feed();
      feedStatus();
    }
    else
    {
      Serial.print("ALERT NO NO FEED");
    }
  }
  else {
    digitalWrite(feedPin,HIGH);
  }
   feedLevel = String(intFeedLevel);
   Serial.println(feedLevel);
}

void feed(){
    digitalWrite(feedPin,LOW);
    delay(600);
    digitalWrite(feedPin,HIGH);
    intFeedLevel -= 1;
}
void feedStatus(){
   // Return feed status
   // Return last time of feeding
   feedString = timeStamp;
   Serial.print(timeStamp);   
}

void setFeedTime(String setFeedTime, String Timevalue){
  if (setFeedTime == "FeedTime1")
    FeedTime1 = Timevalue;
  if (setFeedTime == "FeedTime2")
    FeedTime2 = Timevalue;
  if (setFeedTime == "FeedTime3")
    FeedTime3 = Timevalue;
  
  Serial.print(setFeedTime);
  Serial.print("Have been set to ");
  Serial.print(Timevalue);
  Serial.println();
}

void resetFeedLevel(){
  intFeedLevel = 100;
  Serial.println("INT FEEDLEVEL");
  Serial.print(intFeedLevel);
}
