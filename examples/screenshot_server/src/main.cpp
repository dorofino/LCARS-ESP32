// ============================================================
// LCARS Screenshot Server
//
// Serves a beautiful LCARS-themed web dashboard for capturing
// live screenshots of the ESP32's display over WiFi.
//
// First boot: creates a WiFi hotspot "LCARS-XXXX" for setup.
// Connect to it and browse to 192.168.4.1 to enter WiFi creds.
// After saving, the device connects to your network and serves
// the screenshot dashboard at its IP address.
// ============================================================

#include <lcars.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <math.h>

// ── Globals ──────────────────────────────────────────────────

TFT_eSPI tft;
LcarsEngine engine;
WebServer server(80);
DNSServer dnsServer;
Preferences prefs;

enum AppMode { MODE_SETUP, MODE_CONNECTING, MODE_RUNNING };
AppMode appMode = MODE_SETUP;

bool serverStarted = false;
uint32_t screenshotCount = 0;
String deviceIP = "---";
String apName = "LCARS";
String savedSSID = "";
String savedPass = "";
uint32_t connectStartMs = 0;

// ============================================================
// PROGMEM HTML — WiFi Setup Page (captive portal)
// ============================================================

static const char HTML_SETUP[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LCARS Setup</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Antonio:wght@400;700&display=swap" rel="stylesheet">
<style>
:root{--bg:#000;--gold:#FFAA00;--amber:#FF9900;--butterscotch:#FF9966;
  --sunflower:#FFCC99;--ice:#99CCFF;--violet:#CC99FF;--green:#00FF88;
  --tomato:#FF5555;--white:#F5F6FA;--dim:#666;--track:#1A1A1A}
*{margin:0;padding:0;box-sizing:border-box}
body{background:var(--bg);color:var(--white);
  font-family:'Antonio',sans-serif;text-transform:uppercase;
  min-height:100vh;display:flex;justify-content:center;
  align-items:flex-start;padding:16px;letter-spacing:1px}
.lcars{max-width:520px;width:100%;display:grid;
  grid-template-columns:44px 1fr;grid-template-rows:32px 1fr 24px;gap:0}
.elbow-tl{background:var(--gold);border-bottom-right-radius:24px}
.topbar{background:var(--gold);border-radius:0 16px 16px 0;margin-left:3px;
  display:flex;align-items:center;padding:0 16px;
  color:#000;font-size:16px;font-weight:700;letter-spacing:3px}
.sidebar{display:flex;flex-direction:column;gap:2px;padding:4px 0}
.seg{border-radius:0 5px 5px 0;flex:1;min-height:10px}
.seg-1{background:var(--gold)}.seg-2{background:var(--amber)}
.seg-3{background:var(--butterscotch)}.seg-4{background:var(--violet)}
.content{padding:20px 16px 16px 16px;background:var(--bg)}
.elbow-bl{background:var(--butterscotch);border-top-right-radius:18px}
.botbar{background:var(--butterscotch);border-radius:0 12px 12px 0;
  margin-left:3px;display:flex;align-items:center;padding:0 12px;
  color:#000;font-size:10px;letter-spacing:2px}
h2{color:var(--gold);font-size:15px;margin-bottom:16px;letter-spacing:2px;
  font-weight:700}
label{display:block;color:var(--dim);font-size:12px;margin-bottom:6px;
  letter-spacing:2px}
input[type="text"],input[type="password"]{width:100%;padding:10px 12px;
  background:var(--track);border:1px solid #333;border-radius:8px;
  color:var(--white);font-family:'Antonio',sans-serif;font-size:14px;
  text-transform:none;letter-spacing:1px;outline:none;margin-bottom:14px}
input:focus{border-color:var(--gold)}
.btn{width:100%;padding:12px;background:var(--gold);color:#000;border:none;
  border-radius:10px;font-family:'Antonio',sans-serif;font-size:15px;
  font-weight:700;text-transform:uppercase;letter-spacing:3px;cursor:pointer;
  transition:filter .15s}
.btn:hover{filter:brightness(1.2)}
.hint{color:var(--dim);font-size:11px;margin-top:12px;text-align:center;
  letter-spacing:1px;text-transform:none}
.msg{background:var(--green);color:#000;padding:10px;border-radius:8px;
  text-align:center;font-weight:700;font-size:13px;margin-bottom:14px;
  display:none}
</style>
</head>
<body>
<div class="lcars">
  <div class="elbow-tl"></div>
  <div class="topbar">LCARS WIFI SETUP</div>
  <div class="sidebar">
    <div class="seg seg-1"></div><div class="seg seg-2"></div>
    <div class="seg seg-3"></div><div class="seg seg-4"></div>
  </div>
  <div class="content">
    <div class="msg" id="msg">CREDENTIALS SAVED. REBOOTING...</div>
    <h2>NETWORK CONFIGURATION</h2>
    <form id="wifiForm" method="POST" action="/save-wifi">
      <label>WIFI NETWORK NAME (SSID)</label>
      <input type="text" name="ssid" required autocomplete="off" placeholder="Enter your WiFi name">
      <label>PASSWORD</label>
      <input type="password" name="password" autocomplete="off" placeholder="Enter WiFi password">
      <button type="submit" class="btn">CONNECT</button>
    </form>
    <p class="hint">The device will reboot and connect to your network.<br>
    The IP address will be shown on the device screen.</p>
  </div>
  <div class="elbow-bl"></div>
  <div class="botbar">LCARS-ESP32</div>
</div>
<script>
document.getElementById('wifiForm').addEventListener('submit',function(e){
  e.preventDefault();
  var fd=new FormData(this);
  fetch('/save-wifi',{method:'POST',body:new URLSearchParams(fd)})
  .then(function(r){
    if(r.ok){
      document.getElementById('msg').style.display='block';
      document.getElementById('wifiForm').style.display='none';
    }
  });
});
</script>
</body>
</html>
)rawliteral";

// ============================================================
// PROGMEM HTML — Screenshot Dashboard (served after WiFi connected)
// ============================================================

static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LCARS Screenshot Server</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Antonio:wght@400;700&display=swap" rel="stylesheet">
<style>
:root {
  --bg:#000;--panel:#0a0a0a;
  --gold:#FFAA00;--amber:#FF9900;--orange:#FF8800;
  --butterscotch:#FF9966;--sunflower:#FFCC99;
  --ice:#99CCFF;--violet:#CC99FF;--periwinkle:#9999FF;
  --green:#00FF88;--tomato:#FF5555;
  --white:#F5F6FA;--dim:#666;--track:#1A1A1A;
}
*{margin:0;padding:0;box-sizing:border-box}
body{background:var(--bg);color:var(--white);
  font-family:'Antonio',sans-serif;text-transform:uppercase;
  min-height:100vh;display:flex;justify-content:center;
  align-items:flex-start;padding:16px;letter-spacing:1px}

/* ── LCARS Frame Layout ── */
.lcars{max-width:1080px;width:100%;display:grid;
  grid-template-columns:52px 1fr;grid-template-rows:36px 1fr 28px;gap:0}

.elbow-tl{background:var(--gold);border-bottom-right-radius:28px;
  grid-column:1;grid-row:1}
.topbar{background:var(--gold);grid-column:2;grid-row:1;
  border-radius:0 18px 18px 0;margin-left:3px;
  display:flex;align-items:center;padding:0 20px;gap:12px;
  color:#000;font-size:18px;font-weight:700;letter-spacing:3px}
.topbar .pill{margin-left:auto;background:#000;color:var(--gold);
  padding:2px 14px;border-radius:10px;font-size:12px;font-weight:400}

.sidebar{grid-column:1;grid-row:2;display:flex;flex-direction:column;
  gap:2px;padding:4px 0}
.seg{border-radius:0 6px 6px 0;flex:1;min-height:12px}
.seg-1{background:var(--gold)}.seg-2{background:var(--amber)}
.seg-3{background:var(--butterscotch)}.seg-4{background:var(--violet)}
.seg-5{background:var(--sunflower)}.seg-6{background:var(--ice)}

.content{grid-column:2;grid-row:2;padding:16px 16px 12px 19px;
  background:var(--bg);min-height:300px;display:flex;flex-direction:column}

.elbow-bl{background:var(--butterscotch);border-top-right-radius:22px;
  grid-column:1;grid-row:3}
.botbar{background:var(--butterscotch);grid-column:2;grid-row:3;
  border-radius:0 14px 14px 0;margin-left:3px;
  display:flex;align-items:center;padding:0 16px;
  color:#000;font-size:11px;font-weight:400;letter-spacing:2px}
.botbar .status{margin-left:auto;font-size:11px;opacity:.7}

/* Canvas */
.canvas-wrap{background:var(--panel);border:1px solid #222;
  border-radius:6px;padding:8px;text-align:center;flex:1;
  display:flex;align-items:center;justify-content:center;
  min-height:200px;position:relative;overflow:hidden}
.canvas-wrap canvas{max-width:100%;height:auto;image-rendering:pixelated;
  border-radius:3px}
.canvas-wrap .placeholder{color:var(--dim);font-size:14px;letter-spacing:2px}

/* Controls */
.controls{display:flex;align-items:center;gap:8px;margin-top:12px;flex-wrap:wrap}
.btn{border:none;cursor:pointer;font-family:'Antonio',sans-serif;
  text-transform:uppercase;letter-spacing:2px;font-size:13px;
  font-weight:700;border-radius:14px;padding:8px 22px;
  transition:filter .15s,transform .1s}
.btn:hover{filter:brightness(1.2)}
.btn:active{transform:scale(.97)}
.btn-gold{background:var(--gold);color:#000}
.btn-ice{background:var(--ice);color:#000}
.btn-dim{background:var(--track);color:var(--dim);font-weight:400;
  padding:8px 14px;font-size:12px}
.btn-dim.active{background:var(--amber);color:#000;font-weight:700}
.btn-tomato{background:var(--tomato);color:#000;font-size:11px;padding:6px 14px}
.auto-group{display:flex;gap:3px;align-items:center}
.auto-label{color:var(--dim);font-size:11px;margin-right:4px}
.controls .spacer{flex:1}

/* Status */
.status-line{margin-top:10px;color:var(--dim);font-size:12px;
  letter-spacing:2px;display:flex;gap:16px;flex-wrap:wrap}
.status-line .val{color:var(--sunflower)}
.status-line .err{color:var(--tomato)}
.timestamp{color:var(--dim);font-size:11px;margin-top:6px;letter-spacing:1px}

@media(max-width:640px){
  .lcars{grid-template-columns:32px 1fr;grid-template-rows:28px 1fr 22px}
  .topbar{font-size:14px;padding:0 12px;border-radius:0 12px 12px 0}
  .content{padding:10px 10px 8px 13px}
  .btn{padding:6px 14px;font-size:12px}
}
</style>
</head>
<body>

<div class="lcars">
  <div class="elbow-tl"></div>
  <div class="topbar">
    LCARS SCREENSHOT SERVER
    <span class="pill">47634.44</span>
  </div>

  <div class="sidebar">
    <div class="seg seg-1"></div><div class="seg seg-2"></div>
    <div class="seg seg-3"></div><div class="seg seg-4"></div>
    <div class="seg seg-5"></div><div class="seg seg-6"></div>
  </div>

  <div class="content">
    <div class="canvas-wrap" id="canvasWrap">
      <span class="placeholder" id="placeholder">AWAITING SIGNAL...</span>
      <canvas id="screenCanvas" style="display:none"></canvas>
    </div>

    <div class="controls">
      <div class="auto-group">
        <button class="btn btn-dim active" onclick="setAuto(0,this)">OFF</button>
        <button class="btn btn-dim" onclick="setAuto(1000,this)">1S</button>
        <button class="btn btn-dim" onclick="setAuto(3000,this)">3S</button>
        <button class="btn btn-dim" onclick="setAuto(5000,this)">5S</button>
        <button class="btn btn-gold" id="btnCapture" onclick="captureScreen()">CAPTURE</button>
      </div>
      <div class="spacer"></div>
      <button class="btn btn-ice" id="btnDownload" onclick="downloadPNG()" style="display:none">DOWNLOAD PNG</button>
      <button class="btn btn-tomato" onclick="resetWifi()">RESET WIFI</button>
    </div>

    <div class="status-line" id="statusLine">
      <span>SIGNAL: <span class="val" id="stRssi">---</span></span>
      <span>UPTIME: <span class="val" id="stUptime">---</span></span>
      <span>SCREENSHOTS: <span class="val" id="stShots">0</span></span>
    </div>
    <div class="timestamp" id="timestamp"></div>
  </div>

  <div class="elbow-bl"></div>
  <div class="botbar">
    LCARS-ESP32 SCREENSHOT SERVER
    <span class="status" id="botStatus">STANDBY</span>
  </div>
</div>

<script>
var autoTimer=null,capturing=false;

async function captureScreen(){
  if(capturing)return;capturing=true;
  var btn=document.getElementById('btnCapture');
  btn.textContent='CAPTURING...';btn.disabled=true;
  try{
    var resp=await fetch('/screenshot');
    if(!resp.ok)throw new Error('HTTP '+resp.status);
    var buf=await resp.arrayBuffer();
    var dv=new DataView(buf);
    var w=dv.getUint16(0,true),h=dv.getUint16(2,true);
    var cv=document.getElementById('screenCanvas');
    var scale=3;
    cv.width=w*scale;cv.height=h*scale;
    var ctx=cv.getContext('2d');
    var tmp=document.createElement('canvas');
    tmp.width=w;tmp.height=h;
    var tc=tmp.getContext('2d');
    var img=tc.createImageData(w,h);
    for(var i=0;i<w*h;i++){
      var p=dv.getUint16(4+i*2,false);
      img.data[i*4]=((p>>11)&0x1F)<<3;
      img.data[i*4+1]=((p>>5)&0x3F)<<2;
      img.data[i*4+2]=(p&0x1F)<<3;
      img.data[i*4+3]=255;
    }
    tc.putImageData(img,0,0);
    ctx.imageSmoothingEnabled=false;
    ctx.drawImage(tmp,0,0,w*scale,h*scale);
    cv.style.display='block';
    document.getElementById('placeholder').style.display='none';
    document.getElementById('btnDownload').style.display='';
    document.getElementById('timestamp').textContent=
      'CAPTURED '+new Date().toLocaleTimeString().toUpperCase();
    document.getElementById('botStatus').textContent='ACTIVE';
    updateStatus();
  }catch(e){
    document.getElementById('timestamp').innerHTML=
      '<span class="err">FAILED: '+e.message+'</span>';
  }
  btn.textContent='CAPTURE';btn.disabled=false;capturing=false;
}

function setAuto(ms,el){
  if(autoTimer){clearInterval(autoTimer);autoTimer=null}
  document.querySelectorAll('.auto-group .btn-dim').forEach(
    function(b){b.classList.remove('active')});
  el.classList.add('active');
  if(ms>0){captureScreen();autoTimer=setInterval(captureScreen,ms)}
}

function downloadPNG(){
  var cv=document.getElementById('screenCanvas');
  if(!cv.width)return;
  var a=document.createElement('a');
  a.download='lcars-screenshot-'+Date.now()+'.png';
  a.href=cv.toDataURL('image/png');a.click();
}

async function updateStatus(){
  try{
    var r=await fetch('/status');var d=await r.json();
    document.getElementById('stRssi').textContent=d.rssi+'dBm';
    var m=Math.floor(d.uptime/60),s=d.uptime%60;
    document.getElementById('stUptime').textContent=(m>0?m+'M ':'')+s+'S';
    document.getElementById('stShots').textContent=d.screenshots;
  }catch(e){}
}

function resetWifi(){
  if(confirm('Reset WiFi credentials? Device will reboot into setup mode.')){
    fetch('/reset',{method:'POST'}).then(function(){
      document.getElementById('botStatus').textContent='REBOOTING...';
    });
  }
}

document.addEventListener('DOMContentLoaded',function(){
  setTimeout(captureScreen,500);setInterval(updateStatus,5000);
});
</script>
</body>
</html>
)rawliteral";

// ============================================================
// SetupScreen — shown on device during AP mode
// ============================================================

class SetupScreen : public LcarsScreen {
public:
    const char* title() const override { return "WIFI SETUP"; }
    uint32_t refreshIntervalMs() const override { return 500; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        LcarsWidgets::drawLabel(spr, x, y, "SETUP MODE", _theme->accent);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "HOTSPOT", apName.c_str(),
                                    LCARS_GOLD, _theme->textDim);
        y += 15;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "BROWSE TO", "192.168.4.1",
                                    LCARS_ICE, _theme->textDim);
        y += 17;

        y += 4;
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        LcarsWidgets::drawLabel(spr, x, y, "INSTRUCTIONS", _theme->accent);
        y += 14;

        LcarsFont::drawText(spr, "1. CONNECT TO WIFI HOTSPOT",
                            x, y, LCARS_FONT_SM, _theme->textDim);
        y += 14;
        LcarsFont::drawText(spr, "   SHOWN ABOVE",
                            x, y, LCARS_FONT_SM, _theme->textDim);
        y += 14;
        LcarsFont::drawText(spr, "2. OPEN BROWSER TO 192.168.4.1",
                            x, y, LCARS_FONT_SM, _theme->textDim);
        y += 14;
        LcarsFont::drawText(spr, "3. ENTER YOUR WIFI CREDENTIALS",
                            x, y, LCARS_FONT_SM, _theme->textDim);
        y += 18;

        LcarsWidgets::drawIndicator(spr, x + 4, y + 5, 4, LCARS_AMBER, true);
        LcarsWidgets::drawLabel(spr, x + 14, y + 5, "AWAITING CONFIGURATION",
                                LCARS_AMBER, LCARS_FONT_SM, ML_DATUM);
    }
};

// ============================================================
// ConnectingScreen — shown while connecting to WiFi
// ============================================================

class ConnectingScreen : public LcarsScreen {
public:
    const char* title() const override { return "CONNECTING"; }
    uint32_t refreshIntervalMs() const override { return 200; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        LcarsWidgets::drawLabel(spr, x, y, "NETWORK ACCESS", _theme->accent);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "SSID", savedSSID.c_str(),
                                    LCARS_ICE, _theme->textDim);
        y += 15;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "STATUS", "CONNECTING",
                                    LCARS_AMBER, _theme->textDim);
        y += 17;

        y += 4;
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        // Animated progress
        uint32_t elapsed = (millis() - connectStartMs) / 1000;
        char buf[16];
        snprintf(buf, sizeof(buf), "%lu SEC", (unsigned long)elapsed);
        LcarsWidgets::drawStatusRow(spr, x, y, w, "ELAPSED", buf,
                                    _theme->textDim, _theme->textDim);
        y += 17;

        // Animated bar
        float progress = fmodf((float)millis() / 2000.0f, 1.0f);
        LcarsWidgets::drawProgressBar(spr, x, y, w, 6, progress,
                                      LCARS_AMBER, LCARS_BAR_TRACK);
        y += 18;

        LcarsWidgets::drawIndicator(spr, x + 4, y + 5, 4, LCARS_AMBER, true);
        LcarsWidgets::drawLabel(spr, x + 14, y + 5, "ESTABLISHING LINK",
                                LCARS_AMBER, LCARS_FONT_SM, ML_DATUM);
    }
};

// ============================================================
// ServerInfoScreen — shows WiFi + server status
// ============================================================

class ServerInfoScreen : public LcarsScreen {
public:
    const char* title() const override { return "SCREENSHOT SERVER"; }
    uint32_t refreshIntervalMs() const override { return 500; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        LcarsWidgets::drawLabel(spr, x, y, "NETWORK STATUS", _theme->accent);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "SSID", savedSSID.c_str(),
                                    _theme->textDim, _theme->textDim);
        y += 15;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "IP ADDRESS", deviceIP.c_str(),
                                    LCARS_GREEN, _theme->textDim);
        y += 15;

        char rssi[12];
        snprintf(rssi, sizeof(rssi), "%d dBm", WiFi.RSSI());
        int32_t r = WiFi.RSSI();
        uint16_t rssiColor = (r > -50) ? LCARS_GREEN :
                             (r > -70) ? LCARS_AMBER : LCARS_TOMATO;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "SIGNAL", rssi,
                                    rssiColor, _theme->textDim);
        y += 17;

        y += 4;
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        LcarsWidgets::drawLabel(spr, x, y, "SERVER", _theme->accent);
        y += 14;

        char shots[12];
        snprintf(shots, sizeof(shots), "%lu", (unsigned long)screenshotCount);
        LcarsWidgets::drawStatusRow(spr, x, y, w, "SCREENSHOTS", shots,
                                    LCARS_ICE, _theme->textDim);
        y += 15;

        uint32_t up = millis() / 1000;
        char upBuf[16];
        if (up >= 3600) {
            snprintf(upBuf, sizeof(upBuf), "%luH %luM", up / 3600, (up % 3600) / 60);
        } else {
            snprintf(upBuf, sizeof(upBuf), "%luM %luS", up / 60, up % 60);
        }
        LcarsWidgets::drawStatusRow(spr, x, y, w, "UPTIME", upBuf,
                                    _theme->textDim, _theme->textDim);
        y += 17;

        LcarsWidgets::drawIndicator(spr, x + 4, y + 5, 4, LCARS_GREEN, false);
        LcarsWidgets::drawLabel(spr, x + 14, y + 5, "SERVER ACTIVE",
                                LCARS_GREEN, LCARS_FONT_SM, ML_DATUM);
    }
};

// ============================================================
// DemoScreen — visually interesting content to screenshot
// ============================================================

class DemoScreen : public LcarsScreen {
public:
    const char* title() const override { return "MAIN SYSTEMS"; }
    uint32_t refreshIntervalMs() const override { return 100; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        LcarsWidgets::drawLabel(spr, x, y, "POWER SYSTEMS", _theme->accent);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "WARP CORE", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        float warp = 0.7f + 0.1f * sinf((float)millis() / 1000.0f);
        LcarsWidgets::drawProgressBar(spr, x, y, w, 6, warp,
                                      LCARS_ICE, LCARS_BAR_TRACK);
        y += 7;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "SHIELDS", "87%",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 6, 0.87f,
                                      LCARS_VIOLET, LCARS_BAR_TRACK);
        y += 7;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "WEAPONS", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, x, y, w, 6, 0.45f,
                                      LCARS_GOLD, LCARS_BAR_TRACK);
        y += 7;

        y += 4;
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        int16_t gaugeR = 11;
        int16_t thickness = 3;
        int16_t labelW = 28;
        int16_t cellW = (w - labelW * 3) / 3 + labelW;
        int16_t gaugeRowW = cellW * 2 + labelW + gaugeR * 2;
        int16_t gx = x + (w - gaugeRowW) / 2;
        int16_t gaugeCY = y + gaugeR + 5;

        LcarsWidgets::drawLabel(spr, gx, gaugeCY,
                                "EPS", _theme->textDim, LCARS_FONT_SM, ML_DATUM);
        LcarsWidgets::drawDonutGauge(spr, gx + labelW + gaugeR, gaugeCY,
                                     gaugeR, thickness, warp,
                                     LCARS_ICE, LCARS_BAR_TRACK);

        LcarsWidgets::drawLabel(spr, gx + cellW, gaugeCY,
                                "SIF", _theme->textDim, LCARS_FONT_SM, ML_DATUM);
        LcarsWidgets::drawDonutGauge(spr, gx + cellW + labelW + gaugeR, gaugeCY,
                                     gaugeR, thickness, 0.92f,
                                     LCARS_GREEN, LCARS_BAR_TRACK);

        LcarsWidgets::drawLabel(spr, gx + cellW * 2, gaugeCY,
                                "DEF", _theme->textDim, LCARS_FONT_SM, ML_DATUM);
        float def = 0.3f + 0.1f * sinf((float)millis() / 600.0f);
        LcarsWidgets::drawDonutGauge(spr, gx + cellW * 2 + labelW + gaugeR, gaugeCY,
                                     gaugeR, thickness, def,
                                     LCARS_TOMATO, LCARS_BAR_TRACK);
    }
};

