#!/bin/bash

# SIREAS - SHOVAL INCIDENT RESPONSE ENPOINT ARTIFACTS SCANNER
# Developed by Sean Shohat (Github: @seanshx) 2023
# PLATFORM: Linux | VERSION: alpha 1.0
# SIREAS MODULE: SIREAS_MODULE_LINUX_COLLECTALLUSERS

OUTPUT_FILE="SIREAS_MODULE_LINUX_COLLECTALLUSERS.csv"

# CSV headers
echo "Username,UserID,GroupID,HomeDirectory,Shell,LastLogin,PasswordStatus,Groups,SSHAuthorizedKeys,LastPasswordChange,PasswordExpiration" > $OUTPUT_FILE

# Loop through each user in /etc/passwd
while read line; do
    user=$(echo "$line" | cut -d: -f1)
    uid=$(echo "$line" | cut -d: -f3)
    gid=$(echo "$line" | cut -d: -f4)
    home=$(echo "$line" | cut -d: -f6)
    shell=$(echo "$line" | cut -d: -f7)

    # Ignore system users (optional)
    if [[ $uid -lt 1000 && "$user" != "root" ]]; then
        continue
    fi

    # Last login
    last_login=$(last -n 1 "$user" | head -n 1 | awk '{if (NF==10) print $4" "$5" "$6" "$7" "$8; else print "Never logged in"}')

    # Password status (e.g., if the password is locked)
    password_status=$(passwd -S "$user" 2>/dev/null | awk '{print $2}')

    # Groups the user belongs to
    groups_list=$(groups "$user" | cut -d: -f2 | sed 's/^ //')

    # SSH authorized keys (if present)
    ssh_keys="None"
    [[ -f "$home/.ssh/authorized_keys" ]] && ssh_keys=$(cat "$home/.ssh/authorized_keys" | tr '\n' ';')

    # Last password change and password expiration
    last_pass_change=$(chage -l "$user" | grep 'Last password change' | cut -d: -f2)
    pass_expiry=$(chage -l "$user" | grep 'Password expires' | cut -d: -f2)

    # Append to the CSV
    echo "$user,$uid,$gid,$home,$shell,\"$last_login\",$password_status,\"$groups_list\",\"$ssh_keys\",\"$last_pass_change\",\"$pass_expiry\"" >> $OUTPUT_FILE
done < /etc/passwd
