#include <WiFi.h>
#include <WebServer.h>

// Credenciales de la red WiFi
const char* SSID_RED = "IZZI-5AFB";
const char* PAW_RED  = "C60B5MPYFSRE";

WebServer server(80);

// Variables para el control de WiFi asíncrono
unsigned long tiempoUltimoIntentoWifi = 0;
const unsigned long INTERVALO_RECONEXION = 5000; 
bool wifiConectadoAnteriormente = false;

// Definición de pines para los LEDs del semáforo
const int PIN_ROJO = 25;
const int PIN_VERDE = 26;
const int PIN_AMARILLO = 27;
const int PIN_BTN_INICIO = 32;
const int PIN_BTN_PARO = 33;
const int PIN_LED_SISTEMA = 2;

// Estados posibles del semáforo
enum EstadoSemaforo { ROJO, VERDE, AMARILLO };
EstadoSemaforo estadoActual = ROJO;

// Variables de control de estado del sistema
bool sistemaActivo = false;
bool ultimoEstadoBotonInicio = HIGH;
bool ultimoEstadoBotonParo = HIGH;

// Variables de temporización del semáforo
unsigned long tiempoUltimoCambioSemaforo = 0;
unsigned long tiempoAcumuladoEstado = 0;
const unsigned long DURACION_ROJO = 12000;
const unsigned long DURACION_VERDE = 10000;
const unsigned long DURACION_AMARILLO = 3000;

unsigned long tiempoUltimoParpadeoAmarillo = 0;
bool estadoAmarilloParpadeo = LOW;
unsigned long tiempoUltimoCambioSistema = 0;
int pasoSecuenciaSistema = 0;

// RUTAS PARA LA INTERFAZ DE PYTHON

void manejarRutaConectar() {
  server.send(200, "text/plain", "ESP32 Conectado Exitosamente");
  Serial.println("[Servidor] Python conectado.");
}

void manejarRutaDesconectar() {
  server.send(200, "text/plain", "ESP32 Desconectado");
  Serial.println("[Servidor] Python desconectado.");
}

// Envía el texto exacto que tu script de Python espera leer ("ROJO", "VERDE", "AMARILLO")
void manejarRutaEstado() {
  if (!sistemaActivo) {
    server.send(200, "text/plain", "APAGADO");
    return;
  }
  
  switch (estadoActual) {
    case ROJO:     server.send(200, "text/plain", "ROJO");     break;
    case VERDE:    server.send(200, "text/plain", "VERDE");    break;
    case AMARILLO: server.send(200, "text/plain", "AMARILLO"); break;
  }
}

void inicializarWiFi() {
  Serial.println("\n--- Configurando WiFi ---");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_RED, PAW_RED);
  Serial.print("Conectando a: ");
  Serial.println(SSID_RED);  
}

