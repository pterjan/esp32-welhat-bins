#include <WiFi.h>
#include <ESP_SSLClient.h>
#include <WiFiClient.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <MycilaNTP.h>
#include "time.h"
#include "driver/gpio.h"

#include "bin-black.h"
#include "bin-blue.h"
#include "bin-brown.h"

#include "config.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

ESP_SSLClient ssl_client;
WiFiClient basic_client;

#include "parse_bins.h"
#include "sleep_bins.h"

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    int z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;

    // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
    Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Pressure = ");
    Serial.print(z);
    Serial.println();

    sleep_until_5pm();
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void zoom2x(lv_obj_t *img) {
  lv_img_set_antialias(img, true);
  lv_img_set_zoom(img, 512);
  lv_obj_set_width(img, lv_obj_get_width(img) * 2);
  lv_obj_set_height(img, lv_obj_get_height(img) * 2);
}


void check_bins(struct bins *b) {

  ssl_client.setInsecure();
  ssl_client.setBufferSizes(16384 /* rx */, 1024 /* tx */);
  ssl_client.setDebugLevel(4);

  ssl_client.setSessionTimeout(60);
  ssl_client.setClient(&basic_client);

  const char *host = "www.welhat.gov.uk";
  const char *boundary = "xx123456789xx";
  String postData;
  postData.reserve(1024);
  postData.concat("--");
  postData.concat(boundary);
  postData.concat("\r\n");
  postData.concat("Content-Disposition: form-data; name=\"page\"\r\n\r\n");
  postData.concat("492\r\n");
  postData.concat("--");
  postData.concat(boundary);
  postData.concat("\r\n");
  postData.concat("Content-Disposition: form-data; name=\"next\"\r\n\r\n");
  postData.concat("Next\r\n");
  postData.concat("--");
  postData.concat(boundary);
  postData.concat("\r\n");
  postData.concat("Content-Disposition: form-data; name=\"q9f451fe0ca70775687eeedd1e54b359e55f7c10c_0_0\"\r\n\r\n");
  postData.concat(postcode);
  postData.concat("\r\n");
  postData.concat("--");
  postData.concat(boundary);
  postData.concat("\r\n");
  postData.concat("Content-Disposition: form-data; name=\"q9f451fe0ca70775687eeedd1e54b359e55f7c10c_1_0\"\r\n\r\n");
  postData.concat(property);
  postData.concat("\r\n");
  postData.concat("--");
  postData.concat(boundary);
  postData.concat("--\r\n");

  if (ssl_client.connect(host, 443)) {
    char buf[1024];
    int i = 0;
    bool found_line = 0;
    bool found_all = 0;
    Serial.print("Conected to ");
    Serial.println(host);
    Serial.println("Send POST request...");
    ssl_client.print("POST /xfp/form/214 HTTP/1.1\r\n");
    ssl_client.print("Host: ");
    ssl_client.print(host);
    ssl_client.print("\r\n");
    ssl_client.print("Content-Type: multipart/form-data; boundary=");
    ssl_client.print(boundary);
    ssl_client.print("\r\n");
    ssl_client.print("User-Agent: BinDay/0.1\r\n");
    ssl_client.print("Content-Length: ");
    ssl_client.print(postData.length());
    ssl_client.print("\r\n\r\n");
    ssl_client.print(postData);
    Serial.print("Read response...");
    unsigned long ms = millis();
    // <p><table class=""><thead><tr><th>Service</th><th>Next collection</th></tr></thead><tbody><tr><td>Domestic Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr><tr><td>Food Waste Collection Service</td><td>Wednesday 10 December 2025</td></tr><tr><td>Garden Waste Collection Service</td><td>Wednesday 17 December 2025</td></tr><tr><td>Recycling Collection Service</td><td>Wednesday 17 December 2025</td></tr></tbody></table>
    // Store from Next collection to \n

    Serial.println();
    while (ssl_client.connected()) {
      while (ssl_client.connected() && !ssl_client.available()) {
        delay(0);
      }
      while (ssl_client.available() && i < 1023) {
        char c = (char)ssl_client.read();
        if (found_line && c == '\n') {
          //Serial.println("found end of line!");
          found_all = 1;
          break;
        }
        buf[i++] = c;
      }
      if (found_all) {
        //Serial.println("all done!");
        buf[i] = '\0';
        //Serial.println(buf);
        update_bins_from_line(buf, b);
        break;
      }
      if (!found_line) {
        buf[i] = '\0';
        char *next = strstr(buf, "Next collection");
        if (next) {
          //Serial.println("found line!");
          found_line = 1;
          i = i - (next - buf) - strlen("Next collection");
          memmove(buf, next + strlen("Next collection"), i);
        } else if (i > strlen("Next collection")) {
          memmove(buf, buf + i - strlen("Next collection"), strlen("Next collection"));
          i = strlen("Next collection");
        }
      }
    }
    Serial.println();
  } else
    Serial.println("Failed to connect\n");

  ssl_client.stop();
}

