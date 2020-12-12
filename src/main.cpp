//Bibliotecas-------------------
#include <DHT.h>         // sensor de temperatura e umidade amb.
#include <ESP8266WiFi.h> // para utilizar o modulo wifi
#include <NTPClient.h>   // para utilizar servidor de time
#include <Wire.h>        // para utilização do módulo wifi
#include <BH1750.h>      // sensor de luminosidade
#include <NTPClient.h>   // Sincronização online do tempo
#include <WiFiUdp.h>     // Funcões do módulo Wifi
#include <Stepper.h>     // Motor de passo

//Definições--------------------
#define DHTTYPE DHT11 // Definição do tipo do sensor
#define DHTPIN 2      // define a entrada do microcontrolador utilizada pelo sensor
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
WiFiUDP ntpUDP;
int16_t utc = -3;                                                //UTC -3:00 Brazil
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc * 3600, 60000); // função que tem como retorno o horario atual

//Variáveis globais -------------------
String apiKey = "6EI26MRXAHGM7B5C";                    // chave para ter acesso aoservidor Thinkspeak 
const char *server = "api.thingspeak.com";             // Endereço da plataforma onde serão enviados os dados captados float umidade_amb;
float temperatura_amb;
float umidade_solo;
73 float umidade_minima = 200;
float luminosidade;
float ValorADC;
float motordepasso;
int Horario_Abertura = 7;                              // horário em que a estufa será aberta 07:00
int Horario_Fechamento = 23;                           // horário em que a estufa será fechada
19 : 00 bool Chuva = 0;                                // 0 sem chuva, 1 com chuva
bool Teto = 0;                                         // 1 teto aberto, 0 teto fechado
const int stepsPerRevolution = 500;                    // quantidade de passos do motor
Stepper myStepper(stepsPerRevolution, 14, 12, 13, 15); // define pinos utilizados para acionar motores de passo

//funções -------------------
void
ConectToWiFi(void);                         // conecta sistema com um roteador wifi
void SendToThingSpeak(void);                // envia pacote de dados para plataforma
de Iot void SensorDHT11(void);              // Realiza leitura do sensor de umidade e
temperatura ambiente void SensorFc28(void); // Realiza leitura de umidade do solo
void SensorBH1750(void);                    // executa a leitura de luminosidade ambiente
void AbreTeto(void);                        // Função para abrir teto utilizando os motores de
passo void FechaTeto(void);                 // Função para fechar teto utilizando os motores de
passo void Irrigacao(void);                 // aciona bomba de irrigação
void SensorM11RD(void);                     // realiza leitura da precipitação
void VerificacaoTeto(void);                 // verifica o estado do teto (aberto ou fechado)
WiFiClient client;


void setup() 74
{
  Serial.begin(115200);   // Inicia serial com baud rate de 115200
  myStepper.setSpeed(40); // velocidade do motor de passo
  delay(10);              // tempo de espera de 10s
  dht.begin();            // inicia sensor de humidade e temperatura ambiente
  lightMeter.begin();     // inicia sensor de luminosidade
  ConectToWiFi();         // função para conctar em uma rede wifi
  timeClient.begin();     // Inicia serviço de tempo
  timeClient.update();    // atualiza horário
  pinMode(10, OUTPUT);    //rele para acionar valvula de irrigação
  pinMode(16, INPUT);     //para ler o sensor de chuva
}


void loop() // PROGRAMA PRINCIPAL
{

  checkOST();        // horário atual
  SensorDHT11();     //sesor temperatura e umidade ambiente
  SensorBH1750();    //sensor de luminosidade
  SensorFc28();      //sensor de umidade do solo
  SensorM11RD();     // sensor de chuva
  VerificacaoTeto(); // verifica qual é a posição do teto

  // se não estiver chovendo E se o teto estiver fechado E se estiver na hora de abrir
  if ((Chuva == 0) && (Teto == 0) && (timeClient.getHours() >= Horario_Abertura) && (timeClient.getHours() <= Horario_Fechamento)) 
    {
      AbreTeto();
    }

  if ((Chuva == 1) && (Teto == 1)) // se estiver chovendo E o teto estiver
    aberto 75
    {
      FechaTeto();
    }

  if (umidade_solo < umidade_minima)
  {
    Irrigacao(); //aciona sistema de irrigação e medidor de fluxo de água
  }
  SendToThingSpeak(); // envia dados dos sensores para a plataforma
  IOT
} //final void loop


