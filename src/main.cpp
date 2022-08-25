#include <Arduino.h>
#include <Ethernet.h>
#include "func.h"
#include "NordPool.h"

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
// IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
String server = "nordpoolveas.jonaspettersen.no"; // name address for Google (using DNS)
// char server[] = "www.google.com";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
// EthernetClient client;

EthernetClient client;
NordPool NP;
#define UPDATEPRICEINTERVALL 15000

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer Wserver(80);

void setup()
{
    // Open serial communications and wait for port to open:
    startSerial();
    startEthernet();
    NP.setServer(server);
}

void loop()
{

    static unsigned long updateTimer = 0;
    if (millis() > updateTimer + UPDATEPRICEINTERVALL)
    {
        updateTimer = millis();
        NP.update();
        NP.print();
    }

    // listen for incoming clients
    client = Wserver.available();
    if (client)
    {
        Serial.println("new client");
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                Serial.write(c);
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close"); // the connection will be closed after completion of the response
                    client.println("Refresh: 5");        // refresh the page automatically every 5 sec
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");

                    // output the value of each analog input pin
                    for (int clock = 0; clock < 25; clock++)
                    {

                        float price = NP.getPrice(clock);
                        client.print(clock);
                        client.print(": \t");
                        client.print(price);
                        client.println("<br />");
                    }
                    client.println("</html>");
                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disconnected");
    }

    ////////////////////
    ////// DEBUG //////
    //////////////////
    // printPortStatus();
    // DEBUG_ReadKeyFromSerial();

    // Check for incomming "enter" from Serial.
    //  if (DEBUG_ReadSerial()){
    //    //read one line (available date) from HTTP stream.
    //      DEBUG_printOneLineFromHTTP();
    //      DEBUG_printOneLineFromHTTP_ASCII();
    //      DEBUG_printOneLineFromHTTP_HEX();
    //  }

    ////////////////////
    ////// production //////
    //////////////////
}
