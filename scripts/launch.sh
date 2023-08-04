#!/usr/bin/env bash

if [ $# -lt 1 ]
then
    (>&2 echo "### error: expecting work file")
    exit 1
fi
work_file=$1
shift

nr_clients=1
if [ $# -eq 1 ]
then
    nr_clients=$1
fi

(>&2 echo "### info: work file '$work_file' running with $nr_clients clients")

SERVER_INFO="server_info.txt"

worker_server --workfile "$work_file" --server_info $SERVER_INFO &
if [ $? -ne 0 ]
then
    (>&2 echo "### error: failed to launch server")
    exit 2
fi

sleep 1

uuid=$(cut -d ' ' -f 1 $SERVER_INFO)
server=$(cut -d ' ' -f 2 $SERVER_INFO)

(>&2 echo "### info: server launhed at '$server' with UUID '$uuid'")

for i in $(seq $nr_clients)
do
    worker_client --server "$server" --uuid "$uuid" &
    if [ $? -ne 0 ]
    then
        (>&2 echo "### error: failed launching client $i")
    else
        (>&2 echo "### info: client '$i' launched")
    fi
done

wait
