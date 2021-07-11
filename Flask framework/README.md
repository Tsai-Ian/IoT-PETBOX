IoT_Petbox
SERVER : RASPBERRY PI
DATABASE :　MARIADB
BROWSER - WEBSOCKET - FLASK WEB APP - MQTT - MOSQUITTO - MQTT - ESP32

安裝 Maria DB

安裝前先確認系統為最新版本

$ sudo apt-get update
$ sudo apt-get upgrade

 MYSQL 安裝到 RASPBERRY PI

$ sudo apt-get install mariadb-server

MYSQL 安全性設置

$ sudo mysql_secure_installation

以 root 進入 MYSQL

$ sudo mysql -u root -p

