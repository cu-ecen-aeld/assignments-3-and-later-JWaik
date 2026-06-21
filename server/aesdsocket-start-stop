#!/bin/sh
# /etc/init.d/S99aesdsocket

DAEMON=/usr/bin/aesdsocket

case "$1" in
    start)
        echo "Starting aesdsocket..."
        start-stop-daemon --start \
            --exec $DAEMON \
            -- -d
        ;;
    stop)
        echo "Stopping aesdsocket..."
        start-stop-daemon --stop \
            --exec $DAEMON \
            --signal TERM \
            --oknodo
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
        ;;
esac
