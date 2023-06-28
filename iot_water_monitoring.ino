//Identificadores da aplicação Blynk
#define BLYNK_TEMPLATE_ID "TMPLisNt0fHs"
#define BLYNK_DEVICE_NAME "TemplateVoiceFlow01"
#define BLYNK_PRINT Serial
#define BLYNK_AUTH_TOKEN "0vUypNDzGeVr7ecKqV8G4Tv3kxhx6UzS"
//---------------------------------------------------------
#define sensor 1
#define scl_esp 0
#define sda_esp 2

//Inclusão de biblioteas
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <RtcDS3231.h>

RtcDS3231<TwoWire> rtcObject(Wire); //declaração para permitir conexão I2C com o DS3231
WiFiClient client; //declaração para permitir envio de dados ao ThingSpeak

char ssid[] = "casa_ers"; //SSID da rede wi-fi
char pass[] = "casa_4576045760"; //senha da rede wi-fi
char auth[] = "0vUypNDzGeVr7ecKqV8G4Tv3kxhx6UzS"; //token de autenticação do dispositivo para conexão ao Blynk iot
char server[] = "api.thingspeak.com"; //endereço API ThingSpeak
String api_key ="DVL5G03WIJWEQ4DS"; //chave da API ThingSpeak

//Variáveis globais
double vol_d = 0; //receberá o volume diário
double vold_antes = 0; //receberá o volume diário anterior
double vol_s = 0; //receberá o volume acumulado durante a semana
double vols_antes = 0; //receberá o volume semanal antigo
double pulsos = 0; //receberá o número de pulsos dados pelo sensor de volume
int seg; //receberá a contagem de segundos do DS3231
int seg_antes; //receberá a contagem de segundos antiga do DS3231
int cont_d = 0; //contará quantos dias da semana se passaram
int cont_seg = 0; //contará quantos segundos se passaram
int dia; //receberá a contagem de minutos do DS3231
int dia_ant = 0; //receberá a contagem de minutos antiga do DS3231

//FUNÇÕES
void ICACHE_RAM_ATTR conta_pulsos(); //armazenamento da função na RAM
void ICACHE_RAM_ATTR enviats(); //armazenamento da função na RAM

//----------------------------------------------------------------------------------------

void setup() {
  sei(); //habilita as interrupções
  pinMode(sensor, INPUT); //define o pino "sensor" como entrada
  attachInterrupt(digitalPinToInterrupt(sensor), conta_pulsos, RISING); //declara a interrupção no pino "sensor"
  Wire.begin(scl_esp,sda_esp); //define os pinos de SCL e SDA para o ESP01
  rtcObject.Begin(); //inicia comunicação com o DS3231
  //RtcDateTime currentTime = RtcDateTime(22, 11, 02, 17, 38, 00); //define a data e hora atuais
  //rtcObject.SetDateTime(currentTime); //configura o DS3231 com a data definida
  RtcDateTime currentTime = rtcObject.GetDateTime();
  Blynk.begin(auth, ssid, pass); //inicia conexão com a Blynk iot
  delay(5000);
  dia_ant = currentTime.Day(); //pega o dia atual do DS3231
  seg = currentTime.Second(); //pega o segundo atual do DS3231
  seg_antes = seg;
}

void loop() {
  Blynk.run(); //permite conexão com a Blynk iot
    if((seg >= (seg_antes + 1))||(seg < seg_antes)){
    seg_antes = seg;
    vol_d = vold_antes + 100*pulsos;
    vold_antes = vol_d;
    vol_s = vols_antes + pulsos;
    vols_antes = vol_s;
    pulsos = 0;
    Blynk.virtualWrite(V0, vol_d); //escreve o volume diário no pino V0 da Blynk iot
    Blynk.virtualWrite(V1, "{\"status\":" + String(vol_d) + "}"); //envia json para o pino V1, o dado poderá ser lido pelo Voiceflow
      if((dia > dia_ant)||(dia < dia_ant)){
        dia_ant = dia;
        cont_d++;
        vol_d = 0;
        vold_antes = vol_d;
        if(cont_d >= 7){
          vol_s=0;
          vols_antes=vol_s;
        }
      }
    cont_seg++;
      if(cont_seg>=15){
        enviats();
        cont_seg=0;
      }
    }
  RtcDateTime currentTime = rtcObject.GetDateTime();
  seg = currentTime.Second();
  dia = currentTime.Day();
}

void ICACHE_RAM_ATTR conta_pulsos(){ //função propriamente dita, contará os pulsos recebidos pelo sensor de volume
  pulsos = pulsos + 0.1;
}

void ICACHE_RAM_ATTR enviats(void){ //função propriamente dita, envia dados ao ThingSpeak
   if (client.connect(server, 80)) 
   { 
     String postStr = api_key;
     postStr += "&field1=";
     postStr += String(vol_s, 1); //escreve o volume como uma string
     postStr += "\r\n\r\n";
     
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
     client.print("https://api.thingspeak.com/update?api_key=DVL5G03WIJWEQ4DS&field1=9");
   }
   client.stop();
}