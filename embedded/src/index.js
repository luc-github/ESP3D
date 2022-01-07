import "./style.css";
import { initMenus } from "./menu";
let isConsoleExpanded = false;
let isAutoScroll = true;
let consoleContentList = [];
let consoleContent;
let isFileSystemExpanded = true;
let isFirmwareUpdateExpanded = false;
let consolePanel;
let fileSystem;
let firmware;
let consoleHeader;
let consoleBody;
let fileSystemHeader;
let fileSystemBody;
let filecontentFooter;
let firmwareHeader;
let firmwareBody;
let filesInput;
let fwInput;
let filesButton;
let firmwareButton;
let refreshButton;
let createDirButton;
let message;
let messageLimited;
let websocketStarted = false;
let wsSource;
let wsMsg = "";
let logOff = false;
let pageId = "";
let currentPath = "/";
const version = "3.0.0.a4";
let xmlhttpupload;
let prgfiletext;
let prgfile;
let uploadType = 0;
let restartTime;
let loginLink;
let loginModal;

window.onload = function () {
  consolePanel = document.getElementById("consolePanel");
  fileSystem = document.getElementById("fileSystem");
  firmware = document.getElementById("firmware");
  consoleHeader = document.getElementById("consoleHeader");
  consoleBody = document.getElementById("consoleBody");
  fileSystemHeader = document.getElementById("fileSystemHeader");
  fileSystemBody = document.getElementById("fileSystemBody");
  filecontentFooter = document.getElementById("filecontentFooter");
  firmwareHeader = document.getElementById("firmwareHeader");
  firmwareBody = document.getElementById("firmwareBody");
  consoleContent = document.getElementById("consoleContent");
  fwInput = document.getElementById("filefw");
  filesInput = document.getElementById("files");
  message = document.getElementById("MSG");
  messageLimited = document.getElementById("MSGLimited");
  prgfiletext = document.getElementById("prgfiletext");
  prgfile = document.getElementById("prgfile");
  document.getElementById("cmdBtn").addEventListener("click", function () {
    let input = document.getElementById("customCmdTxt");
    if (input.value.trim().length == 0) return;
    let url = new URL("http://" + window.location.host + "/command");
    url.searchParams.append("cmd", input.value.trim());
    httpGet(url, processCmdJson);
    consoleContentUpdate(
      "<span class='cmd'>" + input.value.trim() + "</span>\n"
    );
    input.value = "";
  });
  document
    .getElementById("customCmdTxt")
    .addEventListener("keyup", function (event) {
      if (event.keyCode === 13) {
        document.getElementById("cmdBtn").click();
      }
    });
  consoleHeader.addEventListener("click", function () {
    isConsoleExpanded = !isConsoleExpanded;
    if (isConsoleExpanded) consoleBody.style.display = "block";
    else consoleBody.style.display = "none";
  });
  loginLink = document.getElementById("loginLink");
  loginModal = document.getElementById("loginpage");
  loginLink.addEventListener("click", function () {
    loginModal.classList.remove("hide");
  });
  document.getElementById("loginbutton").addEventListener("click", function () {
    loginModal.classList.add("hide");
    let user = document.getElementById("loginInput").value.trim();
    let password = document.getElementById("passwordInput").value.trim();
    let url = new URL("http://" + window.location.host + "/login");
    url.searchParams.append("USER", user);
    url.searchParams.append("PASSWORD", password);
    url.searchParams.append("SUBMIT", "yes");
    let xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function () {
      if (xmlhttp.readyState == 4) {
        if (xmlhttp.status != 200) {
          if (xmlhttp.status == 401) {
            loginModal.classList.remove("hide");
            loginLink.classList.remove("hide");
          } else {
            console.log(xmlhttp.status);
            ErrorMSG(
              "Error: board does not answered properly " + xmlhttp.status
            );
          }
        } else {
          getFWData();
        }
      }
    };
    xmlhttp.open("GET", url, true);
    xmlhttp.send();
  });

  refreshButton = document.getElementById("refresh");
  refreshButton.addEventListener("click", function () {
    SendFileCommand("list", "all");
  });
  firmwareButton = document.getElementById("uploapFW");
  firmwareButton.addEventListener("click", function () {
    prgupdatetext.innerHTML = "";
    fwInput.click();
  });
  filesButton = document.getElementById("uploadFiles");
  filesButton.innerHTML = fileIcon(true);
  filesButton.addEventListener("click", function () {
    filesInput.click();
  });
  fwInput.addEventListener("change", function () {
    if (fwInput.value) {
      if (!confirm("Confirm Firmware Update ?")) {
        console.log("abort");
        fwInput.value = "";
        return;
      }
      uploadFirmware();
    }
  });
  filesInput.addEventListener("change", function () {
    console.log("upload");
    if (filesInput.value) {
      uploadFiles();
    }
  });
  document
    .getElementById("monitor_enable_autoscroll")
    .addEventListener("click", function () {
      if (document.getElementById("monitor_enable_autoscroll").checked) {
        isAutoScroll = true;
        autoscroll();
      } else {
        isAutoScroll = false;
      }
    });
  consoleBody.style.display = "none";
  fileSystemHeader.addEventListener("click", function () {
    isFileSystemExpanded = !isFileSystemExpanded;
    if (isFileSystemExpanded) {
      fileSystemBody.style.display = "block";
      fileSystemBody.scrollIntoView();
    } else {
      fileSystemBody.style.display = "none";
    }
  });
  firmwareHeader.addEventListener("click", function () {
    isFirmwareUpdateExpanded = !isFirmwareUpdateExpanded;
    if (isFirmwareUpdateExpanded) {
      firmwareBody.style.display = "block";
      firmwareBody.scrollIntoView();
    } else firmwareBody.style.display = "none";
  });
  firmwareBody.style.display = "none";
  createDirButton = document.getElementById("createdir");
  createDirButton.innerHTML = dirIcon(true);
  createDirButton.addEventListener("click", function () {
    let filename = prompt("Directory name", "");
    if (filename != null && filename.trim().length > 0) {
      SendFileCommand("createdir", filename.trim());
    }
  });
  getFWData();
  initMenus();
};

