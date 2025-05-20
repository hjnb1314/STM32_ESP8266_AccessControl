"use strict";
const common_vendor = require("../../common/vendor.js");
const _sfc_main = {
  data() {
    return {
      // 门禁功能相关
      socketTask: null,
      info: "未连接",
      statusText: "未知",
      loading: false,
      // 天气显示
      weatherText: "",
      // 密码弹窗相关
      showPwdModal: true,
      inputPwd: "",
      pwdError: ""
    };
  },
  onLoad() {
    this.getWeather();
  },
  methods: {
    // 1. 密码弹窗逻辑
    checkPwd() {
      if (this.inputPwd === "123456") {
        this.showPwdModal = false;
        this.inputPwd = "";
        this.pwdError = "";
        this.connectWS();
      } else {
        this.pwdError = "密码错误，请重试";
        this.inputPwd = "";
      }
    },
    // 2. 天气组件逻辑（极简版佛山）
    getWeather() {
      common_vendor.index.request({
        url: "https://wttr.in/foshan?format=%C+%t",
        success: (res) => {
          this.weatherText = res.data;
        },
        fail: () => {
          this.weatherText = "天气获取失败";
        }
      });
    },
    // 3. 门禁WebSocket功能
    connectWS() {
      if (this.socketTask) {
        this.socketTask.close();
        this.socketTask = null;
      }
      this.info = "正在连接WebSocket...";
      this.socketTask = common_vendor.index.connectSocket({
        url: "wss://ad37-219-135-121-157.ngrok-free.app",
        success: () => {
          this.info = "WebSocket连接中...";
        },
        fail: () => {
          this.info = "WebSocket连接失败";
        }
      });
      this.socketTask.onOpen(() => {
        this.info = "WebSocket已连接";
      });
      this.socketTask.onMessage((msg) => {
        try {
          const data = JSON.parse(msg.data);
          if (data.type === "state") {
            this.statusText = data.k ? "已打开" : "已关闭";
          }
          if (data.type === "result") {
            this.info = data.msg;
            this.loading = false;
          }
        } catch (e) {
          this.info = "收到非法消息";
        }
      });
      this.socketTask.onError(() => {
        this.info = "WebSocket连接失败";
      });
      this.socketTask.onClose(() => {
        this.info = "WebSocket已断开";
      });
    },
    // 门禁指令
    sendCommand(cmd) {
      if (this.socketTask) {
        this.loading = true;
        this.socketTask.send({
          data: JSON.stringify({ cmd })
        });
      } else {
        this.info = "WebSocket未连接";
      }
    },
    openDoor() {
      this.sendCommand("open");
    },
    closeDoor() {
      this.sendCommand("close");
    }
  }
};
function _sfc_render(_ctx, _cache, $props, $setup, $data, $options) {
  return common_vendor.e({
    a: $data.showPwdModal
  }, $data.showPwdModal ? common_vendor.e({
    b: $data.inputPwd,
    c: common_vendor.o(($event) => $data.inputPwd = $event.detail.value),
    d: common_vendor.o((...args) => $options.checkPwd && $options.checkPwd(...args)),
    e: $data.pwdError
  }, $data.pwdError ? {
    f: common_vendor.t($data.pwdError)
  } : {}) : {}, {
    g: $data.weatherText
  }, $data.weatherText ? {
    h: common_vendor.t($data.weatherText)
  } : {}, {
    i: common_vendor.t($data.info),
    j: common_vendor.t($data.statusText),
    k: common_vendor.o((...args) => $options.openDoor && $options.openDoor(...args)),
    l: $data.loading,
    m: common_vendor.o((...args) => $options.closeDoor && $options.closeDoor(...args)),
    n: $data.loading
  });
}
const MiniProgramPage = /* @__PURE__ */ common_vendor._export_sfc(_sfc_main, [["render", _sfc_render]]);
wx.createPage(MiniProgramPage);
//# sourceMappingURL=../../../.sourcemap/mp-weixin/pages/index/index.js.map
