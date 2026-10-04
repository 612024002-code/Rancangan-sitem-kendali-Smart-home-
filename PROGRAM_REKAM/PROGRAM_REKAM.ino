#include <driver/i2s.h>

// =====================================================
// PIN INMP441 → ESP32-S3
// =====================================================
#define I2S_WS   42
#define I2S_SCK  41
#define I2S_SD   2

#define I2S_PORT I2S_NUM_0
#define BUFFER_LEN 64

int32_t sBuffer[BUFFER_LEN];

// =====================================================
// SETUP
// =====================================================
void setup() {

  // Binary audio dikirim ke Python
  Serial.begin(921600);

  // ===================================================
  // KONFIGURASI I2S
  // ===================================================
  const i2s_config_t i2s_config = {

    .mode = (i2s_mode_t)(
      I2S_MODE_MASTER |
      I2S_MODE_RX
    ),

    // Sample rate 16 kHz
    .sample_rate = 16000,

    // INMP441 menggunakan frame 32-bit
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,

    // L/R INMP441 terhubung ke GND → LEFT
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,

    .communication_format =
      i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),

    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,

    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_LEN,

    .use_apll = false,

    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  // ===================================================
  // PIN I2S
  // ===================================================
  const i2s_pin_config_t pin_config = {

    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,

    // Tidak digunakan karena RX
    .data_out_num = I2S_PIN_NO_CHANGE,

    // Data dari INMP441
    .data_in_num = I2S_SD
  };

  // ===================================================
  // INSTALL I2S DRIVER
  // ===================================================
  esp_err_t err = i2s_driver_install(
    I2S_PORT,
    &i2s_config,
    0,
    NULL
  );

  if (err != ESP_OK) {
    while (1);
  }

  // ===================================================
  // SET PIN
  // ===================================================
  err = i2s_set_pin(
    I2S_PORT,
    &pin_config
  );

  if (err != ESP_OK) {
    while (1);
  }

  // Mulai I2S
  i2s_start(I2S_PORT);
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  size_t bytes_read = 0;

  // Baca data dari INMP441
  esp_err_t result = i2s_read(
    I2S_PORT,
    sBuffer,
    sizeof(sBuffer),
    &bytes_read,
    portMAX_DELAY
  );

  if (result == ESP_OK && bytes_read > 0) {

    int samples_read =
      bytes_read / sizeof(int32_t);

    for (int i = 0; i < samples_read; i++) {

      // =================================================
      // AMBIL DATA AUDIO INMP441
      // =================================================
      int32_t sample = sBuffer[i] >> 8;

      // =================================================
      // DIGITAL GAIN
      //
      // >>5 = sekitar 8x lebih besar dibanding versi
      // sebelumnya yang menggunakan >>8 tambahan.
      // =================================================
      int32_t amplified = sample >> 5;

      // =================================================
      // PROTEKSI CLIPPING
      // =================================================
      if (amplified > 32767)
        amplified = 32767;

      if (amplified < -32768)
        amplified = -32768;

      // =================================================
      // KONVERSI KE 16-BIT PCM
      // =================================================
      int16_t pcm = (int16_t)amplified;

      // =================================================
      // KIRIM BINARY PCM KE PYTHON
      // =================================================
      Serial.write(
        (uint8_t*)&pcm,
        sizeof(pcm)
      );
    }
  }
}