function padNumber(num, size) {
  let s = num.toString().padStart(size, "0");
  return s;
}

function getPCTime() {
  let d = new Date();
  return (
    d.getFullYear() +
    "-" +
    padNumber(d.getMonth() + 1, 2) +
    "-" +
    padNumber(d.getDate(), 2) +
    "-" +
    padNumber(d.getHours(), 2) +
    "-" +
    padNumber(d.getMinutes(), 2) +
    "-" +
    padNumber(d.getSeconds(), 2)
  );
}

function isLimitedEnvironment(wifiMode) {
  let sitesList = [
    "clients3.google.com", //Android Captive Portal Detection
    "connectivitycheck.",
    //Apple iPhone, iPad with iOS 6 Captive Portal Detection
    "apple.com",
    ".akamaitechnologies.com",
    //Apple iPhone, iPad with iOS 7, 8, 9 and recent versions of OS X
    "www.appleiphonecell.com",
    "www.itools.info",
    "www.ibook.info",
    "www.airport.us",
    "www.thinkdifferent.us",
    ".akamaiedge.net",
    //Windows
    ".msftncsi.com",
    "microsoft.com",
  ];
  if (wifiMode != "AP") return false;
  for (let i = 0; i < sitesList.length; i++) {
    if (document.location.host.indexOf(sitesList[i]) != -1) return true;
  }
  return false;
}

function autoscroll() {
  if (isAutoScroll) consoleContent.scrollTop = consoleContent.scrollHeight;
}

function consoleContentUpdate(message) {
  if (message) {
    consoleContentList = consoleContentList.concat(message);
    consoleContentList = consoleContentList.slice(-300);
    let output = "";
    for (let i = 0; i < consoleContentList.length; i++) {
      output += consoleContentList[i];
    }
    document.getElementById("consoleContent").innerHTML = output;
    autoscroll();
  }
}

function processCmdJson(text) {
  let json;
  try {
    json = JSON.parse(text);
    consoleContentUpdate(JSON.stringify(json, null, " ") + "\n");
  } catch (e) {
    consoleContentUpdate(text + "\n");
  }
}

