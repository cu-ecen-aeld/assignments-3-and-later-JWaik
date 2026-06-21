#!/bin/sh
# /etc/init.d/S99aesdsocket

DAEMON=/usr/bin/aesdsocket
PIDFILE=/var/run/aesdsocket.pid

case "$1" in
    start)
        echo "Starting aesdsocket..."
        start-stop-daemon --start \
            --exec $DAEMON \
            --pidfile $PIDFILE \
            --make-pidfile \
            --background \
            -- -d
        ;;
    stop)
        echo "Stopping aesdsocket..."
        start-stop-daemon --stop \
            --exec $DAEMON \
            --pidfile $PIDFILE \
            --signal TERM \
            --oknodo
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac
