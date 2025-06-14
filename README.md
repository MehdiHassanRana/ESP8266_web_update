# ESP8266_web_update
ESP8266 will update its firmware when connected to web if new firmware available.

Change the following code as per your need. 
Export as compiled binary (.bin), rename it as firmware.bin, change the version number in version.txt & in the sketch (must be matched) and upload to your github repo. Before starting web update, the first program should be upload to your ESP8266 directly via USB as usual.

const char* ssid = "myssid";
const char* password = "mypassword";

// Define the current firmware version installed on the ESP8266:
#define CURRENT_FIRMWARE_VERSION "1.1.18"

// URLs for version checking and firmware update (GitHub raw URLs)
#define VERSION_URL "https://raw.githubusercontent.com/MehdiHassanRana/ESP8266_web_update/main/version.txt"
#define FIRMWARE_URL "https://raw.githubusercontent.com/MehdiHassanRana/ESP8266_web_update/main/firmware.bin"
