HOST=ESP82-65048
mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/cmd -m "\"init\""
echo -n "Wait..."
sleep 10
echo -n "Sending..."
# ./b64 build/tinyEsp.ota.bin 100 | xargs -I{} -n 1 echo " -h limero.ddns.net -t dst/$HOST/ota/data -q 1  -m" '{}' \"'{}'\"
./b64 build/tinyEsp.ota.bin 100 | xargs -I{} -n 1 mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/data -q 1  -m \"{}\"
echo -n "Wait..."
sleep 10
echo -n "Ending..."
mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/cmd -m "\"end\""
sleep 2
mosquitto_pub -h limero.ddns.net -t dst/$HOST/ota/cmd -m "\"exec\""
