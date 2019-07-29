./routerPI &
PID=$(pidof routerPI)
sleep 1m
kill -INT $PID
wait $PID
ifconfig eth0 10.0.85.46
./routerPI &
PID=$(pidof routerPI)
sleep 1m
kill -INT $PID
wait $PID
ifconfig eth0 10.0.85.36
