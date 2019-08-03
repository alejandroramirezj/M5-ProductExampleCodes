#include <M5StickC.h>
#include <WiFi.h>
#include "esp_http_server.h"
#include "carControl.h"
#include "page.h"


#define ALL_LED 7
uint32_t allColor = 0;

static void initWifi();
static esp_err_t http_server_init();
// the setup routine runs once when M5StickC starts up

void BtnSet() {
  pinMode(37, INPUT_PULLUP);
}

int readBtn() {
  if (digitalRead(37) == 0) {
    leftwheel(70);
    rightwheel(70);
    delay(500);
    leftwheel(-70);
    rightwheel(-70);
    delay(500);
    leftwheel(0);
    rightwheel(0);
  } 
}

void blink() {
  for(int num = 0; num < 7; num++) {
    uint32_t color = 0x11 << 16;
    led(num, color);
    delay(100);
    led(num, 0x00);
  }
  delay(100);
  for(int num = 0; num < 7; num++) {
    uint32_t color = 0x11 << 16;
    led(num, color);
    delay(100);
    led(num, 0x00);
  }
  delay(100);
  for(int num = 0; num < 7; num++) {
    uint32_t color = 0x11 << 16;
    led(num, color);
    delay(100);
    led(num, 0x00);
  }
}

void setup() {
  
  // initialize the M5StickC object
  M5.begin();
  Wire.begin(0, 26);

  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(40, 20, 1);
  M5.Lcd.setTextSize(2);
  
  // Lcd display
//  M5.Lcd.fillScreen(WHITE);
  initWifi();
  BtnSet();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  http_server_init();
  
  blink();
}

// the loop routine runs over and over again forever
void loop(){
  readBtn();
}

void carLRcontrol(int8_t left, int8_t right) {
  Serial.printf("left:%d ,right:%d\r\n", left, right);
  leftwheel(left);
  rightwheel(right);
}

esp_err_t controlPage(httpd_req_t *req) {
//  Serial.println("control recv");
  size_t buf_len;
  char * buf;
  char xval[32] = {0};
  char yval[32] = {0};
  char color[32] = {0};

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (httpd_query_key_value(buf, "left", xval, sizeof(xval)) == ESP_OK &&
                httpd_query_key_value(buf, "right", yval, sizeof(yval)) == ESP_OK &&
                httpd_query_key_value(buf, "color", color, sizeof(color)) == ESP_OK) {
                  
            } else {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int num = 3 - atoi(color);
    if (num < 3) {
      allColor = 0x11 << (num * 8);
      led(ALL_LED, allColor);  
    } else {
      led(ALL_LED, 0x00);
    }
//    Serial.printf("color %d", num);
    
    int xint = atoi(xval);
    int yint = atoi(yval);
    carLRcontrol(xint, yint);
    return httpd_resp_send(req, NULL, 0);  
}

esp_err_t test_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    return httpd_resp_send(req, (const char *)ctlPage, 6463);
}

static esp_err_t http_server_init(){
    httpd_handle_t server;

    httpd_uri_t hello_word = {
        .uri = "/ctl",
        .method = HTTP_GET,
        .handler = test_handler,
        .user_ctx = NULL
    };

    httpd_uri_t control = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = controlPage,
        .user_ctx = NULL   
    };
  
    httpd_config_t http_options = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &http_options));

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &hello_word));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &control));
    return ESP_OK;
}

static void initWifi() {

  WiFi.mode(WIFI_AP_STA);
  String Mac = WiFi.macAddress();
  String SSID = "BeetleC:"+ Mac;
  bool result = WiFi.softAP(SSID.c_str(), "12345678", 0, 0);
  if (!result){
    Serial.println("AP Config failed.");
  } else
  {
    Serial.println("AP Config Success. AP NAME: " + String(SSID));
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  M5.Lcd.println("BeetleC");
  M5.Lcd.setTextSize(1);
  M5.Lcd.println();
  M5.Lcd.setCursor(30, 40, 1);
  M5.Lcd.printf(Mac.c_str());
  M5.Lcd.setCursor(20, 50, 2);
  M5.Lcd.print("password:12345678");
}