//Função: faz a conexão WiFi
const char *ssid = "GVT-D8A1";       //Nome da rede WiFi
const char *password = "S1EB560251"; //senha da rede WiFi
void ConectToWiFi(void)
{
 Serial.println("\nTCC Smart Greenhouse\n UTFPR\n Felipe A.
Fonseca\n Luiz schmitz");
 delay(5000);
 WiFi.begin(ssid, password);
 Serial.println("");
 Serial.println("Conectando-se a rede WiFi...");
 WiFi.begin(ssid, password);

 while (WiFi.status() != WL_CONNECTED)
76
 {
    delay(1000);
    Serial.print(".");
 }
 Serial.println("");
 Serial.println("WiFi connectado com sucesso!");
 Serial.println("IP obtido: ");
 Serial.println(WiFi.localIP());
 Serial.println("\nInicializando...");
 delay(2000);
}


//Função: Envia dados coletados pelos sensors para a plataforma de IoT
ThingSpeak void SendToThingSpeak(void)
{
  if (client.connect(server, 80))
  {

    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(umidade_amb);
    postStr += "&field2=";
    postStr += String(temperatura_amb);
    postStr += "&field3=";
    postStr += String(umidade_solo);
    postStr += "&field4=";
    postStr += String(luminosidade);
    postStr += "&field5=";
    postStr += String(Chuva);
    postStr += "&field6=";
    postStr += String(Teto);
    77 postStr += "\r\n\r\n\r\n\r\n\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("Enviando...");
    client.stop();

    delay(6000);
  }
}


//Função: Relógio
uint32_t currentMillis = 0;
uint32_t previousMillis = 0;
void forceUpdate(void)
{
  timeClient.forceUpdate();
}
void checkOST(void)
{
  currentMillis = millis(); //Tempo atual em ms

  if (currentMillis - previousMillis > 1000) //Lógica de verificação do tempo
  {
    78 previousMillis = currentMillis; // Salva o tempo atual
                                       //printf("Hora: %d: ", timeClient.getEpochTime()); //exibe o horário em
    mS
        Serial.print("\n");
    Serial.println(timeClient.getFormattedTime()); //exibe o horário em
    formato gregoriano
  }
}


//Função: Inicia sensor de temperature e umidade ambiente
void SensorDHT11(void)
{
  umidade_amb = dht.readHumidity();
  temperatura_amb = dht.readTemperature();
  delay(5000);

  if (isnan(umidade_amb) || isnan(temperatura_amb))
  {
    //Serial.println("Falha na Leitura do sensor DHT11");
    // return;
  }

  Serial.print("Temperatura Amb.: ");
  Serial.print(temperatura_amb);
  Serial.print("\nUmidade Amb.: ");
  Serial.print(umidade_amb);
}


//Função: Verifica condição atual do teto
void VerificacaoTeto(void) 79
{
  if (Teto == 1)
  {
    Serial.print("\nTeto: Aberto\n");
  }
  else
  {
    Serial.print("\nTeto: Fechado\n");
  }
}


//Função: faz a leitura do nível de umidade
//Retorno: umidade percentual (0-100)
void SensorFc28(void)
{
  float UmidadePercentual;
  ValorADC = analogRead(0); //418 -> 1.0V
  umidade_solo = ValorADC;
  Serial.print("\nUmidade Solo.: ");
  Serial.print(umidade_solo);

  return;
}


//Função: Aciona sistema de irrigação
void Irrigacao(void)
{
  80 digitalWrite(10, LOW); //liga válvula responsavel pela irrigação
  Serial.print("\nSistema de Irrigação: LIGADO\n");
  delay(10000);             //espera 10s
  digitalWrite(10, HIGH);   //desliga válvula responsavel pela irrigação
  Serial.print("Sistema de Irrigação: DESLIGADO\n");
}
//Função: realiza leituras da precipitação


void SensorM11RD(void)
{
  if (digitalRead(0) == LOW)
  {
    Chuva = 1;
    Serial.print("\nPrecipitação: SIM");
  }
  else
  {
    Chuva = 0;
    Serial.print("\nPrecipitação: NÃO ");
  }
}


// Funções responsáveis por acionar o motor de passo para abrir ou fechar o teto int x; 
// variável do contador
int n_voltas = 10;   // numero de rotações necessárias para o motor de passo abrir / fechar a estufa 81 void AbreTeto(void)
{
  Serial.print("\nAbrindo Teto...\n");

  for (x = 0; x < n_voltas; x++)
  {
    myStepper.step(stepsPerRevolution);
    delay(5);
  }
  Serial.print("Teto aberto\n");
  Teto = 1;
}


void FechaTeto(void)
{
  Serial.print("\nFechando Teto...\n");
  for (x = 0; x < n_voltas; x++)
  {
    myStepper.step(-stepsPerRevolution);
    delay(5);
  }

  Serial.print("Teto fechado\n");
  Teto = 0;
  82
}