function processFWJson(text) {
  let json;
  try {
    json = JSON.parse(text);
  } catch (e) {
    ErrorMSG("Error: json data are incorrect!<br/>" + text);
    consoleContentUpdate(text);
    return;
  }
  if (json.FWVersion) {
    let verLink = document.getElementById("verLink");
    verLink.innerHTML = "v" + json.FWVersion;
    verLink.addEventListener("click", function () {
      let url = new URL("http://" + window.location.host + "/config");
      window.open(url, "_blank");
    });
  } else {
    ErrorMSG(
      "Error: json data are incorrect!<br/>" +
        JSON.stringify(json, null, "<br/> ")
    );
    return;
  }
  document.getElementById("espLink").addEventListener("click", function () {
    let url = new URL("http://" + window.location.host);
    window.open(url);
  });
  consolePanel.classList.remove("hide");
  if (json.Filesystem && json.Filesystem != "None")
    fileSystem.classList.remove("hide");
  if (json.WebUpdate == "Enabled") firmware.classList.remove("hide");
  if (json.WiFiMode && json.WebSocketIP) {
    if (isLimitedEnvironment(json.WiFiMode)) {
      let address =
        "http://" +
        json.WebSocketIP +
        (window.location.port == 80 ? "" : ":" + window.location.port);
      InfoMSGLimited(
        "It seems you are in limited environment,<br> please open a browser using<BR>" +
          address +
          "<br>to get all features working"
      );
    }
  }
  if (json.Hostname) document.title = json.Hostname;
  startSocket(json.WebSocketIP, json.WebSocketPort, json.WebCommunication);
  SendFileCommand("list", "all");
}

function startSocket(ip, port, sync) {
  if (websocketStarted) {
    wsSource.close();
  }

  wsSource = new WebSocket(
    "ws://" + ip + ":" + port + (sync == "Asynchronous" ? "/ws" : ""),
    ["arduino"]
  );
  wsSource.binaryType = "arraybuffer";
  wsSource.onopen = function (e) {
    websocketStarted = true;
  };
  wsSource.onclose = function (e) {
    websocketStarted = false;
    //seems sometimes it disconnect so wait 3s and reconnect
    //if it is not a log off
    if (!logOff)
      setTimeout(() => {
        startSocket(ip, port, sync);
      }, 3000);
  };
  wsSource.onerror = function (e) {};
  wsSource.onmessage = function (e) {
    let msg = "";
    //bin
    if (e.data instanceof ArrayBuffer) {
      let bytes = new Uint8Array(e.data);
      for (let i = 0; i < bytes.length; i++) {
        msg += String.fromCharCode(bytes[i]);
        if (bytes[i] == 10 || bytes[i] == 13) {
          wsMsg += msg;
          consoleContentUpdate(wsMsg);
          wsMsg = "";
          msg = "";
        }
      }
      wsMsg += msg;
    } else {
      msg = e.data;
      let tval = msg.split(":");
      if (tval.length >= 2) {
        if (tval[0] == "currentID") {
          pageId = tval[1];
        }
        if (tval[0] == "activeID") {
          if (pageId != tval[1]) {
            logOff = true;
            wsSource.close();
            InfoMSG(
              "<br/><br/>It seems you are connect from another location, you are now disconnected."
            );
            document.title = document.title + "(disconnected)";
            consolePanel.classList.add("hide");
            firmware.classList.add("hide");
            fileSystem.classList.add("hide");
            document.getElementById("verLink").classList.add("disabled");
          }
        }
        if (tval[0] == "ERROR") {
          xmlhttpupload.abort();
          uploadError(tval[2], tval[1]);
        }
      }
    }
  };
}

function uploadError(msg, nb) {
  let status = "Upload failed";
  if (nb != 0 && typeof nb != "undefined") {
    status = msg + "(" + nb + ")";
  }
  alert(status + "!");
  if (uploadType == 1) {
    let currentstatus = filecontentFooter.innerHTML;
    if (currentstatus.length > 0) {
      filecontentFooter.innerHTML =
        "Status: " +
        status +
        "&nbsp&nbsp" +
        currentstatus.substring(currentstatus.indexOf("|"));
    } else {
      filecontentFooter.innerHTML = status;
    }
  } else if (uploadType == 2) {
    prgupdatetext.innerHTML = status;
  }
  //location.reload();
}

function ErrorMSG(msg) {
  message.innerHTML = msg;
  message.className = "text-error";
}

