
# SilverArrowTeamComputer

![logo](https://github.com/DylanK46/SilverArrowTeamComputer/blob/master/computerlogo.png?raw=true)

The code for the Harrow School Silver Arrow Greenpower Team's onboard computer; Using MXCHIP IOT Devkit (AZ3166).


The program at Getstarted has everything needed to stream data over http post requests to any specified endpoint.
The data is encoded in JSON format and is send in the request body.

Data streamed is : Acceleration [X,Y,Z] , Gyroscope [X,Y,Z] . Magnetomter [X,Y,Z] , Temperature , Pressure , Humidity.

It is proved to work with a thingsboard instance running in a Docker container on AWS.

## Usage

It is best to follow Microsoft's Guide on how to setup MXCHIP Iot Devkit with Visual Studio Code. This involves downloading the Arduino Extension, Adding the MXCHIP board URL's and installing the ST-Link Driver.

Thingsboard Runs in a Docker Container in AWS and is a working solution; However, you could stream the data to Excel, Google Sheets, Wayscript, Integromat, Zapier or anywhere that has an API endpoint. 
