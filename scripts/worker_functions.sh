function compute_numactl_args {
    # --------------------------------------------------------------------------
    # compute the numactl arguments for each client
    #
    # arguments
    #   * the number of cores per client
    #   * the nodes
    # 
    # return value
    #   space separated list of numaclt arguments, one per client
    #
    # exit status
    #   * 1: a client would run on multiple nodes, so numactl makes no sense
    #   * 2: not all cores in the node file have been used
    # --------------------------------------------------------------------------
    local num_cores=$1
    shift
    local nodes=( $@ )

    # array to hold the argument for numactl for each client
    local numactl_args=$()

    # local variables
    local numa_arg=''
    local offset=0
    local node=''

    for ((node_num=0; node_num < ${#nodes[@]}; node_num++))
    do
        # when starting a new client, add the arguments for the previous to
        # the array numactl_args
        if [ $(($node_num % $num_cores)) -eq 0 ]
        then
            if [ -n $numa_arg ]
            then
                numactl_args+=($numa_arg)
            fi
            # when starting a client on a new node, reset the core offset
            if [ "$node" != ${nodes[$node_num]} ]
            then
                offset=0
            fi
            node=${nodes[$node_num]}
            numa_arg=''
        fi
        # check whether all cores are on the same ndoe for a client, if
        # not, return with a non-zero exit status
        if [ ${nodes[$node_num]} != $node ]
        then
            (>&2 echo "### error: work items span nodes" )
            return 1
        fi
        if [ -z $numa_arg ]
        then
            numa_arg="+$offset"
        else
            numa_arg="$numa_arg,$offset"
        fi
        offset=$(($offset + 1))
    done
    if [ -n $numa_arg ]
    then
        # warn if not all cores were used
        if [ $(( $offset % $num_cores )) -eq 0 ]
        then
            numactl_args+=($numa_arg)
            echo ${numactl_args[@]}
            return 0
        else
            (>&2 echo "### warning: some cores are unused" )
            echo ${numactl_args[@]}
            return 2
        fi
    fi
}

function compute_host_file {
    # --------------------------------------------------------------------------
    # compute the hostfile entries for a single client
    #
    # arguments
    #   * number of cores per client
    #   * nodes for a single client
    # 
    # return value
    #   
    #
    # exit status
    #   * 1: not enought cores to run the client
    # --------------------------------------------------------------------------
    local num_cores=$1
    shift
    if (( "$#" < "$num_cores" ))
    then
        (>&2 echo "### error: not enough cores to run the client, $num_cores versus $#" )
        return 1
    fi
    declare -A node_map
    for node in $@
    do
        if [ ${node_map[$node]+_} ]
        then
            node_map[$node]=$(( ${node_map[$node]} + 1 ))
        else
            node_map[$node]=1
        fi
    done
    local nodes=''
    for node in ${!node_map[@]}
    do
        if [ -z $nodes ]
        then
            nodes="$node:${node_map[$node]}"
        else
            nodes="$node:${node_map[$node]} $nodes"
        fi
    done
    echo $nodes
}

function compute_env_vars {
    # --------------------------------------------------------------------------
    # compute environment variables to pass to the clients
    #
    # arguments
    #   * any number of grep regular expressions
    # 
    # return value
    #   list of options to pass to the client
    #
    # exit status
    # --------------------------------------------------------------------------
    local env_variables=''
    for expr in $@
    do
        for variable in $(env | grep -e "$expr")
        do
            env_variables="$env_variables --env \"$variable\""
        done
    done
    echo $env_variables
}
