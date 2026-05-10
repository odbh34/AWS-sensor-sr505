# Sistema de Seguridad con Sensor SR505 + ESP32 + AWS

**Alumno:** Oscar David Barrientos Huillca - 225419

Sistema IoT de monitoreo de movimiento en tiempo real. El ESP32 lee el sensor PIR SR505 y envía los datos vía WiFi a un servidor Flask en AWS EC2, donde se almacenan en PostgreSQL y se visualizan en una interfaz web con gráfico de señal digital.

---

## Interfaz Web

![Página principal](image.png)

> Gráfico de señal digital en tiempo real — rojo = movimiento detectado, verde = sin movimiento.

---

## Estructura del Proyecto

```
sensorAWS/
├── platformIO/               # Proyecto PlatformIO (firmware ESP32)
│   ├── src/
│   │   └── main.cpp          # Código fuente del ESP32
│   └── platformio.ini        # Configuración de la placa y framework
│
└── sensor_proyecto/          # Servidor Flask (AWS EC2)
    ├── app.py                # API REST principal
    └── templates/
        ├── index.html        # Página principal con gráfico
        └── historial.html    # Página de historial de detecciones
```

---

## Arquitectura del Sistema

```
SR505 → ESP32 (WiFi) ──HTTP POST──▶ Flask (EC2) ──▶ PostgreSQL
                                         │
                     Navegador ◀── HTML/JSON
```

| Capa | Tecnología |
|---|---|
| Hardware | ESP32 WROOM-32 + Sensor SR505 |
| Firmware | C++ con Arduino Framework (PlatformIO) |
| Backend | Python 3.12 + Flask |
| Base de datos | PostgreSQL 14 |
| Servidor | AWS EC2 Ubuntu 24.04 |
| Protocolo | HTTP REST sobre WiFi |

---

## Conexión del Hardware

![ESP32 con SR505](sr505.png)

| Pin SR505 | Pin ESP32 | Función |
|---|---|---|
| VCC | VIN (5V) | Alimentación |
| GND | GND | Tierra |
| OUT | GPIO13 | Señal digital |

> El LED integrado en **GPIO2** parpadea al arrancar y se enciende cuando hay movimiento.

---

## Endpoints de la API

| Endpoint | Método | Descripción |
|---|---|---|
| `/` | GET | Página principal con gráfico |
| `/historial` | GET | Página de historial |
| `/sensor` | POST | Recibe datos del ESP32 |
| `/estado` | GET | Estado actual en JSON |
| `/historial_json` | GET | Lista de detecciones (filtrable por fecha) |
| `/conteo_hoy` | GET | Total de detecciones del día |
| `/control` | POST | Enciende o apaga el sensor |
| `/estado_control` | GET | Estado actual del control |

---

## Historial de Detecciones

![Historial](historial.png)

Página separada accesible desde el botón **Ver historial**. Permite filtrar detecciones por fecha y muestra el total de registros encontrados.

---

## Instalación y Despliegue

### Requisitos del servidor (EC2)
- Ubuntu 24.04 LTS
- Python 3.12+
- PostgreSQL 14+
- Puerto 5000 abierto en el Security Group

### 1. Preparar el entorno en EC2

```bash
sudo apt update
sudo apt install python3-full python3-venv postgresql -y
sudo timedatectl set-timezone America/Lima

mkdir ~/sensor_proyecto && cd ~/sensor_proyecto
mkdir templates
python3 -m venv venv
source venv/bin/activate
pip install flask psycopg2-binary
```

### 2. Configurar PostgreSQL

```bash
sudo -u postgres psql -c "CREATE DATABASE sensordb;"
sudo -u postgres psql -c "CREATE USER sensoruser WITH PASSWORD 'tu_password';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE sensordb TO sensoruser;"
sudo -u postgres psql -d sensordb -c "GRANT ALL ON SCHEMA public TO sensoruser;"
```

### 3. Configurar el servicio systemd

Crear `/etc/systemd/system/sensor.service`:

```ini
[Unit]
Description=Sensor SR505 Flask App
After=network.target

[Service]
User=ubuntu
WorkingDirectory=/home/ubuntu/sensor_proyecto
Environment="PATH=/home/ubuntu/sensor_proyecto/venv/bin"
ExecStart=/home/ubuntu/sensor_proyecto/venv/bin/python3 app.py
Restart=always

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable sensor
sudo systemctl start sensor
```

### 4. Configurar el firmware (ESP32)

En `platformIO/src/main.cpp` editar estas líneas:

```cpp
const char* ssid      = "TU_WIFI";
const char* password  = "TU_PASSWORD";
const char* serverUrl = "http://TU_IP_EC2:5000/sensor";
const char* controlUrl = "http://TU_IP_EC2:5000/estado_control";
```

Luego en PlatformIO: **Build** → **Upload**.

---

## 🛠️ Comandos útiles

```bash
# Ver estado del servicio
sudo systemctl status sensor

# Ver logs en tiempo real
journalctl -u sensor -f

# Reiniciar servicio
sudo systemctl restart sensor

# Ver últimas detecciones en la BD
sudo -u postgres psql -d sensordb -c \
  "SELECT * FROM detecciones ORDER BY id DESC LIMIT 10;"

# Limpiar tabla de detecciones
sudo -u postgres psql -d sensordb -c \
  "TRUNCATE TABLE detecciones RESTART IDENTITY;"
```

---

## Características

- Gráfico de señal digital en tiempo real (últimos 8 minutos)
- Registro de detecciones con timestamp en hora peruana
- Historial consultable con filtro por fecha
- Mecanismo anti-rebote (cooldown de 5 segundos)
- Servicio systemd para disponibilidad continua
- Control del sensor desde la interfaz web

---

## Limitaciones conocidas

- La comunicación no usa HTTPS (sin cifrado)
- No hay autenticación de usuarios
- El gráfico se reinicia al recargar la página (datos en memoria)
- Flask corre en modo desarrollo (no apto para alta concurrencia)