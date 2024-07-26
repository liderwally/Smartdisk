#include <stdint.h>
#include <String.h>

String commands[]= {"download","back","forward","add","delete","upload","rename","extract","home"};  //command list for the shell


String indexHtml = R"(
<!DOCTYPE html>
<html lang='en'>

    <head>
        <title>Smartdisk-(Backup Web)</title>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
    </head>
    <style>
        :root {
            --baseColor: #4675e3;
            --firstColor: #aab8e1;
            --secColor: rgb(163, 168, 173);
            --thirdColor: #b85838;
            --fourthColor: #af3341;
            --fifthColor: #52172b;
            --tab-color-red: 0;
            --tab-color-green: 0;
            --tab-color-blue: 0;
            --tab-rev-color-red: 255;
            --tab-rev-color-green: 255;
            --tab-rev-color-blue: 255;

            --tab-opacity: 0.175;
        }

        * {
            transition: 1s;
        }

        body {
            background-color: var(--baseColor);
            display: inline;

        }
        .row{
            width: 100%;
            height: fit-content;
        }
        .col{
            height: 100%;
            width:fit-content;
            padding: 10px;
            margin:10px;
        }
        .col-sep{
            height: 90%;
            border-right: 1px solid black;
            width:1px;
            margin: 3px;
        }
        .row-sep{
            width: 100%;
            height:1px;
            border-bottom: 1px solid black;
            margin: 3px;
        }
        .tab{
            margin: 10px;
            padding: 10px;
            margin-left: auto;
            margin-right: auto;
            background-color: rgba(var(--tab-color-red),var(--tab-color-green),var(--tab-color-blue),var(--tab-opacity));
            color: black;
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
            border-radius: 20px;
        }
        .container {
            width: 90vw;
            align-self: center;
            display: flex;
            position: relative;
            justify-self: center;
     
        }

        #top_cont {
            height: fit-content;
            padding: 12px;
            display: flex;
            flex-direction: column;
            border: 2px solid black;
            
            

        }
        #mid_cont {
            height: 65vh;
            padding: 12px;
            display: flex;
            flex-direction: row;
            border: 2px solid black;
            

        }

        #midrightcont {
            height: 60vh;
            width: 78%;
            background-color: rgb(163, 168, 173);
            border-radius: 0;
            display: flex;
            flex-wrap: wrap;
            align-content: flex-start;
            flex-direction: row;
            justify-content: flex-start;
            resize: both;
            border-top-right-radius: 10px 10px;
            border-bottom-right-radius: 10px 10px;
            overflow: auto;

        }
        #midleftcont{
            height: 60vh;
            width: 18%;
            background-color: rgb(163, 168, 173);
            border-radius:10px 10px ;
            scrollbar-width: 2px;
            scrollbar-gutter: 2px;
            border-radius: 0;
            resize: horizontal;
            overflow: auto;
            border-top-left-radius: 10px 10px;
            border-bottom-left-radius: 10px 10px;
        }
        #bottom_cont {
            height: 5vh;
            display: flex;
            flex-direction: row;
            flex-wrap: nowrap;
            align-content: center;
            justify-content: space-between;
            align-items: center;

        }

        .dirctfolder {
            height:50px;
            width: 97%;
            position: relative;
            display: flex;
            border-radius: 10px;
            flex-direction: row;
            flex-wrap: nowrap;
            align-content: space-around;
            align-items: center;
            justify-content: space-between;
            background-color: var(--secColor);

        }

        .scndbar_component {
            height: 50px;
            width: 15%;
            padding: 10px;
            margin: 10px;
            transition: 05.s;
            border-radius: 15px;
            align-items: center;
            display: inline-flex;
            position: relative;
            flex-wrap: wrap;
            flex-direction: column;
            justify-content: space-around;
            align-content: space-around;
        }


        .scndbar_component:hover {
            background-color: rgba(var(--tab-rev-color-red),var(--tab-rev-color-green),var(--tab-rev-color-blue),var(--tab-opacity));
            color: black;

            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
        }
        #dirBar{
            border: none;
            background-color: rgba(var(--tab-rev-color-red),var(--tab-rev-color-green),var(--tab-rev-color-blue),var(--tab-opacity));
            color: black;
            width: 53%;
        }
        #searchBar{
            border: none;
            background-color: rgba(var(--tab-rev-color-red),var(--tab-rev-color-green),var(--tab-rev-color-blue),var(--tab-opacity));
            color: black;
        }

        .file,
        .folders {
            height: fit-content;
            max-height: 150px;
            width: 100px;
            position: relative;
            margin: 10px;
            text-align: center;
            word-wrap: break-word;
            left: 0px;
            border-radius: 10px;
            color: #aab8e1;
            display: flex;
            flex-direction: column;


        }

        .scndbar_component img {
            height: 35px;
            width: 35px;
            position: relative;
            color: aliceblue;
        }

        .scndbar_component~.comptext {
            width: 30px;
            height: 20px;
            position: relative;
            color: aliceblue;

        }

        .folders:hover {
            box-shadow: 1px 1px 2px 1px black;
            backdrop-filter: blur(5px);
            -webkit-backdrop-filter: blur(5px);
        }

        .file:hover {
            box-shadow: 1px 1px 2px 1px black;
            backdrop-filter: blur(5px);
            -webkit-backdrop-filter: blur(5px);

        }

        .dirctfolder img {
            height: 20px;
            width: 30px;
            position: relative;
            justify-self: center;
            align-self: center;

        }

        .dirctfolder input {
            height: 60%;
            width: 40%;
            align-self: center;
            margin-right: 10px;
            border-radius: 10px;

        }
        #dir2tab{
            height: 90px;
            
        }

        #searchBar{
        background-image: url('./icons?iconName = "search"');
        background-size: 20px 20px;
        background-repeat: no-repeat;
        background-position: calc(100% - 10px) 10px;

    }
    .listdir{
        width: 94%;
        height: 25px;
        padding:0px 5px 0px 5px;
        margin-top:5px ;
        border-bottom: 1px solid rgba(var(--tab-color-red),var(--tab-color-green),var(--tab-color-blue),var(--tab-opacity));
        color: black;
    }
    .filespecstab{
        width: fit-content;
        height: 100%;
        display: flex;
        flex-direction: row;
        justify-content: flex-start;
        align-content: center;
        align-items: center;
    }
    </style>

    <body>

        <!-- **************************************upper container ************************ -->
        <div class='tab container' id='top_cont'>
            <div class='tab row dirctfolder'>

                <img src='/icons?iconName = "home"' alt='home'  onclick='/'>

                <div class='col-sep'></div>

                <input type='text' class='inputbar' name='directory' id='dirBar' value='/' style='height: 35px; margin: 10px;'
                    onchange='getToLocation()'>

                <div class='col-sep'></div>

                <input type='text' name='directory' id='searchBar' class='inputbar'  placeholder='Search here...'  style='height: 35px; margin: 10px;'
                onchange='findLocation()'>

            </div>

            <div class='row-sep'></div>
            
            <div class='tab row dirctfolder' id='dir2tab'>
                <p class='scndbar_component row'>
                    <img src='/icons?iconName=back' alt='back Icon' srcset=' onclick='>
                    <span class='comptext'>back</span>
                </p>
                <div class='col-sep'></div>
                <p class='scndbar_component row'>
                    <img src='/icons?iconName=download2' alt='download Icon' srcset=' onclick='>
                    <span class='comptext'>download</span>
                </p>
                <div class='col-sep'></div>
                <p class='scndbar_component row'>
                    <img src=''/icons?iconName=upload2' alt='upload Icon' srcset=' onclick='upload()'>
                    <span class='comptext'>upload</span>
                </p>
                <div class='col-sep'></div>
                <p class='scndbar_component row'>
                    <img src=''/icons?iconName=forward' alt='forward Icon' srcset=' onclick='>
                    <span class='comptext'>forward</spanp>
                </p>
                <div class='col-sep'></div>
                <p class='scndbar_component row'>
                    <img src='/icons?iconName=plus' alt='plus Icon' srcset=' onclick='>
                    <span class='comptext'>add</spanp>
                </p>
                <div class='col-sep'></div>
                <p class='scndbar_component row'>
                    <img src='/icons?iconName=minus' alt='remove Icon' srcset=' onclick='>
                    <span class='comptext'>remove</spanp>
                </p>
                <div class='col-sep'></div>
                <p class='scndbar_component row'>
                    <img src='/icons?iconName=info' alt='Info Icon' onclick=''>
                    <span class='comptext'>info</spanp>
                </p>
                <div class='col-sep'></div>
                <p class='scndbar_component row'>
                    <img src='/icons?iconName =settings2' alt='settings Icon' onclick=''>
                    <span class='comptext'>settings</spanp>
                </p>

            </div>
        </div>

        <!-- ************************************** mid container ************************** -->
        <div class='tab container' id='mid_cont'>
            <div class='tab col' id='midleftcont'>
                <div class='row listdir'></div>
                <tr></tr>
                <div class='row listdir'></div>
                <tr></tr>
                <div class='row listdir'></div>
                <tr></tr>
            </div>

            <div class='col-sep'></div>

            <div class='tab col' id='midrightcont'>
                
            </div>  

        </div>
        </div>

        <!-- **************************************bottom container ************************** -->
        <div class='tab row container' id='bottom_cont'>
            <table class='fixed col' id = 'uploader' >
                    <table >
                        <tr>
                            <td>
                                <label for='newfile'>Upload a file</label>
                            </td>
                            <td colspan='2'>
                                <input id='newfile' type='file' onchange='setpath()' style='width:100%;'>
                            </td>
                            <td>
                                <button id='upload' type='button' onclick='upload()'>Upload</button>
                            </td>
                        </tr>
                    </table>
               </table>
            <div class='col-sep'></div>
            <div class='col filespecstab'>
                
            </div>
        </div>


    </body>

    <script>

        function setpath() {

            var default_path = document.getElementById('dirBar').value;
            document.getElementById('filepath').value = default_path;

        }

        function upload() {

            var filePath = document.getElementById('filepath').value;
            var upload_path = '/upload?' + filePath;
            var fileInput = document.getElementById('newfile').files;

            /* Max size of an individual file. Make sure this
             * value is same as that set in file_server.c */
            var MAX_FILE_SIZE = 200 * 1024;
            var MAX_FILE_SIZE_STR = '200KB';

            if (fileInput.length == 0) {
                alert('No file selected!');
            } else if (filePath.length == 0) {
                alert('File path on server is not set!');
            } else if (filePath.indexOf(' ') >= 0) {
                alert('File path on server cannot have spaces!');
            } else if (filePath[filePath.length - 1] == '/') {
                alert('File name not specified after path!');
            } else if (fileInput[0].size > 200 * 1024) {
                alert('File size must be less than 200KB!');
            } else {
                document.getElementById('newfile').disabled = true;
                document.getElementById('filepath').disabled = true;
                document.getElementById('upload').disabled = true;

                var file = fileInput[0];
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function () {
                    if (xhttp.readyState == 4) {
                        if (xhttp.status == 200) {
                            document.open();
                            document.write(xhttp.responseText);
                            document.close();
                        } else if (xhttp.status == 0) {
                            alert('Server closed the connection abruptly!');
                            location.reload()
                        } else {
                            alert(xhttp.status + ' Error!\n' + xhttp.responseText);
                            location.reload()
                        }
                    }
                };
                xhttp.open('POST', upload_path, true);
                xhttp.send(file);
            }
        }



        var top_container = document.getElementById('top_cont');
        var mid_container = document.getElementById('midrightcont');
        var bottom_container = document.getElementById('bottom_cont');
        var objCount = 0;




        if (window.XMLHttpRequest) {
            // code for modern browsers
            xhttp = new XMLHttpRequest();
        } else {
            // code for old IE browsers
            xhttp = new ActiveXObject('Microsoft.XMLHTTP');
        }



        // for assinging different data to the web viewer 
        var metadatas = {
            origin: 'D:/sdCard/',
            tree: [ { name: `${origin}`,  directories: [] }],
            currentDir : `${origin}`,
            siblings: '',
            drive: ''
        };
        // for declairing few initial files to the user
        var mifiles = [
            {
                name: 'index.html',
                encoding: 'text/html',
                path: `${metadatas.origin}/${name}`,
                type: 0,   // 0 for file, 1 for folder , 2 for raw, 3 for unknown 
                size: 20000000, //in bytes
                last_mod_time: '20:20:03-20,01,2023'
            },
            {
                name: 'Games',
                encoding: 'text/html',
                path: `${metadatas.origin}/${this.name}`,
                type: 1,
                size: 200000000,
                last_mod_time: '20:24:03-23,01,2023'
            },
            {
                name: 'Movies',
                encoding: 'text/html',
                path: `${metadatas.origin}/${this.name}`,
                type: 1,
                size: 200000000,
                last_mod_time: '20:24:03-23,01,2023'
            },
            {
                name: 'Documents',
                encoding: 'text/html',
                path: `${metadatas.origin}/${this.name}`,
                type: 1,
                size: 200000000,
                last_mod_time: '20:24:03-23,01,2023'
            },
             {
                 name: 'Dumps',
                 encoding: 'text/html',
                 path: `${metadatas.origin}/${this.name}`,
                 type: 1,
                 size: 200000000,
                 last_mod_time: '20:24:03-23,01,2023'
             },
            {
                name: 'needforspeed.exe',
                encoding: 'text/html',
                path: `${metadatas.origin}/${this.name}`,
                type: 0,
                size: 200000000,
                last_mod_time: '20:24:03-23,01,2023'
            }
        ];

        updateFiles(mifiles);

        function focusme(elmnt) {
            let props = JSON.parse(elmnt.getAttributeNode('props').value);
            let thisLocation = elmnt.childNodes[elmnt.childNodes.length - 1].innerText;
            let infor = ('<p>size :'+ props.size / 8000 +' Kilobytes</p>');
            infor += '<p> Last modified time : '+props.last_mod_time+ '</p>';
            document.getElementById('dirBar').value = metadatas.origin + thisLocation;
            let inforTab = document.querySelector('.filespecstab');

            inforTab.innerHTML = infor;




        }

        function getfile(elmnt) {
            console.log(elmnt.getAttributeNode('props').value);
            let thisLocation = elmnt.getAttributeNode('props').value.path;
            console.log('preparing to download: '+thisLocation);
            alert(`you realy want to download file ${thisLocation}??`);
            chdir(thisLocation, state = 'download');
        }

        function chdir(elemnt) {
            let thisLocation = elmnt.childNodes[elmnt.childNodes.length - 1].innerText;
            console.log('changing directory to :'+ thisLocation);
            chdir(thisLocation);
        }

        function getElmntStr(obj) {
            objCount += 1;
            let val1 = 'obj_' + objCount + '';
            let val2 = obj.name;
            let innerTexts = [];
            let str1 = ';
            let str3 = '<div class='col-sep' ></div><p>' + val2 + '</p></div>';

            switch (obj.type) {
                case 1:
                    str1 = '<div class='tab folders' id ='' + val1 + '' onclick='focusme(' + val1 + ')' props=' + JSON.stringify(obj) + ' ondblclick='chdir(' + val1 + ')'>';
                    let str12 = '<img src=''./icons?iconName = "folder"' alt='folder' srcset=''>';
                    innerTexts = [str1, str12, str3];
                    break;
                default:
                    str1 = '<div class='tab folders' id ='' + val1 + '' onclick='focusme(' + val1 + ')' props=' + JSON.stringify(obj) + ' ondblclick='getfile(' + val1 + ')'>';
                    let str02 = '<img src=''/icons?iconName = "document"' alt='file' srcset=''>';
                    innerTexts = [str1, str02, str3];
                    break;
            }
            let returnable = innerTexts.join('\n');
            console.log(returnable);
            console.log(objCount);
            return returnable;
        }

        function updateFiles(jsonFile) {
            let objsStr = [];
            jsonFile.forEach(el => {
                objsStr.push(getElmntStr(el));
            })
            mid_container.innerHTML = '';
            mid_container.innerHTML = objsStr.join('\n\r');
        }

        function chdir(filename, state = 'chdir') {
            let dataUrl = window.location.href + '/' + filename+'state='+state;
            console.log(dataUrl);
            xhttp.open('GET', dataUrl, true);
            xhttp.send();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    let txt = this.responseText;
                    let mifile = JSON.parse(txt);
                    updateFiles(milife.files);
                    
                }
            }
        }

    </script>

</html>
)";

const char index_html[] PROGMEM = R"=====(
  <!DOCTYPE html> <html>
    <head>
      <title>ESP32 Captive Portal</title>
      <style>
        body {background-color:#06cc13;}
        h1 {color: white;}
        h2 {color: white;}
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
    </head>
    <body>
      <h1>Hello World!</h1>
      <h2>This is a captive portal emergence window. All requests with problems will be redirected here </h2>
      <div>Click the link below to redirect the page to main page</div>
      <a href="http://smartdisk.local/" ><a/>
    </body>
  </html>
)=====";



String defaultErrorMessage =  R"( <!DOCTYPE html><html><head><meta http-equiv='Content-Type' content='text/html;charset=utf-8'><title>Error response</title></head><body><h1>Error response</h1><p>Error code: %(code)d</p><p>Message: %(message)s.</p><p>Error code explanation: %(code)s - %(explain)s.</p></body></html>)";