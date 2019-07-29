# Launch script in background
rm ./*txt
echo "Runnung routerPI (Redirecting output to routerPI.log)"
./routerPI > routerPI.log &
# Get its PID
PID=$!
# Wait for 2 seconds
sleep 2s
# Kill it
echo "Sending Ctrl+c signal (Wait for program termination)"
kill -INT $PID
wait $PID
echo "Program Terminated"
