>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CODIGO ESP32<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

# Semáforo Inteligente con ESP32 y Servidor Web HTTP

Este proyecto implementa un sistema de control para un semáforo automatizado utilizando un microcontrolador ESP32. El dispositivo actúa como un servidor web asíncrono que expone API endpoints en formato de texto plano para interactuar con una interfaz gráfica externa desarrollada en Python (PyQt6).

## Características principales

*   **Control Físico Completo:** Botones independientes de inicio (reanudar) y paro (pausa) con lógica antirrebote (*debounce*).
*   **Temporización Asíncrona:** Ciclos de luces calculados mediante `millis()`, evitando el bloqueo del código por retardos (`delay()`).
*   **Conexión WiFi Resiliente:** Reconexión automática en segundo plano cada 5 segundos si se pierde el acceso a la red.
*   **Indicador de Estado:** Secuencia intermitente especializada en el LED integrado del sistema para validar el correcto funcionamiento.
*   **API REST Ligera:** Tres rutas HTTP optimizadas para la lectura remota del estado del semáforo.

## Requisitos de Hardware

*   1x Placa de desarrollo ESP32 
*   1x LED Rojo, 1x LED Verde, 1x LED Amarillo.
*   1x LED de estado del sistema (Pin 2 / Integrado).
*   2x Pulsadores (Botón Inicio y Botón Paro).
*   Resistencias adecuadas para los LEDs y configuraciones de pull-up.

## Asignación de Pines (GPIO)


| Componente | Pin ESP32 | Configuración |
| :--- | :--- | :--- |
| LED Rojo | `GPIO 25` | Salida (Digital) |
| LED Verde | `GPIO 26` | Salida (Digital) |
| LED Amarillo | `GPIO 27` | Salida (Digital) |
| LED Sistema | `GPIO 2` | Salida (Digital) |
| Botón Inicio | `GPIO 32` | Entrada con `INPUT_PULLUP` |
| Botón Paro | `GPIO 33` | Entrada con `INPUT_PULLUP` |

## Parámetros del Ciclo de Semáforo

*   **Fase Roja:** 12 segundos (Estático).
*   **Fase Verde:** 10 segundos (Estático).
*   **Fase Amarilla:** 3 segundos (Parpadeo intermitente cada 500 ms).

## Endpoints de la API (Puerto 80)

La interfaz en Python consulta periódicamente las siguientes rutas URL:

1.  `GET /conectar`: Registra y confirma el enlace inicial con el script de Python. Retorna `ESP32 Conectado Exitosamente`.
2.  `GET /desconectar`: Libera la sesión de control remoto. Retorna `ESP32 Desconectado`.
3.  `GET /estado`: Devuelve el estado actual de la lógica en mayúsculas: `ROJO`, `VERDE`, `AMARILLO` o `APAGADO`.

## Configuración e Instalación

1.  Abre el código en el **IDE de Arduino**.
2.  Asegúrate de tener instalado el paquete de tarjetas ESP32 en el Gestor de Tarjetas.
3.  Modifica las constantes de red con tus credenciales locales:
    ```cpp
    const char* SSID_RED = "TU_SSID_WIFI";
    const char* PAW_RED  = "TU_CONTRASEÑA_WIFI";
    ```
4.  Selecciona tu placa ESP32 y el puerto COM correcto.
5.  Sube el programa al microcontrolador.
6.  Abre el Monitor Serie a **115200 baudios** para obtener la dirección IP asignada por el router.



>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CODIGO PyQT6<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

# Monitor de Semáforo ESP32 

Aplicación de escritorio desarrollada en **Python** y **PyQt6** que funciona como interfaz gráfica (GUI) para monitorear en tiempo real el estado de un semáforo controlado por un módulo **ESP32** a través de la red local.

## Características

*   **Renderizado Dinámico:** Luces de semáforo dibujadas con gráficos vectoriales (`QPainter`).
*   **Monitoreo en Tiempo Real:** Actualización automática del estado cada 500ms mediante peticiones HTTP.
*   **Gestión de Conexión:** Manejo de tiempos de espera (`timeout`) para evitar congelamientos en la interfaz.
*   **Alertas Integradas:** Notificaciones de error si el dispositivo no es accesible en la red.

## Requisitos Previos

Antes de ejecutar la aplicación, asegúrate de tener instalado Python 3.8 o superior y las siguientes dependencias:

```bash
pip install PyQt6 requests
```

## Estructura de Archivos

Para que la interfaz cargue correctamente todos los componentes visuales, tu carpeta de proyecto debe lucir así:

```text
├── tu_script.py      # Código principal de la aplicación
└── GIT.png           # Logotipo o imagen institucional (120x60 px aprox.)
```

## Instrucciones de Uso

1.  **Conectar el ESP32:** Enciende tu placa ESP32 y asegúrate de que esté ejecutando el servidor web correspondiente.
2.  **Misma Red Local:** Confirma que tu computadora y el ESP32 estén conectados a la misma red Wi-Fi.
3.  **Ejecutar la App:**
    ```bash
    python tu_script.py
    ```
4.  **Establecer Enlace:** Introduce la dirección IP asignada al ESP32, el puerto (por defecto `80`) y presiona **Conectar**.

## Endpoints HTTP Utilizados

La aplicación interactúa con el ESP32 mediante las siguientes rutas:
*   `GET /conectar`: Valida el enlace inicial con el hardware.
*   `GET /desconectar`: Informa al servidor el fin de la sesión de monitoreo.