function InfoMSGLimited(msg) {
  messageLimited.innerHTML = msg;
  messageLimited.className = "text-error";
}

function InfoMSG(msg) {
  message.innerHTML = msg;
  message.className = "text-primary";
}

function getFWData() {
  let url = new URL("http://" + window.location.host + "/command");
  url.searchParams.append("cmd", "[ESP800]time=" + getPCTime());
  httpGet(url, processFWJson);
}

function SendFileCommand(action, filename) {
  let url = new URL("http://" + window.location.host + "/files");
  url.searchParams.append("action", action);
  url.searchParams.append("filename", filename);
  url.searchParams.append("path", currentPath);
  httpGet(url, dispatchFileStatus);
}

function trashIcon() {
  return "<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='red' stroke-width='2' stroke-linecap='round' stroke-linejoin='round' class='feather feather-trash-2'><polyline points='3 6 5 6 21 6'></polyline><path d='M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2'></path><line x1='10' y1='11' x2='10' y2='17'></line><line x1='14' y1='11' x2='14' y2='17'></line></svg>";
}

function backIcon() {
  return "<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2' stroke-linecap='round' stroke-linejoin='round' ><polyline points='10 9 15 4 20 9'></polyline><path d='M4 20h7a4 4 0 0 0 4-4V4'></path></svg>";
}

