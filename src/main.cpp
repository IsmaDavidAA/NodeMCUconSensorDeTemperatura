/*
******************
SERVIDOR CLOUDMQTT
******************

La página del servidor es:
https://www.cloudmqtt.com/

Login:
  Email:    idaasannmvgd@gmail.com   luisagreda@gmail.com
  Password: Monkeystr                532Unindiaco

INSTANCIA: nodemcu
Server      postman.cloudmqtt.com     m24.cloudmqtt.com
User        vqtjgzus                  ypkgkakk 
Password    F-U1EenyzNl2              3DCfLGnsf-u8 
Port        17705                     19051 
SSL Port    27705                     29051 
Websockets  37705                     Port (TLS only)    39051 
Connection  limit                     5
API Key     f504dd64-1137-47f2-ad3d-1134557c364e    a2be7354-1030-435c-905c-cb28d505a977
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h> 

#define ID_MQTT    "ds18b20_0"    //identificador mqtt para la sesion
#define TOPICO_PUB "test/temp"  //tópico para temperatura que se publica en el Broker
#define TOPICO_SUB "test/led"    //tópico al que se subscribe

#define led   LED_BUILTIN

const char* SSID =      "COMTECO-95268074";
const char* PASSWORD =  "UPHEN83108";
const char* BROKER_MQTT = "postman.cloudmqtt.com";
const int BROKER_PORT = 17705;
const char* mqttUser = "vqtjgzus";
const char* mqttPassword = "F-U1EenyzNl2";
 
unsigned long intervalo = 1000;   // 1 segundo
unsigned long valorActual;

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire); 

WiFiClient espClient;
PubSubClient client(espClient);

//Prototipos
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexWiFiyMQTT(void);
void InitOutput(void);

void InitOutput(void) {
  //IMPORTANTE: El led de la placa funciona con lógica invertida (o sea,
  //enviar HIGH para apagar el led/ enviar LOW para encenderlo)
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);          
}

void initSerial() {
  Serial.begin(9600);
}

void initWiFi() {
    delay(10);
    Serial.println("------Conexion WiFi------");
    Serial.print("Conectandose a la red: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
  client.setServer(BROKER_MQTT, BROKER_PORT);   //IP y puerto del Broker
  client.setCallback(mqtt_callback);            
}

//esta función es llamada cuando llega información de uno de los tópicos subscritos
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  //obtiene la cadena del payload recibido
  for(unsigned int i = 0; i < length; i++) {
    char c = (char)payload[i];
      msg += c;
  }
   
  //toma acción dependiendo de la cadena recibida:
  //verifica si debe colocar nivel alto o bajo en la salida LED_BUILTIN:
  if (msg.equals("L")) {
    digitalWrite(led, LOW);
  }
  if (msg.equals("H")) {
    digitalWrite(led, HIGH);
   }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("* Intentando conectar al Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (client.connect(ID_MQTT, mqttUser, mqttPassword)) {
    //if (client.connect(ID_MQTT)) {
        Serial.println("Conectado con éxito al Broker MQTT!");
        client.subscribe(TOPICO_SUB); 
    } 
    else {
        Serial.println("Falla al reconectar al Broker.");
        Serial.println("Habra nueva tentativa de conexion en 2s");
        delay(2000);
    }
  }
}
  
void reconectWiFi() {
  //si ya está conectado a la rede WI-FI, no hace nada 
  //caso contrário, son efectuadas tentativas de conexión
  if (WiFi.status() == WL_CONNECTED)
    return;
  WiFi.begin(SSID, PASSWORD); //se conecta a la red WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
   
  Serial.println();
  Serial.print("Conectado con éxito a la red WiFi ");
  Serial.print(SSID);
  Serial.println("IP obtenido: ");
  Serial.println(WiFi.localIP());
}

void VerificaConexWiFiyMQTT(void) {
  if (!client.connected()) 
    reconnectMQTT();  //si se pierde la conexión con el Broker, se reinicia
  reconectWiFi();     //si se pierde la conexión con la red WiFi, se reinicia
}

void EnviaTempMQTT(void) {
  if(millis() - valorActual >= intervalo) {
    valorActual = millis();
    sensors.requestTemperatures();
    float celsius = sensors.getTempCByIndex(0);
    Serial.println(sensors.getTempCByIndex(0),1); //1 decimal
    char tempstring [3];
    dtostrf(celsius,3, 1, tempstring);        // convert float to char 
    client.publish(TOPICO_PUB, tempstring);   // send char  
  }
}

void setup() {
  initSerial();
  InitOutput();
  initWiFi();
  initMQTT();
 }

void loop() {
  VerificaConexWiFiyMQTT();
  EnviaTempMQTT();
  client.loop();
}