// ============================================================
// StatusScreen — status rows + data cascade
// ============================================================

class StatusScreen : public LcarsScreen {
public:
    const char* title() const override { return "SYSTEM STATUS"; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        // Stardate
        char stardate[32];
        uint32_t sd = 47000 + (millis() / 100) % 999;
        snprintf(stardate, sizeof(stardate), "%d.%d", sd / 10, sd % 10);
        LcarsWidgets::drawLabel(spr, x, y, "STARDATE", _theme->textDim);
        LcarsFont::drawText(spr, stardate, x + 80, y, LCARS_FONT_SM, _theme->accent);
        y += 15;

        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        LcarsWidgets::drawStatusRow(spr, x, y, w, "HULL INTEGRITY", "98.7%",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "LIFE SUPPORT", "NOMINAL",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "INERTIAL DAMPERS", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "DEFLECTOR ARRAY", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, x, y, w, "COMM ARRAY", "ACTIVE",
                                    _theme->statusOk, _theme->textDim);
        y += 17;

        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 3;
        int16_t cascadeH = c.y + c.h - y;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, x, y, w, cascadeH, _theme->accent);
        }
    }
};

// ============================================================
// TacticalScreen — pill buttons + indicators + value labels
// ============================================================

class TacticalScreen : public LcarsScreen {
public:
    const char* title() const override { return "TACTICAL"; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        int16_t btnW = (w - 6) / 3;
        int16_t btnH = 16;

        LcarsWidgets::drawPillButton(spr, x, y, btnW, btnH,
                                     "PHASERS", LCARS_TOMATO, LCARS_WHITE);
        LcarsWidgets::drawPillButton(spr, x + btnW + 3, y, btnW, btnH,
                                     "TORPEDOES", LCARS_AMBER, LCARS_BLACK);
        LcarsWidgets::drawPillButton(spr, x + (btnW + 3) * 2, y, btnW, btnH,
                                     "SHIELDS", LCARS_ICE, LCARS_BLACK);
        y += btnH + 4;

        int16_t sep1Y = y;
        LcarsWidgets::drawSeparator(spr, x, sep1Y, w, _theme->textDim);

        int16_t sectionH = 52;
        int16_t contentH = 46;
        int16_t valY = sep1Y + 1 + (sectionH - contentH) / 2;
        int16_t colW = w / 3;

        char phaserBuf[8];
        int phaserPct = 85 + (millis() / 200) % 15;
        snprintf(phaserBuf, sizeof(phaserBuf), "%d%%", phaserPct);
        LcarsWidgets::drawValueLabel(spr, x + 4, valY, phaserBuf, "PHASER PWR",
                                     LCARS_TOMATO, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);

        LcarsWidgets::drawValueLabel(spr, x + colW + 4, valY, "247", "TORPEDOES",
                                     LCARS_AMBER, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);

        char freqBuf[12];
        float freq = 257.4f + (float)((millis() / 300) % 20) * 0.1f;
        snprintf(freqBuf, sizeof(freqBuf), "%.1f", freq);
        LcarsWidgets::drawValueLabel(spr, x + colW * 2 + 4, valY, freqBuf, "SHIELD MHZ",
                                     LCARS_ICE, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);

        y = sep1Y + sectionH + 4;
        LcarsWidgets::drawSeparator(spr, x, y, w, _theme->textDim);
        y += 5;

        int16_t indR = 4;
        int16_t indCY = y + 6;

        LcarsWidgets::drawIndicator(spr, x + 8, indCY, indR, LCARS_TOMATO, true);
        LcarsWidgets::drawLabel(spr, x + 16, indCY, "ALERT",
                                LCARS_TOMATO, LCARS_FONT_SM, ML_DATUM);

        LcarsWidgets::drawIndicator(spr, x + 68, indCY, indR, LCARS_GREEN, false);
        LcarsWidgets::drawLabel(spr, x + 76, indCY, "SHIELDS",
                                LCARS_GREEN, LCARS_FONT_SM, ML_DATUM);

        LcarsWidgets::drawIndicator(spr, x + 142, indCY, indR, LCARS_AMBER, true);
        LcarsWidgets::drawLabel(spr, x + 150, indCY, "TARGET LOCK",
                                LCARS_AMBER, LCARS_FONT_SM, ML_DATUM);
        y += 16;

        indCY = y + 6;

        LcarsWidgets::drawIndicator(spr, x + 8, indCY, indR, LCARS_VIOLET, false);
        LcarsWidgets::drawLabel(spr, x + 16, indCY, "CLOAK DET",
                                LCARS_VIOLET, LCARS_FONT_SM, ML_DATUM);

        LcarsWidgets::drawIndicator(spr, x + 96, indCY, indR, LCARS_ICE, false);
        LcarsWidgets::drawLabel(spr, x + 104, indCY, "SENSORS",
                                LCARS_ICE, LCARS_FONT_SM, ML_DATUM);
    }
};

