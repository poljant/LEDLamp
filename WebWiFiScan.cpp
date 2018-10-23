#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
//#include "Relay.h"
#include <EEPROM.h>

//#define POLISH
//#define DEBUG
extern String version;
//extern Relay R1;
//extern Relay R2;
extern const char* ssid;
extern const char* pass;
extern uint8_t etemp;
extern uint8_t Eetemp;
extern bool LED_ON;
extern int LED_PWM;
extern int pwmstep;
extern int PIN_PWM();
extern int Percent2PWM(int );
// login i hasło do sytemu (dla update)
const char* www_login = "admin";
const char* www_pass = "esp8266";
const char* epass[32];
const char* essid[64];

const char* modes[] = {"NULL","STA","AP","STA+AP"};
const char* phymodes[] = { "","B", "G", "N"};
const char* encrypType[] = {"OPEN", "WEP", "WPA", "WPA2", "WPA_WPA2"};

const int port = 80;                 // port serwera www
ESP8266WebServer server(port);
ESP8266HTTPUpdateServer httpUpdate;

char* IPAdrToStr(int ip)
{
  char* ip_str = new char[16];
  sprintf(ip_str, "%d.%d.%d.%d",
    (ip 	) & 0xFF,
    (ip >> 8) & 0xFF,
    (ip >> 16) & 0xFF,
    (ip >> 24) & 0xFF);
  return ip_str;
}

String HTMLHeader() {           //  nagłówek strony
	String  h = "<!DOCTYPE html>\n";
	  h += "<html>";
	  h += "<head>";
#ifdef POLISH
    h += "<meta http-equiv=\"Content-Language\" content=\"pl\" />";
#else
    h += "<meta http-equiv=\"Content-Language\" content=\"en\" />";
#endif
	  h += "<title>LEDLamp</title>";
	  h += "<meta charset=\"utf-8\">";
	  h += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
	  h += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\" >";
	  h += "</head>";
	  h += "<body style=\"text-align: center;color: white; background: black;font-size: 1.5em;\">\n";
	  return h;
	}

String HTMLFooter() {             //  stopka strony www
	String  f = "";
	  f += "<p>Jan Trzciński &copy; 2018</p></td></tr>";
	  f += "</body>\n";
	  f += "</html>\n";
	  return f;
	}

