/*
   Code in FreeRTOS that makes the microcontroller MEGA2560
   cabaple of moving two servomotors at the moment it receives
   a message through the CAN transceiver. After acting, it can
   send back an update of the state of the tractor hood via CAN.
*/
// ADDING LIBRARIES
#include <Arduino_FreeRTOS.h>
#include <SPI.h>
#include <mcp2515.h>
#include <Servo.h>

// PIN DEFINITION
#define SERVOTOP  7
#define SERVOFRONT 6

// GLOBAL VARIABLES
struct can_frame Enviado;
struct can_frame Recibido;
MCP2515 CAN(10);
Servo servoTop, servoFront;

bool estadoCofre = false;
bool estadoServoTop = false;
bool estadoServo = false;
bool mensaje = false;

// TASK DECLARATION
void TaskEstadoCofre(void *param); 

void setup() {
  Serial.begin(9600);

  Enviado.can_id = 8;
  Enviado.can_dlc = 1;
  Enviado.data[0] = 0;
  
  servoTop.attach(SERVOTOP);
  servoTop.write(90);
  servoFront.attach(SERVOFRONT);
  servoFront.write(90);

  while (!Serial);

  CAN.reset();
  CAN.setBitrate(CAN_125KBPS);
  CAN.setNormalMode();

  Serial.println("Iniciando CAN en MEGA");
  Serial.println("ID\tEstado");

  xTaskCreate(TaskEstadoCofre, "EstadoCofre", 128, NULL, 1, NULL);
  vTaskStartScheduler();
}

void TaskEstadoCofre(void *param) {
  while (1) {
    if (CAN.readMessage(&Recibido) == MCP2515::ERROR_OK) {
      Serial.print(Recibido.can_id, HEX);
      Serial.print(" ");
      Serial.print(Recibido.data[0], HEX);
      Serial.println(" ");

      if (Recibido.data[0] == 1) { //If open
        estadoCofre = true;
      }
      if (Recibido.data[0] == 0) { //If close
        estadoCofre = false;
      }
    }
    moverServo();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void moverServo(void) {
  //If wanting to open and is closed
  if (estadoCofre && !estadoServo) { 
    for (int pos = 90; pos > 20; pos--) {
      servoTop.write(pos);
      servoFront.write(100);
      vTaskDelay(70 / portTICK_PERIOD_MS);
    }
    servoFront.write(90);
    estadoServo = true;
    mensaje = false;
    enviarCAN();
  }
  //If wanting to close and is open
  else if (!estadoCofre && estadoServo) { 
    for (int pos = 20; pos < 110; pos++) {
      servoTop.write(pos);
      servoFront.write(72);
      vTaskDelay(70 / portTICK_PERIOD_MS);
    }
    servoFront.write(90);
    estadoServo = false;
    mensaje = false;
    enviarCAN();
  }
}

void enviarCAN(void) {
  //If update message is not sent yet
  if (!mensaje) { 

    if (estadoServo) { 
      Enviado.data[0] = 1;
      CAN.sendMessage(&Enviado);
      Serial.println("abierto");
    }
    else if (!estadoServo) {
      Enviado.data[0] = 0;
      CAN.sendMessage(&Enviado);
      Serial.println("cerrado");
    }
    mensaje = true;
  }
}

// Ignoring Loop
void loop() {}