// ============================================================
// EngineeringScreen — large gauge + chambers + cascade
// ============================================================

class EngineeringScreen : public LcarsScreen {
public:
    const char* title() const override { return "ENGINEERING"; }
    uint32_t refreshIntervalMs() const override { return 60; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& c) override {
        int16_t x = c.x;
        int16_t y = c.y;
        int16_t w = c.w;

        int16_t gaugeR = 28;
        int16_t gaugeCX = x + gaugeR + 4;
        int16_t gaugeCY = y + gaugeR + 2;
        int16_t thickness = 6;

        float warpPct = 0.6f + 0.3f * sinf((float)millis() / 800.0f);
        LcarsWidgets::drawDonutGauge(spr, gaugeCX, gaugeCY,
                                     gaugeR, thickness, warpPct,
                                     LCARS_GOLD, LCARS_BAR_TRACK);

        char pctBuf[8];
        snprintf(pctBuf, sizeof(pctBuf), "%d", (int)(warpPct * 100));
        LcarsFont::drawText(spr, pctBuf, gaugeCX, gaugeCY - 4,
                            LCARS_FONT_MD, LCARS_GOLD, LCARS_BLACK, TC_DATUM);

        LcarsWidgets::drawLabel(spr, gaugeCX, gaugeCY + gaugeR + 4,
                                "WARP CORE", _theme->textDim, LCARS_FONT_SM, TC_DATUM);

        int16_t rx = x + gaugeR * 2 + 20;
        int16_t rw = w - (gaugeR * 2 + 20);
        int16_t ry = y;

        LcarsWidgets::drawLabel(spr, rx, ry, "DILITHIUM MATRIX",
                                _theme->accent, LCARS_FONT_SM);
        ry += 14;

        LcarsWidgets::drawLabel(spr, rx, ry, "CHAMBER 1", _theme->textDim, LCARS_FONT_SM);
        ry += 18;
        LcarsWidgets::drawProgressBar(spr, rx, ry, rw, 5, 0.95f,
                                      LCARS_GREEN, LCARS_BAR_TRACK);
        ry += 7;

        LcarsWidgets::drawLabel(spr, rx, ry, "CHAMBER 2", _theme->textDim, LCARS_FONT_SM);
        ry += 18;
        LcarsWidgets::drawProgressBar(spr, rx, ry, rw, 5, 0.88f,
                                      LCARS_GREEN, LCARS_BAR_TRACK);
        ry += 7;

        LcarsWidgets::drawLabel(spr, rx, ry, "CHAMBER 3", _theme->textDim, LCARS_FONT_SM);
        ry += 18;
        float ch3 = 0.3f + 0.15f * sinf((float)millis() / 500.0f);
        uint16_t ch3Color = (ch3 < 0.4f) ? LCARS_TOMATO : LCARS_AMBER;
        LcarsWidgets::drawProgressBar(spr, rx, ry, rw, 5, ch3,
                                      ch3Color, LCARS_BAR_TRACK);
        ry += 7;

        LcarsWidgets::drawIndicator(spr, rx + 4, ry + 5, 3, LCARS_TOMATO, true);
        LcarsWidgets::drawLabel(spr, rx + 12, ry + 1, "REALIGN",
                                LCARS_TOMATO, LCARS_FONT_SM);

        int16_t cascadeY = y + gaugeR * 2 + 18;
        cascadeY += 4;
        LcarsWidgets::drawSeparator(spr, x, cascadeY, w, _theme->textDim);
        cascadeY += 5;

        int16_t cascadeH = c.y + c.h - cascadeY;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, x, cascadeY, w, cascadeH,
                                          LCARS_GOLD, 42);
        }
    }
};

