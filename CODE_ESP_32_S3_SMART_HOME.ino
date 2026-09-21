#include <Arduino.h>
#include "ESP_I2S.h"
#include "ESP_SR.h"

#define I2S_BCLK 17
#define I2S_WS   15
#define I2S_DIN  16

#define RELAY_LAMPU_1 18
#define RELAY_LAMPU_2 19

#define RELAY_ON  LOW
#define RELAY_OFF HIGH

I2SClass i2s;

enum
{
  CMD_LAMPU_ON,
  CMD_LAMPU_OFF
};

// =====================================
// DAFTAR PERINTAH SUARA
// =====================================

static const sr_cmd_t commands[] =
{
  {
    CMD_LAMPU_ON,
    "Turn on the light"
  },

  {
    CMD_LAMPU_ON,
    "Switch on the light"
  },

  {
    CMD_LAMPU_ON,
    "Lights on"
  },

  {
    CMD_LAMPU_OFF,
    "Turn off the light"
  },

  {
    CMD_LAMPU_OFF,
    "Switch off the light"
  },

  {
    CMD_LAMPU_OFF,
    "Lights off"
  }
};

// EVENT SPEECH RECOGNITION

void onSrEvent(
  sr_event_t event,
  int command_id,
  int phrase_id
)
{
  switch (event)
  {
    // WAKE WORD

    case SR_EVENT_WAKEWORD:

      Serial.println();
      Serial.println("Wake word terdeteksi!");

      ESP_SR.setMode(SR_MODE_COMMAND);

      break;

    // COMMAND TERDETEKSI

    case SR_EVENT_COMMAND:

      Serial.print("Command ID: ");
      Serial.println(command_id);

      // LAMPU ON

      if (command_id == CMD_LAMPU_ON)
      {
        digitalWrite(
          RELAY_LAMPU_1,
          RELAY_ON
        );

        Serial.println(">>> LAMPU MENYALA");
      }


      // MATIKAN LAMPU

      else if (command_id == CMD_LAMPU_OFF)
      {
        digitalWrite(
          RELAY_LAMPU_1,
          RELAY_OFF
        );

        Serial.println(">>> LAMPU MATI");
      }

      // Tetap berada di mode command
      ESP_SR.setMode(SR_MODE_COMMAND);

      break;

    // TIMEOUT

    case SR_EVENT_TIMEOUT:

      Serial.println("Command timeout");

      ESP_SR.setMode(SR_MODE_WAKEWORD);

      break;


    default:

      break;
  }
}

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" SMART HOME VOICE CONTROL");
  Serial.println(" ESP32-S3");
  Serial.println("==============================");

  pinMode(RELAY_LAMPU_1, OUTPUT);
  pinMode(RELAY_LAMPU_2, OUTPUT);

  // Kondisi awal lampu mati
  digitalWrite(RELAY_LAMPU_1, RELAY_OFF);
  digitalWrite(RELAY_LAMPU_2, RELAY_OFF);

  i2s.setPins(
    I2S_BCLK,
    I2S_WS,
    -1,
    I2S_DIN
  );

  i2s.begin(
    I2S_MODE_STD,
    16000,
    I2S_DATA_BIT_WIDTH_16BIT,
    I2S_SLOT_MODE_MONO,
    I2S_STD_SLOT_LEFT
  );
  // SPEECH RECOGNITION
  ESP_SR.onEvent(onSrEvent);

  ESP_SR.begin(
    i2s,
    commands,
    sizeof(commands) / sizeof(sr_cmd_t),
    SR_CHANNELS_MONO,
    SR_MODE_WAKEWORD,
    "M"
  );

  Serial.println("Sistem siap.");
  Serial.println("Ucapkan wake word terlebih dahulu.");
}

void loop()
{
  delay(100);
}