String HTMLPage1() {      // pierwsza część strony www
	 String t;
	 unsigned long sec = millis() / 1000;
	 unsigned long min = (sec / 60);
	 unsigned long hour = (min / 60);
	 unsigned long days = hour / 24;
	 sec = sec % 60;
	 min = min % 60;
	 hour = hour % 24;
#ifdef POLISH
   t  = "<h1>Lampa LED.</h1>";
#else
	 t  = "<h1>LED Lamp</h1>";
#endif
	 t += "<p> Version "+version;
	 t += "</p>";
#ifdef POLISH
	 t += "<p>Czas działania: dni: ";
#else
   t += "<p>Time of action: days ";
#endif
	 t += (days);
#ifdef POLISH
	 t += " godz:" ;
#else
   t += " hours " ;
#endif
	 t += ((hour<10) ? "0" : "");  //gdy mniej niż 10 wstaw zero wiodące
	 t += (hour);
	 t += ":";
	 t += ((min<10) ? "0" : "");  //gdy mniej niż 10 wstaw zero wiodące
	 t += (min);
	 t += ":";
	 t += ((sec < 10) ? "0" : "");  //gdy mniej niż 10 wstaw zero wiodące
	 t += (sec);

 #ifdef POLISH
   t += ( (LED_ON) ? "<p><a href = \"/lamp/0\"><button class=\"btn btn-danger\">Lampa LED - ZAŁ</button></a></p>\n" : "<p><a href = \"/lamp/1\"><button class=\"btn btn-success\">Lampa LED - WYŁ</button></a></p>\n");
    t += "<p> Jasność %:</p> ";
    t +="<p><a href = \"/PWM/sub\"><button class=\"btn btn-info\">  - " + String(pwmstep) + "</button></a>[ ";
    t += (LED_PWM);
    t += "% ]<a href = \"/PWM/add\"><button class=\"btn btn-info\">  + " + String(pwmstep) + "</button></a></p>\n";
  
 #else
   t += ( (LED_ON) ? "<p><a href = \"/lamp/0\"><button class=\"btn btn-danger\">LED Lamp - ON</button></a></p>\n" : "<p><a href = \"/lamp/1\"><button class=\"btn btn-success\">LED Lamp - OFF</button></a></p>\n");
    t += "<p> Light %:</p> ";
    t +="<p><a href = \"/PWM/sub\"><button class=\"btn btn-info\">  - " + String(pwmstep) + "</button></a>[ ";
    t += (LED_PWM);
    t += "% ]<a href = \"/PWM/add\"><button class=\"btn btn-info\">  + " + String(pwmstep) + "</button></a></p>\n";

 #endif
	 t += "</p>";
 //  t += "<p> <input type=\"number\" name=\"PWM_LED\" min=\"0\" ";
 //  t += " max=\"1023\" step=\"20\" id=\"PWM_LED\"></p>";
#ifdef POLISH
	 t +="<p><a href = \"/wifiscan\"><button class=\"btn btn-info\">Skanuj sieć WiFi</button></a></p>";
   t +="<p><a href = \"/\"><button class=\"btn btn-info\">Odświerz stronę</button></a></p>";
#else
   t +="<p><a href = \"/wifiscan\"><button class=\"btn btn-info\">Scan the WiFi network.</button></a></p>";
   t +="<p><a href = \"/\"><button class=\"btn btn-info\">Reload</button></a></p>";
#endif
	 return t;
	}

 
