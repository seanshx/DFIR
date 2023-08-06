#!/bin/bash

# SIREAS - SHOVAL INCIDENT RESPONSE ENPOINT ARTIFACTS SCANNER
# Developed by Sean Shohat (Github: @seanshx) 2023
# PLATFORM: Linux | VERSION: alpha 1.0
# SIREAS MODULE: SIREAS_MODULE_LINUX_GETSOCKETS

OUTPUT_FILE="SIREAS_MODULE_LINUX_GETSOCKETS.csv"

# CSV headers
echo "TimestampUTC,Protocol,LocalAddress,RemoteAddress,Status,ProcessName,ProcessPID,ProcessPPID,SHA256,RunningUser" > $OUTPUT_FILE

# Gather TCP and UDP socket information
for protocol in tcp udp; do
    ss -tuln | grep $protocol | while read proto state local remote rest; do
        timestamp=$(date --utc '+%Y-%m-%d %H:%M:%S')
        
        # Try to get process information
        pid=$(ss -tulnp | grep "$local" | sed -E 's/.*pid=([0-9]+),.*/\1/' | uniq)
        if [[ ! -z "$pid" && -d "/proc/$pid" ]]; then
            exe=$(readlink -f /proc/$pid/exe)
            ppid=$(cat /proc/$pid/status | grep PPid | cut -f2)
            sha256=$(sha256sum $exe 2>/dev/null | cut -d ' ' -f 1)
            user=$(ls -lhd /proc/$pid | awk '{print $3}')
        else
            exe="N/A"
            ppid="N/A"
            sha256="N/A"
            user="N/A"
        fi

        echo "$timestamp,$protocol,$local,$remote,$state,$exe,$pid,$ppid,$sha256,$user" >> $OUTPUT_FILE
    done
done