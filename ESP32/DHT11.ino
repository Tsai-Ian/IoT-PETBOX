#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // 設定DHT的腳位、類型


void setup_DHT11() 
{
  dht.begin(); // dht感測器初始化
}

void loop_DHT11() 
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  dtostrf(temperature, 4, 1, tempString); // dtostrf將變數改成文字:變數,寬度,小數位數,字串名稱
//  Serial.print("Temperature: ");
//  Serial.println(temperature);

  
  dtostrf(humidity, 2, 0, humString);
//  Serial.print("Humidity: ");
//  Serial.println(humidity);

}
