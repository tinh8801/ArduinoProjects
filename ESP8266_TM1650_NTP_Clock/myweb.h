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

<h2>ESP8266 NTP Clock<h2>
<h3>Settings</h3>

<form action="/action_page">
  Time Scroll Speed
  <input type="text" name="timescroll" value=""> (ms)
  <br><br>
  Date Scroll Speed
  <input type="text" name="datescroll" value=""> (ms)
  <br><br>
  <input type="submit" value="Set">
</form>
<br><br>
<form action="/brightness_page">
Display Brightness
  <input type="text" name="brightness" value=""> (0-7)
     <input type="submit" value="Set">
</form> 
<br><br>
<form action="/autobrightness_page">
Auto Brightness
  <input type="text" name="autobrightness" value=""> (yes/no)
     <input type="submit" value="Set">
</form> 
<br><br>
<font size='18' color='red'>Reset</font>
<form action="/reset_page">
     <input type="submit" value="Reset">
</form> 
@@@
</body>
</html>
)=====";