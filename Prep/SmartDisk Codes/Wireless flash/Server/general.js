var htmlElement = document.getElementsByTagName( "html" );
var canvas  = document.createElement("canvas");
var currentPath = "./server/"; // default path is the root directory of this script
var visitedPaths = Array(20); // Stores the paths that have been visited by users.
var lastWindowIndex = 0;

canvas.id = "bgCanva";
htmlElement[0].appendChild(canvas);

visitedPaths.push(currentPath); // The first path is empty,



//functions to create overlay for displaying informations
//screen
function createScreen(){
    let modScreen = document.createElement("div");
    modScreen.style += " width: 90vw; height:90vh; position:absolute; backdrop-filter: blur(10px);-webkit-backdrop-filter: blur(10px);border-radius: 15px; " ;
    return modScreen;
}
//titleBar
function createTitleBar() {
    let titleBar = document.createElement('div');
    titleBar.className='row titlebar' ;
    return titleBar;
}
//input field
function createInputField(type,editable,placeHolder ){
    let inputField=document.createElement('input');
    inputField.type= type;
    if ( editable == false ) {
        inputField.readOnly="true";
    }
    inputField.placeholder= placeHolder;
    return inputField;
}
//button
function createButton(name) {
    let button = document.createElement('button');
    button.innerHTML = name;
    button.className ="btn btn-primary";
    return button;
}

function createButton(name,func){
    let button = createButton(name);
    button.addEventListener("click",func) ;
    return button;
}

//window
function createWindow(w,h) {
    let  window = document.createElement("div");
        window.style.width= w ;
        window.style.height= h ;
        window.className="window";
        
    // add title bar to the window
    let titleBar = document.createElement('div');
        titleBar.className='titlebar'
    
    // add minimize button to the title bar
    let minButton = document.createElement('button') ;
        minButton.innerHTML = '\u2610';   //minimize symbol
        minButton.className ="minimize";
        minButton.onclick = function() {
            if (window.style.display ==      "none"){
                window.style.display =       "block";
                minButton.innerHTML = "\u2610";
            } else {
                window.style.display =       "none";
                minButton.innerHTML = "\u2611";
            };
        };

    // add close button to the title bar
    let closeButton = document.createElement('button');
        closeButton.innerHTML = '\u00d7';  //close symbol
        closeButton.className ="close"
        closeButton.onclick = function() {
           windowsArray.removeElement(window);
           window.parentNode.removeChild(window);
        };

    titleBar.appendChild(minButton);
    titleBar.appendChild(closeButton);  
    return window;
}







//when renaming
function requestNewFileName() {
    // Create a new screen element

let screen = createScreen();

// Create a new window element with a width of 400 pixels and a height of 200 pixels
let window = createWindow('400px', 'fit-content');
window.className = "infoWindow";

// Add a title bar to the window element
let titleBar = createTitleBar();
titleBar.innerHTML = "Save New File As...";
window.appendChild(titleBar);

// Create a new input field with a label of "New File Name"
let inputField = createInputField("text", true, "New File Name");
window.appendChild(inputField);

// Create a new button with a label of "Submit"
let button = createButton("Submit");

// Add an event listener to the button that calls the requestNewFileName() function
button.addEventListener("click", function() {
  let newName = inputField.value;

  // Rename the file here using the oldName and newName
});

// Add the button to the window element
window.appendChild(button);

// Append the window element to the screen element
screen.appendChild(window);

// Append the screen element to the HTML element
htmlElement[0].appendChild(screen);
setTimeout(function(){
htmlElement[0].removeChild(screen);

}, (1 * 60 * 1000 ));
return newName;
}



function getBackPath(){
 return visitedPaths[visitedPaths.length-1];
}
function getForwardPath(){
    return visitedPaths[visitedPaths.length-2];
}



//when User Requests file info
function infoWindow(elmnt) {
console.log(elmnt);
let infos = JSON.parse(elmnt.getAttributeNode("props").value);

let fileName = infos.name;
let fileencoding = infos.encoding;
let filetype = ()=>{
    if(infos.type){
        return 'file';
    }else{
        return 'directory';
    }
};
let filesize =  infos.size;
let modTime = infos.last_mod_time;

let screen = createScreen();
screen.style.position="fixed";
let infoTab = createWindow("500px","fit-content");
infoTab.id = 'infotab';

//create Title and Textarea for Info
infoTab.appendChild(createInputField("text",false,"file name: "+fileName ));
infoTab.appendChild(createInputField("text",false,"file encoding: "+fileencoding));
infoTab.appendChild(createInputField("text",false,"filetype: "+filetype()));
infoTab.appendChild(createInputField("text",false,"fileSize: "+filesize));
infoTab.appendChild(createInputField("text",false,"Modified Time : "+modTime));



htmlElement[0].appendChild(screen);

screen.appendChild(infoTab);

setTimeout(function(){

htmlElement[0].removeChild(screen);

}, (1 * 10 * 1000 ));

}


function preupload(){
    let screen = createScreen();
    let dropArea = createWindow("90vw", "90vh");
    dropArea.style.display ="flex" ;
    dropArea.style.justifyContent= "center" ;
    dropArea.style.alignItems= "center" ;
    dropArea.style.flexDirection= "column" ;
    //dropArea.style.border= "dashed grey 3px" ;
    let form = document.createElement('div');
    form.innerHTML = "<div id='drop-area'><form class='my-form'><p>Upload multiple files with the file dialog or by dragging and dropping images onto the dashed region</p><input type='file' id='fileElem' multiple accept='image' onchange='handleFiles(this.files)'><label class='button' for='fileElem'>Select some files</label></form></div>";
    form.style.width="90%" ;

    ;['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
        dropArea.addEventListener(eventName, preventDefaults, false)
      })
      
      function preventDefaults (e) {
        e.preventDefault()
        e.stopPropagation()
      }
      
      ;['dragenter', 'dragover'].forEach(eventName => {
        dropArea.addEventListener(eventName, highlight, false)
      })
      
      ;['dragleave', 'drop'].forEach(eventName => {
        dropArea.addEventListener(eventName, unhighlight, false)
      })
      
      function highlight(e) {
        dropArea.classList.add('highlight')
      }
      
      function unhighlight(e) {
        dropArea.classList.remove('highlight')
      }

      
    
    
    
    
    
    
    htmlElement[0].appendChild(screen);
    setTimeout(function(){

        htmlElement[0].removeChild(screen);
        
        }, (5 * 10 * 1000 ));
        
}