// ============================================================
// QuartersScreen — dual sidebar, 4-corner elbows
// ============================================================

class QuartersScreen : public LcarsScreen {
public:
    const char* title() const override { return "QUARTERS"; }
    bool wantsFrame() const override { return false; }
    uint32_t refreshIntervalMs() const override { return 100; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& fullRect) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;

        spr.fillSprite(_theme->background);

        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R,
                              _theme->elbowTop, LCARS_ELBOW_TL);
        LcarsFrame::drawElbow(spr, W - SW - R, 0, SW, TH, R,
                              _theme->barTop, LCARS_ELBOW_TR);
        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              _theme->elbowBottom, LCARS_ELBOW_BL);
        LcarsFrame::drawElbow(spr, W - SW - R, H - BH - R, SW, BH, R,
                              _theme->barBottom, LCARS_ELBOW_BR);

        int16_t barLX = SW + R + GAP;
        int16_t barRX = W - SW - R - GAP;
        int16_t barW = barRX - barLX;
        if (barW > 0) {
            LcarsFrame::drawBar(spr, barLX, 0, barW, TH,
                                _theme->barTop, LCARS_CAP_NONE, LCARS_CAP_NONE);
            LcarsFrame::drawBar(spr, barLX, H - BH, barW, BH,
                                _theme->barBottom, LCARS_CAP_NONE, LCARS_CAP_NONE);
        }

        int16_t sideTop = TH + R + 2;
        int16_t sideBot = H - BH - R - 2;
        int16_t sideH = sideBot - sideTop;
        if (sideH > 0 && _theme->sidebarCount > 0) {
            LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                    _theme->sidebar, _theme->sidebarCount, 2);
        }

        uint16_t rSidebar[] = {
            _theme->accent, _theme->progressFg,
            _theme->gaugeColor, _theme->statusOk
        };
        if (sideH > 0) {
            LcarsFrame::drawSidebar(spr, W - SW, sideTop, SW, sideH,
                                    rSidebar, 4, 2);
        }

        LcarsFont::drawTextUpper(spr, "QUARTERS",
                                 barLX + 4, TH / 2,
                                 LCARS_FONT_SM, _theme->textOnBar,
                                 _theme->barTop, ML_DATUM);

        int16_t cx = SW + R + GAP + 1;
        int16_t cy = TH + 4;
        int16_t cw = W - 2 * (SW + R + GAP) - 2;
        int16_t y = cy;

        LcarsWidgets::drawLabel(spr, cx, y, "ENVIRONMENTAL", _theme->accent);
        y += 14;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "TEMPERATURE", "22.4 C",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, cx, y, cw, 6, 0.62f,
                                      LCARS_ICE, LCARS_BAR_TRACK);
        y += 7;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "HUMIDITY", "45%",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, cx, y, cw, 6, 0.45f,
                                      LCARS_VIOLET, LCARS_BAR_TRACK);
        y += 7;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "LIGHTING", "70%",
                                    _theme->statusOk, _theme->textDim);
        y += 18;
        LcarsWidgets::drawProgressBar(spr, cx, y, cw, 6, 0.70f,
                                      LCARS_GOLD, LCARS_BAR_TRACK);
        y += 7;

        y += 4;
        LcarsWidgets::drawSeparator(spr, cx, y, cw, _theme->textDim);
        y += 5;

        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "REPLICATOR", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        y += 15;
        LcarsWidgets::drawStatusRow(spr, cx, y, cw, "COMM PANEL", "STANDBY",
                                    _theme->statusWarn, _theme->textDim);
    }
};

