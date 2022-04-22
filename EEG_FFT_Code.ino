// Code for FFT analysis for EEG IDP Project
// Written by Malcolm Moss

// FFT Setup
#define LOG_OUT 0 // Setup FFT output
#define FFT_N 128 // set to 128 point fft
#define LIN_OUT 1 // Setup fft_mag_lin()

// Library Setup
#include <SPI.h>
#include <Wire.h>
#include <FFT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Setup Screen Height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Setup Connection to Screen
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Set analog pin
#define ANALOG_PIN A1

void setup() {
  // Setup and check for board connection
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  // Clears the buffer of the default values
  display.clearDisplay();
}

// Update the screen based on the input values
void set_screen(int delta_value, int theta_value, int alpha_value, int beta_value, int rms, int noise) {
  // Clears and sets the display up
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Displays the bar graph labels
  display.setCursor(5, 25);
  display.println("D");
  display.setCursor(25, 25);
  display.println("T");
  display.setCursor(45, 25);
  display.println("A");
  display.setCursor(65, 25);
  display.println("B");

  // Displays the RMS and Noise values along with the ratio
  display.setCursor(80, 0);
  display.println("RMS");
  display.setCursor(103, 0);
  display.println(rms);
  display.setCursor(80, 8);
  display.println("NOISE");
  display.setCursor(80, 16);
  display.println(noise);
  display.setCursor(80, 24);
  display.println("N/R");
  display.setCursor(103, 24);
  display.println((float) noise / (float) rms);

  // Displays the four bar graphs
  display.fillRect(0, 24 - delta_value, 16, delta_value, SSD1306_WHITE);
  display.fillRect(20, 24 - theta_value, 16, theta_value, SSD1306_WHITE);
  display.fillRect(40, 24 - alpha_value, 16, alpha_value, SSD1306_WHITE);
  display.fillRect(60, 24 - beta_value, 16, beta_value, SSD1306_WHITE);

  // Renders everything on the display
  display.display();
}

void loop() {
  // Record the data
  int i;
  for (i = 0; i < 256; i += 2) {
    // Read the analog pin and store it
    fft_input[i] = analogRead(ANALOG_PIN);
    // Add the imaginary component to the FFT input
    fft_input[i + 1] = 0;
    // Delay so that we have 128 samples per second
    delayMicroseconds(7700);
  }

  // Perform the FFT
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_lin(); // find the magnitudes for each bin

  // Calculate and display the bar graph values
  int scaling_factor = sum(fft_lin_out, 2, 32);
  int noise = sum(fft_lin_out, 33, 63);
  set_screen((24 * sum(fft_lin_out, 2, 4)) / scaling_factor, (24 * sum(fft_lin_out, 5, 9)) / scaling_factor, (24 * sum(fft_lin_out, 10, 14)) / scaling_factor, (24 * sum(fft_lin_out, 15, 32)) / scaling_factor, scaling_factor, noise);
}

// Function used to sum part of an array
int sum(unsigned int *input_array, int start, int finish) {
  int total = 0;
  for (int i = start; i < finish + 1; i++) {
    total = total + int(input_array[i]);
  }
  return total;
}
