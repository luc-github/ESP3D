const express = require("express");
const chalk = require("chalk");
let path = require("path");
const fs = require("fs");
const port = 8080;
/*
 * Web Server for development
 * Web Socket server for development
 */
const wscolor = chalk.cyan;
const expresscolor = chalk.green;
const commandcolor = chalk.white;
const WebSocket = require("ws");
let currentID = 0;
const app = express();
const fileUpload = require("express-fileupload");
let serverpath = path.normalize(__dirname + "/../server/public/");

let WebSocketServer = require("ws").Server,
  wss = new WebSocketServer({ port: 81 });
app.use(fileUpload({ preserveExtension: true, debug: false }));
app.listen(port, () =>
  console.log(expresscolor(`[express] Listening on port ${port}!`))
);

//app.use(express.urlencoded({ extended: false }));

function SendBinary(text) {
  const array = new Uint8Array(text.length);
  for (let i = 0; i < array.length; ++i) {
    array[i] = text.charCodeAt(i);
  }
  wss.clients.forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(array);
    }
  });
}

app.post("/login", function (req, res) {
  res.send("");
  return;
});

app.get("/config", function (req, res) {
  res.send(
    "chip id: 56398\nCPU Freq: 240 Mhz<br/>" +
      "CPU Temp: 58.3 C<br/>" +
      "free mem: 212.36 KB<br/>" +
      "SDK: v3.2.3-14-gd3e562907<br/>" +
      "flash size: 4.00 MB<br/>" +
      "size for update: 1.87 MB<br/>" +
      "FS type: LittleFS<br/>" +
      "FS usage: 104.00 KB/192.00 KB<br/>" +
      "baud: 115200<br/>" +
      "sleep mode: none<br/>" +
      "wifi: ON<br/>" +
      "hostname: esp3d<br/>" +
      "HTTP port: 80<br/>" +
      "Telnet port: 23<br/>" +
      "WebDav port: 8383<br/>" +
      "sta: ON<br/>" +
      "mac: 80:7D:3A:C4:4E:DC<br/>" +
      "SSID: WIFI_OFFICE_A2G<br/>" +
      "signal: 100 %<br/>" +
      "phy mode: 11n<br/>" +
      "channel: 11<br/>" +
      "ip mode: dhcp<br/>" +
      "ip: 192.168.1.61<br/>" +
      "gw: 192.168.1.1<br/>" +
      "msk: 255.255.255.0<br/>" +
      "DNS: 192.168.1.1<br/>" +
      "ap: OFF<br/>" +
      "mac: 80:7D:3A:C4:4E:DD<br/>" +
      "serial: ON<br/>" +
      "notification: OFF<br/>" +
      "Target Fw: repetier<br/>" +
      "FW ver: 3.0.0.a91<br/>" +
      "FW arch: ESP32 "
  );
  return;
});