// ============================================================
// SplitPanelScreen — mid-bar divider, upper/lower zones
// ============================================================

class SplitPanelScreen : public LcarsScreen {
public:
    const char* title() const override { return "SHIP MAP"; }
    bool wantsFrame() const override { return false; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& fullRect) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;
        const int16_t MID_H = 12;

        spr.fillSprite(_theme->background);

        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R,
                              _theme->elbowTop, LCARS_ELBOW_TL);
        int16_t barX = SW + R + GAP;
        LcarsFrame::drawBar(spr, barX, 0, W - barX, TH,
                            _theme->barTop, LCARS_CAP_NONE, LCARS_CAP_PILL);

        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              _theme->elbowBottom, LCARS_ELBOW_BL);
        LcarsFrame::drawBar(spr, barX, H - BH, W - barX, BH,
                            _theme->barBottom, LCARS_CAP_NONE, LCARS_CAP_PILL);

        int16_t midY = H / 2 - MID_H / 2;
        LcarsFrame::drawBar(spr, barX, midY, W - barX, MID_H,
                            _theme->accent, LCARS_CAP_NONE, LCARS_CAP_PILL);

        int16_t sideTop = TH + R + 2;
        int16_t sideBot = H - BH - R - 2;
        int16_t sideH = sideBot - sideTop;
        if (sideH > 0 && _theme->sidebarCount > 0) {
            LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                    _theme->sidebar, _theme->sidebarCount, 2);
        }

        LcarsFont::drawTextUpper(spr, "SHIP MAP",
                                 barX + 4, TH / 2,
                                 LCARS_FONT_SM, _theme->textOnBar,
                                 _theme->barTop, ML_DATUM);

        int16_t cx = barX + 1;
        int16_t cw = W - barX - 3;
        int16_t uy = TH + 4;

        LcarsWidgets::drawLabel(spr, cx, uy, "PRIMARY SYSTEMS", _theme->accent);
        uy += 14;

        LcarsWidgets::drawStatusRow(spr, cx, uy, cw, "WARP DRIVE", "ONLINE",
                                    _theme->statusOk, _theme->textDim);
        uy += 15;
        LcarsWidgets::drawStatusRow(spr, cx, uy, cw, "IMPULSE", "ACTIVE",
                                    _theme->statusOk, _theme->textDim);
        uy += 15;
        LcarsWidgets::drawStatusRow(spr, cx, uy, cw, "NAVIGATION", "NOMINAL",
                                    _theme->statusOk, _theme->textDim);

        int16_t ly = midY + MID_H + 4;

        LcarsWidgets::drawLabel(spr, cx, ly, "SENSOR LOG", _theme->accent);
        ly += 14;

        int16_t cascadeH = (H - BH) - ly - 2;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, cx, ly, cw, cascadeH,
                                          _theme->accent, 77);
        }
    }
};

