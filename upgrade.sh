HOST=$1
echo -n "init"
#mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/cmd -m "\"init\""
echo -n "Wait..."
#sleep 10
echo -n "Send data..."
mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/data -q 1  -f build/tinyEsp.ota.bin
# ./b64 build/tinyEsp.ota.bin 100 | xargs -I{} -n 1 echo " -h limero.ddns.net -t dst/$HOST/ota/data -q 1  -m" '{}' \"'{}'\"
# ./b64 build/tinyEsp.ota.bin $1 | xargs -I{} -n 1 mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/data -q 1  -m \"{}\"
echo -n "End..."
#mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/cmd -m "\"end\""
echo -n "Exec..."
#mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/cmd -m "\"exec\""
