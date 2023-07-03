#pragma once
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
p {
color: navy;
text-transform: uppercase;
}
</style>
</head>
<body>
<script>

setInterval(function(){getData();},1000);


function getData(){
var xhttp=new XMLHttpRequest();
xhttp.onreadystatechange=function(){
  if(this.readyState==4 && this.status==200){
document.getElementById("thoigian").innerHTML=this.responseText;
    
  }
};
  xhttp.open("GET","showTime",true);
  xhttp.send();
}
</script>

<h2>ESP8266 MAX7219 NTP Clock<h2>
<h3>Settings</h3>
<form action="/brightness_page">
Display Brightness
  <input type="text" name="brightness" value=""> (0-15)
     <input type="submit" value="Set">
</form> 
<br><br>
<form action="/autobrightness_page">
Auto Brightness
  <input type="text" name="autobrightness" value=""> (yes/no)
     <input type="submit" value="Set">
</form> 
<br><br>
<div>
Time: <span id="thoigian">0</span><br>
</div>
<font size='18' color='red'>Reset</font>
<form action="/reset_page">
     <input type="submit" value="Reset">
</form> 
@@@
</body>
</html>
)=====";
