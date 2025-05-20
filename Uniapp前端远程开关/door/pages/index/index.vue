<template>
  <view class="container">
    <!-- 密码弹窗 -->
    <view v-if="showPwdModal" class="pwd-mask">
      <view class="pwd-modal">
        <input v-model="inputPwd" password maxlength="6" placeholder="请输入6位密码" class="pwd-input"/>
        <button type="primary" @click="checkPwd">确定</button>
        <view class="error" v-if="pwdError">{{ pwdError }}</view>
      </view>
    </view>

    <!-- 设置域名弹窗 -->
    <view v-if="showDomainModal" class="pwd-mask">
      <view class="pwd-modal">
        <input v-model="inputDomain" placeholder="请输入 ngrok 域名" class="pwd-input"/>
        <button type="primary" @click="saveDomain">保存并重连</button>
        <button @click="showDomainModal=false">取消</button>
        <view class="error" v-if="domainError">{{ domainError }}</view>
      </view>
    </view>

    <!-- 天气信息 -->
    <view class="weather-card">
      <view class="weather-title">当前天气:佛山</view>
      <view v-if="weatherText">
        {{ weatherText }}
      </view>
      <view v-else>
        正在加载天气...
      </view>
    </view>

    <!-- 门禁功能 -->
    <view class="info">{{ info }}</view>
    <view class="status">当前门禁状态：{{ statusText }}</view>
    <button type="primary" @click="openDoor" :disabled="loading">打开门禁</button>
    <button type="warn" @click="closeDoor" :disabled="loading">关闭门禁</button>
    <button @click="showDomainModal=true" type="default">设置ngrok域名</button>
    <view style="font-size:24rpx;color:#aaa;margin-top:8rpx;">
      当前域名: {{ wsDomain }}
    </view>
  </view>
</template>

<script>
export default {
  data() {
    return {
      // 门禁功能相关
      socketTask: null,
      info: '未连接',
      statusText: '未知',
      loading: false,

      // 天气显示
      weatherText: '',

      // 密码弹窗
      showPwdModal: true,
      inputPwd: '',
      pwdError: '',

      // 设置域名弹窗
      showDomainModal: false,
      inputDomain: '',
      domainError: '',

      // 当前ws域名
      wsDomain: 'ad37-219-135-121-157.ngrok-free.app', // 默认域名

      // 新增
      reconnectTimer: null,
      pingTimer: null,
    }
  },
  onLoad() {
    // 优先用本地存储域名
    const saved = uni.getStorageSync('wsDomain');
    if (saved) {
      this.wsDomain = saved;
    }
    this.getWeather();
  },
  onUnload() {
    // 页面卸载清理定时器和断开ws
    this.cleanupWS();
  },
  methods: {
    // 密码弹窗逻辑
    checkPwd() {
      if (this.inputPwd === '123456') {
        this.showPwdModal = false;
        this.inputPwd = '';
        this.pwdError = '';
        this.connectWS(); // 密码对了才连接WebSocket
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

    // 新增：清理所有定时器和socket
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

    // 门禁WebSocket（新增心跳和断线重连）
    connectWS() {
      this.cleanupWS();

      this.info = "正在连接WebSocket...";
      const wsurl = 'wss://' + this.wsDomain;

      let that = this; // 保证this指向
      this.socketTask = uni.connectSocket({
        url: wsurl,
        success: () => { that.info = "WebSocket连接中..."; },
        fail: () => { that.info = "WebSocket连接失败"; }
      });

      // 兼容小程序与H5，必须放onOpen事件里注册
      this.socketTask.onOpen(() => {
        that.info = "WebSocket已连接";
        // 启动前端心跳，每30秒发一次
        that.pingTimer = setInterval(() => {
          try {
            if (that.socketTask) {
              that.socketTask.send({ data: JSON.stringify({ type: 'ping' }) });
            }
          } catch (e) { /* 忽略 */ }
        }, 30000);
      });

      this.socketTask.onMessage((msg) => {
        try {
          const data = JSON.parse(msg.data);
          if (data.type === 'ping') return; // 心跳包直接忽略
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
      this.cleanupWS(); // 先清理一遍防止多连
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
        // 防止永远卡死：4秒超时自动解锁
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
.container { padding: 40rpx 24rpx; position: relative; }
.weather-card {
  padding: 24rpx;
  margin-bottom: 36rpx;
  background: #f2f5fc;
  border-radius: 14rpx;
  box-shadow: 0 2rpx 12rpx #eef2fa;
}
.weather-title { color: #7ba3f7; font-weight: bold; margin-bottom: 12rpx; }
.status { margin-bottom: 40rpx; font-size: 32rpx; color: #333; }
.info { margin-bottom: 32rpx; color: #888; }
button { margin-bottom: 24rpx; }

/* 密码弹窗样式 */
.pwd-mask {
  position: fixed; top: 0; left: 0; right: 0; bottom: 0;
  background: rgba(0,0,0,0.15); z-index: 1000;
  display: flex; align-items: center; justify-content: center;
}
.pwd-modal {
  background: #fff; padding: 48rpx 36rpx; border-radius: 16rpx;
  box-shadow: 0 8rpx 32rpx #eee;
  width: 80vw; max-width: 520rpx; display: flex; flex-direction: column; align-items: center;
}
.pwd-input {
  border: 1rpx solid #aaa;
  margin-bottom: 28rpx;
  padding: 22rpx 16rpx;
  width: 100%;
  border-radius: 8rpx;
  font-size: 34rpx;
  background: #f9f9f9;
}
.error {
  color: #e8442c;
  font-size: 26rpx;
  margin-top: 16rpx;
}
</style>
