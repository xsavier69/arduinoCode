#include <Arduino.h>              // Incluir la biblioteca Arduino para placas ESP32
#include <WiFi.h>                 // Incluir la biblioteca WiFi para placas ESP32
#include <Firebase_ESP_Client.h>  // Incluir la biblioteca del cliente de Firebase
#include <addons/TokenHelper.h>   // Incluir la biblioteca de ayuda de tokens de Firebase
#include <addons/RTDBHelper.h>    // Incluir la biblioteca de ayuda de Firebase RTDB

// Credenciales WiFi
const char* WIFI_SSID = "ola";            // Definir el SSID (nombre de red) WiFi
const char* WIFI_PASSWORD = "123456789";  // Definir la contraseña WiFi

// Credenciales de Firebase
const char* API_KEY = "AIzaSyDDJb7f5qPX8QOKFNfq3dhlrk1yEKN1Y0k";                  // Definir la clave API de Firebase
const char* DATABASE_URL = "https://fir-app-f574f-default-rtdb.firebaseio.com/";  // Definir la URL de la base de datos de Firebase
const char* USER_EMAIL = "chiquito123@boly.uets";                                 // Definir el correo electrónico del usuario de Firebase
const char* USER_PASSWORD = "123456789";                                          // Definir la contraseña del usuario de Firebase

FirebaseData fbdo;      // Crear un objeto FirebaseData para manejar las operaciones de Firebase
FirebaseAuth auth;      // Crear un objeto FirebaseAuth para manejar la autenticación de usuario
FirebaseConfig config;  // Crear un objeto FirebaseConfig para configurar el cliente de Firebase

// Definir los pines de los botones y el número de botones
const int BUTTON_PINS[] = { 27, 26, 25, 33, 32, 35 };
const int NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

// Definir los nombres de las habitaciones correspondientes a los pines de los botones
const String ROOM_NAMES[] = { "sala", "cocina", "comedor", "bano", "dormitorio1", "dormitorio2" };

unsigned long sendDataPrevMillis = 0;  // Variable para almacenar el tiempo anterior en que se envió datos

// Función para configurar la conexión WiFi
void setup_WIFI() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // Conectarse a la red WiFi
  Serial.print("Connecting to Wi-Fi");

  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - ms < 20000)) {  // Esperar a que se establezca la conexión
    Serial.print(".");
    delay(300);
  }

  if (WiFi.status() != WL_CONNECTED) {  // Si la conexión falla, imprimir un mensaje de error
    Serial.println("Failed to connect to Wi-Fi");
    return;
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());  // Imprimir la dirección IP del dispositivo conectado
  Serial.println();
}

// Función para configurar la conexión con Firebase
void setupFirebase() {
  config.api_key = API_KEY;                            // Establecer la clave API de Firebase
  config.database_url = DATABASE_URL;                  // Establecer la URL de la base de datos de Firebase
  auth.user.email = USER_EMAIL;                        // Establecer el correo electrónico del usuario de Firebase
  auth.user.password = USER_PASSWORD;                  // Establecer la contraseña del usuario de Firebase
  config.token_status_callback = tokenStatusCallback;  // Establecer la función de devolución de llamada del estado del token

  Firebase.begin(&config, &auth);  // Inicializar el cliente de Firebase

  if (Firebase.ready()) {  // Comprobar si el cliente de Firebase está listo
    Serial.println("Firebase initialized successfully!");
  } else {
    Serial.println("Firebase initialization failed!");
    Serial.printf("Firebase error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.reconnectNetwork(true);            // Volver a conectar el cliente de Firebase a la red si es necesario
  fbdo.setBSSLBufferSize(4096, 1024);         // Establecer el tamaño del búfer BSSL para el cliente de Firebase
  fbdo.setResponseSize(2048);                 // Establecer el tamaño de la respuesta para el cliente de Firebase
  Firebase.setDoubleDigits(5);                // Establecer el número de dígitos decimales para valores double
  config.timeout.serverResponse = 10 * 1000;  // Establecer el tiempo de espera de la respuesta del servidor a 10 segundos

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);  // Imprimir la versión del cliente de Firebase
}

void setup() {
  Serial.begin(115200);  // Inicializar la comunicación serial

  setup_WIFI();     // Configurar la conexión WiFi
  setupFirebase();  // Configurar la conexión con Firebase

  for (int i = 0; i < NUM_BUTTONS; i++) {  // Configurar los pines de los botones como entrada con resistencias de pull-up
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  delay(2000);  // Esperar 2 segundos antes de iniciar el bucle principal
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {  // Comprobar si el cliente de Firebase está listo y ha pasado al menos 1 segundo desde el último envío de datos
    for (int i = 0; i < NUM_BUTTONS; i++) {
      int buttonState = digitalRead(BUTTON_PINS[i]);  // Leer el estado del botón
      String buttonPath = "/casa/" + ROOM_NAMES[i];   // Crear la ruta de la base de datos RTDB de Firebase para el botón

      //Mostrar el estado de los botones
      Serial.printf("Button %s state: %d\n", ROOM_NAMES[i].c_str(), buttonState);
      Serial.printf("Set int path: %s\n", buttonPath.c_str());

      if (Firebase.RTDB.setInt(&fbdo, buttonPath.c_str(), buttonState)) {  // Establecer el estado del botón en la base de datos RTDB de Firebase
        Serial.println("Set int... ok");
      } else {
        Serial.printf("Set int... failed, reason: %s\n", fbdo.errorReason().c_str());
      }
    }

    sendDataPrevMillis = millis();  // Actualizar el momento en que se envió los datos por última vez
  }
}