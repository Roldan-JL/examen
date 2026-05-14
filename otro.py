import sys
import requests

from PyQt6.QtWidgets import (
    QApplication,
    QWidget,
    QLabel,
    QPushButton,
    QLineEdit,
    QVBoxLayout,
    QHBoxLayout,
    QMessageBox
)

# SE AGREGÓ QPixmap AQUÍ
from PyQt6.QtGui import QPainter, QColor, QFont, QPixmap
from PyQt6.QtCore import Qt, QTimer


# ------------------CLASE PARA DIBUJAR LAS LUCES------------------------------

class LuzSemaforo(QWidget):

    def __init__(self, color):
        super().__init__()
        self.color = color
        self.encendido = False
        self.setFixedSize(90, 90)

    def setEstado(self, estado):
        self.encendido = estado
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        if self.encendido:
            painter.setBrush(QColor(self.color))
        else:
            painter.setBrush(QColor(80, 80, 80))

        painter.drawEllipse(5, 5, 80, 80)


# ----------------------VENTANA PRINCIPAL----------------------------

class Ventana(QWidget):

    def __init__(self):
        super().__init__()
        self.conectado = False
        self.initUI()

# -----------------------Timer para actualizar estado del semáforo cada 500ms---------------
        self.timer = QTimer()
        self.timer.timeout.connect(self.actualizarEstado)

    
#------------------------ INTERFAZ----------------------------
   
    def initUI(self):
        self.setWindowTitle("Monitor Semáforo ESP32")
        self.setGeometry(200, 200, 650, 440) # Se aumentó la altura a 440 para dar espacio a la imagen

     
#--------------------- MENSAJE SUPERIOR-----------------------------------------
        
        self.labelMensaje = QLabel("")
        self.labelMensaje.setStyleSheet(
            "color: green; font-size: 16px; font-weight: bold;"
        )

       
# ------------------------------LUCES---------------------------------
       
        self.luzRoja = LuzSemaforo("red")
        self.luzAmarilla = LuzSemaforo("yellow")
        self.luzVerde = LuzSemaforo("green")

        lucesLayout = QVBoxLayout()
        lucesLayout.addWidget(self.luzRoja)
        lucesLayout.addWidget(self.luzAmarilla)
        lucesLayout.addWidget(self.luzVerde)

#------- CAMPOS IP Y PUERTO----------------------------------
       
        self.inputIP = QLineEdit()
        self.inputPuerto = QLineEdit()

        self.inputIP.setPlaceholderText("Ej: 10.106.120.144")
        self.inputPuerto.setText("80")

        self.inputIP.setFixedWidth(180)
        self.inputPuerto.setFixedWidth(180)

        labelIP = QLabel("Dirección IP")
        labelPuerto = QLabel("Puerto")

        labelIP.setStyleSheet(
            "background-color: #8BC34A; color: white; padding: 8px; font-weight: bold; border-radius: 3px;"
        )
        labelPuerto.setStyleSheet(
            "background-color: #8BC34A; color: white; padding: 8px; font-weight: bold; border-radius: 3px;"
        )

        ipLayout = QHBoxLayout()
        ipLayout.addWidget(labelIP)
        ipLayout.addWidget(self.inputIP)

        puertoLayout = QHBoxLayout()
        puertoLayout.addWidget(labelPuerto)
        puertoLayout.addWidget(self.inputPuerto)

       
#----------------------- BOTÓN CONEXIÓN---------------------------------------------
      
        self.botonConexion = QPushButton("Conectar")
        self.botonConexion.setFixedHeight(50)
        self.botonConexion.setStyleSheet(
            """
            QPushButton {
                background-color: #8BC34A;
                color: white;
                font-size: 16px;
                font-weight: bold;
                border-radius: 10px;
            }
            QPushButton:hover {
                background-color: #7CB342;
            }
            """
        )
        self.botonConexion.clicked.connect(self.controlConexion)

#------------------- PANEL DERECHO----------------------------------------
      
        derechaLayout = QVBoxLayout()

        titulo = QLabel("Monitor de Estado")
        titulo.setFont(QFont("Arial", 16, QFont.Weight.Bold))
        titulo.setAlignment(Qt.AlignmentFlag.AlignCenter)

        derechaLayout.addWidget(titulo)
        derechaLayout.addSpacing(20)
        derechaLayout.addLayout(ipLayout)
        derechaLayout.addLayout(puertoLayout)
        derechaLayout.addSpacing(30)
        derechaLayout.addWidget(self.botonConexion)
        
