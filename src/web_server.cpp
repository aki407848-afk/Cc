#include "web_server.h"
#include "config.h"
#include "hardware.h"
#include "ir_controller.h"
#include "file_manager.h"
#include <Update.h>

WebServer server(80);

// === HTML ИНТЕРФЕЙС ===
String getIndexHtml() {
    return R"(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>C3 Super Tool</title>
<style>
    body{background:#121212;color:#e0e0e0;font-family:sans-serif;margin:0;padding:20px;}
    h1,h2,h3{color:#bb86fc;}
    .nav{display:flex;gap:10px;margin-bottom:20px;border-bottom:1px solid #333;padding-bottom:10px;}
    .nav button{background:#1f1f1f;color:#fff;border:none;padding:10px 20px;cursor:pointer;border-radius:5px;}
    .nav button.active{background:#bb86fc;color:#000;}
    .tab-content{display:none;}
    .tab-content.active{display:block;}
    input,select,textarea{background:#2c2c2c;border:1px solid #444;color:#fff;padding:8px;width:100%;box-sizing:border-box;margin-bottom:10px;}
    button.action{background:#03dac6;color:#000;border:none;padding:10px;cursor:pointer;width:100%;margin-top:5px;}
    button.danger{background:#cf6679;}
    #term-out{background:#000;color:#0f0;height:300px;overflow-y:auto;padding:10px;font-family:monospace;font-size:12px;}
    .row{display:flex;gap:10px;} .col{flex:1;}
</style>
</head>
<body>
<h1>ESP32-C3 Super Tool</h1>
<div class="nav">
    <button onclick="showTab('wifi')" class="active">WiFi</button>
    <button onclick="showTab('ir')">IR</button>
    <button onclick="showTab('pins')">PINS</button>
    <button onclick="showTab('ota')">OTA</button>
    <button onclick="showTab('files')">Files</button>
    <button onclick="showTab('term')">Terminal</button>
</div>

<!-- WIFI TAB -->
<div id="wifi" class="tab-content active">
    <h3>WiFi Manager</h3>
    <div class="row">
        <div class="col"><label>SSID:</label><input type="text" id="w-ssid" value=")</script>" + apSsid + R"("></div>
        <div class="col"><label>Password:</label><input type="text" id="w-pass" value=")</script>" + apPass + R"("></div>
    </div>
    <button class="action" onclick="saveWifi()">Save & Reboot WiFi</button>
    <hr>
    <button class="action" onclick="fetch('/api/wifi/scan')">Scan Networks</button>
    <div id="scan-res"></div>
    <hr>
    <h3>Attack Tools</h3>
    <input type="text" id="spam-ssid" placeholder="Fake AP SSID">
    <button class="action danger" onclick="startSpam()">Start AP Spam</button>
    <button class="action danger" onclick="stopSpam()">Stop Spam</button>
    <p>Status: <span id="spam-status">Idle</span></p>
</div>

<!-- IR TAB -->
<div id="ir" class="tab-content">
    <h3>TV Remote</h3>
    <div class="row">
        <button class="action" onclick="sendIr('lg')">LG Power</button>
        <button class="action" onclick="sendIr('samsung')">Samsung Power</button>
    </div>
    <hr>
    <h3>Brute Force</h3>
    <select id="brute-proto"><option value="nec">NEC</option><option value="sony">Sony</option></select>
    <button class="action danger" onclick="startBrute()">Start Brute</button>
    <button class="action" onclick="stopBrute()">Stop</button>
</div>

<!-- PINS TAB -->
<div id="pins" class="tab-content">
    <h3>Pin Configuration (-1 to Disable)</h3>
    <div class="row"><div class="col">IR TX:<input type="number" id="p-ir" value="-1"></div><div class="col">OLED SDA:<input type="number" id="p-sda" value="-1"></div></div>
    <div class="row"><div class="col">OLED SCL:<input type="number" id="p-scl" value="-1"></div><div class="col">NRF CE:<input type="number" id="p-ce" value="-1"></div></div>
    <div class="row"><div class="col">NRF CSN:<input type="number" id="p-csn" value="-1"></div><div class="col">Btn 1:<input type="number" id="p-b1" value="-1"></div></div>
    <div class="row"><div class="col">Btn 2:<input type="number" id="p-b2" value="-1"></div><div class="col">Btn 3:<input type="number" id="p-b3" value="-1"></div></div>
    <button class="action" onclick="savePins()">Save Pins & Reboot</button>
</div>

<!-- OTA TAB -->
<div id="ota" class="tab-content">
    <h3>Firmware Update</h3>
    <form method="POST" action="/update" enctype="multipart/form-data">
        <input type="file" name="update">
        <button type="submit" class="action">Upload Firmware</button>
    </form>
</div>

<!-- FILES TAB -->
<div id="files" class="tab-content">
    <h3>File Manager</h3>
    <div id="file-list">Loading...</div>
    <hr>
    <h4>Create New JSON</h4>
    <input type="text" id="new-file-name" placeholder="filename.json">
    <textarea id="new-file-content" rows="5" placeholder="{ 'key': 'value' }"></textarea>
    <button class="action" onclick="createFile()">Create File</button>
</div>

<!-- TERMINAL TAB -->
<div id="term" class="tab-content">
    <h3>Serial Terminal</h3>
    <div id="term-out"></div>
    <input type="text" id="term-in" placeholder="Type command...">
    <button class="action" onclick="sendTerm()">Send</button>
    <p><small>Commands: checks, color [1-3], create json [name], active [name], del [name]</small></p>
</div>

<script>
function showTab(id){document.querySelectorAll('.tab-content').forEach(d=>d.classList.remove('active'));document.querySelectorAll('.nav button').forEach(b=>b.classList.remove('active'));document.getElementById(id).classList.add('active');event.target.classList.add('active');}
function saveWifi(){let s=document.getElementById('w-ssid').value,p=document.getElementById('w-pass').value;fetch('/api/wifi/save?s='+s+'&p='+p).then(()=>alert('Saved! Rebooting...'));}
function savePins(){let d={ir:document.getElementById('p-ir').value,sda:document.getElementById('p-sda').value,scl:document.getElementById('p-scl').value,b1:document.getElementById('p-b1').value,b2:document.getElementById('p-b2').value,b3:document.getElementById('p-b3').value,ce:document.getElementById('p-ce').value,csn:document.getElementById('p-csn').value};fetch('/api/pins/save', {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(d)}).then(()=>alert('Saved! Rebooting...'));}
function sendIr(type){fetch('/api/ir/send?type='+type);}
function startSpam(){let s=document.getElementById('spam-ssid').value;fetch('/api/wifi/spam/start?ssid='+s);document.getElementById('spam-status').innerText='Running';}
function stopSpam(){fetch('/api/wifi/spam/stop');document.getElementById('spam-status').innerText='Stopped';}
function startBrute(){fetch('/api/ir/brute/start');}
function stopBrute(){fetch('/api/ir/brute/stop');}
function sendTerm(){let c=document.getElementById('term-in').value;fetch('/api/term?cmd='+c).then(r=>r.text()).then(t=>{let o=document.getElementById('term-out');o.innerHTML+= '> '+c+'<br>'+t+'<br>';o.scrollTop=o.scrollHeight;});document.getElementById('term-in').value='';}
// Load files list on open
setInterval(()=>fetch('/api/files/list').then(r=>r.json()).then(d=>{let h='<ul>';d.forEach(f=>h+='<li>'+f.name+' <button onclick="delFile(\''+f.name+'\')">Del</button> <button onclick="runFile(\''+f.name+'\')">Run</button></li>');h+='</ul>';document.getElementById('file-list').innerHTML=h;}), 2000);
function delFile(n){fetch('/api/files/del?name='+n);}
function runFile(n){fetch('/api/files/run?name='+n);}
function createFile(){let n=document.getElementById('new-file-name').value,c=document.getElementById('new-file-content').value;fetch('/api/files/create?name='+n+'&content='+encodeURIComponent(c));}
</script>
</body>
</html>
)";
}

// === API HANDLERS ===

void handleRoot() { server.send(200, "text/html", getIndexHtml()); }

void handleWifiSave() {
    String s = server.arg("s");
    String p = server.arg("p");
    if(s.length() > 0 && p.length() >= 8) {
        apSsid = s;
        apPass = p;
        saveConfig();
        server.send(200, "text/plain", "OK");
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Invalid SSID or Pass (min 8 chars)");
    }
}

void handleWifiScan() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for(int i=0; i<n; i++) {
        json += "{\"ssid\":\""+WiFi.SSID(i)+"\",\"rssi\":"+WiFi.RSSI(i)+",\"enc\":\""+String(WiFi.encryptionType(i))+"\"}";
        if(i<n-1) json+=",";
    }
    json += "]";
    server.send(200, "application/json", json);
}

void handlePinsSave() {
    String body = server.arg("plain");
    StaticJsonDocument<512> doc;
    deserializeJson(doc, body);
    
    pins.ir_tx = doc["ir"];
    pins.oled_sda = doc["sda"];
    pins.oled_scl = doc["scl"];
    pins.btn_1 = doc["b1"];
    pins.btn_2 = doc["b2"];
    pins.btn_3 = doc["b3"];
    pins.nrf_ce = doc["ce"];
    pins.nrf_csn = doc["csn"];
    
    saveConfig();
    server.send(200, "text/plain", "OK");
    ESP.restart();
}

void handleOTA() {
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START){
        if(!Update.begin(UPDATE_SIZE_UNKNOWN)){
            Update.printError(Serial);
        }
    } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
            Update.printError(Serial);
        }
    } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){
            server.send(200, "text/plain", "Update Success");
        } else {
            server.send(500, "text/plain", "Update Failed");
        }
    }
}

void startWebServer() {
    server.on("/", handleRoot);
    server.on("/api/wifi/save", handleWifiSave);
    server.on("/api/wifi/scan", handleWifiScan);
    server.on("/api/pins/save", handlePinsSave);
    server.on("/update", HTTP_POST, [](){ server.sendHeader("Connection", "close"); server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK"); ESP.restart(); }, handleOTA);
    
    // IR & Files & Term handlers are in their respective cpp files but registered here for simplicity or via extern
    // For clean code, register them in their own init functions, but here we do it directly:
    
    server.on("/api/ir/send", handleIrSend);
    server.on("/api/wifi/spam/start", handleSpamStart);
    server.on("/api/wifi/spam/stop", handleSpamStop);
    server.on("/api/ir/brute/start", handleBruteStart);
    server.on("/api/ir/brute/stop", handleBruteStop);
    server.on("/api/files/list", handleFileList);
    server.on("/api/files/del", handleFileDel);
    server.on("/api/files/create", handleFileCreate);
    server.on("/api/term", handleTerminalCmd);

    server.begin();
}