function fileIcon(plus = false) {
  return (
    "<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='" +
    (plus ? "white" : "currentColor") +
    "' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M13 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9z'></path><polyline points='13 2 13 9 20 9'></polyline>" +
    (plus
      ? "<line x1='12' y1='11' x2='12' y2='17'></line><line x1='9' y1='14' x2='15' y2='14'></line>"
      : "") +
    "</svg>"
  );
}
function dirIcon(plus = false) {
  return (
    "<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24' fill='none' stroke='" +
    (plus ? "white" : "currentColor") +
    "' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z'></path>" +
    (plus
      ? "<line x1='12' y1='11' x2='12' y2='17'></line><line x1='9' y1='14' x2='15' y2='14'></line>"
      : "") +
    "</svg>"
  );
}
function dispatchFileStatus(jsonresponse) {
  let json;
  let eventslisteners = [];
  let showESP3Dbutton = false;
  try {
    json = JSON.parse(jsonresponse);
    //ugly but code is smaller
    filecontentFooter.innerHTML =
      "<span>Status: " +
      json.status +
      "&nbsp;&nbsp;</span><span>|&nbsp;&nbsp;Total space: " +
      json.total +
      "&nbsp;&nbsp;</span><span>|&nbsp;&nbsp;Used space: " +
      json.used +
      "&nbsp;&nbsp;</span><span>|&nbsp;&nbsp;Occupation: " +
      "<meter min='0' max='100' high='90' value='" +
      json.occupation +
      "'></meter>&nbsp;" +
      json.occupation +
      "%</span>";
    json.files.sort(function (a, b) {
      return compareStrings(a.name, b.name);
    });
    let content = "";
    document.getElementById("path").innerHTML = currentPath;
    if (currentPath != "/") {
      let pos = currentPath.lastIndexOf("/");
      let newPath;
      if (pos == 0) newPath = "/";
      else newPath = currentPath.substring(0, pos);
      console.log("newpath:" + newPath);
      content +=
        "<div class='fileLine'>" +
        "<div class='fileLineHead'>" +
        "<div class='filetype'>" +
        backIcon() +
        "</div><div class='fileitem' id='updir'> Up..</div></div></div>";
      eventslisteners.push({ action: "updir", id: "updir", target: newPath });
    }
    for (let i1 = 0; i1 < json.files.length; i1++) {
      if (String(json.files[i1].size) == "-1") {
        content +=
          "<div class='fileLine'>" +
          "<div class='fileLineHead'>" +
          "<div class='filetype'>" +
          dirIcon() +
          "</div><div class='fileitem' id='Dir" +
          i1 +
          "'>" +
          json.files[i1].name +
          "</div></div><div class='fileicon'  id='DirDel" +
          i1 +
          "'>" +
          trashIcon() +
          "</div></div>";
        eventslisteners.push({
          action: "dir",
          id: "Dir" + i1,
          target: json.files[i1].name,
        });
        eventslisteners.push({
          action: "dirdel",
          id: "DirDel" + i1,
          target: json.files[i1].name,
        });
      }
    }
    for (let i1 = 0; i1 < json.files.length; i1++) {
      if (String(json.files[i1].size) != "-1") {
        if (
          currentPath == "/" &&
          (json.files[i1].name == "index.html.gz" ||
            json.files[i1].name == "index.html")
        ) {
          showESP3Dbutton = true;
        }
        content +=
          "<div class='fileLine' >" +
          "<div class='fileLineHead'>" +
          "<div class='filetype'>" +
          fileIcon() +
          "</div><div class='fileitem'  id='File" +
          i1 +
          "'>" +
          json.files[i1].name +
          "</div></div><div class='fileLineTail'><div class='filesize'>" +
          json.files[i1].size +
          "</div><div class='fileicon'  id='FileDel" +
          i1 +
          "'>" +
          trashIcon() +
          "</div></div></div>";
        eventslisteners.push({
          action: "file",
          id: "File" + i1,
          target: json.files[i1].name,
        });
        eventslisteners.push({
          action: "filedel",
          id: "FileDel" + i1,
          target: json.files[i1].name,
        });
      }
    }
    document.getElementById("fileList").innerHTML = content;
    for (let i = 0; i < eventslisteners.length; i++) {
      switch (eventslisteners[i].action) {
        case "updir":
          document
            .getElementById(eventslisteners[i].id)
            .addEventListener("click", function () {
              currentPath = eventslisteners[i].target;
              console.log(currentPath);
              SendFileCommand("list", "all");
            });

          break;
        case "file":
          document
            .getElementById(eventslisteners[i].id)
            .addEventListener("click", function () {
              let url = new URL(
                "http://" +
                  window.location.host +
                  currentPath +
                  "/" +
                  eventslisteners[i].target
              );
              window.open(url, "_blank");
            });
          break;
        case "filedel":
          document
            .getElementById(eventslisteners[i].id)
            .addEventListener("click", function () {
              if (
                confirm(
                  "Confirm deletion of file: " + eventslisteners[i].target
                )
              )
                SendFileCommand("delete", eventslisteners[i].target);
            });
          break;
        case "dir":
          document
            .getElementById(eventslisteners[i].id)
            .addEventListener("click", function () {
              currentPath +=
                (currentPath == "/" ? "" : "/") + eventslisteners[i].target;
              console.log(currentPath);
              SendFileCommand("list", "all");
            });

          break;
        case "dirdel":
          document
            .getElementById(eventslisteners[i].id)
            .addEventListener("click", function () {
              if (
                confirm(
                  "Confirm deletion of directory: " + eventslisteners[i].target
                )
              )
                SendFileCommand("deletedir", eventslisteners[i].target);
            });
          break;
      }
    }
  } catch (e) {
    ErrorMSG("Error: json data are incorrect!<br/>" + jsonresponse);
    return;
  } finally {
    if (showESP3Dbutton) {
      document.getElementById("espLink").classList.remove("hide");
    } else {
      document.getElementById("espLink").classList.add("hide");
    }
  }
}

function httpGet(url, processfn) {
  let xmlhttp = new XMLHttpRequest();
  xmlhttp.onreadystatechange = function () {
    if (xmlhttp.readyState == 4) {
      if (xmlhttp.status == 200) {
        ErrorMSG("");
        processfn(xmlhttp.responseText);
      } else if (xmlhttp.status == 401) {
        loginModal.classList.remove("hide");
        loginLink.classList.remove("hide");
      } else {
        console.log(xmlhttp.status);
        ErrorMSG("Error: board does not answered properly " + xmlhttp.status);
      }
    }
  };
  xmlhttp.open("GET", url, true);
  xmlhttp.send();
}

function compareStrings(a, b) {
  // case-insensitive comparison
  a = a.toLowerCase();
  b = b.toLowerCase();
  return a < b ? -1 : a > b ? 1 : 0;
}

