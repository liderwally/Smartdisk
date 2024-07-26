var selectedFiles  = Array();
var selectedFile;
// import {visitedPaths,currentPath} from "./general.js";
function setpath() {

    var default_path = document.getElementById("dirBar").value;
    document.getElementById("filepath").value = default_path;

}

function upload() {

    var filePath = document.getElementById("filepath").value;
    var upload_path = "/upload/" + filePath;
    var fileInput = document.getElementById("newfile").files;

    /* Max size of an individual file. Make sure this
     * value is same as that set in file_server.c */
    var MAX_FILE_SIZE = 100 * 1000 * 1024;
    var MAX_FILE_SIZE_STR = "100MB";

    if (fileInput.length == 0) {
        alert("No file selected!");
    } else if (filePath.length == 0) {
        alert("File path on server is not set!");
    } else if (filePath.indexOf(' ') >= 0) {
        alert("File path on server cannot have spaces!");
    } else if (filePath[filePath.length - 1] == '/') {
        alert("File name not specified after path!");
    } else if (fileInput[0].size > 200 * 1024) {
        alert("File size must be less than 200KB!");
    } else {
        document.getElementById("newfile").disabled = true;
        document.getElementById("filepath").disabled = true;
        document.getElementById("upload").disabled = true;

        var file = fileInput[0];
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (xhttp.readyState == 4) {
                if (xhttp.status == 200) {
                    document.open();
                    document.write(xhttp.responseText);
                    document.close();
                } else if (xhttp.status == 0) {
                    alert("Server closed the connection abruptly!");
                    location.reload()
                } else {
                    alert(xhttp.status + " Error!\n" + xhttp.responseText);
                    location.reload()
                }
            }
        };
        xhttp.open("POST", upload_path, true);
        xhttp.send(file);
    }
}



var top_container = document.getElementById("top_cont");
var mid_container = document.getElementById("midrightcont");
var bottom_container = document.getElementById("bottom_cont");
var objCount = 0;




if (window.XMLHttpRequest) {
    // code for modern browsers
    xhttp = new XMLHttpRequest();
} else {
    // code for old IE browsers
    xhttp = new ActiveXObject("Microsoft.XMLHTTP");
}



// for assinging different data to the web viewer 
var metadatas = {
    origin: "D:/sdCard/",
    tree: [ { name: `${origin}`,  directories: [] }],
    currentDir : `${origin}`,
    siblings: "",
    drive: ""
};
// for declairing few initial files to the user
var mifiles = [
    {
        name: "index.html",
        encoding: "text/html",
        path: `${metadatas.origin}/${name}`,
        type: 0,   // 0 for file, 1 for folder , 2 for raw, 3 for unknown 
        size: 20000000, //in bytes
        last_mod_time: "20:20:03-20,01,2023"
    },
    {
        name: "Games",
        encoding: "text/html",
        path: `${metadatas.origin}/${this.name}`,
        type: 1,
        size: 200000000,
        last_mod_time: "20:24:03-23,01,2023"
    },
    {
        name: "Movies",
        encoding: "text/html",
        path: `${metadatas.origin}/${this.name}`,
        type: 1,
        size: 200000000,
        last_mod_time: "20:24:03-23,01,2023"
    },
    {
        name: "Documents",
        encoding: "text/html",
        path: `${metadatas.origin}/${this.name}`,
        type: 1,
        size: 200000000,
        last_mod_time: "20:24:03-23,01,2023"
    },
    {
        name: "Dumps",
        encoding: "text/html",
        path: `${metadatas.origin}/${this.name}`,
        type: 1,
        size: 200000000,
        last_mod_time: "20:24:03-23,01,2023"
    },
    {
        name: "needforspeed.exe",
        encoding: "text/html",
        path: `${metadatas.origin}/${this.name}`,
        type: 0,
        size: 200000000,
        last_mod_time: "20:24:03-23,01,2023"
    }
];

updateFiles(mifiles);

function focusme(elmnt) {
    selectedFiles.push(elmnt);
    selectedFile = elmnt;
    document.activeElement.blur();
    elmnt.focus();
    let props = JSON.parse(elmnt.getAttributeNode("props").value);
    let thisLocation = elmnt.childNodes[elmnt.childNodes.length - 1].innerText;
    let infor = ("<p>size :"+ props.size / 8000 +" Kilobytes</p>");
    infor += "<p> Last modified time : "+props.last_mod_time+ "</p>";
    document.getElementById("dirBar").value = metadatas.origin + thisLocation;
    let inforTab = document.querySelector(".filespecstab");

}

