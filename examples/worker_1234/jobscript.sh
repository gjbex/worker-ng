#!/usr/bin/env bash
#SBATCH --accout lp_worker_test
#SBATCH --time=3:00:00
#SBATCH --nodes=1 --ntasks=1 --cpus-per-task=1
#SBATCH hetjob
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=3gb

#
# variables that should be subsituted
#   * shebang, shebang for this script (#!/usr/bin/env bash)
#   * scheduler_directives
#   * worker_path (/home/gjb/Projects/worker-ng)
#   * server_info (/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/server_info.txt)
#   * server_log_opt, e.g., --log path_to_log_file (--log "/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/server.log")
#   * port_opt, e.g., --port 1234 (--port 5555)
#   * server_start_delay, 5 (in seconds) (5)
#   * workfile (/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/workerfile.txt)
#   * client_log_prefix_opt, e.g., --log_prefix client_log_prefix (--log_prefix "/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/client_")
#   * env_var_exprs, e.g., '^VSC' '^PBS_' ('^VSC_' '^SLURM_')
#   * num_cores (1)
#   * exit_on_client_fail, i.e., true or false (false)
#

# source worker software environment variables (LD_LIBRARY_PATH et al.)
source "/home/gjb/Projects/worker-ng/conf/worker_env.sh"

# source worker functions used in this script
source "/home/gjb/Projects/worker-ng/scripts/worker_functions.sh"

# file to store the server details
SERVER_INFO=/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/server_info.txt

# start the server
srun --exclusive --nodes=1 --ntasks=1 --cpus-per-task=1 \
    "/home/gjb/Projects/worker-ng/bin/worker_server" \
        --log "/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/server.log" \
        --port 5555 \
        --workfile "/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/workerfile.txt" \
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
            "/home/gjb/Projects/worker-ng/bin/worker_client" \
                --server "$server" --uuid "$uuid" --log_prefix "/home/gjb/Projects/worker-ng/worker_${SLURM_JOB_ID}/client_" \
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

