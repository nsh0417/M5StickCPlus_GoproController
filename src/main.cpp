/**
 * M5StickC+をGoProと接続し、M5ボタンで録画開始/停止を行う
 *
 * note
 * - 開発者が持つGoProはHERO+のみのため、それ用の動作になります。
 * - 各APIは下記を参考にしているので、そちらを見て書き換えてください。
 * - [goprowifihack](https://github.com/KonradIT/goprowifihack/)
 *
 * - M5StickC : https://make-muda.net/2019/09/6906/
 * - M5StickC+: https://github.com/m5stack/M5StickC-Plus
 * - https://msr-r.net/m5stack-wifi/
 *
 * TODO: 追加予定機能
 * 1. 画面スリープ、CPUクロック低下、LCD低下（バッテリ対策）
 *    - 画面スリープはしない方がいいかも、ということで未対応
 *      - [M5StickCであそぶ 〜バッテリーを使う〜 MUDAなことをしよう](https://make-muda.net/2019/09/6946/)
 * 2. メッセージが古いものから消えるように対応する
 * 3. 画面表示をアイコンとか使って表示する
 * 4. コメントの書き方とかが適当なので修正する
 */
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <HTTPClient.h>

// LCDの明るさ（min 7 /max 12)
// https://lang-ship.com/reference/unofficial/M5StickC/Class/AXP192/
#define LCD_BRIGHTNESS 10
// CPU周波数: 240(default), 160, 80, 40, 20, 10から選択可
#define CPU_FREQUENCY_MHZ 80

// GPIO
#define LED_PIN   10
// LEDのON/OFF
#define LED_ON  LOW
#define LED_OFF HIGH
// 表示メッセージ
#define MESSAGE_SETUP "Gopro setup completed!"
#define MESSAGE_START_REC "Recording started."
#define MESSAGE_STOP_REC "Recording stopped."
#define MESSAGE_KEEP_ALIVE_OK "keep alive: OK"
// KEEP ALIVE間隔(msec)
#define KEEP_ALIVE_MSEC 30 * 1000

// GoPro接続情報
// FIXME: ユーザ独自の設定ファイルとかにして分けたい
#define SSID "gopro"
#define PASSWORD "gopro"

// GoPro APIのURI
// note : https://github.com/KonradIT/goprowifihack/blob/master/HERO/WifiCommands.md
//        https://android.benigumo.com/20180116/gopro-hero6-wifi/
//        http://hiroeki1.blog129.fc2.com/blog-entry-400.html
#define GOPRO_URI "http://10.5.5.9/gp/"
#define GOPRO_SETUP GOPRO_URI "gpControl/execute?p1=gpStream&c1=restart"
#define GOPRO_REC_START GOPRO_URI "gpControl/command/shutter?p=1"
#define GOPRO_REC_STOP GOPRO_URI "gpControl/command/shutter?p=0"

#define GOPRO_STATUS GOPRO_URI "gpControl/status"

// #define GOPRO_URI "http://192.168.11.1/login.html"

// 録画中か否か
boolean is_recording = false;

boolean gopro_setup();
boolean start_rec();
boolean stop_rec();
// test
boolean get_status();

boolean control_gopro(const char* uri, const uint8_t led, const char* output);
void keep_alive(void* arg);

void setup() {
  M5.begin();
  // Pin設定
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  // 画面の明るさを設定
  M5.Axp.ScreenBreath(LCD_BRIGHTNESS);

  // CPU周波数設定
  // XXX: setCpuFrequencyMhz()で設定が空振ることがあるのか、以後の動作がおかしくなるときがある模様
  //      そのため、設定が反映(trueが返る)まで繰り返す処理にしている
  while (!setCpuFrequencyMhz(CPU_FREQUENCY_MHZ)) {
    ; // NOP
  }

  // フォントサイズをデフォルトの2倍に
  // M5.Lcd.setTextSize(2);
  // ボタンBが上になるような向き
  M5.Lcd.setRotation(1);

  // WiFi設定
  WiFi.begin(SSID, PASSWORD);

  M5.Lcd.printf("%s connecting, please wait", SSID);
  // FIXME: 接続後に再起動すると、接続されない。次回再起動後に正常に接続可
  //        シャットダウン前に開放するなどの処理が必要？
  //        WiFiルータでは問題なし、GoProのみ発生
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }
  M5.Lcd.printf("\r\n%s connected!\r\n", SSID);
  gopro_setup();

  // 録画中のkeep alive監視用スレッド
  xTaskCreatePinnedToCore(keep_alive, "keep_alive", 4096, NULL, 1, NULL, 0);
}

void loop() {
  M5.update();

  if (M5.BtnA.wasPressed()) {
    if (!is_recording) {
      is_recording = start_rec();
    } else {
      is_recording = !stop_rec();
    }
  }
}

/**
 * @brief GoProの録画時の設定（詳細は把握していないので実質おまじない）
 *
 * @return boolean 成功/失敗
 */
boolean gopro_setup() {
  return control_gopro(GOPRO_SETUP, LED_OFF, MESSAGE_SETUP);
}
/**
 * @brief 録画開始
 *
 * @return boolean 成功/失敗
 */
boolean start_rec() {
  return control_gopro(GOPRO_REC_START, LED_ON, MESSAGE_START_REC);
}
/**
 * @brief 録画停止
 *
 * @return boolean 成功/失敗
 */
boolean stop_rec() {
  return control_gopro(GOPRO_REC_STOP, LED_OFF, MESSAGE_STOP_REC);
}
boolean get_status() {
  return control_gopro(GOPRO_STATUS, LED_OFF, "Get Status!");
}
/**
 * @brief 録画の制御を行う
 *        録画開始成功：LED点灯、開始の旨を表示
 *        録画停止成功：LED消灯、停止の旨を表示
 *
 * @param uri アクセス先URI
 * @param led 成功時のLEDの状態（LED_ON/LED_OFF）
 * @param lcd_str LCDへ出力する文字列
 * @return boolean 成功: / 失敗
 */
boolean control_gopro(const char* uri, const uint8_t led, const char* lcd_str) {
  HTTPClient http;
  http.begin(uri);

  // M5.Lcd.println(uri);
  int http_code = http.GET();
  if (http_code == HTTP_CODE_OK) {
        digitalWrite(LED_PIN, led);
        M5.Lcd.println(lcd_str);
        return true;
  } else {
    M5.Lcd.printf("HTTP NG: %d\n", http_code);
  }
  return false;
}
/**
 * @brief 録画が止まっていたら再度録画を開始する
 *        実際には録画コマンドを30秒/回で送っているだけ
 *
 * @param arg xTaskCreatePinnedToCoreの中で呼び出すため、特に不要
 */
void keep_alive(void* arg) {
  while(1) {
    if (is_recording) {
      // XXX: 理由は不明だがHTTPClientをここで初期化しないと2回に1回、http_codeが-2になる
      HTTPClient http;
      http.begin(GOPRO_REC_START);
      int http_code = http.GET();
      if (http_code == HTTP_CODE_OK) {
        M5.Lcd.println(MESSAGE_KEEP_ALIVE_OK);
      } else {
        M5.Lcd.printf("Recording is dead, or network unreachtable!: %d\n", http_code);
      }
    }
    delay(KEEP_ALIVE_MSEC);
  }
}

