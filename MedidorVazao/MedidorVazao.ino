/* Comentários */
/* Essas linhas incluem as bibliotecas que serao usadas */
#include <NTPClient.h>
#include <WiFi.h>
#include <WebServer.h>

/* Aqui incluímos o arquivo de configuracoes locais, como nome e senha da rede */
#include "Localconfig.h"

/* Fazemos o Webserver que roda no arduino escutar a porta 80. Esta porta é padrao. */
WebServer server(80);

/* Definimos a constante e o tempo entre leituras. A constante de vazao pode ir pro Localconfig.h no futuro */
const float CONST_VAZAO = 5.5;
const float TEMPO_MEDICAO = 2000; // em milisegundos

/* Cria as variáveis e as definicoes inicias para que o NTP (relógio) funcione */
WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);/* Cria um objeto "NTP" com as configurações.utilizada no Brasil */
String hora, currentDate;
time_t epochTime;
struct tm *ptm; 
int monthDay;
int currentMonth;
int currentYear;

/* Define as variáveis utilizadas no cálculo da medicao */
float vazao;
float acumulado = 0;
int   contaPulso = 0; 
unsigned long int t1, tt;
uint8_t Pin_PulsoMedidor = 4;

/* Rotina pra contar os pulsos */
void IRAM_ATTR incpulso ()
{
  contaPulso++;  
}

void setup() {
  // Inicializa a porta serial do Serial Monitor
  Serial.begin(115200);

  // Definie qual o pino será usado para receber os pulsos do medidor de vazao
  pinMode(Pin_PulsoMedidor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(Pin_PulsoMedidor), incpulso, RISING);
  
  // Inicializa e conecta ao WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Espera até conectar. enquanto isso vai imprimiindo ponto .......
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Imprime pelo Monitor Serial, as informacoes de IP e nome da rede que conectou
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Faz a conexao com o servidor de relógio (NTP)
  ntp.begin();          /* Inicia o protocolo */
  ntp.forceUpdate();    /* Atualização */

  // Define os métodos que serao cahamdos qdo fizermos acesso pelo brownser
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  
  // Inicia o webserver
  server.begin();
  Serial.println("HTTP server started");

  // Imprime o início da medicao
  Serial.println("\n\nInício da medição\n\n");
  Serial.print("Pino de entrada do sinal do medidor: ");
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
    // Só entra nesse c´digo se tiver passado o tempo definido em TEMPO_MEDICAO
  
    /* 
    Serial.print(totalTime);
    Serial.print(" -- ");
    Serial.print(contaPulso);
    Serial.print(" -- ");
    Serial.print(tt);
    Serial.print(" -- ");
    */

    // Pega hora atual
    hora = ntp.getFormattedTime(); 

    /* Aqui usamos um outro tipo de variável para acessar a informacao passada pelo servidor de hora */
    epochTime = ntp.getEpochTime();
    tm *ptm = gmtime ((time_t *)&epochTime); 
    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon+1;
    currentYear = ptm->tm_year+1900;

    /* Colocamos na variáel currentDate, o formato que queremos da data, dia/mes/ano */
    currentDate = String(monthDay) + "/" + String(currentMonth) + "/" +  String(currentYear);

    /* Calculamos a vazao */
    vazao = ((TEMPO_MEDICAO/tt)*contaPulso) / CONST_VAZAO;
    contaPulso = 0;
    t1 = millis();
    /* Somamos a vazao ao valor acumulado, assim temos o volume total do que passou no medidor */
    acumulado = acumulado + vazao/60;
    
    /* Imprimimos pelo Monitor Serial a data, hora, azao e acumulado */    
    Serial.print(currentDate);
    Serial.print(" - ");
    Serial.print(hora);
    Serial.print(" - Vazao: ");
    Serial.print(vazao);
    Serial.print(" l/min  --  ");
    Serial.print("Acumulado: ");
    Serial.print(acumulado);
    Serial.println(" l");
  }
}

/* Essa rotina handle_OnConnect() é executado sempre que alguém conecta o servidor */
void handle_OnConnect() {
  Serial.println("Connection ...");
  server.send(200, "text/html", SendHTML()); 
}

/* Essa rotina handle_NotFound() é executado sempre que alguém tenta acessar uma página no nosso servidor que nao existe */
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

/* Essa rotina SendHTML(void) é chamada para enviar a nossa página pro brownser */
String SendHTML(void){
  /* A cada linha dessa, nós acrescentamos mais uma linha à variável ptr*/
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
  /* return significa que vamos retornar o conteúdo de ptr, ou seja, madamos a página pro brownser */
  return ptr;
}