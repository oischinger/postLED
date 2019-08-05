set +u
CONFIG_PATH=/data/options.json

USERNAME=$(jq --raw-output ".mqtt_username" $CONFIG_PATH)
PASSWORD=$(jq --raw-output ".mqtt_password" $CONFIG_PATH)
HOST=$(jq --raw-output ".mqtt_host_port" $CONFIG_PATH)

cd /usr/src/postled
rmmod snd_bcm2835
setcap 'cap_sys_nice=eip' postled
echo Starting PostLED
./postled -h $HOST -u $USERNAME -p $PASSWORD --led-rows=16 --led-cols=32
echo PostLED terminated


