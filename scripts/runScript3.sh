clear
# remove previews buffer save-files
rm -f ./*txt
# Launch program in background
echo "script : Running routerPI (Redirecting output to routerPI.log)"
stdbuf -oL ./routerPI |& tee routerPI.log &
# Get its PID
PID=$(pidof routerPI)
# Wait for 2 hours
echo "script : waiting for 2 hours"
sleep 2h
# Kill it
echo "script : Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
# Wait for program termination
wait $PID
echo "script : Program Terminated"