String HTMLWiFiScan(void){
	String p="";
	String ix="";
	uint8_t n = WiFi.scanNetworks();
  delay(100);
#ifdef POLISH
	if (n == 0) return "<p>Brak sieci WiFi.</p>";
	p +="<div><h3>Skanowanie sieci WiFi</h3></div>";
#else
  if (n == 0) return "<p>No WiFi networks.</p>";
  p +="<div><h3>WiFi network scanning.</h3></div>";
#endif
  p +="<table  align=\"center\" border=\"2\" >";
	p +="<thead ><tr><th> </th><th style = \"text-align: center;\">SSID</th>";
#ifdef POLISH
	p +="<th>kanał</th><th style = \"text-align: center;\">MAC</th>";
  p +="<th>RSSI</th><th>zabezp</th><th>ukryta</th><tr>";
#else
  p +="<th>channel</th><th style = \"text-align: center;\">MAC</th>";
  p +="<th style = \"text-align: center;\">RSSI</th><th>encryption</th><th>hidden</th><tr>";
#endif
	p +="</thead><tbody>";
	 for (uint8_t i=0; i<n;i++){
	 p +="<tr><td>";
	 p +="<form action=\"/wifiset\" metod=\"post\">";
	 p +="<labe><input id=\"SSID\" type=\"radio\" name=\"SSID\" value=\""+WiFi.SSID(i)+"\">";
	 p +="</label>";
	 p +="</td>";
	 p +="<td>"+WiFi.SSID(i)+"</td>";	//nazwa sieci
	 p +="<td>"+String(WiFi.channel(i))+"</td>";	//kanał
	 p +="<td>"+WiFi.BSSIDstr(i)+"</td>";	//MAC adres
	 p +="<td>"+String(WiFi.RSSI(i))+" dBm</td>"; //siła sygnału
	 p +="<td>";
   p +=(encrypType[WiFi.encryptionType(i)]);// szyfrowanie
	 p +="</td><td>";
#ifdef POLISH
	 p +=((WiFi.isHidden(i)) ? "tak" : "nie");//czy sieć ukryta
#else
   p +=((WiFi.isHidden(i)) ? "yes" : "no");//czy sieć ukryta
#endif
	 p +="</td></tr>";
	 } //end for
	 p +="</tbody></table>";
	 p +="<div><p></p> </div>";
   return p;
}
String HTMLWiFiScan1(void){
  String p="";
  String ix="";  

	 // gdy połączenie z WiFi
	 if(WiFi.status() == WL_CONNECTED){
#ifdef POLISH
		p += "<p>Jesteś połączony z siecią </p>";
#else
    p += "<p>Connected with WiFi </p>";
#endif
		p +="<table align=\"center\" border=\"2\" ><tbody>";
		p +="<thead ><tr><th style = \"text-align: center;\">SSID</th>";
#ifdef POLISH
		p +="<th>kanał</th><th style = \"text-align: center;\">MAC</th>";
#else
    p +="<th>channel</th><th style = \"text-align: center;\">MAC</th>";
#endif
		p +="<th style = \"text-align: center;\">RSSI</th></thead><tbody>";
		p += "<td>"+WiFi.SSID()+"</td>";
		p += "<td>"+String(WiFi.channel())+"</td>";
		p += "<td>"+WiFi.BSSIDstr()+"</td>";
		p += "<td>"+String(WiFi.RSSI())+" dBm</td></tbody></table>";
		p +="<label>IP: "+String(IPAdrToStr(WiFi.localIP()))+"</label>";
#ifdef POLISH
		p += "<p>Aby zmienić sieć WiFi.</p>";
#else
    p += "<p>To change the WiFi network.</p>";
#endif
	 }else{	//gdy brak połączenia z WiFi.
#ifdef POLISH
		 p += "<p>Brak połączenia z siecią WiFi.</p>";
#else
     p += "<p>No connection with WiFi.</p>";
#endif
		 p += "<label>IP AP: "+String(IPAdrToStr(WiFi.softAPIP()))+"</label>";
	 }	//end if
#ifdef POLISH
	 p += "<p>Wybierz sieć powyżej i podaj hasło</p>";
	 p +="<label for=\"password\">Hasło do WiFi:</label>";
#else
   p += "<p>Select the WiFi network above and enter the password</p>";
   p +="<label for=\"password\">WiFi password:</label>";
#endif
	 p +="<input style=\"color: black;\" type=\"password\" name=\"password\" />" ; //required=\"required\"/>";
#ifdef POLISH
	 p +="<input style=\"text-align: center;color: black;\" type=\"submit\" value=\"Połącz z WiFi.\"/></form>";
	 p += "<p><a href = \"/wifiscan\"><button class=\"btn btn-info\">Skanuj ponownie</button></a></p>";
   p += "<p><a href = \"/\"><button class=\"btn btn-info\">Strona główna</button></a></p>";  
#else
   p +="<input style=\"text-align: center;color: black;\" type=\"submit\" value=\"Connect.\"/></form>";
   p += "<p><a href = \"/wifiscan\"><button class=\"btn btn-info\">Scan again</button></a></p>";
	 p += "<p><a href = \"/\"><button class=\"btn btn-info\">Home</button></a></p>";
#endif
	return p;
}
String WebPageScan(){
	return (HTMLHeader()+HTMLWiFiScan()+HTMLWiFiScan1()+HTMLFooter());
}

