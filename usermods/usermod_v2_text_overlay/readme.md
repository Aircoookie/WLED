# Text overlay
* works only on matrix

This user mod adds an text on top of whatever the matrix is rendering (effects, solid colors ...)

I built this UserMod for sending from home assitant the text, notification like :)

### Usage
On the config/usermods there are some configurations:
 - enable
 - scroolStep (the time to scrool the text - in miliseconds)
 - fontsize (values 1, 2, 3, 4)
 - Color R (default collors - RGB values 8bit, 0-255)

 The usermod will read the data from the json api
 Data example: `{"overlay": {"timeout": 13,"color_r": 0,"color_g": 0,"color_b": 255,"text": "Bla bla"}}`
 The text will be added to the matrix with the respective color, if the color is missing the config color will be used. 
 The text will disapear after 13 secons (the timeout value)  

 Fom home assistant you can use shell commands to run a curl to the WLED device.

 ## Home assistant configuration example:

configuration.yaml

 `shell_command:`
 ` curl_post: "curl -X POST -H 'Content-Type: application/json' -i '{{url}}' --data '{{data}}'"`

Creata an automation 
trigger: time pattern, minutes `/1` (every minute)
action: call service, service: shell_command.curl_post

The action need to be edited in yaml mode:

`service: shell_command.curl_post`
`data:`
`  url: http://192.168.1.151/json/state`
`  data: >-`
`    {"overlay": {"timeout":`
`    {{states("input_number.matrix_time_timeout")|int}},"color_r":`
`    {{states("input_number.matrix_time_red")|int}},"color_g":`
`    {{states("input_number.matrix_time_green")|int}},"color_b":`
`    {{states("input_number.matrix_time_blue")|int}},"text":`
`    "{{now().strftime("%H:%M")}}"}}`

The automation will send the time to the matrix, every minute, using that 4 input number helpers.
I personally use an automation that send every 3 minutes the time with a 20 sec timeout, then a delay of 20 sec, then another call that sends the house temp and then some other infos ...

### Installation

Copy and update the example `platformio_override.ini.sample`, rename it to `platformio_override.ini`
This file should be placed in the same directory as `platformio.ini`.
Add this line to your `platformio_override.ini`: `-D USERMOD_TEXT_OVERLAY`
