#!/usr/bin/env bash
#SBATCH --account=lpt2_sysadmin
#SBATCH --time=00:20:00
#SBATCH --nodes=1 --ntasks=1 --cpus-per-task=1
#SBATCH hetjob
#SBATCH --nodes=1
#SBATCH --ntasks=10
#SBATCH --cpus-per-task=1

#
# variables that should be subsituted
#   * shebang, shebang for this script (#!/usr/bin/env bash)
#   * scheduler_directives
#   * worker_path (/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/distr_genius/bin/..)
#   * server_info (/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/server_info.txt)
#   * server_log_opt, e.g., --log path_to_log_file (--log "/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/server.log")
#   * port_opt, e.g., --port 1234 (--port 5555)
#   * server_start_delay, 5 (in seconds) (5)
#   * workfile (/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/workerfile.txt)
#   * client_log_prefix_opt, e.g., --log_prefix client_log_prefix (--log_prefix "/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/client_")
#   * env_var_exprs, e.g., '^VSC' '^PBS_' ('^VSC_' '^SLURM_')
#   * num_cores (1)
#   * exit_on_client_fail, i.e., true or false (false)
#

# source worker software environment variables (LD_LIBRARY_PATH et al.)
env_conf_file="/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/distr_genius/bin/../conf/worker_env_${SLURM_CLUSTER_NAME}.sh"
if [ ! -e "${env_conf_file}" ]
then
    (>&2 echo "### error: can not read environment configuration file '${env_conf_file}'")
    exit 1
fi
source "${env_conf_file}"

# source worker functions used in this script
worker_functions_file="/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/distr_genius/bin/../scripts/worker_functions.sh"
if [ ! -e "${worker_functions_file}" ]
then
    (>&2 echo "### error: can not read worker functions file '${worker_functions_file}'")
    exit 1
fi
source "${worker_functions_file}"

# file to store the server details
SERVER_INFO=/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/server_info.txt

# add worker library path to LD_LIBRARY_PATH
LD_LIBRARY_PATH="/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/distr_genius/bin/../lib:${LD_LIBRARY_PATH}"

# check worker server and client
worker_server_exec="/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/distr_genius/bin/../bin/worker_server"
if [ ! -e "${worker_server_exec}" ]
then
    (>&2 echo "### error: can not find worker server executable '${worker_server_exec}'")
    exit 1
fi
worker_client_exec="/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/distr_genius/bin/../bin/worker_client"
if [ ! -e "${worker_client_exec}" ]
then
    (>&2 echo "### error: can not find worker client executable '${worker_client_exec}'")
    exit 1
fi

# start the server
srun --exclusive --nodes=1 --ntasks=1 --cpus-per-task=1 \
    "${worker_server_exec}" \
        --log "/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/server.log" \
        --port 5555 \
        --workfile "/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/workerfile.txt" \
        --server_info "$SERVER_INFO" &
server_exit=$?
if [ $server_exit -ne 0 ]
then

    (>&2 echo "### error: failed to launch server")
    exit $server_exit
fi

# ensure it is up and running and check whether it is actually up
sleep 5
if [ ! -f "$SERVER_INFO" ]
then
    (>&2 echo "### error: failed to launch server, no server info file created")
    exit 1
fi

# determine the server address and UUID for the clients to use
uuid=$(cut -d ' ' -f 1 "$SERVER_INFO")
server=$(cut -d ' ' -f 2 "$SERVER_INFO")

(>&2 echo "### info: server launched at '$server' with UUID '$uuid'")

export OMP_PROC_BIND=true
export OMP_PLACES=cores

for (( client_id=1; client_id <= $SLURM_NTASKS_HET_GROUP_1; client_id++ ))
do
    srun --exclusive --het-group=1 \
        --ntasks=1 --nodes=1 --cpus-per-task=$SLURM_CPUS_PER_TASK_HET_GROUP_1 \
        --threads-per-core=1 \
            "${worker_client_exec}" \
                --server "$server" --uuid "$uuid" --log_prefix "/vsc-hard-mounts/leuven-data/301/vsc30140/Projects/worker-ng/examples/failures/worker_${SLURM_JOB_ID}/client_" \
                $numactl_opt --host_info "$host_info" &
    client_exit=$?
    if [ $client_exit -eq 0 ]
    then
        (>&2 echo "### info: client $client_id launched")
    else
        (>&2 echo "### error: client $client_id failed to launch, exit status $client_exit")
        if [ false ]
        then
            exit $client_exit
        fi
    fi
done

# wait for the server to finish
wait

