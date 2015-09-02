// Class to simplify HTTP fetching on Particle
// Original version for Arduino https://github.com/amcewen/HttpClient/
// Released under Apache License, version 2.0 by Kevin Cooper

#include "application.h"
#include "HttpClient.h"
HttpClient http;

const byte kIp[] = {104, 238, 136, 31};
const char kHostname[] = "icanhazip.com";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/";

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 5*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 200;

void setup() {
  Serial.begin(9600);
  while (!Serial.available()) {
    Serial.println("Press any key to start.");
    Particle.process();
    delay (1000);
  }
}

void loop() {
  int err = 0;
  unsigned long httpStartTime = millis();
  // If only hostname is supplied a DNS lookup is used to get the IP.
  // It is faster to provide IP and hostname if possible.

  // Using IP
//  err = http.get(kIp, kHostname, kPath);

  // Using Hostname
  err = http.get(kHostname, kPath);

  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();

        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char body[bodyLen+1];
        char c;
        int i = 0;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout))
        {
          if (http.available())
          {
            c = http.read();
            body[i] = c;
            i++;

            if (http.endOfBodyReached()) {
              //Workaround for TCPClient.Connected() bug
              http.stop();
            }

            // We read something, reset the timeout counter
            timeoutStart = millis();
          }
          else
          {
            // We haven't got any data, so let's pause to allow some to
            // arrive
            delay(kNetworkDelay);
          }
        }
        body[i] = '\0';
        //return body;
        Serial.println();
        Serial.println("body variable ==");
        Serial.print(body);
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();

  Serial.println();
  Serial.print("http request took : ");
  Serial.print(millis()-httpStartTime);
  Serial.println("ms");

   // And just stop, now that we've tried a download
  while(1) { Particle.process(); };
}
