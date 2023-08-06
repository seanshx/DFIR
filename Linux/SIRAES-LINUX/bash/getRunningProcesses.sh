#!/bin/bash

# SIREAS - SHOVAL INCIDENT RESPONSE ENPOINT ARTIFACTS SCANNER
# Developed by Sean Shohat (Github: @seanshx) 2023
# PLATFORM: Linux | VERSION: alpha 1.0
# SIREAS MODULE: SIREAS_MODULE_LINUX_GETRUNNINGPROCESSES

OUTPUT_FILE="SIREAS_MODULE_LINUX_GETRUNNINGPROCESSES.csv"

# CSV headers
echo "SHA256,RunningTime,RunningUser,PID,PPID,CreationTime,ProcessPath,CommandLine,Status,VirtualMemorySize,ResidentSetSize,MemoryPercentage,OpenFiles,ListeningPorts,CurrentWorkingDir,EnvironmentVars" > $OUTPUT_FILE

# Go through each process
ps -eo user,pid,ppid,lstart | tail -n +2 | while read user pid ppid lstart; do
    # Convert the lstart time to UTC
    creation_time_utc=$(date --date="$lstart" --utc '+%Y-%m-%d %H:%M:%S')

    # Running time
    running_time=$(ps -o etimes= -p $pid | tr -d ' ')

    # SHA-256 and process path
    if [[ -e /proc/$pid/exe ]]; then
        sha256=$(sha256sum /proc/$pid/exe | cut -d ' ' -f 1)
        process_path=$(readlink -f /proc/$pid/exe)
    else
        sha256="N/A"
        process_path="N/A"
    fi

    # Command line
    cmdline="N/A"
    [[ -e /proc/$pid/cmdline ]] && cmdline=$(cat /proc/$pid/cmdline | tr '\0' ' ')

    # Status, VirtualMemorySize, ResidentSetSize from /proc/$pid/status
    status=$(grep "State:" /proc/$pid/status | awk '{print $2}')
    vmsize=$(grep "VmSize:" /proc/$pid/status | awk '{print $2}') # in kB
    rss=$(grep "VmRSS:" /proc/$pid/status | awk '{print $2}')     # in kB
    mem_perc=$(awk -v rss=$rss 'BEGIN { printf "%.2f", (rss * 1024 * 100) / (1024*1024*1024) }') # Assuming total RAM is 1GB. Adjust if needed.

    # Open files count
    open_files_count=$(ls /proc/$pid/fd | wc -l 2>/dev/null || echo "N/A")

    # Listening ports
    listening_ports=$(netstat -ltnp 2>/dev/null | grep $pid | awk '{print $4}' | cut -d: -f2 | tr '\n' ',' | sed 's/,$//')

    # Current working directory
    cwd="N/A"
    [[ -e /proc/$pid/cwd ]] && cwd=$(readlink -f /proc/$pid/cwd)

    # Environment variables (NOTE: this might contain sensitive information)
    env_vars="N/A"
    [[ -e /proc/$pid/environ ]] && env_vars=$(cat /proc/$pid/environ | tr '\0' ';' | sed 's/;$/ /')

    # Append to the CSV
    echo "$sha256,$running_time,$user,$pid,$ppid,$creation_time_utc,$process_path,\"$cmdline\",$status,$vmsize,$rss,$mem_perc,$open_files_count,$listening_ports,$cwd,\"$env_vars\"" >> $OUTPUT_FILE
done
