//void getNTP() { /// Working alone but not inside this program
//  WiFi.hostByName(ntpServerName, timeServerIP); 
//  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
//  // wait to see if a reply is available
//  delay(1000);
//  
//  int cb = udp.parsePacket();
//  if (!cb) {
//    Serial.println("no packet yet");
//  }
//  else {
//    Serial.print("packet received, length=");
//    Serial.println(cb);
//    // We've received a packet, read the data from it
//    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
//
//    //the timestamp starts at byte 40 of the received packet and is four bytes,
//    // or two words, long. First, esxtract the two words:
//    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
//    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
//    // combine the four bytes (two words) into a long integer
//    // this is NTP time (seconds since Jan 1 1900):
//    unsigned long secsSince1900 = highWord << 16 | lowWord;
//    Serial.print("Seconds since Jan 1 1900 = " );
//    Serial.println(secsSince1900);
//
//    // now convert NTP time into everyday time:
//    Serial.print("Unix time = ");
//    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
//    const unsigned long seventyYears = 2208988800UL;
//    // subtract seventy years:
//    epoch = secsSince1900 - seventyYears;
//   // unsigned long epoch = secsSince1900 - seventyYears;
//    // print Unix time:
//    Serial.println(epoch);
//  }
//}
//
//// send an NTP request to the time server at the given address
//unsigned long sendNTPpacket(IPAddress& address) {
//  Serial.println("sending NTP packet...");
//  // set all bytes in the buffer to 0
//  memset(packetBuffer, 0, NTP_PACKET_SIZE);
//  // Initialize values needed to form NTP request
//  // (see URL above for details on the packets)
//  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
//  packetBuffer[1] = 0;     // Stratum, or type of clock
//  packetBuffer[2] = 6;     // Polling Interval
//  packetBuffer[3] = 0xEC;  // Peer Clock Precision
//  // 8 bytes of zero for Root Delay & Root Dispersion
//  packetBuffer[12]  = 49;
//  packetBuffer[13]  = 0x4E;
//  packetBuffer[14]  = 49;
//  packetBuffer[15]  = 52;
//
//  // all NTP fields have been given values, now
//  // you can send a packet requesting a timestamp:
//  udp.beginPacket(address, 123); //NTP requests are to port 123
//  udp.write(packetBuffer, NTP_PACKET_SIZE);
//  udp.endPacket();
//}
//
//void sync_time() {
//    getNTP();
//    setTime(epoch);
////    setTime(12, 41, 0, 25, 2, 2017);   //set the system time to ...
////    RTC.set(now());                    //set the RTC from the system time
////    setSyncProvider(RTC.get);   // the function to get the time from the RTC}  
//    if(timeStatus() != timeSet) {
//        Serial.println("Unable to sync with the NTP");
//    }
//    else {
//      Serial.println("NTP has set the system time");
//      timeReceived = true;
//    }  
//    digital_clock_display();
//}
//
//void print_digits(int digits) {
//    // utility function for digital clock display: prints preceding colon and leading 0
//    Serial.print(':');
//    if(digits < 10)
//      Serial.print('0');
//      Serial.print(digits);
//}
//
//void digital_clock_display(void) {
//    // digital clock display of the time
//    Serial.print(hour());
//    print_digits(minute());
//    print_digits(second());
//    Serial.print(' ');
//    Serial.print(day());
//    Serial.print(' ');
//    Serial.print(month());
//    Serial.print(' ');
//    Serial.print(year()); 
//    Serial.println(); 
//}

