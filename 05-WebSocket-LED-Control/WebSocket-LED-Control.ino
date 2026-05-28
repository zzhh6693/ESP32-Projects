#include <WiFi.h>

// ==================== 配置信息 ====================
const char* ssid = "CS";
const char* password = "11112222";
const int ledPin = 2; // D2引脚
const int maxClients = 5; // 最大同时连接的SSE客户端数量

// ==================== 全局变量 ====================
WiFiServer server(80);
WiFiClient sseClients[maxClients]; // 存储SSE客户端连接
bool ledState = false; // LED当前状态

// ==================== HTML控制网页 ====================
const char* htmlPage = R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 LED控制 (SSE版)</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Arial', sans-serif;
        }
        
        body {
            background-color: #f5f5f5;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            background-color: white;
            padding: 40px;
            border-radius: 12px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.1);
            text-align: center;
            max-width: 400px;
            width: 100%;
        }
        
        h1 {
            color: #333;
            margin-bottom: 30px;
            font-size: 24px;
        }
        
        .status-card {
            padding: 25px;
            border-radius: 8px;
            margin-bottom: 30px;
            transition: all 0.3s ease;
        }
        
        .status-on {
            background-color: #4CAF50;
            color: white;
        }
        
        .status-off {
            background-color: #f44336;
            color: white;
        }
        
        .status-disconnected {
            background-color: #9e9e9e;
            color: white;
        }
        
        .status-text {
            font-size: 22px;
            font-weight: bold;
        }
        
        .button-group {
            display: flex;
            gap: 15px;
            justify-content: center;
        }
        
        button {
            padding: 15px 30px;
            font-size: 18px;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-weight: bold;
            flex: 1;
        }
        
        #onBtn {
            background-color: #4CAF50;
            color: white;
        }
        
        #onBtn:hover {
            background-color: #45a049;
            transform: translateY(-2px);
        }
        
        #onBtn:disabled {
            background-color: #a5d6a7;
            cursor: not-allowed;
            transform: none;
        }
        
        #offBtn {
            background-color: #f44336;
            color: white;
        }
        
        #offBtn:hover {
            background-color: #d32f2f;
            transform: translateY(-2px);
        }
        
        #offBtn:disabled {
            background-color: #ef9a9a;
            cursor: not-allowed;
            transform: none;
        }
        
        .connection-status {
            margin-top: 20px;
            font-size: 14px;
            color: #666;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 LED远程控制</h1>
        
        <div id="statusCard" class="status-card status-disconnected">
            <div id="statusText" class="status-text">正在连接...</div>
        </div>
        
        <div class="button-group">
            <button id="onBtn" disabled>开灯</button>
            <button id="offBtn" disabled>关灯</button>
        </div>
        
        <div id="connectionStatus" class="connection-status">连接状态: 断开</div>
    </div>

    <script>
        const statusCard = document.getElementById('statusCard');
        const statusText = document.getElementById('statusText');
        const onBtn = document.getElementById('onBtn');
        const offBtn = document.getElementById('offBtn');
        const connectionStatus = document.getElementById('connectionStatus');
        
        // 建立SSE连接
        const eventSource = new EventSource('/events');
        
        // SSE连接成功
        eventSource.onopen = function() {
            connectionStatus.textContent = '连接状态: 已连接';
            onBtn.disabled = false;
            offBtn.disabled = false;
        };
        
        // 接收服务器推送的状态
        eventSource.addEventListener('state', function(e) {
            const state = e.data;
            updateUI(state);
        });
        
        // SSE连接错误
        eventSource.onerror = function() {
            connectionStatus.textContent = '连接状态: 断开，正在重连...';
            statusCard.className = 'status-card status-disconnected';
            statusText.textContent = '连接断开';
            onBtn.disabled = true;
            offBtn.disabled = true;
        };
        
        // 更新UI
        function updateUI(state) {
            if (state === 'on') {
                statusCard.className = 'status-card status-on';
                statusText.textContent = 'LED状态: 开启';
            } else if (state === 'off') {
                statusCard.className = 'status-card status-off';
                statusText.textContent = 'LED状态: 关闭';
            }
        }
        
        // 发送控制指令
        async function sendCommand(action) {
            try {
                onBtn.disabled = true;
                offBtn.disabled = true;
                
                const response = await fetch('/control', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: `action=${action}`
                });
                
                if (!response.ok) {
                    throw new Error('控制失败');
                }
            } catch (error) {
                console.error('Error:', error);
                alert('控制失败，请重试');
            } finally {
                onBtn.disabled = false;
                offBtn.disabled = false;
            }
        }
        
        // 按钮点击事件
        onBtn.addEventListener('click', () => sendCommand('on'));
        offBtn.addEventListener('click', () => sendCommand('off'));
    </script>
</body>
</html>
)HTML";

// ==================== 函数声明 ====================
void handleClient(WiFiClient client);
void sendSSEState(WiFiClient &client, const char* state);
void broadcastState(const char* state);
void cleanupClients();
void handleRoot(WiFiClient &client);
void handleEvents(WiFiClient &client);
void handleControl(WiFiClient &client);
String readLine(WiFiClient &client);

