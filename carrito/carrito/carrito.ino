#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> 
#include <Hash.h>
#include <FS.h>

const char* ssid = "mired";
const char* password = "12345678"; 
//Motor Derecha
int OUTPUT4 = 16;
int OUTPUT3 = 5;
//MOTOR IZQUIERDA
int OUTPUT2 = 4;
int OUTPUT1 = 0;
long duracion=0;
//sE DEFINE EL PUERTO 81 PARA WEBSOCKET Y PUERTO 80 PARA PAGINA WEB
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

void setup(void){
  delay(1000);  
  //Velocidad
  Serial.begin(115200);
  //Configuracion de salidas
  pinMode (OUTPUT1, OUTPUT);
  pinMode (OUTPUT2, OUTPUT);
  pinMode (OUTPUT3, OUTPUT);
  pinMode (OUTPUT4, OUTPUT);

  //Establecer la conexión con el AP
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Se establece la conexion y presenta la IP del cliente
  IPAddress myIP = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(myIP);
  //Inicia lectura del archivo
  SPIFFS.begin();
  //Inicia Websocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  //en caso de error 404 no se encuentra
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "Archivo no encontrado");
  });
  //Servidor Web Iniciado
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop(void) {
  //Websocket a la espera de conexiones
  webSocket.loop();
  //Servidor Web a la espera de conexiones
  server.handleClient();
  
}
//Funcion predefinida de un WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch(type) {
    //En caso de que un cliente se desconecte del websocket
    case WStype_DISCONNECTED: {
      Serial.printf("Usuario #%u - Desconectado\n", num);
      break;
    }
    //Cuando un cliente se conecta al websocket presenta la información del cliente conectado, IP y ID
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("Nueva conexión: %d.%d.%d.%d Nombre: %s ID: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);
      break;
    }
    //Caso para recibir información que se enviar vía el servidor Websocket
    case WStype_TEXT: {
       String entrada = "";
       //Se lee la entrada de datos y se concatena en la variable String entrada
      for (int i = 0; i < lenght; i++) {
        entrada.concat((char)payload[i]);
      }
      //Se separan los datos de la posicion X y Y del JoyStick
      String data=entrada;
      if(data){
        int pos = data.indexOf(':');
        long x = data.substring(0, pos).toInt();
        long y = data.substring(pos+1).toInt();
        //Imprime en Monitor Serial
        Serial.print("x:");
        Serial.print(x);
        Serial.print(", y:");
        Serial.println(y);
        //De acuerdo al valor de X y Y del JoyStick, se ejecuta la funcion para habilitar los motores
        if(((x<=50&&x>-50)&&(y<=50&&y>-50))){//PARAR
          parar();
        }else if(x>50&&(y<50||y>-50)){//DERECHA
          derecha();
        }else if(x<-50&&(y<50||y>-50)){//IZQUIERDA
          izquierda();
        }else if(y>50&&(x<50||x>-50)){//ATRAS
          atras();
        }else if(y<-50&&(x<50||x>-50)){//ADELANTE
          adelante();
        }
      }
      break;
    }    
  }
}

//FUNCION PARA IDENTIFICAR EL TIPO DE CONTENIDO DE LOS ARCHIVOS DEL SERVIDOR WEB 
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//FUNCION PARA CARGAR EL ARCHIVO DEL SERVIDOR WEB index.html
bool handleFileRead(String path){
  #ifdef DEBUG
    Serial.println("handleFileRead: " + path);
  #endif
  if(path.endsWith("/")) path += "index.html";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;
}
//FUNCIONES PARA ACCIONAR LOS MOTORES DE ACUERDO A LOS VALORES QUE SE ENVIAN POR MEDIO DEL JOYSTICK
void adelante(){
   Serial.println("adelante");
   digitalWrite(OUTPUT1, 0);
      digitalWrite(OUTPUT2, 1);
      digitalWrite(OUTPUT3, 1);
      digitalWrite(OUTPUT4, 0);
}
void atras(){
  Serial.println("atras");
  digitalWrite(OUTPUT1, 1);
      digitalWrite(OUTPUT2, 0);
      digitalWrite(OUTPUT3, 0);
      digitalWrite(OUTPUT4, 1);
}

void derecha(){
  Serial.println("derecha");
  digitalWrite(OUTPUT1, 0);
      digitalWrite(OUTPUT2, 0);
      digitalWrite(OUTPUT3, 1);
      digitalWrite(OUTPUT4, 0);
}

void izquierda(){
  Serial.println("izquierda");
  digitalWrite(OUTPUT1, 0);
      digitalWrite(OUTPUT2, 1);
      digitalWrite(OUTPUT3, 0);
      digitalWrite(OUTPUT4, 0);
}
void parar(){
  Serial.println("parar");
  digitalWrite(OUTPUT1, 0);
      digitalWrite(OUTPUT2, 0);
      digitalWrite(OUTPUT3, 0);
      digitalWrite(OUTPUT4, 0);
}

