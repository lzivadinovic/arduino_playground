#include <OnkyoRI.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Wifi
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// RI port
#define ONKYO_PIN 5 // int or char D1

WiFiClient wifiClient;
OnkyoRI onkyoClient(ONKYO_PIN);

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Onkyo RI WS</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
   <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      .button {
        padding: 10px 20px;
        margin: 10px;
        font-size: 24px;
        text-align: center;
        outline: none;
        color: #fff;
        background-color: #2f4468;
        border: none;
        border-radius: 5px;
        box-shadow: 0 6px #999;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }  
      .button:hover {background-color: #1f2e45}
      .button:active {
        background-color: #1f2e45;
        box-shadow: 0 4px #666;
        transform: translateY(2px);
      }
    </style>
  </head>
  <body>
    <h1>Onkyo RI Controller</h1>
    <button class="button" onclick="toggleCheckbox('pwr');">Power On</button><br/>
    <button class="button" onclick="toggleCheckbox('vup');">VOL +</button><br/>
    <button class="button" onclick="toggleCheckbox('vdw');">VOL -</button><br/>
    <button class="button" onclick="toggleCheckbox('mut');">Mute</button><br/>
    <button class="button" onclick="toggleCheckbox('umut');">Unmute</button><br/>
    <button class="button" onclick="toggleCheckbox('d1');">D1</button><br/>
    <button class="button" onclick="toggleCheckbox('d2');">D2</button><br/>
    <button class="button" onclick="toggleCheckbox('d3');">D3</button><br/>
    <button class="button" onclick="toggleCheckbox('d4');">D4</button><br/>
    <button class="button" onclick="toggleCheckbox('a1');">A1</button><br/>
    <button class="button" onclick="toggleCheckbox('a2');">A2</button><br/>
    <button class="button" onclick="toggleCheckbox('a3');">A3</button><br/>
    <button class="button" onclick="toggleCheckbox('a4');">A4</button><br/>
    <button class="button" onclick="toggleCheckbox('phono');">Phono</button><br/>
    <button class="button" onclick="toggleCheckbox('mainIn');">Main in</button><br/>
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + x, true);
     xhr.send();
   }
  </script>
  </body>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

// Create struct for linked list
struct Node {
  int data;
  struct Node* next;
};

struct Node* createNode(int data) {
  // Allocate memory for new Node struct
  struct Node* newNode = (struct Node*)malloc(sizeof(Node));
  // Set data for new Node
  newNode->data = data;
  // Set 'next' for new Node (because its new, there is nothing at the end, so NULL)
  newNode->next = NULL;
  // Return it
  return newNode;
}

void appendNode(struct Node** head, int data) {
  // Create new Node with pushed data
  struct Node* newNode = createNode(data);
  // If head is NULL (ie, entry node does not exist, just create it and return)
  if (*head == NULL){
    *head = newNode;
    return;
  }
  // Create pointer to passed node
  struct Node* temp = *head;
  // Go trough all untill we hit end
  // Traverse list
  while (temp->next != NULL) {
    temp = temp->next;
  }
  // Set next to newly created node
  temp->next = newNode;
}

void sendOnkyo(struct Node* head) {
    // Create pointer to first node
    struct Node* temp = head;
    // Send first node data
    while (temp != NULL) {
        printf("Sending signal %d\n", temp->data);
        onkyoClient.send(temp->data);
        // Need additional timeout for my AMP
        delay(50);
        temp = temp->next;
    }
    printf("Command done!\n");
}

