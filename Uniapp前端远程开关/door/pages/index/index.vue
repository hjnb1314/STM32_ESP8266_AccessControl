<template>
  <view class="container">
    <!-- 密码弹窗 -->
    <view v-if="showPwdModal" class="pwd-mask">
      <view class="pwd-modal">
        <view class="modal-title">门禁登录</view>
        <input v-model="inputPwd" password maxlength="6" placeholder="请输入6位密码" class="pwd-input"/>
        <button type="primary" class="modal-btn" @click="checkPwd">确定</button>
        <view class="error" v-if="pwdError">{{ pwdError }}</view>
      </view>
    </view>

    <!-- 设置域名弹窗 -->
    <view v-if="showDomainModal" class="pwd-mask">
      <view class="pwd-modal">
        <view class="modal-title">设置 ngrok 域名</view>
        <input v-model="inputDomain" placeholder="请输入 ngrok 域名" class="pwd-input"/>
        <button type="primary" class="modal-btn" @click="saveDomain">保存并重连</button>
        <button class="modal-btn cancel-btn" @click="showDomainModal=false">取消</button>
        <view class="error" v-if="domainError">{{ domainError }}</view>
      </view>
    </view>

    <!-- 天气信息 -->
    <view class="card weather-card">
      <view class="weather-title">🌤️ 当前天气：佛山</view>
      <view class="weather-content">
        <text v-if="weatherText">{{ weatherText }}</text>
        <text v-else style="color:#999;">正在加载天气...</text>
      </view>
    </view>

    <!-- 门禁与状态 -->
    <view class="card status-card">
      <view class="info">{{ info }}</view>
      <view class="status-row">
        <text class="status-label">当前门禁状态：</text>
        <text :class="statusText==='已打开'?'open':'close'">{{ statusText }}</text>
      </view>
      <view class="btn-row">
        <button type="primary" class="main-btn" @click="openDoor" :disabled="loading">打开门禁</button>
        <button type="warn" class="main-btn" @click="closeDoor" :disabled="loading">关闭门禁</button>
      </view>
      <button @click="showDomainModal=true" type="default" class="minor-btn">设置ngrok域名</button>
      <view class="domain-tip">当前域名：<text class="domain-value">{{ wsDomain }}</text></view>
    </view>
  </view>
</template>

<script>
export default {
  data() {
    return {
      socketTask: null,
      info: '未连接',
      statusText: '未知',
      loading: false,
      weatherText: '',
      showPwdModal: true,
      inputPwd: '',
      pwdError: '',
      showDomainModal: false,
      inputDomain: '',
      domainError: '',
      wsDomain: 'ad37-219-135-121-157.ngrok-free.app',
      reconnectTimer: null,
      pingTimer: null,
    }
  },
  onLoad() {
    const saved = uni.getStorageSync('wsDomain');
    if (saved) {
      this.wsDomain = saved;
    }
    this.getWeather();
  },
  onUnload() {
    this.cleanupWS();
  },
  methods: {
    // 密码弹窗逻辑
    checkPwd() {
      if (this.inputPwd === '123456') {
        this.showPwdModal = false;
        this.inputPwd = '';
        this.pwdError = '';
        this.connectWS();
      } else {
        this.pwdError = '密码错误，请重试';
        this.inputPwd = '';
      }
    },

    // 设置域名逻辑
    saveDomain() {
      if (!this.inputDomain || !/^[\w\.\-]+\.ngrok\-free\.app$/.test(this.inputDomain)) {
        this.domainError = '请输入有效ngrok域名！';
        return;
      }
      this.domainError = '';
      this.wsDomain = this.inputDomain;
      uni.setStorageSync('wsDomain', this.wsDomain);
      this.showDomainModal = false;
      if (!this.showPwdModal) {
        this.connectWS();
      }
    },

    // 天气
    getWeather() {
      uni.request({
        url: "https://wttr.in/foshan?format=%C+%t",
        success: (res) => {
          this.weatherText = res.data;
        },
        fail: () => {
          this.weatherText = "天气获取失败";
        }
      });
    },

    // 清理定时器和socket
    cleanupWS() {
      if (this.socketTask) {
        try { this.socketTask.close(); } catch(e){}
        this.socketTask = null;
      }
      if (this.pingTimer) {
        clearInterval(this.pingTimer);
        this.pingTimer = null;
      }
      if (this.reconnectTimer) {
        clearTimeout(this.reconnectTimer);
        this.reconnectTimer = null;
      }
    },

    // WebSocket
    connectWS() {
      this.cleanupWS();
      this.info = "正在连接WebSocket...";
      const wsurl = 'wss://' + this.wsDomain;

      let that = this;
      this.socketTask = uni.connectSocket({
        url: wsurl,
        success: () => { that.info = "WebSocket连接中..."; },
        fail: () => { that.info = "WebSocket连接失败"; }
      });

      this.socketTask.onOpen(() => {
        that.info = "WebSocket已连接";
        that.pingTimer = setInterval(() => {
          try {
            if (that.socketTask) {
              that.socketTask.send({ data: JSON.stringify({ type: 'ping' }) });
            }
          } catch (e) { }
        }, 30000);
      });

      this.socketTask.onMessage((msg) => {
        try {
          const data = JSON.parse(msg.data);
          if (data.type === 'ping') return;
          if (data.type === 'state') {
            that.statusText = data.k ? "已打开" : "已关闭";
          }
          if (data.type === 'result') {
            that.info = data.msg;
            that.loading = false;
          }
        } catch (e) {
          that.info = "收到非法消息";
        }
      });

      this.socketTask.onError(() => {
        that.info = "WebSocket连接失败";
        that.loading = false;
        that.autoReconnect();
      });

      this.socketTask.onClose(() => {
        that.info = "WebSocket已断开";
        that.loading = false;
        that.autoReconnect();
      });
    },

    // 自动重连
    autoReconnect() {
      this.cleanupWS();
      this.reconnectTimer = setTimeout(() => {
        this.connectWS();
      }, 2000);
    },

    // 门禁指令
    sendCommand(cmd) {
      if (this.socketTask) {
        this.loading = true;
        try {
          this.socketTask.send({
            data: JSON.stringify({ cmd: cmd })
          });
        } catch (e) {
          this.info = "发送失败";
          this.loading = false;
          return;
        }
        setTimeout(() => {
          this.loading = false;
        }, 4000);
      } else {
        this.info = "WebSocket未连接";
        this.loading = false;
      }
    },
    openDoor() {
      this.sendCommand("open");
    },
    closeDoor() {
      this.sendCommand("close");
    }
  }
}
</script>

