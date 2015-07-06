void setup()
{
  Serial.begin(115200);  // The Bluetooth Mate defaults to 115200bps
}

void loop()
{
//  if(bluetooth.available())  // If the bluetooth sent any characters
//  {
//    // Send any characters the bluetooth prints to the serial monitor
//    Serial.print((char)bluetooth.read());  
//  }
//  if(Serial.available())  // If stuff was typed in the serial monitor
//  {
    // Send any characters the Serial monitor prints to the bluetooth
    Serial.print("hello world! \n");
    delay(1000);
  //}
  // and loop forever and ever!
}

