

void loop_LED() {
  // 讀取LED狀態，以便之後方送訊息
  bulbStatus();
  
  if (timeStamp == BulbStart)
  {
    digitalWrite(ledPin, HIGH);
  }
  if (timeStamp == BulbStop)
  {
    digitalWrite(ledPin, LOW);
  }
}

void switch_bulb(String bulb_state) 
{
  if (digitalRead(ledPin) == HIGH)
  {
    digitalWrite(ledPin, LOW);
  }
  else if (digitalRead(ledPin) == LOW)
  {
    digitalWrite(ledPin, HIGH);
  }
}
void bulbStatus() {
    // 讀取LED狀態，以便之後方送訊息
  if (digitalRead(ledPin) == HIGH)
  {
    bulbString = "on";
  }
  else if (digitalRead(ledPin) == LOW)
  {
    bulbString = "off";
  }
}
void setBulbTime(String setBulb, String Timevalue){
  if (setBulb == "BulbStart")
    BulbStart = Timevalue;
  if (setBulb == "BulbStop")
    BulbStop = Timevalue;
 
  
  Serial.println(setBulb);
  Serial.print("Have been set to ");
  Serial.print(Timevalue);
  Serial.println();
}
