// Configuración datos WiFi y servidor MQTT

const char *ssid[] = {"wifi1", "wifi2", ""};                                                                                                             // SSID de la conexión WiFi
const char *password[] = {"pass1", "pass2", ""}; // Password conexión WiFi

const char *hostname = "ESPAlerta1"; // Hostname dispositvio en la LAN

const char *mqtt_server = "190.190.190.190";  // Dirección IP del servidor MQTT
const uint16_t mqtt_server_port = 1883; // Puerto de conexión al servidor
const char *mqtt_User = "USER_MQTT";
const char *mqtt_Password = "PASS_MQTT";

String clientId = "ALERTA01-";

#define topic_ip "casa/alerta1/ip"           // Dirección IP del dispositivo
#define topic_alive "casa/alerta1/alive"     // Tiempo activo
#define topic_nivel "casa/alerta1/nivel"     // Nivel de alarma pre-configurado
#define topic_mensaje "casa/alerta1/mensaje" // Mostrar un mensaje
#define FRECUENCIA 60000                     // Frecuencia publicación datos "alive" en ms
#define DEBUG