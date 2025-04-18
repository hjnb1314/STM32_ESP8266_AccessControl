bool toggle = false;

void setup() {
  // 初始化串口通信，波特率设置为9600
  Serial.begin(9600);
}

void loop() {
  // 根据toggle变量的值发送字符'0'或'1'
  if (toggle) {
    Serial.write('1');
  } else {
    Serial.write('0');
  }

  // 切换状态，每10秒钟更改一次发送数据
  toggle = !toggle;
  delay(10000); // 延时10秒 (10000毫秒)
}