void setup() {
  Serial.begin(115200);

  // Pull pin down
  pinMode(ONKYO_PIN, OUTPUT);
  digitalWrite(ONKYO_PIN, LOW);

  // Create commands for onkyo A-9150
  struct Node* volup = createNode(0x2);

  struct Node* voldown = createNode(0x3);

  struct Node* pwr = createNode(0x4);
  
  struct Node* mute = createNode(0x5);

  struct Node* unmute = createNode(0xD8);

  // Since we don't have mappings for all commands
  // Make this command composition of two or more commands
  struct Node* mainIn = createNode(0x20);
  appendNode(&mainIn, 0xD6);

  struct Node* d1 = createNode(0x20);
  
  struct Node* d2 = createNode(0xE0);
  
  struct Node* d3 = createNode(0x170);

  struct Node* d4 = createNode(0x170);
  appendNode(&d4, 0xD5);
    
  struct Node* a1 = createNode(0x170);
  appendNode(&a1, 0xD5);
  appendNode(&a1, 0xD5);

  struct Node* a2 = createNode(0x170);
  appendNode(&a2, 0xD5);
  appendNode(&a2, 0xD5);
  appendNode(&a2, 0xD5);
  
  struct Node* a3 = createNode(0x170);
  appendNode(&a3, 0xD5);
  appendNode(&a3, 0xD5);
  appendNode(&a3, 0xD5);
  appendNode(&a3, 0xD5);
  
  struct Node* a4 = createNode(0x170);
  appendNode(&a4, 0xD5);
  appendNode(&a4, 0xD5);
  appendNode(&a4, 0xD5);
  appendNode(&a4, 0xD5);
  appendNode(&a4, 0xD5);

  struct Node* phono = createNode(0x170);
  appendNode(&phono, 0xD5);
  appendNode(&phono, 0xD5);
  appendNode(&phono, 0xD5);
  appendNode(&phono, 0xD5);
  appendNode(&phono, 0xD5);
  appendNode(&phono, 0xD5);

  setupWifi();

  // handle entrypoint
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // because 3rd param is lambda function, we need to catch variables from current scope
  // so we can pass them to lambda, that's why we have [pwr]
  server.on("/pwr", HTTP_GET, [pwr](AsyncWebServerRequest *request) {
    sendOnkyo(pwr);
    request->send(200, "text/plain", "Power");
  });

  server.on("/a1", HTTP_GET, [a1](AsyncWebServerRequest *request) {
    sendOnkyo(a1);
    request->send(200, "text/plain", "A1");
  });
  
  server.on("/a2", HTTP_GET, [a2](AsyncWebServerRequest *request) {
    sendOnkyo(a2);
    request->send(200, "text/plain", "A2");
  });

  server.on("/a3", HTTP_GET, [a3](AsyncWebServerRequest *request) {
    sendOnkyo(a3);
    request->send(200, "text/plain", "A3");
  });
  
  server.on("/a4", HTTP_GET, [a4](AsyncWebServerRequest *request) {
    sendOnkyo(a4);
    request->send(200, "text/plain", "A4");
  });

  server.on("/d1", HTTP_GET, [d1](AsyncWebServerRequest *request) {
    sendOnkyo(d1);
    request->send(200, "text/plain", "D1");
  });

  server.on("/d2", HTTP_GET, [d2](AsyncWebServerRequest *request) {
    sendOnkyo(d2);
    request->send(200, "text/plain", "D2");
  });

  server.on("/d3", HTTP_GET, [d3](AsyncWebServerRequest *request) {
    sendOnkyo(d3);
    request->send(200, "text/plain", "D3");
  });

  server.on("/d4", HTTP_GET, [d4](AsyncWebServerRequest *request) {
    sendOnkyo(d4);
    request->send(200, "text/plain", "D4");
  });

  server.on("/phono", HTTP_GET, [phono](AsyncWebServerRequest *request) {
    sendOnkyo(phono);
    request->send(200, "text/plain", "Phono");
  });

  server.on("/mainIn", HTTP_GET, [mainIn](AsyncWebServerRequest *request) {
    sendOnkyo(mainIn);
    request->send(200, "text/plain", "Main in");
  });


  server.on("/vup", HTTP_GET, [volup](AsyncWebServerRequest *request) {
    sendOnkyo(volup);
    request->send(200, "text/plain", "Volume +");
  });

  server.on("/vdw", HTTP_GET, [voldown](AsyncWebServerRequest *request) {
    sendOnkyo(voldown);
    request->send(200, "text/plain", "Volume -");
  });

  server.on("/mut", HTTP_GET, [mute](AsyncWebServerRequest *request) {
    sendOnkyo(mute);
    request->send(200, "text/plain", "Mute");
  });

  server.on("/umut", HTTP_GET, [unmute](AsyncWebServerRequest *request) {
    sendOnkyo(unmute);
    request->send(200, "text/plain", "Unmute");
  });

  server.onNotFound(notFound);
  server.begin();
}

void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
}
