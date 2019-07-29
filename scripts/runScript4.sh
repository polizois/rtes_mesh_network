mkdir m1 m2 m3 &> /dev/null
rm -f m1/routerPI.log m2/routerPI.log m3/routerPI.log
cp ./routerPI m1
cp ./routerPI m2
cp ./routerPI m3

# remove previews buffer save-files
cd m1
rm -f ./*txt
# Launch program in background
clear
echo "script : Running routerPI-m1 (Redirecting output to routerPI.log)"
stdbuf -oL ./routerPI |& tee routerPI.log &
# Get its PID
PID=$(pidof routerPI)
# Wait for 30 minutes
echo "script : Waiting for 40 minutes"
sleep 40m
# Kill it
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
# Wait for program termination
wait $PID
echo "script : Program-m1 Terminated"

#Simulate machine2
cd ../m2
ifconfig eth0 10.0.85.48
rm -f ./*txt
clear
echo "script : Running routerPI-m2 (Redirecting output to routerPI.log)"
stdbuf -oL ./routerPI |& tee routerPI.log &
PID=$(pidof routerPI)
echo "script : Waiting for 40 minutes"
sleep 40m
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
wait $PID
echo "script : Program-m1 Terminated"

#Simulate machine3
cd ../m3
ifconfig eth0 10.0.85.58
rm -f ./*txt
clear
echo "script : Running routerPI-m3 (Redirecting output to routerPI.log)"
stdbuf -oL ./routerPI |& tee routerPI.log &
PID=$(pidof routerPI)
echo "script : Waiting for 40 minutes"
sleep 40m
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
wait $PID
echo "script : Program-m3 Terminated"

ifconfig eth0 10.0.85.38
