#!/usr/bin/env bash
#
# variables that should be subsituted
#   * shebang, shebang for this script
#   * scheduler_directives
#   * worker_path
#   * server_info
#   * log_opt, e.g., --log path_to_log_file
#   * --port 5555, e.g., --port 1234
#   * server_start_delay, 5 (in seconds)
#   * workfile
#   * env_var_exprs, e.g., '^VSC' '^PBS_'
#   * num_cores
#   * exit_on_client_fail, i.e., true or false
#
#PBS -A lp_worker_test
#PBS -l nodes=1:ppn=2
#PBS -l walltime=3:00:00
#PBS -l mem=3gb


# source worker functions used in this script
source "/home/gjb/Documents/worker-ng/scripts/worker_functions.sh"

# file to store the server details
SERVER_INFO=/home/gjb/Documents/worker-ng/scripts/worker_${PBS_JOBID}/server_info.txt

# start the server
"/home/gjb/Documents/worker-ng/bin/worker_server" \
    --log "/home/gjb/Documents/worker-ng/scripts/worker_${PBS_JOBID}/server.log" \
    --port 5555 \
    --workfile "/home/gjb/Documents/worker-ng/scripts/worker_${PBS_JOBID}/workerfile.txt" \
    --server_info "/home/gjb/Documents/worker-ng/scripts/worker_${PBS_JOBID}/server_info.txt" &
server_exit=$?
if [ $server_exit -ne 0 ]
then

    (>&2 echo "### error: failed to launch server")
    exit $server_exit
fi

# ensure it is up and running
sleep 5

# determine the server address and UUID for the clients to use
uuid=$(cut -d ' ' -f 1 $SERVER_INFO)
server=$(cut -d ' ' -f 2 $SERVER_INFO)

(>&2 echo "### info: server launhed at '$server' with UUID '$uuid'")

# environment variables will not be available on non-server nodes,
# so they need to be passed to the client explicitly
env_variables=$(compute_env_vars '^VSC_' '^PBS_')

# launch clients
nodes=$(cat $PBS_NODEFILE)
num_clients=$(( ${#nodes[@]} / num_cores ))

# compute numactl arguments
numactl_args=( $( compute_numactl_args $num_cores $nodes ) )
numactl_status=$?

for (( client_num=0; client_num < num_clients; client_num++ ))
do
    client_nodes="${nodes[@_]:$(( client_num*num_cores )):$num_cores}"
    host_info=$( compute_host_file $num_cores $client_nodes )
    numactl_opt=''
    if [ $numactl_status ]
    then
        numactl_opt="--numactl ${numactl_args[$client_num]}"
    fi
    ssh ${client_nodes[0]} "/home/gjb/Documents/worker-ng/bin/worker_client" \
        --server "$server" --uuid "$uuid" \
        $env_variables $numactl_opt --host_info "$host_info" &
    client_exit=$?
    if [ $client_exit -eq 0 ]
    then
        (>&2 echo "### info: client launhed on '$client_node'")
    else
        (>&2 echo "### error: client failed to launhed on '$client_node', exit status $client_exit")
        if [ false ]
        then
            exit $client_exit
        fi
    fi
done

# wait for the server to finish
wait

