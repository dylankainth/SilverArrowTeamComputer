
# SilverArrowTeamComputer

![logo](https://github.com/DylanK46/SilverArrowTeamComputer/blob/master/computerlogo.png?raw=true)

The code for the Harrow School Silver Arrow Greenpower Team's onboard computer; Using MXCHIP IOT Devkit (AZ3166).


The program at Getstarted has everything needed to stream data over http post requests to any specified endpoint.
The data is encoded in JSON format and is send in the request body.

Data streamed is : Acceleration [X,Y,Z] , Gyroscope [X,Y,Z] . Magnetomter [X,Y,Z] , Temperature , Pressure , Humidity and Voltage across a specified pin.

It is proved to work with a thingsboard instance running in a Docker container on AWS t2 micro EC2.

## Usage

It is best to follow Microsoft's Guide on how to setup MXCHIP Iot Devkit with Visual Studio Code. This involves downloading the Arduino Extension to Visual Studio Code, Adding the [MXCHIP board URL's](https://raw.githubusercontent.com/VSChina/azureiotdevkit_tools/master/package_azureboard_index.json) and installing the [ST-Link Driver](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-utilities/stsw-link009.html?dl=h7V82kGQr7Jnogpwu2WmTg%3D%3D%2CAMYUlg5DRLy%2FPuK6xFzpxwj0mndryzOWDXtmJN8lLb4W9e4Xs79tjTZtV5nQXjcLtjz110bltEJvt5Wc7dHl8Dl57JGz3BjDG12uP1PHO4CmwOmM%2BJPGTrjfjbkbPmLTinpgQo3oNTfQUqL9gCWd3v9oir9aQ6m91EbaHmNXk2IQTU5JZ7U8YQYtAuzQ4uSTiVdCBfU%2F%2B%2FoJnR9rRL%2FKAPe6iUTLD2tOsSE3BvfrFrvENaAWP0LFwKubX%2BbKXGkB01ZJ%2FunCtkCiR9%2Fk5qDay%2BZWT7oLWzm2Onr9EWqu2ZtibqUrUGXxwMM2C0vqEnKN&uid=8d18BbbErnPJvaSFf4UFf81qYVIFi1lC#get-software).

Thingsboard Runs in a Docker Container in AWS and is a working solution; However, you could stream the data to Excel, Google Sheets, Wayscript, Integromat, Zapier or anywhere that has an API endpoint. 