// ============================================================
// MedicalScreen — split dual frames side by side
// ============================================================

class MedicalScreen : public LcarsScreen {
public:
    const char* title() const override { return "SICKBAY"; }
    bool wantsFrame() const override { return false; }
    uint32_t refreshIntervalMs() const override { return 80; }

    void onDraw(TFT_eSprite& spr, const LcarsFrame::Rect& fullRect) override {
        const int16_t W = SCR_WIDTH;
        const int16_t H = SCR_HEIGHT;
        const int16_t SW = LCARS_SIDEBAR_W;
        const int16_t R  = LCARS_ELBOW_R;
        const int16_t TH = LCARS_TOPBAR_H;
        const int16_t BH = LCARS_BOTBAR_H;
        const int16_t GAP = LCARS_BAR_GAP;
        const int16_t SPLIT_GAP = 6;

        spr.fillSprite(_theme->background);

        int16_t splitX = W / 2;
        int16_t leftW = splitX - SPLIT_GAP / 2;
        int16_t rightX = splitX + SPLIT_GAP / 2;

        // Left frame
        LcarsFrame::drawElbow(spr, 0, 0, SW, TH, R,
                              _theme->elbowTop, LCARS_ELBOW_TL);
        int16_t lBarX = SW + R + GAP;
        int16_t lBarW = leftW - lBarX;
        if (lBarW > 0) {
            LcarsFrame::drawBar(spr, lBarX, 0, lBarW, TH,
                                _theme->barTop, LCARS_CAP_NONE, LCARS_CAP_PILL);
        }

        LcarsFrame::drawElbow(spr, 0, H - BH - R, SW, BH, R,
                              _theme->elbowBottom, LCARS_ELBOW_BL);
        if (lBarW > 0) {
            LcarsFrame::drawBar(spr, lBarX, H - BH, lBarW, BH,
                                _theme->barBottom, LCARS_CAP_NONE, LCARS_CAP_PILL);
        }

        int16_t sideTop = TH + R + 2;
        int16_t sideBot = H - BH - R - 2;
        int16_t sideH = sideBot - sideTop;
        if (sideH > 0 && _theme->sidebarCount > 0) {
            LcarsFrame::drawSidebar(spr, 0, sideTop, SW, sideH,
                                    _theme->sidebar, _theme->sidebarCount, 2);
        }

        LcarsFont::drawTextUpper(spr, "VITALS",
                                 lBarX + 4, TH / 2,
                                 LCARS_FONT_SM, _theme->textOnBar,
                                 _theme->barTop, ML_DATUM);

        // Right frame
        LcarsFrame::drawElbow(spr, W - SW - R, 0, SW, TH, R,
                              _theme->accent, LCARS_ELBOW_TR);
        int16_t rBarX = rightX;
        int16_t rBarW = W - SW - R - GAP - rBarX;
        if (rBarW > 0) {
            LcarsFrame::drawBar(spr, rBarX, 0, rBarW, TH,
                                _theme->accent, LCARS_CAP_PILL, LCARS_CAP_NONE);
        }

        LcarsFrame::drawElbow(spr, W - SW - R, H - BH - R, SW, BH, R,
                              _theme->gaugeColor, LCARS_ELBOW_BR);
        if (rBarW > 0) {
            LcarsFrame::drawBar(spr, rBarX, H - BH, rBarW, BH,
                                _theme->gaugeColor, LCARS_CAP_PILL, LCARS_CAP_NONE);
        }

        uint16_t rSidebar[] = {
            _theme->accent, _theme->gaugeColor,
            _theme->progressFg, _theme->statusOk
        };
        if (sideH > 0) {
            LcarsFrame::drawSidebar(spr, W - SW, sideTop, SW, sideH,
                                    rSidebar, 4, 2);
        }

        if (rBarW > 0) {
            LcarsFont::drawTextUpper(spr, "MEDICAL",
                                     rBarX + TH / 2 + 4, TH / 2,
                                     LCARS_FONT_SM, _theme->textOnBar,
                                     _theme->accent, ML_DATUM);
        }

        // Left content (Vitals)
        int16_t lcx = lBarX + 1;
        int16_t lcw = lBarW - 3;
        int16_t ly = TH + 4;

        char hrBuf[8];
        int hr = 72 + (millis() / 400) % 8;
        snprintf(hrBuf, sizeof(hrBuf), "%d", hr);
        LcarsWidgets::drawValueLabel(spr, lcx, ly, hrBuf, "HEART BPM",
                                     LCARS_GREEN, _theme->textDim,
                                     LCARS_FONT_LG, LCARS_FONT_SM);
        ly += 46;

        LcarsWidgets::drawStatusRow(spr, lcx, ly, lcw, "O2 SAT", "98%",
                                    _theme->statusOk, _theme->textDim);
        ly += 18;
        LcarsWidgets::drawProgressBar(spr, lcx, ly, lcw, 5, 0.98f,
                                      LCARS_GREEN, LCARS_BAR_TRACK);
        ly += 7;

        LcarsWidgets::drawStatusRow(spr, lcx, ly, lcw, "BP", "120/80",
                                    _theme->statusOk, _theme->textDim);
        ly += 18;
        LcarsWidgets::drawProgressBar(spr, lcx, ly, lcw, 5, 0.75f,
                                      LCARS_ICE, LCARS_BAR_TRACK);

        // Right content (Medical)
        int16_t rcx = rBarX + TH / 2 + 1;
        int16_t rcw = rBarW - TH / 2 - 3;
        int16_t ry = TH + 4;

        LcarsWidgets::drawLabel(spr, rcx, ry, "BIOBED 1", _theme->accent);
        ry += 14;

        LcarsWidgets::drawStatusRow(spr, rcx, ry, rcw, "NEURAL", "NORMAL",
                                    _theme->statusOk, _theme->textDim);
        ry += 15;
        LcarsWidgets::drawStatusRow(spr, rcx, ry, rcw, "CORTISOL", "LOW",
                                    _theme->statusWarn, _theme->textDim);
        ry += 15;
        LcarsWidgets::drawStatusRow(spr, rcx, ry, rcw, "INAPROVALINE", "2CC",
                                    _theme->statusOk, _theme->textDim);
        ry += 17;

        LcarsWidgets::drawSeparator(spr, rcx, ry, rcw, _theme->textDim);
        ry += 5;

        int16_t cascadeH = (H - BH) - ry - 2;
        if (cascadeH > 8) {
            LcarsWidgets::drawDataCascade(spr, rcx, ry, rcw, cascadeH,
                                          _theme->accent, 33);
        }
    }
};

