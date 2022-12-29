#include <WiFi.h>
#include <WebServer.h>

#include "Localconfig.h"

WebServer server(80);

char buff[10];

uint8_t LED1pin = 15;
bool LED1status = LOW;

const float CONST_VAZAO = 5.5;
const int TEMPO_MEDICAO = 3000; // em milisegundos

uint8_t LED2pin = 5;
bool LED2status = LOW;

//Define as variáveis e as inicia com valor zero
float vazao;
float acumulado = 0;
int contaPulso = 0; 
int i = 0; 
int Min = 00; 
float Litros = 0;
float MiliLitros = 0;

long int t1;

uint8_t Pin_PulsoMedidor = 2;
uint8_t Saida_Simulacao = 4;

void setup() {
  Serial.begin(115200);
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);

  // Medidor de vazao
  pinMode(Pin_PulsoMedidor, INPUT);
  pinMode(Saida_Simulacao, OUTPUT);
  attachInterrupt(Pin_PulsoMedidor, incpulso, RISING);
  
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
  sei(); // Inicializa interrupcao
  t1 = millis(); // inicializa tempo
  
}
void loop() {
  server.handleClient();
  if(LED1status)
  {digitalWrite(LED1pin, HIGH);}
  else
  {digitalWrite(LED1pin, LOW);}
  
  if(LED2status)
  {digitalWrite(LED2pin, HIGH);}
  else
  {digitalWrite(LED2pin, LOW);}

  // Medidor de vazao
  // Verifica se passou 1 seg pra pegar os valores medidos
  if ((millis() - t1) > TEMPO_MEDICAO ){
    t1 = millis();
    vazao = contaPulso / CONST_VAZAO;
    contaPulso = 0;
    acumulado = acumulado + vazao/60;
    Serial.print("Vazao: ");
    Serial.print(vazao);
    Serial.print(" l/min  --  ");
    Serial.print("Acumulado: ");
    Serial.print(acumulado);
    Serial.println(" l");
  }

  // Simulacao de pulsos  
  //contaPulso++;
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

String SendHTML(float vazao, float acumulado){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Medidor de Vaz&atilde;o</title>\n";
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
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
  ptr +="<p>Vaz&atilde;o: ";
  ptr +=String(vazao);
  ptr +=" l/min<p>";

  ptr +="<p>Acumulado: ";
  ptr +=String(acumulado);
  ptr +=" l<p>";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

// Conta os pulsos
void incpulso ()
{
 contaPulso++;
}