app.get("/command", function (req, res) {
  console.log(commandcolor(`[server]/command params: ${req.query.cmd}`));
  let url = req.query.cmd;
  if (url.startsWith("[ESP800]")) {
    res.json({
      FWVersion: "3.0.0.a28",
      FWTarget: 40,
      SDConnection: "none",
      Authentication: "Disabled",
      WebCommunication: "Synchronous",
      WebSocketIP: "localhost",
      WebSocketPort: "81",
      Hostname: "esp3d",
      WiFiMode: "STA",
      WebUpdate: "Enabled",
      Filesystem: "SPIFFS",
      Time: "none",
      Cam_ID: "4",
      Cam_name: "ESP32 Cam",
    });
    return;
  }
  if (url.indexOf("ESP111") != -1) {
    res.send("192.168.1.111");
    return;
  }
  if (url.indexOf("ESP420") != -1) {
    res.json({
      Status: [
        { id: "chip id", value: "38078" },
        { id: "CPU Freq", value: "240 Mhz" },
        { id: "CPU Temp", value: "50.6 C" },
        { id: "free mem", value: "217.50 KB" },
        { id: "SDK", value: "v3.3.1-61-g367c3c09c" },
        { id: "flash size", value: "4.00 MB" },
        { id: "size for update", value: "1.87 MB" },
        { id: "FS type", value: "SPIFFS" },
        { id: "FS usage", value: "39.95 KB/169.38 KB" },
        { id: "baud", value: "115200" },
        { id: "sleep mode", value: "none" },
        { id: "wifi", value: "ON" },
        { id: "hostname", value: "esp3d" },
        { id: "HTTP port", value: "80" },
        { id: "Telnet port", value: "23" },
        { id: "Ftp ports", value: "21, 20, 55600" },
        { id: "sta", value: "ON" },
        { id: "mac", value: "30:AE:A4:21:BE:94" },
        { id: "SSID", value: "WIFI_OFFICE_B2G" },
        { id: "signal", value: "100 %" },
        { id: "phy mode", value: "11n" },
        { id: "channel", value: "2" },
        { id: "ip mode", value: "dhcp" },
        { id: "ip", value: "192.168.1.43" },
        { id: "gw", value: "192.168.1.1" },
        { id: "msk", value: "255.255.255.0" },
        { id: "DNS", value: "192.168.1.1" },
        { id: "ap", value: "OFF" },
        { id: "mac", value: "30:AE:A4:21:BE:95" },
        { id: "serial", value: "ON" },
        { id: "notification", value: "OFF" },
        { id: "FW ver", value: "3.0.0.a28" },
        { id: "FW arch", value: "ESP32" },
      ],
    });
    return;
  }

  if (url.indexOf("ESP410") != -1) {
    res.json({
      AP_LIST: [
        {
          SSID: "HP-Setup>71-M277 LaserJet",
          SIGNAL: "92",
          IS_PROTECTED: "0",
        },
        { SSID: "WIFI_OFFICE_B2G", SIGNAL: "88", IS_PROTECTED: "1" },
        { SSID: "NETGEAR70", SIGNAL: "66", IS_PROTECTED: "1" },
        { SSID: "ZenFone6&#39;luc", SIGNAL: "48", IS_PROTECTED: "1" },
        { SSID: "Livebox-EF01", SIGNAL: "20", IS_PROTECTED: "1" },
        { SSID: "orange", SIGNAL: "20", IS_PROTECTED: "0" },
      ],
    });
    return;
  }

  if (url.indexOf("ESP400") != -1) {
    res.json({
      Settings: [
        {
          F: "network/network",
          P: "130",
          T: "S",
          V: "esp3d",
          H: "hostname",
          S: "32",
          M: "1",
        },
        {
          F: "network/network",
          P: "0",
          T: "B",
          V: "1",
          H: "radio mode",
          O: [{ none: "0" }, { sta: "1" }, { ap: "2" }],
        },
        {
          F: "network/sta",
          P: "1",
          T: "S",
          V: "WIFI_OFFICE_B2G",
          S: "32",
          H: "SSID",
          M: "1",
        },
        {
          F: "network/sta",
          P: "34",
          T: "S",
          N: "1",
          V: "********",
          S: "64",
          H: "pwd",
          M: "8",
        },
        {
          F: "network/sta",
          P: "99",
          T: "B",
          V: "1",
          H: "ip mode",
          O: [{ dhcp: "1" }, { static: "0" }],
        },
        {
          F: "network/sta",
          P: "100",
          T: "A",
          V: "192.168.0.1",
          H: "ip",
        },
        {
          F: "network/sta",
          P: "108",
          T: "A",
          V: "192.168.0.1",
          H: "gw",
        },
        {
          F: "network/sta",
          P: "104",
          T: "A",
          V: "255.255.255.0",
          H: "msk",
        },
        {
          F: "network/ap",
          P: "218",
          T: "S",
          V: "ESP3D",
          S: "32",
          H: "SSID",
          M: "1",
        },
        {
          F: "network/ap",
          P: "251",
          T: "S",
          N: "1",
          V: "********",
          S: "64",
          H: "pwd",
          M: "8",
        },
        {
          F: "network/ap",
          P: "316",
          T: "A",
          V: "192.168.0.1",
          H: "ip",
        },
        {
          F: "network/ap",
          P: "118",
          T: "B",
          V: "11",
          H: "channel",
          O: [
            { 1: "1" },
            { 2: "2" },
            { 3: "3" },
            { 4: "4" },
            { 5: "5" },
            { 6: "6" },
            { 7: "7" },
            { 8: "8" },
            { 9: "9" },
            { 10: "10" },
            { 11: "11" },
            { 12: "12" },
            { 13: "13" },
            { 14: "14" },
          ],
        },
        {
          F: "service/http",
          P: "328",
          T: "B",
          V: "1",
          H: "enable",
          O: [{ no: "0" }, { yes: "1" }],
        },
        {
          F: "service/http",
          P: "121",
          T: "I",
          V: "80",
          H: "port",
          S: "65001",
          M: "1",
        },
        {
          F: "service/telnetp",
          P: "329",
          T: "B",
          V: "1",
          H: "enable",
          O: [{ no: "0" }, { yes: "1" }],
        },
        {
          F: "service/telnetp",
          P: "125",
          T: "I",
          V: "23",
          H: "port",
          S: "65001",
          M: "1",
        },
        {
          F: "service/ftp",
          P: "1021",
          T: "B",
          V: "1",
          H: "enable",
          O: [{ no: "0" }, { yes: "1" }],
        },
        {
          F: "service/ftp",
          P: "1009",
          T: "I",
          V: "21",
          H: "control port",
          S: "65001",
          M: "1",
        },
        {
          F: "service/ftp",
          P: "1013",
          T: "I",
          V: "20",
          H: "active port",
          S: "65001",
          M: "1",
        },
        {
          F: "service/ftp",
          P: "1017",
          T: "I",
          V: "55600",
          H: "passive port",
          S: "65001",
          M: "1",
        },
        {
          F: "service/notification",
          P: "1004",
          T: "B",
          V: "1",
          H: "auto notif",
          O: [{ no: "0" }, { yes: "1" }],
        },
        {
          F: "service/notification",
          P: "116",
          T: "B",
          V: "0",
          H: "notification",
          O: [{ none: "0" }, { pushover: "1" }, { email: "2" }, { line: "3" }],
        },
        {
          F: "service/notification",
          P: "332",
          T: "S",
          V: "********",
          S: "63",
          H: "t1",
          M: "0",
        },
        {
          F: "service/notification",
          P: "396",
          T: "S",
          V: "********",
          S: "63",
          H: "t2",
          M: "0",
        },
        {
          F: "service/notification",
          P: "855",
          T: "S",
          V: " ",
          S: "127",
          H: "ts",
          M: "0",
        },
        {
          F: "system/system",
          P: "461",
          T: "B",
          V: "40",
          H: "targetfw",
          O: [
            { repetier: "50" },
            { marlin: "20" },
            { marlinkimbra: "35" },
            { smoothieware: "40" },
            { grbl: "10" },
            { unknown: "0" },
          ],
        },
        {
          F: "system/system",
          P: "112",
          T: "I",
          V: "115200",
          H: "baud",
          O: [
            { 9600: "9600" },
            { 19200: "19200" },
            { 38400: "38400" },
            { 57600: "57600" },
            { 74880: "74880" },
            { 115200: "115200" },
            { 230400: "230400" },
            { 250000: "250000" },
            { 500000: "500000" },
            { 921600: "921600" },
          ],
        },
        {
          F: "system/system",
          P: "320",
          T: "I",
          V: "10000",
          H: "bootdelay",
          S: "40000",
          M: "0",
        },
        {
          F: "system/system",
          P: "129",
          T: "F",
          V: "255",
          H: "outputmsg",
          O: [{ M117: "16" }, { serial: "1" }, { telnet: "2" }],
        },
      ],
    });
    return;
  }
  SendBinary("ok\n");
  res.send("");
});