void verificarConexionWiFi() {
  unsigned long tiempoActual = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiConectadoAnteriormente) {
      Serial.println("\n[WiFi] ¡Conexión establecida con éxito!");
      Serial.print("[WiFi] Dirección IP asignada: ");
      Serial.println(WiFi.localIP());
      wifiConectadoAnteriormente = true;
    }
  } else {
    if (tiempoActual - tiempoUltimoIntentoWifi >= INTERVALO_RECONEXION) {
      tiempoUltimoIntentoWifi = tiempoActual;
      if (wifiConectadoAnteriormente) {
        Serial.println("\n[WiFi] Alerta: Conexión perdida. Intentando reconectar...");
        wifiConectadoAnteriormente = false;
        WiFi.begin(SSID_RED, PAW_RED); 
      } else {
        Serial.print("."); 
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_ROJO, OUTPUT);
  pinMode(PIN_VERDE, OUTPUT);
  pinMode(PIN_AMARILLO, OUTPUT);
  pinMode(PIN_LED_SISTEMA, OUTPUT);
  pinMode(PIN_BTN_INICIO, INPUT_PULLUP);
  pinMode(PIN_BTN_PARO, INPUT_PULLUP);

  apagarSemaforo();
  digitalWrite(PIN_LED_SISTEMA, LOW);
  
  inicializarWiFi();
  Serial.println("Sistema inicializado. Presione INICIO en el circuito físico para activar.");

  // Declaración de las tres rutas requeridas por tu interfaz PyQt6
  server.on("/conectar", manejarRutaConectar);
  server.on("/desconectar", manejarRutaDesconectar);
  server.on("/estado", manejarRutaEstado);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  verificarConexionWiFi(); 
  server.handleClient(); 
  leerBotones();

  if (sistemaActivo) {
    ejecutarSemaforo();
    ejecutarLedSistema();
  } else {
    digitalWrite(PIN_LED_SISTEMA, LOW);
    tiempoUltimoCambioSistema = millis();
    pasoSecuenciaSistema = 0;
  }
}

void leerBotones() {
  bool lecturaInicio = digitalRead(PIN_BTN_INICIO);
  bool lecturaParo = digitalRead(PIN_BTN_PARO);

  if (lecturaInicio == LOW && ultimoEstadoBotonInicio == HIGH) {
    delay(50); 
    if (digitalRead(PIN_BTN_INICIO) == LOW) {
      if (!sistemaActivo) {
        sistemaActivo = true;
        tiempoUltimoCambioSemaforo = millis() - tiempoAcumuladoEstado;
        tiempoUltimoCambioSistema = millis();
        Serial.println(">> Sistema REANUDADO / INICIADO");
      }
    }
  }
  ultimoEstadoBotonInicio = lecturaInicio;

  if (lecturaParo == LOW && ultimoEstadoBotonParo == HIGH) {
    delay(50); 
    if (digitalRead(PIN_BTN_PARO) == LOW) {
      if (sistemaActivo) {
        sistemaActivo = false;
        tiempoAcumuladoEstado = millis() - tiempoUltimoCambioSemaforo;
        apagarSemaforo();
        Serial.println(">> Sistema DETENIDO");
      }
    }
  }
  ultimoEstadoBotonParo = lecturaParo;
}

void ejecutarSemaforo() {
  unsigned long tiempoActual = millis();
  unsigned long tiempoTranscurrido = tiempoActual - tiempoUltimoCambioSemaforo;

  switch (estadoActual) {
    case ROJO:
      digitalWrite(PIN_ROJO, HIGH);
      digitalWrite(PIN_VERDE, LOW);
      digitalWrite(PIN_AMARILLO, LOW);
      if (tiempoTranscurrido >= DURACION_ROJO) {
        estadoActual = VERDE;
        tiempoUltimoCambioSemaforo = tiempoActual;
        tiempoAcumuladoEstado = 0;
      }
      break;

    case VERDE:
      digitalWrite(PIN_ROJO, LOW);
      digitalWrite(PIN_VERDE, HIGH);
      digitalWrite(PIN_AMARILLO, LOW);
      if (tiempoTranscurrido >= DURACION_VERDE) {
        estadoActual = AMARILLO;
        tiempoUltimoCambioSemaforo = tiempoActual;
        tiempoAcumuladoEstado = 0;
        tiempoUltimoParpadeoAmarillo = tiempoActual;
        estadoAmarilloParpadeo = HIGH; 
      }
      break;

    case AMARILLO:
      digitalWrite(PIN_ROJO, LOW);
      digitalWrite(PIN_VERDE, LOW);
      if (tiempoActual - tiempoUltimoParpadeoAmarillo >= 500) {
        estadoAmarilloParpadeo = !estadoAmarilloParpadeo;
        tiempoUltimoParpadeoAmarillo = tiempoActual;
      }
      digitalWrite(PIN_AMARILLO, estadoAmarilloParpadeo);
      if (tiempoTranscurrido >= DURACION_AMARILLO) {
        estadoActual = ROJO;
        tiempoUltimoCambioSemaforo = tiempoActual;
        tiempoAcumuladoEstado = 0;
      }
      break;
  }
}

void ejecutarLedSistema() {
  unsigned long tiempoActual = millis();
  unsigned long tiempoTranscurrido = tiempoActual - tiempoUltimoCambioSistema;

  if (pasoSecuenciaSistema < 6) {
    unsigned long tiempoLimite = (pasoSecuenciaSistema % 2 == 0) ? 600 : 400;
    if (tiempoTranscurrido >= tiempoLimite) {
      pasoSecuenciaSistema++;
      tiempoUltimoCambioSistema = tiempoActual;
    }
    digitalWrite(PIN_LED_SISTEMA, (pasoSecuenciaSistema % 2 == 0 && pasoSecuenciaSistema < 6));
  } 
  else if (pasoSecuenciaSistema == 6) { 
    digitalWrite(PIN_LED_SISTEMA, LOW);
    if (tiempoTranscurrido >= 2000) {
      pasoSecuenciaSistema = 0; 
      tiempoUltimoCambioSistema = tiempoActual;
    }
  }
}

void apagarSemaforo() {
  digitalWrite(PIN_ROJO, LOW);
  digitalWrite(PIN_VERDE, LOW);
  digitalWrite(PIN_AMARILLO, LOW);
}