// ==================== 初始化函数 ====================
void setup() {
    Serial.begin(115200);
    Serial.println("\nESP32 SSE LED Controller Starting...");
    
    // 初始化LED引脚
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    
    // 初始化SSE客户端数组
    for (int i = 0; i < maxClients; i++) {
        sseClients[i] = WiFiClient();
    }
    
    // 连接WiFi
    Serial.printf("Connecting to %s...\n", ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // 启动HTTP服务器
    server.begin();
    Serial.println("HTTP server started on port 80");
}

// ==================== 主循环 ====================
void loop() {
    // 检查WiFi连接
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.reconnect();
        delay(1000);
        return;
    }
    
    // 接受新的客户端连接
    WiFiClient client = server.available();
    if (client) {
        handleClient(client);
    }
    
    // 清理超时的客户端
    cleanupClients();
    
    delay(10);
}

// ==================== 客户端请求处理（核心修复部分） ====================
void handleClient(WiFiClient client) {
    // 等待客户端发送数据
    unsigned long timeout = millis();
    while (client.connected() && !client.available()) {
        if (millis() - timeout > 1000) {
            client.stop();
            return;
        }
        delay(1);
    }
    
    // 读取请求行
    String requestLine = readLine(client);
    Serial.println("Request: " + requestLine);
    
    // 读取并丢弃请求头（关键：不读取请求体）
    String header;
    do {
        header = readLine(client);
        // Serial.println("Header: " + header); // 调试用
    } while (header != ""); // 空行表示请求头结束
    
    // 路由请求
    if (requestLine.indexOf("GET / ") != -1) {
        handleRoot(client);
    } else if (requestLine.indexOf("GET /events") != -1) {
        handleEvents(client);
    } else if (requestLine.indexOf("POST /control") != -1) {
        handleControl(client); // 请求体留到handleControl中读取
    } else {
        // 404 Not Found
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("404 Not Found");
        client.stop();
    }
}

// ==================== 按行读取HTTP数据 ====================
String readLine(WiFiClient &client) {
    String line = "";
    char c;
    while (client.available()) {
        c = client.read();
        if (c == '\r') {
            // 读取换行符
            if (client.available()) {
                client.read(); // 读取'\n'
            }
            break;
        }
        line += c;
    }
    return line;
}

// ==================== 根路径处理(返回网页) ====================
void handleRoot(WiFiClient &client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println("Connection: close");
    client.println();
    client.print(htmlPage);
    client.stop();
}

// ==================== SSE连接处理 ====================
void handleEvents(WiFiClient &client) {
    // 查找空闲的客户端槽位
    int freeSlot = -1;
    for (int i = 0; i < maxClients; i++) {
        if (!sseClients[i].connected()) {
            freeSlot = i;
            break;
        }
    }
    
    if (freeSlot == -1) {
        // 服务器已满
        client.println("HTTP/1.1 503 Service Unavailable");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("Too many connections");
        client.stop();
        return;
    }
    
    // 发送SSE响应头
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/event-stream");
    client.println("Cache-Control: no-cache");
    client.println("Connection: keep-alive");
    client.println("Access-Control-Allow-Origin: *");
    client.println();
    
    // 保存客户端连接
    sseClients[freeSlot] = client;
    
    // 立即发送当前状态
    sendSSEState(sseClients[freeSlot], ledState ? "on" : "off");
    
    Serial.printf("New SSE client connected. Slot: %d\n", freeSlot);
}

// ==================== 控制指令处理（修复后） ====================
void handleControl(WiFiClient &client) {
    // 读取POST请求体
    String body = "";
    while (client.available()) {
        body += (char)client.read();
    }
    
    Serial.println("POST Body: " + body); // 调试用，查看实际收到的请求体
    
    // 解析action参数
    int actionIndex = body.indexOf("action=");
    if (actionIndex == -1) {
        client.println("HTTP/1.1 400 Bad Request");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("Missing action parameter");
        client.stop();
        return;
    }
    
    String action = body.substring(actionIndex + 7);
    action.trim();
    
    Serial.println("Action received: " + action); // 调试用
    
    // 执行控制
    bool stateChanged = false;
    if (action == "on" && !ledState) {
        digitalWrite(ledPin, HIGH);
        ledState = true;
        stateChanged = true;
        Serial.println("LED turned ON");
    } else if (action == "off" && ledState) {
        digitalWrite(ledPin, LOW);
        ledState = false;
        stateChanged = true;
        Serial.println("LED turned OFF");
    } else {
        Serial.println("No state change needed");
    }
    
    // 返回成功响应
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("OK");
    client.stop();
    
    // 如果状态改变，广播给所有SSE客户端
    if (stateChanged) {
        broadcastState(ledState ? "on" : "off");
    }
}

// ==================== SSE辅助函数 ====================
void sendSSEState(WiFiClient &client, const char* state) {
    if (client.connected()) {
        client.print("event: state\n");
        client.printf("data: %s\n\n", state);
    }
}

void broadcastState(const char* state) {
    Serial.printf("Broadcasting state: %s\n", state);
    for (int i = 0; i < maxClients; i++) {
        if (sseClients[i].connected()) {
            sendSSEState(sseClients[i], state);
        }
    }
}

void cleanupClients() {
    for (int i = 0; i < maxClients; i++) {
        if (sseClients[i].connected() && !sseClients[i].available()) {
            // 发送一个空数据来检测连接是否仍然活跃
            if (!sseClients[i].connected()) {
                sseClients[i].stop();
                Serial.printf("Client %d disconnected\n", i);
            }
        }
    }
}
