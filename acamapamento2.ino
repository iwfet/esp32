#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>

AsyncWebServer server(80);
WebSocketsServer webSocket(8080);

const char* ssid = "OsCheirosos";       // Nome da rede Wi-Fi
const char* password = "OsCheirosos";  // Senha da rede Wi-Fi
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Endereço IP do Access Point: ");
  Serial.println(IP);
  if (!SPIFFS.begin()) {
    Serial.println("Falha ao montar sistema de arquivos SPIFFS");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index2.html", "text/html");
  });

 server.on("/enviar", HTTP_POST, [](AsyncWebServerRequest* request){
    if(request->hasParam("mensagem", true)){
      String mensagem = request->getParam("mensagem", true)->value();
      
      StaticJsonDocument<200> jsonDoc;
      jsonDoc["mensagem"] = mensagem;

      String jsonStr;
      serializeJson(jsonDoc, jsonStr);
      
      webSocket.broadcastTXT(jsonStr);
      
      request->send(200, "text/plain", "Mensagem enviada com sucesso");
    }
    else{
      request->send(400, "text/plain", "Parâmetro 'mensagem' ausente");
    }
  });

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  server.begin();
}

void loop() {
  webSocket.loop();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from IP: %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] Received: %s\n", num, payload);
      // Aqui você pode enviar mensagens de volta para o cliente WebSocket, se necessário
      // Exemplo: webSocket.sendTXT(num, "Mensagem de resposta");
      break;
  }
}