#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const char* ssid     = "CS";
const char* password = "11112222";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>自拍打卡</title>
  <style>
    body{text-align:center; margin-top:50px; font-family:微软雅黑;}
    button{padding:15px 30px; font-size:18px; background:#03A9F4; color:white; border:none; border-radius:10px;}
    input{display:none;}
    img{max-width:300px; margin-top:20px; border:2px solid #09F;}
  </style>
</head>
<body>
  <h2>ESP32 自拍打卡系统</h2>
  <label for="file"><button>选择自拍照片打卡</button></label>
  <input type="file" id="file" accept="image/*" capture="camera">
  <img id="preview">
  <div id="msg"></div>

<script>
const fileInput = document.getElementById('file');
const img = document.getElementById('preview');
const msg = document.getElementById('msg');

fileInput.onchange = async (e) => {
  const file = e.target.files[0];
  if (!file) return;
  
  img.src = URL.createObjectURL(file);
  const formData = new FormData();
  formData.append('photo', file);

  const res = await fetch('/upload', {
    method: 'POST',
    body: formData
  });
  
  msg.innerText = await res.text();
  alert("打卡成功！");
};
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleUpload() {
  if (server.hasArg("photo")) {
    Serial.println("==================== 打卡成功 ====================");
    Serial.println("已收到自拍照片");
    Serial.println("==================================================\n");
    server.send(200, "text/plain", "✅ 自拍打卡成功！");
  } else {
    server.send(200, "text/plain", "❌ 打卡失败");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi 连接成功！");
  Serial.print("打卡地址：http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, handleUpload);
  server.begin();
}

void loop() {
  server.handleClient();
}