function fileSizeString(size) {
  let s;
  if (size < 1024) return size + " B";
  if (size < 1024 * 1024) return (size / 1024).toFixed(2) + " KB";
  if (size < 1024 * 1024 * 1024)
    return (size / (1024 * 1024)).toFixed(2) + " MB";
  if (size < 1024 * 1024 * 1024 * 1024)
    return (size / (1024 * 1024 * 1024)).toFixed(2) + " GB";
  return "X B";
}

function filesList(mypath) {
  let res = '{"files":[';
  let nb = 0;
  let total = 1.31 * 1024 * 1024;
  let totalused = getTotalSize(serverpath);
  let currentpath = path.normalize(serverpath + mypath);
  console.log("[path]" + currentpath);
  fs.readdirSync(currentpath).forEach((fileelement) => {
    let fullpath = path.normalize(currentpath + "/" + fileelement);
    let fst = fs.statSync(fullpath);
    let fsize = -1;

    if (fst.isFile()) {
      fsize = fileSizeString(fst.size);
    }
    if (nb > 0) res += ",";
    res += '{"name":"' + fileelement + '","size":"' + fsize + '"}';
    nb++;
  });
  res +=
    '],"path":"' +
    mypath +
    '","occupation":"' +
    ((100 * totalused) / total).toFixed(0) +
    '","status":"ok","total":"' +
    fileSizeString(total) +
    '","used":"' +
    fileSizeString(totalused) +
    '"}';
  return res;
}

