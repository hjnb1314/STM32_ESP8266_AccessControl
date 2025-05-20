const mqtt = require('mqtt');
const WebSocket = require('ws');

// ==== MQTT连接参数 ====
const MQTT_HOST = 'mqtt://117.78.5.125:1883';
const MQTT_OPTIONS = {
  clientId: '67f74c6f2902516e867003b8_66666666_0_0_2025041005',
  username: '67f74c6f2902516e867003b8_66666666',
  password: '53d46fc2ee51d7e7e5b91ce3542b5886fd2b6b3bd3748cedd363f8fcda3bc668'
};
const DEVICE_ID = '67f74c6f2902516e867003b8_66666666';
const MQTT_TOPIC = `$oc/devices/${DEVICE_ID}/sys/properties/report`;

const WS_PORT = 3001;
const mqttClient = mqtt.connect(MQTT_HOST, MQTT_OPTIONS);

let currentState = false; // 只本地记录，不再主动同步云端

// =============== WebSocket 服务器 ===============
const wss = new WebSocket.Server({ port: WS_PORT });r
console.log(`WebSocket服务器已启动，端口：${WS_PORT}`);

wss.on('connection', function connection(ws, req) {
  ws.send(JSON.stringify({ type: 'state', k: currentState }));
  ws.on('message', function incoming(message) {
    try {
      const msg = JSON.parse(message.toString());
      if (msg.cmd === 'open' || msg.cmd === 'close') {
        const kVal = msg.cmd === 'open';
        // 只向云端上报属性
        const payload = JSON.stringify({
          services: [{ service_id: "door", properties: { k: kVal } }]
        });
        mqttClient.publish(MQTT_TOPIC, payload, {}, (err) => {
          if (err) {
            ws.send(JSON.stringify({ type: 'result', success: false, msg: '发布失败' }));
          } else {
            currentState = kVal;
            ws.send(JSON.stringify({ type: 'result', success: true, msg: `门禁${kVal ? "打开" : "关闭"}指令已发送` }));
            broadcastState();
          }
        });
      } else {
        ws.send(JSON.stringify({ type: 'result', success: false, msg: '命令未知！' }));
      }
    } catch (e) {
      ws.send(JSON.stringify({ type: 'result', success: false, msg: '消息格式错误' }));
    }
  });
});

// 广播函数：只推送本地状态
function broadcastState() {
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify({ type: 'state', k: currentState }));
    }
  });
}

mqttClient.on('connect', () => {
  console.log('MQTT连接成功');
});
mqttClient.on('error', (err) => {
  console.error('MQTT连接错误:', err.message);
});