// ============================================================
// Screenshot endpoint
// ============================================================

void handleScreenshot() {
    TFT_eSprite& spr = engine.sprite();
    uint16_t w = spr.width();
    uint16_t h = spr.height();
    uint16_t* buf = (uint16_t*)spr.getPointer();

    if (!buf) {
        server.send(503, "text/plain", "Sprite not ready");
        return;
    }

    uint32_t pixelBytes = (uint32_t)w * h * 2;
    uint32_t totalSize = 4 + pixelBytes;

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Cache-Control", "no-cache");
    server.setContentLength(totalSize);
    server.send(200, "application/octet-stream", "");

    WiFiClient client = server.client();
    client.write((uint8_t*)&w, 2);
    client.write((uint8_t*)&h, 2);

    const uint32_t chunkSize = 1024;
    uint8_t* ptr = (uint8_t*)buf;
    uint32_t remaining = pixelBytes;
    while (remaining > 0) {
        uint32_t toSend = (remaining > chunkSize) ? chunkSize : remaining;
        client.write(ptr, toSend);
        ptr += toSend;
        remaining -= toSend;
    }
    screenshotCount++;
}

// ============================================================
// Web server handlers
// ============================================================

void handleStatus() {
    String json = "{";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"screenshots\":" + String(screenshotCount);
    json += "}";
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
}