const getAllFiles = function (dirPath, arrayOfFiles) {
  let files = fs.readdirSync(dirPath);

  arrayOfFiles = arrayOfFiles || [];

  files.forEach(function (file) {
    if (fs.statSync(dirPath + "/" + file).isDirectory()) {
      arrayOfFiles = getAllFiles(dirPath + "/" + file, arrayOfFiles);
    } else {
      arrayOfFiles.push(dirPath + "/" + file);
    }
  });

  return arrayOfFiles;
};

const getTotalSize = function (directoryPath) {
  const arrayOfFiles = getAllFiles(directoryPath);

  let totalSize = 0;

  arrayOfFiles.forEach(function (filePath) {
    totalSize += fs.statSync(filePath).size;
  });

  return totalSize;
};

function deleteFolderRecursive(path) {
  if (fs.existsSync(path) && fs.lstatSync(path).isDirectory()) {
    fs.readdirSync(path).forEach(function (file, index) {
      let curPath = path + "/" + file;

      if (fs.lstatSync(curPath).isDirectory()) {
        // recurse
        deleteFolderRecursive(curPath);
      } else {
        // delete file
        fs.unlinkSync(curPath);
      }
    });

    console.log(`[server]Deleting directory "${path}"...`);
    if (fs.existsSync(path)) fs.rmdirSync(path);
  } else console.log(`[server]No directory "${path}"...`);
}

app.all("/updatefw", function (req, res) {
  res.send("ok");
});

app.all("/files", function (req, res) {
  let mypath = req.query.path;
  let url = req.originalUrl;
  let filepath = path.normalize(serverpath + mypath + "/" + req.query.filename);
  if (url.indexOf("action=deletedir") != -1) {
    console.log("[server]delete directory " + filepath);
    deleteFolderRecursive(filepath);
    fs.readdirSync(mypath);
  } else if (url.indexOf("action=delete") != -1) {
    fs.unlinkSync(filepath);
    console.log("[server]delete file " + filepath);
  }
  if (url.indexOf("action=createdir") != -1) {
    fs.mkdirSync(filepath);
    console.log("[server]new directory " + filepath);
  }
  if (typeof mypath == "undefined") {
    if (typeof req.body.path == "undefined") {
      console.log("[server]path is not defined");
      mypath = "/";
    } else {
      mypath = (req.body.path == "/" ? "" : req.body.path) + "/";
    }
  }
  console.log("[server]path is " + mypath);
  if (!req.files || Object.keys(req.files).length === 0) {
    return res.send(filesList(mypath));
  }
  let myFile = req.files.myfiles;
  if (typeof myFile.length == "undefined") {
    let fullpath = path.normalize(serverpath + mypath + myFile.name);
    console.log("[server]one file:" + fullpath);
    myFile.mv(fullpath, function (err) {
      if (err) return res.status(500).send(err);
      res.send(filesList(mypath));
    });
    return;
  } else {
    console.log(myFile.length + " files");
    for (let i = 0; i < myFile.length; i++) {
      let fullpath = path.normalize(serverpath + mypath + myFile[i].name);
      console.log(fullpath);
      myFile[i].mv(fullpath).then(() => {
        if (i == myFile.length - 1) res.send(filesList(mypath));
      });
    }
  }
});

wss.on("connection", (socket, request) => {
  console.log(wscolor("[ws] New connection"));
  console.log(wscolor(`[ws] currentID:${currentID}`));
  socket.send(`currentID:${currentID}`);
  wss.clients.forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(`activeID:${currentID}`);
    }
  });
  currentID++;
  socket.on("message", (message) => {
    console.log(wscolor("[ws] received: %s", message));
  });
});
wss.on("error", (error) => {
  console.log(wscolor("[ws] Error: %s", error));
});
