#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Ping.h>

// Update these with values suitable for your network.

const char* ssid = "SSID"; // Wifi SSID
const char* password = "XXXXX"; // Wifi Password
const char* mqtt_server = "X.X.X.X"; //IP
const unsigned int writeInterval = 1000;   // write interval (in ms)

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE  (50)
#define verde 33
#define amarillo 26
#define rojo 14
#define uS_TO_S_FACTOR 1000000  //Convierte a segundos

char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    count = count +1;
    if (count>=25){
      esp_sleep_enable_timer_wakeup(10 * uS_TO_S_FACTOR); // Se duerme 2 segundos si es que hay un problema al intentar conectarse a la red. 
      esp_deep_sleep_start();
    }
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
  }
  //Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    int count2 = 0;
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("FI/SALON/CO2/2");   // Tópico
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds. Restarting.");
      esp_sleep_enable_timer_wakeup(10 * uS_TO_S_FACTOR); // Se duerme 10 segundos si es que hay un problema y en el envío de datos.
      esp_deep_sleep_start();
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(verde, OUTPUT);
  pinMode(rojo, OUTPUT);
  pinMode(amarillo, OUTPUT);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int MQ135_data = analogRead(39);
  snprintf (msg, MSG_BUFFER_SIZE, "%ld", MQ135_data);
  client.publish("FI/SALON/CO2/2", msg);     // Tópico
  if(MQ135_data < 500){
    //Serial.print("Fresh Air: ");
    digitalWrite(verde, HIGH);
    digitalWrite(amarillo, LOW);
    digitalWrite(rojo, LOW);
    
  } else if ( MQ135_data>=500 && MQ135_data < 800){
    //Serial.print("Warning: "); 
    digitalWrite(verde, LOW);
    digitalWrite(amarillo, HIGH);
    digitalWrite(rojo, LOW);
    
  } else {
    //Serial.print("Poor Air: "); 
    digitalWrite(verde, LOW);
    digitalWrite(amarillo, LOW);
    digitalWrite(rojo, HIGH);
  }
    //Serial.print(MQ135_data);
    //Serial.println(" PPM");
  delay(writeInterval); // delay
  esp_sleep_enable_timer_wakeup(6 * uS_TO_S_FACTOR);//obtener los segundos con la formula seg=[(1-ciclodetrabajo)*6]/ciclodetrabajo -- consideramos que 6seg se tarda en realizar una medicion
  esp_deep_sleep_start();
}
