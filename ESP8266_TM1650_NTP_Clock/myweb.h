#pragma once
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
p {
color: navy;
text-transform: uppercase;
}
</style>
</head>
<body>
<h2>ESP8266 TM1650 NTP Clock<h2>
<h3>Settings</h3>

<div>
<span>Scroll Speed:</span>
<span>Fast </span><input type="range" min="1" max="10" class="slider" id="scrollSlider"><span> Slow</span>
</div>
<br><br>
<div>
<span>Intensity: </span><input type="range" min="0" max="7" class="slider" id="intensitySlider">
</div>
<br><br>
<div>
<span>12h Mode: </span><input type="checkbox" id="mode12h" tabindex="0">
</div>
<br><br>
<div>
<span>Auto Intensity: </span><input type="checkbox" id="autointensity" tabindex="0">
</div>
<br><br>
<div>
Status:<br><span id="allstatus"></span>
</div>
<br><br>
<p><font size='18' color='red'>Reset</font></p>
<br><br>
<form action="/reset_page">
<input type="submit" value="Reset">
</form>

<script>
setInterval(function(){receiveAllStatus();receiveDisplayMode();receiveAIMode();receiveScrollSlider();receiveIntensitySlider();},1000);

document.getElementById("scrollSlider").oninput = function(){
  var scrollVal=this.value;
  var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
          }
        };
        xhttp.open("GET","setScroll?scrollVal="+scrollVal,true);
        xhttp.send();
  };
  
document.getElementById("intensitySlider").oninput = function(){
  var intensityVal=this.value;
  var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
          }
        };
        xhttp.open("GET","setIntensity?intensityVal="+intensityVal, true);
        xhttp.send();
  };
  
document.getElementById("mode12h").onclick = function() {
        // access properties using this keyword
        var display12hMode;
        if (this.checked) {
          display12hMode = "ON";
        } else {
          display12hMode = "OFF";
        }
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            //console.log("Successfully received ");
          }
        };
        
        xhttp.open("GET","toggleDisplayMode?display12hMode="+display12hMode, true);
        xhttp.send();
      };
      
document.getElementById("autointensity").onclick = function() {
        // access properties using this keyword
        var autoIntensity;
        if (this.checked) {
          autoIntensity = "ON";
        } else {
          autoIntensity = "OFF";
        }
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
          }
        };
        xhttp.open("GET","toggleAIMode?autoIntensity="+autoIntensity, true);
        xhttp.send();
      };
      
function receiveAllStatus(){
var xhttp=new XMLHttpRequest();
xhttp.onreadystatechange=function(){
  if(this.readyState==4 && this.status==200){
document.getElementById("allstatus").innerHTML=this.responseText;
  }
};
  xhttp.open("GET","updateStatus",true);
  xhttp.send();
}

function receiveDisplayMode(){
var xhttp=new XMLHttpRequest();
xhttp.onreadystatechange=function(){
if(this.readyState==4 && this.status==200){
if(this.responseText=="true"){
document.getElementById("mode12h").checked=true;
}else{
  document.getElementById("mode12h").checked=false;
  }
}
};
xhttp.open("GET","update12hCheckbox",true);
xhttp.send();
}

function receiveAIMode(){
var xhttp=new XMLHttpRequest();
xhttp.onreadystatechange=function(){
if(this.readyState==4 && this.status==200){
if(this.responseText=="true"){
document.getElementById("autointensity").checked=true;
}else{
  document.getElementById("autointensity").checked=false;
  }
}
};
xhttp.open("GET","updateAICheckbox",true);
xhttp.send();
}

function receiveScrollSlider(){
var xhttp=new XMLHttpRequest();
xhttp.onreadystatechange=function(){
if(this.readyState==4 && this.status==200){
document.getElementById("scrollSlider").setAttribute("value",this.responseText);
}
};
xhttp.open("GET","updateScrollSlider",true);
xhttp.send();
}

function receiveIntensitySlider(){
var xhttp=new XMLHttpRequest();
xhttp.onreadystatechange=function(){
if(this.readyState==4 && this.status==200){
document.getElementById("intensitySlider").setAttribute("value",this.responseText);
}
};
xhttp.open("GET","updateIntensitySlider",true);
xhttp.send();
}
</script>
</body>
</html>
)=====";