void display_bins(struct bins *b) {
  lv_obj_t *imgs[3];
  int bin_count = 0;
  int offset;
  if (b->blue) {
    LV_IMAGE_DECLARE(bin_blue);
    imgs[bin_count] = lv_image_create(lv_screen_active());
    lv_image_set_src(imgs[bin_count], &bin_blue);
    bin_count++;
  }
  if (b->brown) {
    LV_IMAGE_DECLARE(bin_brown);
    imgs[bin_count] = lv_image_create(lv_screen_active());
    lv_image_set_src(imgs[bin_count], &bin_brown);
    bin_count++;
  }
  if (b->black) {
    LV_IMAGE_DECLARE(bin_black);
    imgs[bin_count] = lv_image_create(lv_screen_active());
    lv_image_set_src(imgs[bin_count], &bin_black);
    bin_count++;
  }

  lv_obj_t *label1 = lv_label_create(lv_screen_active());
  char buf[32];
  strftime(buf, 30, "%F", localtime(&b->next_date));
  lv_label_set_text(label1, buf);
  lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label1, LV_ALIGN_BOTTOM_MID, 0, 0);

  switch (bin_count) {
    case 1:
      zoom2x(imgs[0]);
      lv_obj_align(imgs[0], LV_ALIGN_CENTER, 0, 0);
      break;
    case 2:
      zoom2x(imgs[0]);
      zoom2x(imgs[1]);
      offset = (SCREEN_WIDTH - lv_obj_get_width(imgs[0]) - lv_obj_get_width(imgs[1])) / 3;
      lv_obj_align(imgs[0], LV_ALIGN_LEFT_MID, offset, 0);
      lv_obj_align(imgs[1], LV_ALIGN_RIGHT_MID, -offset, 0);
      break;
    case 3:
      offset = (SCREEN_WIDTH - lv_obj_get_width(imgs[0]) - lv_obj_get_width(imgs[1]) - lv_obj_get_width(imgs[2])) / 4;
      lv_obj_align(imgs[0], LV_ALIGN_LEFT_MID, offset, 0);
      lv_obj_align(imgs[1], LV_ALIGN_CENTER, 0, 0);
      lv_obj_align(imgs[2], LV_ALIGN_RIGHT_MID, -offset, 0);
      break;
    default:
      return;
  }
}


void setup() {
  struct bins b;

  Serial.begin(115200);
  delay(1000);  // Take some time to open up the Serial Monitor

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Mycila::NTP.setTimeZone("Europe/London");
  Mycila::NTP.sync("pool.ntp.org");

  // Check when we next need to get some bins out
  check_bins(&b);

  // If collection is not tomorrow, sleep until tomorrow 5pm
  sleep_if_not_tonight(&b);

  // If we did not go to sleep, display bins until midnight

  // Disconnect from the network
  WiFi.disconnect(true);
  // Switch WiFi off
  WiFi.mode(WIFI_OFF);

  // Start LVGL
  lv_init();

  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t *disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  display_bins(&b);
}

void loop() {
  lv_task_handler();
  delay(5);
  lv_tick_inc(5);
  sleep_at_night();
}
