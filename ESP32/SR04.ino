const int trigPin = 33;
const int echoPin = 32;
// defines variables



void setup_SR04() 
{
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop_SR04() 
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
 
  duration = pulseIn(echoPin, HIGH);
  
  if (isnan(duration)) {
      Serial.println("Failed to read from SR04 sensor!");
      return;
  }
  // Calculating the distance
  distance= (duration/2)/29.1;
  waterlevelper = (792 - (distance * 100)) / 5.5;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  

  // dtostrf 將 float 數字改成文字
  dtostrf(waterlevelper, 2, 0, distanceString);

}
