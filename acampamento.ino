#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);

const char* ssid = "NomeDaRede";       // Nome da rede Wi-Fi
const char* password = "SenhaDaRede";  // Senha da rede Wi-Fi

void setup() {
  Serial.begin(115200);

  // Inicializar o modo Wi-Fi como Access Point
  WiFi.mode(WIFI_AP);

  // Configurar as informações da rede Wi-Fi
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

  server.on("/mensagem", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (request->contentType() == "application/json") {
      // Ler o JSON recebido
      DynamicJsonDocument doc(1024);  // Tamanho máximo do JSON
      deserializeJson(doc, request->getContent());
      
      // Armazenar o JSON em um array
      File file = SPIFFS.open("/data.json", "a");  // Abre o arquivo em modo de adição (append)
      if (!file) {
        Serial.println("Falha ao abrir o arquivo JSON");
        request->send(500);
        return;
      }
      
      serializeJson(doc, file);
      file.println();  // Adiciona uma nova linha após cada JSON para facilitar a leitura posteriormente
      file.close();
      
      request->send(200);
    } else {
      request->send(400);
    }
  });

  server.on("/arquivo", HTTP_GET, [](AsyncWebServerRequest* request){
  File file = SPIFFS.open("/data.json", "r"); // Abre o arquivo JSON para leitura
  if (!file) {
    Serial.println("Falha ao abrir o arquivo JSON");
    request->send(404); // Retorna código 404 (Not Found) se o arquivo não for encontrado
    return;
  }

  size_t fileSize = file.size();
  if (fileSize > 0) {
    std::unique_ptr<char[]> buf(new char[fileSize]);

    file.readBytes(buf.get(), fileSize);

    request->send(200, "application/json", buf.get(), fileSize); // Retorna o conteúdo do arquivo JSON como resposta
  } else {
    request->send(500); // Retorna código 500 (Internal Server Error) se o arquivo estiver vazio
  }

  file.close();
});


  // Inicializar servidor
  server.begin();
}

void loop() {
  // O código principal do seu programa pode ser colocado aqui
}