void handleSaveWifi() {
    String ssid = server.arg("ssid");
    String pass = server.arg("password");

    if (ssid.length() == 0) {
        server.send(400, "text/plain", "SSID required");
        return;
    }

    prefs.begin("lcars-ss", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();

    server.send(200, "text/plain", "OK");
    Serial.printf("[LCARS] WiFi saved: %s\n", ssid.c_str());

    delay(1000);
    ESP.restart();
}

void handleReset() {
    prefs.begin("lcars-ss", false);
    prefs.clear();
    prefs.end();

    server.send(200, "text/plain", "OK");
    Serial.println("[LCARS] Credentials cleared, rebooting");

    delay(1000);
    ESP.restart();
}

// ============================================================
// AP mode (captive portal for WiFi setup)
// ============================================================

void startAPMode() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%02X%02X", mac[4], mac[5]);
    apName = String("LCARS-") + suffix;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName.c_str());

    // DNS server redirects all domains to our IP (captive portal)
    dnsServer.start(53, "*", WiFi.softAPIP());

    // Serve setup page
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", HTML_SETUP);
    });
    server.on("/save-wifi", HTTP_POST, handleSaveWifi);
    server.on("/generate_204", HTTP_GET, []() {   // Android captive portal check
        server.send_P(200, "text/html", HTML_SETUP);
    });
    server.on("/hotspot-detect.html", HTTP_GET, []() {  // iOS captive portal check
        server.send_P(200, "text/html", HTML_SETUP);
    });
    server.onNotFound([]() {
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
    });
    server.begin();

    Serial.printf("[LCARS] AP Mode: %s  IP: %s\n",
                  apName.c_str(), WiFi.softAPIP().toString().c_str());
}

// ============================================================
// STA mode (screenshot server)
// ============================================================

void startSTAMode() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    connectStartMs = millis();
    Serial.printf("[LCARS] Connecting to: %s\n", savedSSID.c_str());
}

void startScreenshotServer() {
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", HTML_PAGE);
    });
    server.on("/screenshot", HTTP_GET, handleScreenshot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/reset", HTTP_POST, handleReset);
    server.begin();

    serverStarted = true;
    deviceIP = WiFi.localIP().toString();
    Serial.printf("[LCARS] Server at http://%s/\n", deviceIP.c_str());
}

// ============================================================
// Screen instances
// ============================================================

LcarsBootScreen    bootScreen;
SetupScreen        setupScreen;
ConnectingScreen   connectingScreen;
ServerInfoScreen   serverInfoScreen;
DemoScreen         demoScreen;
StatusScreen       statusScreen;
TacticalScreen     tacticalScreen;
EngineeringScreen  engineeringScreen;
QuartersScreen     quartersScreen;
SplitPanelScreen   splitPanelScreen;
MedicalScreen      medicalScreen;

LcarsScreen* runScreens[] = {
    &serverInfoScreen, &demoScreen, &statusScreen,
    &tacticalScreen, &engineeringScreen, &quartersScreen,
    &splitPanelScreen, &medicalScreen
};
const uint8_t RUN_SCREEN_COUNT = 8;
uint8_t  currentScreenIdx = 0;
uint32_t screenSwitchMs = 0;
bool     bootDone = false;

// ── Button (BOOT / GPIO0) ────────────────────────────────────
#define BTN_PIN 0
bool     btnLastState = HIGH;
uint32_t btnDebounceMs = 0;

// ============================================================
// Setup
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[LCARS] Screenshot Server");

    pinMode(BTN_PIN, INPUT_PULLUP);

    // Initialize LCARS engine
    engine.begin(tft);
    engine.setTheme(LCARS_THEME_TNG);

    #if defined(TFT_BL)
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
        engine.setBLPin(TFT_BL);
    #endif

    // Check NVS for saved WiFi credentials
    prefs.begin("lcars-ss", true);  // read-only
    savedSSID = prefs.getString("ssid", "");
    savedPass = prefs.getString("pass", "");
    prefs.end();

    if (savedSSID.length() > 0) {
        // Credentials exist → connect
        appMode = MODE_CONNECTING;
        bootScreen.setNextScreen(&connectingScreen);
        startSTAMode();
    } else {
        // No credentials → AP mode
        appMode = MODE_SETUP;
        bootScreen.setNextScreen(&setupScreen);
        startAPMode();
    }

    engine.setScreen(&bootScreen);
    Serial.printf("[LCARS] Mode: %s\n",
                  appMode == MODE_SETUP ? "SETUP" : "CONNECTING");
}

// ============================================================
// Loop
// ============================================================

void loop() {
    engine.update();
    server.handleClient();

    if (appMode == MODE_SETUP) {
        dnsServer.processNextRequest();
    }

    // Boot sequence completion
    if (!bootDone && bootScreen.isComplete() &&
        engine.currentScreen() == &bootScreen) {
        engine.setScreen(bootScreen.nextScreen());
        screenSwitchMs = millis();
        bootDone = true;
    }

    // MODE_CONNECTING: wait for WiFi
    if (appMode == MODE_CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            appMode = MODE_RUNNING;
            bootDone = true;
            startScreenshotServer();
            engine.setScreen(&serverInfoScreen);
            screenSwitchMs = millis();
        }
        // Timeout after 20 seconds → fall back to AP mode
        else if (millis() - connectStartMs > 20000) {
            Serial.println("[LCARS] WiFi timeout, falling back to AP mode");
            WiFi.disconnect();
            appMode = MODE_SETUP;
            savedSSID = "";
            savedPass = "";
            prefs.begin("lcars-ss", false);
            prefs.clear();
            prefs.end();
            startAPMode();
            engine.setScreen(&setupScreen);
            screenSwitchMs = millis();
        }
    }

    // ── Button: cycle screens on press ──
    if (appMode == MODE_RUNNING && bootDone && !engine.isTransitioning()) {
        bool btnState = digitalRead(BTN_PIN);
        if (btnState == LOW && btnLastState == HIGH &&
            millis() - btnDebounceMs > 200) {
            btnDebounceMs = millis();
            currentScreenIdx = (currentScreenIdx + 1) % RUN_SCREEN_COUNT;
            engine.setScreen(runScreens[currentScreenIdx]);
            screenSwitchMs = millis();  // Reset auto-cycle timer
        }
        btnLastState = btnState;
    }

    // MODE_RUNNING: auto-cycle screens
    if (appMode == MODE_RUNNING && bootDone &&
        !engine.isTransitioning() &&
        millis() - screenSwitchMs > 10000) {

        currentScreenIdx = (currentScreenIdx + 1) % RUN_SCREEN_COUNT;
        engine.setScreen(runScreens[currentScreenIdx]);
        screenSwitchMs = millis();
    }
}
