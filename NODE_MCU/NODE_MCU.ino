#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN D4  // Pin kết nối DHT
#define DHTTYPE DHT11  // Loại cảm biến DHT

DHT dht(DHTPIN, DHTTYPE);

// Thông tin mạng Wi-Fi
const char* ssid = "Fablab 2.4G";
const char* password = "Fira@2024";

// Thông tin MQTT broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;  // Cổng MQTT

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();  // Khởi động cảm biến DHT
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Đang kết nối tới ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Đã kết nối WiFi");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT...");
    // Đổi tên Client để dễ phân biệt các thiết bị
    if (client.connect("thai")) {
      Serial.println("Đã kết nối");
      // Đăng ký nhận dữ liệu từ topic
      client.subscribe("NhietDo/sensor");
    } else {
      Serial.print("Kết nối thất bại");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5 giây");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Tin nhắn nhận được từ [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  
  // Gửi dữ liệu mỗi 5 giây
  if (now - lastMsg > 3000) {
    lastMsg = now;
    
    float temperature = dht.readTemperature();  // Đọc nhiệt độ theo độ C
    float humidity = dht.readHumidity();  // Đọc độ ẩm
    
    // Kiểm tra nếu không đọc được giá trị cảm biến
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Không đọc được giá trị từ cảm biến DHT!");
    } else {
      Serial.print("Nhiệt độ: ");
      Serial.print(temperature);
      Serial.println(" °C");
      Serial.print("Độ ẩm: ");
      Serial.print(humidity);
      Serial.println(" %");
      
      // Gửi dữ liệu nhiệt độ và độ ẩm lên MQTT
      String tempStr = String(temperature);
      String humStr = String(humidity);
      
      client.publish("sensors/temperature", tempStr.c_str());
      client.publish("sensors/humidity", humStr.c_str());
    }
  }
  
  delay(3000);  // Đợi 3 giây trước khi đọc lại
}