# ----------------- SECCIÓN DE IMAGEN -----------------

        self.labelImagen = QLabel()
        pixmap = QPixmap("GIT.png")
        
# Redimensiona la imagen a 120x60 píxeles manteniendo la proporción
        pixmap_escalado = pixmap.scaled(120, 60, Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation)
        self.labelImagen.setPixmap(pixmap_escalado)
        
 # Alineación hacia la derecha y hacia arriba
        self.labelImagen.setAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignTop)
        
# Se añade al principio del panel derecho
        derechaLayout.addWidget(self.labelImagen)
        derechaLayout.addSpacing(10) 
 

#-------------------- LAYOUT PRINCIPAL--------------------------------------
     
        principal = QHBoxLayout()
        principal.addLayout(lucesLayout)
        principal.addSpacing(40)
        principal.addLayout(derechaLayout)

        layoutFinal = QVBoxLayout()
        layoutFinal.addWidget(self.labelMensaje)
        layoutFinal.addLayout(principal)

        self.setLayout(layoutFinal)

 # ------------------MENSAJES INFORMATIVOS---------------------------
  
    def mostrarMensaje(self, texto):
        self.labelMensaje.setText(texto)
        QTimer.singleShot(3000, lambda: self.labelMensaje.setText(""))

 
    def controlConexion(self):
        ip = self.inputIP.text().strip()
        puerto = self.inputPuerto.text().strip()
# ----------------CONECTAR / DESCONECTAR-----------------------------------------------
    
        if not ip:
            QMessageBox.warning(self, "Falta Información", "Por favor, ingresa la dirección IP del ESP32.")
            return

        url = f"http://{ip}:{puerto}"

        try:
#-------------------- CONECTAR ---------------------------------------------------
            if not self.conectado:
# Se añade timeout para evitar que la app se congele si la IP no responde
                respuesta = requests.get(url + "/conectar", timeout=3)
                
                if respuesta.status_code == 200:
                    self.conectado = True
                    self.botonConexion.setText("Desconectar")
                    self.botonConexion.setStyleSheet(
                        "QPushButton { background-color: #F44336; color: white; font-size: 16px; font-weight: bold; border-radius: 10px; }"
                        "QPushButton:hover { background-color: #D32F2F; }"
                    )
                    self.timer.start(500)  # Inicia la consulta del estado cada 500ms
                    self.mostrarMensaje(f"Conectado: {respuesta.text}")

# -------------------------------DESCONECTAR--------------------------------------------
            else:
                requests.get(url + "/desconectar", timeout=3)
                self.finalizarConexionLocal()
                self.mostrarMensaje("Se ha desconectado del servidor.")

        except requests.exceptions.RequestException:
            QMessageBox.critical(
                self, 
                "Error de Conexión", 
                f"No se pudo establecer comunicación con el equipo de destino en {url}.\n\n"
                "Verifica que:\n"
                "1. El ESP32 esté encendido.\n"
                "2. Tu computadora y el ESP32 estén en la misma red Wi-Fi.\n"
                "3. La dirección IP sea correcta."
            )

    def finalizarConexionLocal(self):
        self.conectado = False
        self.botonConexion.setText("Conectar")
        self.botonConexion.setStyleSheet(
            "QPushButton { background-color: #8BC34A; color: white; font-size: 16px; font-weight: bold; border-radius: 10px; }"
            "QPushButton:hover { background-color: #7CB342; }"
        )
        self.timer.stop()
        self.apagarTodo()

   
# ----------------APAGAR TODAS LAS LUCES----------------------------------------------
    
    def apagarTodo(self):
        self.luzRoja.setEstado(False)
        self.luzAmarilla.setEstado(False)
        self.luzVerde.setEstado(False)

    
# ---------------ACTUALIZAR ESTADO DESDE ESP32--------------------------------------------
    
    def actualizarEstado(self):
        ip = self.inputIP.text().strip()
        puerto = self.inputPuerto.text().strip()

        url = f"http://{ip}:{puerto}/estado"

        try:
 #--------- Se añade timeout corto para que las fluctuaciones de red no congelen los gráficos
            respuesta = requests.get(url, timeout=1)
            estado = respuesta.text.strip()

            self.apagarTodo()

            if estado == "ROJO":
                self.luzRoja.setEstado(True)
            elif estado == "AMARILLO":
                self.luzAmarilla.setEstado(True)
            elif estado == "VERDE":
                self.luzVerde.setEstado(True)
        except requests.exceptions.RequestException:
            pass

if __name__ == "__main__":
    app = QApplication(sys.argv)
    ventana = Ventana()
    ventana.show()
    sys.exit(app.exec())
