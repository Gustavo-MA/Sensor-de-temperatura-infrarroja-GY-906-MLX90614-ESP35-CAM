//Bibliotecas
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//Datos de WiFi
//const char* ssid = "D-GEC92";  // Aquí debes poner el nombre de tu red, en nuestro caso generamos una red local
//const char* password = "D-064689413";  // Aquí debes poner la contraseña de tu red
const char* ssid = "Gustavo";  // Aquí debes poner el nombre de tu red, en nuestro caso generamos una red local
const char* password = "123456789";  // Aquí debes poner la contraseña de tu red
//Datos del broker MQTT
const char* mqtt_server = "192.168.120.137"; // Si estas en una red local, coloca la IP asignada, en caso contrario, coloca la IP publica
IPAddress server(192,168,120,137);

// Objetos
WiFiClient espClient; // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient); // Este objeto maneja los datos de conexion al broker

// Variables
int flashLedPin = 4;  // Para indicar el estatus de conexión
int statusLedPin = 33; // Para ser controlado por MQTT
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
int data = 0; // Contador
int wait = 1000;  // Indica la espera cada 1 segundos para envío de mensajes MQTT
// Inicialización del programa
void setup() {
  // Iniciar comunicación serial
  Serial.begin (115200);
  Serial.println ("Programa iniciado");
    Wire.begin(14, 12); // SDA, SCL
    mlx.begin();

  //
  pinMode (flashLedPin, OUTPUT);
  pinMode (statusLedPin, OUTPUT);
  digitalWrite (flashLedPin, LOW);
  digitalWrite (statusLedPin, HIGH);

  Serial.println();
  Serial.println();
  Serial.print("Conectar a ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password); // Esta es la función que realiz la conexión a WiFi
 
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión
    digitalWrite (statusLedPin, HIGH);
    delay(500); //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
    digitalWrite (statusLedPin, LOW);
    Serial.print(".");  // Indicador de progreso
    delay (5);
  }
  
  // Cuando se haya logrado la conexión, el programa avanzará, por lo tanto, puede informarse lo siguiente
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Si se logro la conexión, encender led
  if (WiFi.status () > 0){
  digitalWrite (statusLedPin, LOW);
  }
   Serial.println("Linea 88, wifi estatus, se logro la conexion");
  delay (1000); // Esta espera es solo una formalidad antes de iniciar la comunicación con el broker

   Serial.println("Comienza la conexion al broquer");
  // Conexión con el broker MQTT
  client.setServer(server, 1883); // Conectarse a la IP del broker en el puerto indicado
 // client.setCallback(callback); // Activar función de CallBack, permite recibir mensajes MQTT y ejecutar funciones a partir de ellos
  delay(1500);  // Esta espera es preventiva, espera a la conexión para no perder información
Serial.println("Termina la conexion al broquer");
  
  timeLast = millis (); // Inicia el control de tiempo
}// fin del void setup ()

// Cuerpo del programa, bucle principal
void loop() {
  //Serial.println("Cliente conectado, si no reconectar");
  if (!client.connected()) {
    reconnect();  // En caso de que no haya conexión, ejecutar la función de reconexión, definida despues del void setup ()
    Serial.println("Se ha mandado a la funcion reconectar GMA");
  }// fin del if (!client.connected())
  client.loop(); // Esta función es muy importante, ejecuta de manera no bloqueante las funciones necesarias para la comunicación con el broker
  delay(wait);
  timeNow = millis(); // Control de tiempo para esperas no bloqueantes
  if (timeNow - timeLast > wait) { // Manda un mensaje por MQTT cada cinco segundos
   //if (true) {
    timeLast = timeNow; // Actualización de seguimiento de tiempo
    // Obtener temperatura y humedad
  float to=mlx.readObjectTempC();
  float ta=mlx.readAmbientTempC();
  Serial.print(to);
  Serial.println("°C");
  Serial.print("Temperatura ambiente: ");
  Serial.print(ta);
  Serial.println("°C");
  delay(1000);
  
    // Secuencia que se asegura de que la conexión con el sensor exista
    if (isnan(to) || isnan(ta)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    //printLocalTime();
    //String json = "{\"id\":\"Incubadora\",\"temp\":"+String(t)+"}";
    String json = "{\"id\":\"Temperatura\",\"tempAbt\":"+String(ta)+",\"tempObj\":"+String(to)+"}";
    Serial.println(json); // Se imprime en monitor solo para poder visualizar que el string esta correctamente creado
    int str_len = json.length() + 1;//Se calcula la longitud del string
    char char_array[str_len];//Se crea un arreglo de caracteres de dicha longitud
    json.toCharArray(char_array, str_len);//Se convierte el string a char array    
    client.publish("FCaEI/mqtt/GMA/TemAmbObj", char_array); 
    //codigoIoT/flow5/mqtt/clima, o checar si es: codigoIoT/Mor/mqtt/flow4 (checar  *) 
  }// fin del if (timeNow - timeLast > wait)
}// fin del void loop ()

// Funciones de usuario

// Esta función permite tomar acciones en caso de que se reciba un mensaje correspondiente a un tema al cual se hará una suscripción

// Función para reconectarse
void reconnect() {
  // Bucle hasta lograr conexión
  while (!client.connected()) { // Pregunta si hay conexión
    Serial.print("Tratando de contectarse...");
    // Intentar reconexión
    if (client.connect("ESP32CAMClient")) { //Pregunta por el resultado del intento de conexión
      Serial.println("Conectado");
      //client.subscribe("codigoIoT/mqtt"); // Esta función realiza la suscripción al tema
      client.subscribe("FCaEI/mqtt/GMA/TemAmbObj"); // Esta función realiza la suscripción al tema
    }// fin del  if (client.connect("ESP32CAMClient"))
    else {  //en caso de que la conexión no se logre
      Serial.print("Conexion fallida, Error rc=");
      Serial.print(client.state()); // Muestra el codigo de error
      Serial.println(" Volviendo a intentar en 5 segundos");
      // Espera de 5 segundos bloqueante
      delay(1000);
      Serial.println (client.connected ()); // Muestra estatus de conexión
    }// fin del else
  }// fin del bucle while (!client.connected())
}// fin de void reconnect(

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
