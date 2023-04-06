#include <WiFi.h>
#include <WebServer.h>

#include "Localconfig.h"

WebServer server(80);

const float CONST_VAZAO = 5.7;
const float TEMPO_MEDICAO = 2000; // em milisegundos

// Esse é o branch IP Fixo

//Define as variáveis e as inicia com valor zero
float vazao;
float acumulado = 0;
int   contaPulso = 0; 
unsigned long int t1, tt;
uint8_t Pin_PulsoMedidor = 4;

// Conta os pulsos
void IRAM_ATTR incpulso ()
{
  contaPulso++;  
}

void setup() {
  Serial.begin(115200);

  // Medidor de vazao
  pinMode(Pin_PulsoMedidor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Pin_PulsoMedidor), incpulso, RISING);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("\n\nInício da medição\n\n");
  Serial.println(digitalPinToInterrupt(Pin_PulsoMedidor));
  t1 = millis(); // inicializa tempo
  contaPulso = 0; // zera contador
  
}
void loop() {
  server.handleClient();
  
  // Medidor de vazao
  // Verifica se passou 1 seg pra pegar os valores medidos
  tt = (millis() - t1);  
  if ( tt > TEMPO_MEDICAO ){
    /* 
    Serial.print(totalTime);
    Serial.print(" -- ");
    Serial.print(contaPulso);
    Serial.print(" -- ");
    Serial.print(tt);
    Serial.print(" -- ");
    */
    vazao = ((TEMPO_MEDICAO/tt)*contaPulso) / CONST_VAZAO;
    contaPulso = 0;
    t1 = millis();
    acumulado = acumulado + vazao/60;
    Serial.print("Vazao: ");
    Serial.print(vazao);
    Serial.print(" l/min  --  ");
    Serial.print("Acumulado: ");
    Serial.print(acumulado);
    Serial.println(" l");
  }
}

void handle_OnConnect() {
  Serial.println("Connection ...");
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(void){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Medidor de Vaz&atilde;o</title>\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"5\">\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="table{font-family: arial, sans-serif;border-collapse:collapse;width: 75%;}td,th{border: 1px solid #dddddd;text-align:left;padding:8px;}tr:nth-child(even){background-color:#dddddd;}\n";
  ptr +="body{margin-top: 50px;} h2 {color: #4233FF;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +=".center{margin-left:auto;margin-right:auto;}\n";
  ptr +="p {font-size: 14px;color: #999;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h2>Medidor de Vaz&atilde;o</h2>\n";
  
  ptr +="<table class=\"center\"><tr><td>Vaz&atilde;o</td><td>";
  ptr +=String(vazao);
  ptr +=" l/min</td></tr>";

  ptr +="<tr><td>Acumulado</td><td>";
  ptr +=String(acumulado);
  ptr +=" l</td></tr></table>";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}