<style>
.container { padding: 36rpx 18rpx; background: #f6f8fa; min-height: 100vh; }

.card {
  background: #fff;
  border-radius: 16rpx;
  box-shadow: 0 4rpx 16rpx #ecf0fa;
  margin-bottom: 30rpx;
  padding: 26rpx 22rpx;
}

.weather-card {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
}
.weather-title {
  color: #388aff;
  font-weight: 600;
  font-size: 34rpx;
  margin-bottom: 10rpx;
}
.weather-content {
  font-size: 28rpx;
}

.status-card {
  margin-top: 8rpx;
  padding-top: 30rpx;
}
.info {
  font-size: 28rpx;
  color: #888;
  margin-bottom: 18rpx;
}
.status-row {
  display: flex;
  align-items: center;
  margin-bottom: 20rpx;
}
.status-label {
  font-size: 30rpx;
  color: #333;
  margin-right: 12rpx;
}
.open {
  color: #17bc68;
  font-size: 32rpx;
  font-weight: bold;
}
.close {
  color: #f56c6c;
  font-size: 32rpx;
  font-weight: bold;
}

.btn-row {
  display: flex;
  justify-content: flex-start;
  gap: 20rpx;
  margin-bottom: 18rpx;
}
.main-btn {
  min-width: 160rpx;
  font-size: 30rpx;
  border-radius: 10rpx;
}
.minor-btn {
  margin-top: 10rpx;
  width: 96%;
  font-size: 28rpx;
  border-radius: 8rpx;
  color: #388aff;
  border: 1rpx solid #c6d2ef;
  background: #f8fafc;
}

.domain-tip {
  margin-top: 12rpx;
  color: #99a7ba;
  font-size: 22rpx;
  word-break: break-all;
}
.domain-value {
  color: #2462d1;
  font-weight: 500;
}

.pwd-mask {
  position: fixed; top: 0; left: 0; right: 0; bottom: 0;
  background: rgba(30,36,60,0.18); z-index: 1000;
  display: flex; align-items: center; justify-content: center;
}
.pwd-modal {
  background: #fff;
  padding: 54rpx 36rpx 34rpx 36rpx;
  border-radius: 20rpx;
  box-shadow: 0 8rpx 32rpx #e3e7ef;
  width: 80vw; max-width: 520rpx;
  display: flex; flex-direction: column; align-items: center;
}
.modal-title {
  font-size: 34rpx;
  font-weight: bold;
  color: #345eb5;
  margin-bottom: 30rpx;
  letter-spacing: 2rpx;
}
.pwd-input {
  border: 1.5rpx solid #b4bce5;
  margin-bottom: 30rpx;
  padding: 22rpx 16rpx;
  width: 96%;
  border-radius: 10rpx;
  font-size: 34rpx;
  background: #f8faff;
}
.modal-btn {
  min-width: 100rpx;
  font-size: 30rpx;
  border-radius: 10rpx;
  margin-bottom: 18rpx;
  background: #388aff;
  color: #fff;
}
.cancel-btn {
  background: #f3f3f3 !important;
  color: #1a2738 !important;
  border: 1rpx solid #b4bce5 !important;
}
.error {
  color: #e8442c;
  font-size: 26rpx;
  margin-top: 16rpx;
  text-align: center;
}
</style>