function getfile(elmnt) {
    console.log(elmnt.getAttributeNode("props").value);
    let thisLocation = elmnt.getAttributeNode("props").value.path;
    console.log("preparing to download: "+thisLocation);
    alert(`you realy want to download file ${thisLocation}??`);
    chdir(thisLocation, state = 'download');
}

function chdir(elmnt) {
    let thisLocation = elmnt.childNodes[elmnt.childNodes.length - 1].innerText;
    console.log("changing directory to :"+ thisLocation);
    chdir(thisLocation);
}

function getElmntStr(obj) {
    objCount += 1;
    let val1 = "obj_" + objCount + "";
    let val2 = obj.name;
    let innerTexts = [];
    let str1 = "";
    let str3 = "<div class='col-sep' ></div><p>" + val2 + "</p></div>";

    switch (obj.type) {
        case 1:
            str1 = "<div class='tab folders' id ='" + val1 + "' onclick='focusme(" + val1 + ")' props=" + JSON.stringify(obj) + " ondblclick='chdir(" + val1 + ")'>";
            let str12 = "<img src='icons/folder/folder-100.png' alt='folder' srcset=''>";
            innerTexts = [str1, str12, str3];
            break;
        default:
            str1 = "<div class='tab folders' id ='" + val1 + "' onclick='focusme(" + val1 + ")' props=" + JSON.stringify(obj) + " ondblclick='getfile(" + val1 + ")'>";
            let str02 = "<img src='icons/document/document-75.png' alt='file' srcset=''>";
            innerTexts = [str1, str02, str3];
            break;
    }
    let returnable = innerTexts.join("\n");
    console.log(returnable);
    console.log(objCount);
    return returnable;
}

function postCommand(opt, comm1, comm2, comm3){
let url = opt + "?comm1="+comm1+"&comm2="+comm2+"&comm3="+comm3;
console.log(url);
chdir(url);
}



function updateFiles(jsonFile) {
    let objsStr = [];
    jsonFile.forEach(el => {
        objsStr.push(getElmntStr(el));
    })
    mid_container.innerHTML = '';
    mid_container.innerHTML = objsStr.join("\n\r");
}

function chdir(url) {
    let dataUrl = window.location.href + url;
    console.log(dataUrl);
    xhttp.open("GET", dataUrl, true);
    xhttp.send();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            let txt = this.responseText;
            let milife = JSON.parse(txt);
            updateFiles(milife);
        }
    }
}



function command(spell) {
    let path = document.getElementById("dirBar").value;
    console.log(path);
    if(visitedPaths){visitedPaths.push(path);}else{visitedPaths=[]; visitedPaths.push(path)}
    let apt = " ";
    let comm1= " ";
    let comm2= "";
    let comm3= "";
    let direct = false;
    switch (spell) {
        case "add": 
            apt = "/add";
            comm1 = path;
            direct = true;
            break;

        case "remove": 
        // remove from file list
            apt = "/remove";
            comm1 = path;
            direct = true;
            break;

        case "rename": 
            apt = "/rename";
            comm1 = path;
            comm2 = requestNewFileName();
            direct = true;
            break;

        case "download":
            apt = "/download";
            comm1 = path;
            direct = true;
            break;

        case "home":
            apt = "/";
            comm1 = "";
            direct = true;
            break;

        case "upload": 
            upload();
            direct = false;
            break;

        case "forward": 
            path = getForwardPath(path);
            direct = false;
            break;

        case "back": 
            path = getBackPath(path);
            direct = false;
            break;

        case "info": 
            console.log(selectedFile);
            infoWindow(selectedFile);
            direct = false;
            break;

        case "chdir": 
            apt = "/chdir";
            comm1 = path;
            direct = true;
            break;

        case "metadata": 
            apt = "/metadata";
            comm1 = "";
            direct = true;
            break;
    }

    if(direct){
        postCommand(apt,comm1,comm2,comm3);
    }
    currentPath = path;
}