String WebPage() {       // połącz wszystkie części strony www
 return (HTMLHeader()+ HTMLPage1()+HTMLFooter());
}
String WebPageGet() {

  return (LED_ON ? "1" : "0");
}
String WebPagePWMGet() {

  return String(LED_PWM);
}
// funkcja ustawia wszystkie strony www
void setservers(){
	 httpUpdate.setup(&server,"/update", www_login, www_pass); // umożliwia aktualizację poprzez WiFi

	 server.on("/", [](){
	    server.send(200, "text/html", WebPage());
       //server.send(200, "text/html", ((WiFi.status() == WL_CONNECTED)? WebPage():WebPageScan()));
	  });

	 server.on("/wifiscan", [] ()
	  {
		 server.send(200, "text/html", WebPageScan());
	  });
   
//ustanawia połączenie z wybraną siecią wifi
	 server.on("/wifiset", [] ()
	  {
    //pobierz przysłane dane
		String ESSID = server.arg("SSID");
		String Epass = server.arg("password");
    EEPROM.begin(128);
    EEPROM.get(63, etemp);
    if(etemp == 1 or etemp == 0xFF){
      etemp=0;
      EEPROM.put(Eetemp,etemp);
    };
    if (((sizeof(ESSID)>=4) && (sizeof(Epass)>=8))) { //pomiń gdy brak nazwy lub hasła
    //konwertuj ze string do char
		 const char*	essid = ESSID.c_str();
		 const char* epass = Epass.c_str();
     etemp = 1;
#ifdef DEBUG
      Serial.println(WiFi.status());
      Serial.println("ESSID i Epass OK.");
      Serial.println(essid);
      Serial.println(epass);
#endif
    // zapisz SSID i password do EEPROM
     EEPROM.put(0,essid);
     EEPROM.put(64,epass);
     EEPROM.put(Eetemp,etemp);
     delay(20);
     EEPROM.get(0,ssid);
     EEPROM.get(64,pass);
     EEPROM.get(Eetemp,etemp);
 
		 WiFi.disconnect() ; //rozłacz obecne połączenie
		 WiFi.mode(WIFI_AP_STA);//ustw tryb AP+STA
 
		 WiFi.begin(ssid,pass); //uruchom łączenie z wybraną siecią wifi
		 delay(200);
    }
    EEPROM.end();
		int it = 20; //ustal maksymalny czas czekania na 10sek.(20x500ms)
		while ((WiFi.status() != WL_CONNECTED and it>0)) {  //  czekaj na połączenie z WiFi
#ifdef DEBUG
      Serial.print(".");
#endif
    delay(500);
	  it--;
	 }
#ifdef DEBUG
    if (it<=0){
      Serial.println("Błąd w ESSID lub Epass.");
      Serial.println(WiFi.status());
      Serial.println(ssid);
      Serial.println(pass);
      };
#endif
   server.send(200, "text/html", WebPageScan());
	});

  server.on("/lamp/0", [] ()     //  wyłącz przekaźnik 1
  {
    LED_ON = false;
 //   R1.setOff();
    server.send(200, "text/html", WebPage());
   });

  server.on("/lamp/1", []()      // załącz przekaźnik 1
  {
   LED_ON = true;
//   R1.setOn();
   server.send(200, "text/html", WebPage());
  }); 



  server.on("/lamp/get", [] ()     //  podaj stan przekaźników
  {
     server.send(200, "text/html", WebPageGet()); //{

//  return (LED_ON ? "1" : "0");
//});
  });

  server.on("/PWM/get", [] ()     //  podaj stan przekaźników
  {
     server.send(200, "text/html", WebPagePWMGet()); //{

//  return (String(LED_PWM));
//};
  });
   server.on("/PWM/add", []()      // zwiększ wartość PWM
    {LED_PWM = ((LED_PWM+pwmstep)>100) ? 100 : LED_PWM+pwmstep;
 //    analogWrite(pinPWM,val2);
    if(LED_ON)  analogWrite(PIN_PWM(),Percent2PWM(LED_PWM));
 //   bwwwfull = false;
     server.send(200, "text/html", WebPage());
    });

   server.on("/PWM/sub", []()      // zmniejsz wartość PWM
    {LED_PWM = ((LED_PWM-pwmstep)<0) ? 0 : LED_PWM-pwmstep;
    if(LED_ON) analogWrite(PIN_PWM(),Percent2PWM(LED_PWM));
//     analogWrite(pinPWM,val2);
//     bwwwfull = false;
     server.send(200, "text/html", WebPage());
    });

  
server.begin();                // Start serwera www
#ifdef DEBUG
Serial.println("Server started");
#endif
}
