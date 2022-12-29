#include <WiFi.h>
#include <WebServer.h>

#include "Localconfig.h"

WebServer server(80);

const float CONST_VAZAO = 5.5;
const float TEMPO_MEDICAO = 3000; // em milisegundos

//Define as variáveis e as inicia com valor zero
float vazao;
float acumulado = 0;
int contaPulso = 0; 
int i = 0; 
int Min = 00; 
float Litros = 0;
float MiliLitros = 0;

long int t1, tt;

uint8_t Pin_PulsoMedidor = 4;
uint8_t Saida_Simulacao = 14;

// Conta os pulsos
void IRAM_ATTR incpulso ()
{
 contaPulso++;
 //Serial.println("..int..");
}

void setup() {
  Serial.begin(115200);

  // Medidor de vazao
  pinMode(Pin_PulsoMedidor, INPUT_PULLUP);
  pinMode(Saida_Simulacao, OUTPUT);
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
  //sei(); // Inicializa interrupcao
  t1 = millis(); // inicializa tempo
  
}
void loop() {
  server.handleClient();

  // Medidor de vazao
  // Verifica se passou 1 seg pra pegar os valores medidos
  tt = (millis() - t1);  
  if ( tt > TEMPO_MEDICAO ){
    //detachInterrupt(digitalPinToInterrupt(Pin_PulsoMedidor));
    Serial.print(contaPulso);
    Serial.print(" -- ");
    vazao = ((TEMPO_MEDICAO/tt)*contaPulso) / CONST_VAZAO;
    contaPulso = 0;
    //attachInterrupt(digitalPinToInterrupt(Pin_PulsoMedidor), incpulso, FALLING);
    t1 = millis();
    acumulado = acumulado + vazao/60;
    Serial.print("Vazao: ");
    Serial.print(vazao);
    Serial.print(" l/min  --  ");
    Serial.print("Acumulado: ");
    Serial.print(acumulado);
    Serial.println(" l");
  }

  // Simulacao de pulsos  
  // contaPulso++;
  if (digitalRead(Saida_Simulacao) == HIGH) {
    digitalWrite(Saida_Simulacao, LOW);
  } else {
    digitalWrite(Saida_Simulacao, HIGH);
  }
}

void handle_OnConnect() {
  Serial.println("Connection ...");
  server.send(200, "text/html", SendHTML(vazao, acumulado)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float vazao_gui, float acumulado_gui){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Medidor de Vaz&atilde;o</title>\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"5\">\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Medidor de Vaz&atilde;o</h1>\n";
  
  ptr +="<h3><p>Vaz&atilde;o: ";
  ptr +=String(vazao_gui);
  ptr +=" l/min<h3><p>";

  ptr +="<h3><p>Acumulado: ";
  ptr +=String(acumulado_gui);
  ptr +=" l<p><h3>";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}