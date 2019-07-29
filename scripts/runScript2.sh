mkdir m1 m2 &> /dev/null
rm -f m1/routerPI.log m2/routerPI.log
cp ./routerPI m1
cp ./routerPI m2
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
echo "script : Waiting for 30 minutes"
sleep 30m
# Kill it
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
# Wait for program termination
wait $PID
echo "script : Program-m1 Terminated"

#Simulate machine2
cd ../m2
rm -f ./*txt
ifconfig eth0 10.0.85.46
clear
echo "script : Running routerPI-m2 (Redirecting output to routerPI.log)"
stdbuf -oL ./routerPI |& tee routerPI.log &
PID=$(pidof routerPI)
echo "script : Waiting for 30 minutes"
sleep 30m
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
wait $PID
echo "script : Program-m2 Terminated"

#Return to machine1
cd ../m1
ifconfig eth0 10.0.85.36
clear
echo "script : Running routerPI-m1 (Redirecting output to routerPI.log)"
stdbuf -oL ./routerPI |& tee -a routerPI.log &
PID=$(pidof routerPI)
echo "script : Waiting for 30 minutes"
sleep 30m
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
wait $PID
echo "script : Program-m1 Terminated"

#Return to machine2
cd ../m2
ifconfig eth0 10.0.85.46
clear
echo "script : Running routerPI-m2 (Redirecting output to routerPI.log)"
stdbuf -oL ./routerPI |& tee -a routerPI.log &
PID=$(pidof routerPI)
echo "script : Waiting for 30 minutes"
sleep 30m
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
wait $PID
echo "script : Program-m2 Terminated"

ifconfig eth0 10.0.85.36