function uploadFiles() {
  let files = filesInput.files;
  if (files.length == 0) return;
  let formData = new FormData();
  uploadType = 1;
  filesButton.classList.add("hide");
  createDirButton.classList.add("hide");
  refreshButton.classList.add("hide");
  prgfiletext.classList.remove("hide");
  prgfile.classList.remove("hide");
  prgfile.value = 0;
  prgfiletext.innerHTML = "Uploading ...0%";
  formData.append("path", currentPath);
  let currentpath = currentPath;
  if (!currentpath.endsWith("/")) currentpath += "/";
  for (let i3 = 0; i3 < files.length; i3++) {
    let file = files[i3];
    let arg = currentpath + file.name + "S";
    //append file size first to check updload is complete
    formData.append(arg, file.size);
    formData.append("myfiles", file, currentpath + file.name);
  }
  xmlhttpupload = new XMLHttpRequest();
  xmlhttpupload.open("POST", "/files", true);
  //progress upload event
  xmlhttpupload.upload.addEventListener("progress", updateProgress, false);
  //progress function
  function updateProgress(oEvent) {
    if (oEvent.lengthComputable) {
      let percentComplete = (oEvent.loaded / oEvent.total) * 100;
      prgfile.value = percentComplete;
      prgfiletext.innerHTML =
        "Uploading ..." + percentComplete.toFixed(0) + "%";
    } else {
      // Impossible because size is unknown
      console.log("oops");
    }
  }

  xmlhttpupload.onloadend = function () {
    filesButton.classList.remove("hide");
    createDirButton.classList.remove("hide");
    refreshButton.classList.remove("hide");
    prgfile.classList.add("hide");
    prgfiletext.classList.add("hide");
    filesInput.value = "";
    if (xmlhttpupload.status === 200) {
      dispatchFileStatus(xmlhttpupload.responseText);
    } else if (xmlhttp.status == 401) {
      loginModal.classList.remove("hide");
      loginLink.classList.remove("hide");
    } else uploadError("Error", xmlhttpupload.status);
  };

  xmlhttpupload.send(formData);
}

function uploadFirmware() {
  let files = fwInput.files;
  if (files.length == 0) return;
  let formData = new FormData();
  uploadType = 2;
  firmwareButton.classList.add("hide");
  prgupdatetext.classList.remove("hide");
  prgupdate.classList.remove("hide");
  prgupdate.value = 0;
  prgupdatetext.innerHTML = "Uploading ...0%";
  formData.append("path", currentPath);
  let currentpath = currentPath;
  if (!currentpath.endsWith("/")) currentpath += "/";
  for (let i3 = 0; i3 < files.length; i3++) {
    let file = files[i3];
    let arg = currentpath + file.name + "S";
    //append file size first to check updload is complete
    formData.append(arg, file.size);
    formData.append("myfiles", file, currentpath + file.name);
  }
  xmlhttpupload = new XMLHttpRequest();
  xmlhttpupload.open("POST", "/updatefw", true);
  //progress upload event
  xmlhttpupload.upload.addEventListener("progress", updateProgress, false);
  //progress function
  function updateProgress(oEvent) {
    if (oEvent.lengthComputable) {
      let percentComplete = (oEvent.loaded / oEvent.total) * 100;
      prgupdate.value = percentComplete;
      prgupdatetext.innerHTML =
        "Uploading ..." + percentComplete.toFixed(0) + "%";
    } else {
      // Impossible because size is unknown
      console.log("oops");
    }
  }

  xmlhttpupload.onloadend = function () {
    firmwareButton.classList.remove("hide");
    prgupdate.classList.add("hide");
    fwInput.value = "";
    if (xmlhttpupload.status === 200) {
      prgupdatetext.classList.add("hide");
      firmware.classList.add("hide");
      consolePanel.classList.add("hide");
      fileSystem.classList.add("hide");
      document.getElementById("verLink").classList.add("hide");
      document.getElementById("espLink").classList.add("hide");
      restartTime = 40;
      InfoMSG("<br/>Restarting... please wait 40s");
      setInterval(function () {
        restartTime--;
        InfoMSG("<br/>Restarting... please wait " + restartTime + "s");
        if (restartTime == 0) location.reload();
      }, 1000);
    } else if (xmlhttp.status == 401) {
      loginModal.classList.remove("hide");
      loginLink.classList.remove("hide");
    } else uploadError("Error", xmlhttpupload.status);
  };

  xmlhttpupload.send(formData);
}
