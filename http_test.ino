// Class to simplify HTTP fetching on Particle
// Original version for Arduino https://github.com/amcewen/HttpClient/
// Released under Apache License, version 2.0 by Kevin Cooper

#include "application.h"
#include "HttpClient.h"

HttpClient http;

// If Content-Length isn't given this is used for the body length increments
const int kFallbackContentLength = 100;

typedef struct
{
  String hostname;
  IPAddress ip;
  String path;
  int port;
  String body;
} http_request_t;

/**
 * HTTP Response struct.
 * status  response status code.
 * body    response body
 */
typedef struct
{
  int status;
  String body;
} http_response_t;

http_response_t response;
http_request_t request;

void doGetRequest() {
  int err = 0;
  response.status = 0;
  response.body = '\0';
  unsigned long httpStartTime = millis();

  // If only hostname is supplied a DNS lookup is used to get the IP.
  // It is faster to provide IP and hostname if possible.

  // Using IP
//  err = http.get(request.ip, request.hostname, request.path);

  // Using Hostname
  err = http.get(request.hostname, request.path);

  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);
      response.status = err;

      if (err >= 200 || err < 300)
      {
        err = http.skipResponseHeaders();
        if (err >= 0)
        {
          int bodyLen = http.contentLength();
          Serial.print("Content length is: ");
          Serial.println(bodyLen);

          // To test Malloc and Remalloc
          //bodyLen = 0;

          // Now we've got to the body, so we can print it out
          unsigned long timeoutStart = millis();
          if (bodyLen <= 0)
            bodyLen = kFallbackContentLength;

          Serial.print("Body length is: ");
          Serial.println(bodyLen);

          char *body = (char *) malloc( sizeof(char) * ( bodyLen + 1 ) );

          char c;
          int i = 0;
          // Whilst we haven't timed out & haven't reached the end of the body
          while ( (http.connected() || http.available()) &&
                 ((millis() - timeoutStart) < http.kHttpResponseTimeout))
          {
            // Let's make sure this character will fit into our char array
            if (i == bodyLen)
            {
              // No it won't fit. Let's try and resize our body char array
              char *temp = (char *) realloc(body, sizeof(char) * (bodyLen+kFallbackContentLength));

              if ( temp != NULL ) //resize was successful
              {
                Serial.println("EXTENDING: "+String(bodyLen));
                bodyLen += kFallbackContentLength;
                body = temp;
              }
              else //there was an error
              {
                Serial.println("Error allocating memory!");
                break;
              }
            }

            if (http.available())
            {
              c = http.read();

              body[i] = c;
              i++;
              // We read something, reset the timeout counter
              timeoutStart = millis();
            }
            else
            {
              // We haven't got any data, so let's pause to allow some to
              // arrive
              delay(http.kHttpWaitForDataDelay);
            }
          }
          body[i] = '\0';
          //return body;
          response.body = String(body);
          free(body);
        }
        else
        {
          Serial.print("Failed to skip response headers: ");
          Serial.println(err);
        }
      }
      else
      {
        Serial.print("Response code < 200 or >= 300");
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

  //Serial.println();
  Serial.print("http request took : ");
  Serial.print(millis()-httpStartTime);
  Serial.println("ms");
}

void setup() {
  Serial.begin(9600);

  //request.ip = { 185, 31, 18, 133 };
  request.hostname = "blog.particle.io";
  request.path = "/";

  while (!Serial.available()) {
    Serial.println("Press any key to start.");
    Particle.process();
    delay (1000);
  }
}

void loop() {
  doGetRequest();
  Serial.println("BODY == ");
  Serial.println(response.body);
   // And just stop, now that we've tried a download
  while(1) { Particle.process(); }
}
