/****************************************************************************************************************************
  SimpleAuthentication.ino - Simple Arduino web server sample for SAMD21 running WiFiNINA shield
  For any WiFi shields, such as WiFiNINA W101, W102, W13x, or custom, such as ESP8266/ESP32-AT, Ethernet, etc
  
  WiFiWebServer is a library for the ESP32-based WiFi shields to run WebServer
  Based on and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Based on  and modified from Arduino WiFiNINA library https://www.arduino.cc/en/Reference/WiFiNINA
  Built by Khoi Hoang https://github.com/khoih-prog/WiFiWebServer
  Licensed under MIT license
 ***************************************************************************************************************************************/

#include "defines.h"    

int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received
int is_id = 0;
WiFiWebServer server(80);

//Check if header is present and correct
bool is_authenticated()
{
  Serial.println(F("Enter is_authenticated"));
  
  if (server.hasHeader(F("Cookie")))
  {
    Serial.print(F("Found cookie: "));
    String cookie = server.header(F("Cookie"));
    Serial.println(cookie);
    if (cookie.indexOf(F("NINASESSIONID=1")) != -1)
    {
      Serial.println(F("Authentication Successful"));
      return true;
    }
  }
  Serial.println(F("Authentication Failed"));
  return false;
}

//login page, also called for disconnect
void handleLogin()
{
  String msg;
  if (server.hasHeader(F("Cookie")))
  {
    Serial.print(F("Found cookie: "));
    String cookie = server.header(F("Cookie"));
    Serial.println(cookie);
  }

  if (server.hasArg("DISCONNECT"))
  {
    Serial.println(F("Disconnection"));
    server.sendHeader(F("Location"), F("/login"));
    server.sendHeader(F("Cache-Control"), F("no-cache"));
    server.sendHeader(F("Set-Cookie"), F("NINASESSIONID=0"));
    server.send(301);
    is_id=0;
    return;
  }
  
  if (server.hasArg(F("USERNAME")) && server.hasArg(F("PASSWORD")))
  {
    if (server.arg(F("USERNAME")) == "admin" &&  server.arg(F("PASSWORD")) == "admin")
    {
      server.sendHeader(F("Location"), F("/"));
      server.sendHeader(F("Cache-Control"), F("no-cache"));
      server.sendHeader(F("Set-Cookie"), F("NINASESSIONID=1"));
      server.send(301);
      Serial.println(F("Log in Successful"));
      is_id=1;
      return;
    }
    msg = F("Wrong username/password! try again.");
    Serial.println(F("Log in Failed"));
    //is_id=0;
  }

  String content = F("<html><body><form action='/login' method='POST'> Log in :<br>");
  content += F("User:<input type='text' name='USERNAME' placeholder='user name'><br>");
  content += F("Password:<input type='password' name='PASSWORD' placeholder='password'><br>");
  content += F("<input type='submit' name='SUBMIT' value='Submit'></form>");
  content += msg;
  content += F("<br>");
  content += F("You also can go <a href='/inline'>here</a></body></html>");
  
  server.send(200, F("text/html"), content);
}

void handleON()
{
  String content = F(" ");
  String header;
  
  if (is_id==1){
    server.sendHeader(F("Location"), F(""));
    server.sendHeader(F("Cache-Control"), F("no-cache"));
    content = F("<html><body><form action='/H' method='GET'> Led turned On <br>");
    digitalWrite(9, HIGH);
    Serial.println(F("LED tuned ON"));
    server.send(200, F("text/html"), content);
    server.send(301); //a sup
  }
  server.sendHeader(F("Location"), F("/login"));
  content = F("<html><body><form Access denied <br>");
}

void handleOFF()
{
  String content = F(" ");
  String header;
    
  if (is_id==1){
    server.sendHeader(F("Location"), F("/"));
    server.sendHeader(F("Cache-Control"), F("no-cache"));
    content = F("<html><body><form action='/L' method='GET'> Led turned Off <br>");
    digitalWrite(9, LOW);
    Serial.println(F("LED tuned OFF"));
    server.send(200, F("text/html"), content);
  }
  server.sendHeader(F("Location"), F("/login"));
  content = F("<html><body><form Access denied <br>");
}

//root page can be accessed only if authentication is ok
void handleRoot()
{
  String header;
  Serial.println(F("Enter handleRoot"));
  
  if (!is_authenticated())
  {
    server.sendHeader(F("Location"), F("/login"));
    server.sendHeader(F("Cache-Control"), F("no-cache"));
    server.send(301);
    
    return;
  }

  String content = F("<html><body><H2>Hello, you successfully connected to WiFiNINA on ");
  content += BOARD_NAME;
  content += F("!</H2>");
  content += F("<hr><br>");
  content += F("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
  content += F("Click <a href=\"/L\">here</a> turn the LED on pin 9 off<br><br>");
    
  if (server.hasHeader(F("User-Agent")))
  {
    content += F("the user agent used is : ");
    content += server.header(F("User-Agent"));
    content += F("<br><br>");
  }
  content += F("You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>");
  server.send(200, F("text/html"), content);
}

//no need authentication
void handleNotFound()
{ 
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");
  
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, F("text/plain"), message);
}

void setup(void)
{
  Serial.begin(115200);
  pinMode(9, OUTPUT);  
  while (!Serial);
  Serial.print(F("\nStarting SimpleAuthentication on "));
  Serial.print(BOARD_NAME);
  Serial.print(F(" with "));
  Serial.println(SHIELD_TYPE); 
  Serial.println(WIFI_WEBSERVER_VERSION);

#if USE_WIFI_NINA
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) 
  {
    Serial.println(F("Please upgrade the firmware"));
  }
#endif

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED)
  {
    Serial.print(F("Connecting to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  server.on(F("/"), handleRoot);
  server.on(F("/login"), handleLogin);
  server.on(F("/H"), handleON);
  server.on(F("/L"), handleOFF);
  server.on(F("/inline"), []()
  {
    server.send(200, F("text/plain"), F("This works without need of authentication"));
  });

  server.onNotFound(handleNotFound);

  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
  Serial.print(F("HTTP server started @ "));
  Serial.println(WiFi.localIP());
}

void loop(void)
{
  server.handleClient();
}
