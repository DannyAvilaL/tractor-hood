#include "arduino_secrets.h"
#include <SPI.h>
#include <mcp2515.h>
#include "thingProperties.h"


struct can_frame Enviado;
struct can_frame Recibido;
MCP2515 mcp2515(15);
//bool abrir = false;
const int boton = 4;
bool estadoBoton = false;
bool estadoCofre = false;

int period = 1000;
unsigned long time_now = 0;

void onAbrirTapaChange();

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500);
  // Defined in thingProperties.h
  initProperties();
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(boton, INPUT);

  Enviado.can_id  = 4;
  Enviado.can_dlc = 1;
  Enviado.data[0] = 0;

  while (!Serial);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

  Serial.println("Buenas soy CAN-node");
  Serial.println("ID Estado");
}


int count = 0;
//estadoTapa = false;

void loop() {
  ArduinoCloud.update();
  time_now = millis();
  delay(600);

  // Reading the button connected to the Node
  if (digitalRead(boton)) {
    Serial.println("estado" + String(digitalRead(boton)));
    if (!estadoBoton) {
      estadoBoton = true;
      onAbrirTapaChange();
      
    }
    if (estadoBoton){
      estadoBoton = false;
      
    }
    onAbrirTapaChange();
  }

  // If the hood is set to open  
  if (estadoCofre) {
    Enviado.data[0] = 1;
    mcp2515.sendMessage(&Enviado);

    // If there is a received message
    if (mcp2515.readMessage(&Recibido) == MCP2515::ERROR_OK) {
      Serial.println("se ha abierto");
      Serial.print(Recibido.can_id, HEX); // print ID
      Serial.print(" ");
      Serial.print(Recibido.data[0], HEX); // print DLC
      Serial.print(" ");
      Serial.println();
      estadoTapa = Recibido.data[0];

    }
  }

  // If the hood is set to close
  if (!estadoCofre) {
    Enviado.data[0] = 0;
    mcp2515.sendMessage(&Enviado);

    if (mcp2515.readMessage(&Recibido) == MCP2515::ERROR_OK) {
      Serial.println("se ha cerrado");
      Serial.print(Recibido.can_id, HEX); // print ID
      Serial.print(" ");
      Serial.print(Recibido.data[0], HEX); // print DLC
      Serial.print(" ");
      Serial.println();
      estadoTapa = Recibido.data[0];
    }
  }
}

// Function to update the state of the variables in the cloud
void onAbrirTapaChange()  {

  if (abrir || estadoBoton) {
    abrir = false;
    estadoBoton = false;
    estadoCofre = false;
  }
  else if (!abrir || !estadoBoton) {
    abrir = true;
    estadoBoton = true;
    estadoCofre = true;
  }
}
