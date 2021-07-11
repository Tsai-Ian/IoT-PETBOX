int IntAlert = 0;
float AlertFrequency = 1; // 處於危險狀態時，每一小時提醒一次(半小時改0.5)
float Alerttime = 0;
void AlertReminder() // 警告3次後要等滿8小時才會再發送(從第一次警告開始計時)
{
  if (temperature > 28 or temperature < 20 or humidity > 75 or intFeedLevel < 10 or waterlevelper < 15)
  // (distance < 15%)
  {
    if (IntAlert < 3) // 警告次數3次內 發出警告(led代替發信，腳位pin2)
    {
      loop_Line();
      loop_sendmail();
      //digitalWrite(ledPin, HIGH);
      //delay(200);
      //digitalWrite(ledPin, LOW);
      IntAlert += 1;
      Alerttime += AlertFrequency;
    }
    else if (Alerttime >= 8)  // 等滿8小時再次發送+等待時間歸零
    {
      loop_Line();
      loop_sendmail();
      //digitalWrite(ledPin, HIGH);
      //delay(200);
      //digitalWrite(ledPin, LOW);
      IntAlert = 1;
      Alerttime = AlertFrequency;
    }
    else // 警告次數大於3次+距離首次警告時間未滿8小時
    {
      Alerttime += AlertFrequency;
    }
  }
  
  Serial.println("-----");
  Serial.println(IntAlert);
  Serial.println(Alerttime);
  Serial.println("-----");
}
