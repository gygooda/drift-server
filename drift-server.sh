#!/bin/bash

red_clr="\033[31m"
grn_clr="\033[32m"
end_clr="\033[0m"

cd `dirname $0`
cwd=`pwd`

appname="drift-server"
lockfile="bin/daemon.pid"

function start()
{
    ./bin/check-single $lockfile
    if [ $? -eq 1 ]
    then
        printf "$red_clr%40s$end_clr\n" "$appname is already running"
        exit 1
    fi

    mkdir -p log

    LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:./bin/" ./bin/${appname} ./conf/${appname}.conf

    sleep 1
    ./bin/check-single $lockfile
    if [ $? -eq 0 ]
    then
        printf "$red_clr%40s$end_clr\n" "start $appname failed."
        exit 1
    fi

    printf "$grn_clr%40s$end_clr\n" "$appname has been started"
}

function stop()
{
    ./bin/check-single $lockfile
    running=$?
    if [ $running -eq 0 ]
    then
        printf "$red_clr%40s$end_clr\n" "$appname is not running"
        exit 1
    fi

    while [ $running -eq 1 ]
    do
        kill `cat $lockfile`
        sleep 1
        ./bin/check-single $lockfile
        running=$?
    done

    printf "$grn_clr%40s$end_clr\n" "$appname has been stopped"
}

function restart()
{
    stop
    start
}

function state()
{
    ./bin/check-single $lockfile
    running=$?
    if [ $running -eq 0 ]
    then
        printf "$red_clr%40s$end_clr\n" "$appname is not running"
        exit 1
    fi

    ps -Lfs `cat $lockfile`
}

function usage()
{
    echo "$0 <start|stop|restart|state|setup>"
}

if [ $# -ne 1 ]; then
    usage
    exit 1
fi

case $1 in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        restart 
        ;;
    state)
        state 
        ;;
    postinstall)
        post_install
        ;;
    setup)
        prepare
        start
        ;;
    *)
        usage 
        ;;
    esac

exit 0
