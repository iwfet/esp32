#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>


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
  listDir();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
   server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

 server.on("/enviar", HTTP_POST, [](AsyncWebServerRequest* request){
    if(request->hasParam("mensagem", true)){
      String mensagem = request->getParam("mensagem", true)->value();
        String usuario = request->getParam("usuario", true)->value();
      
      StaticJsonDocument<200> jsonDoc;
      jsonDoc["mensagem"] = mensagem;
      jsonDoc["usuario"] = usuario;

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

  server.begin();
}

void loop() {
  webSocket.loop();
}



bool listDir() {
  File root = SPIFFS.open("/"); 
  if (!root)   {
    Serial.println(" - falha ao abrir o diretório");
    return false;
  }
  File file = root.openNextFile();
  int qtdFiles = 0;
  while (file) { 
    Serial.print("  FILE : ");
    Serial.print(file.name()); // Imprime o nome do arquivo
    Serial.print("\tSIZE : ");
    Serial.println(file.size()); // Imprime o tamanho do arquivo
    qtdFiles++; // Incrementa a variável de quantidade de arquivos
    file = root.openNextFile();
  }
  if (qtdFiles == 0)  // Se após a visualização de todos os arquivos do diretório
    //                      não houver algum arquivo, ...
  {
    // Avisa o usuário que não houve nenhum arquivo para ler e retorna false.
    Serial.print(" - Sem arquivos para ler. Crie novos arquivos pelo menu ");
    Serial.println("principal, opção 2.");
    return false;
  }
  return true; // retorna true se não houver nenhum erro
}
