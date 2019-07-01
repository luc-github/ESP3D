#include "DHTesp.h"

DHTesp dht;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");

  dht.setup(22); // Connect DHT sensor to GPIO 17
}

void loop()
{  
    
    //delay(dht.getMinimumSamplingPeriod()*1.2);
      static uint32_t last_dht_update= 0;
     uint32_t now_dht = millis();
     if (now_dht - last_dht_update > dht.getMinimumSamplingPeriod() *1.2) {
             last_dht_update = now_dht;
              float humidity = dht.getHumidity();
              float temperature = dht.getTemperature();

              Serial.print(dht.getStatusString());
              Serial.print("\t H:");
              Serial.print(humidity, 1);
              Serial.print("%\t\T :");
              Serial.print(temperature, 1);
              Serial.println("C\t\t");
              //Serial.print(dht.toFahrenheit(temperature), 1);
              //Serial.print("\t\t");
              //Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
              //Serial.print("\t\t");
              //Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
          }
}

