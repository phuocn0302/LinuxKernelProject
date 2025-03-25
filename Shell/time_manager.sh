#!/bin/bash

if [[ $EUID -ne 0 ]]; then
	echo "This script must be run as root. Try: sudo $0"
	exit 1
fi

while true; do
	clear
	echo "================================"
	echo "        TIME MANAGEMENT         "
	echo "================================"
	echo "1) Show current system time"
	echo "2) Show hardware clock time"
	echo "3) Set system time manually"
	echo "4) Sync hardware clock to system time"
	echo "5) Sync system time to hardware clock"
	echo "6) Enable NTP time synchronization"
	echo "7) Exit"
	echo "================================"

	read -rp "Choose an option (1-7): " option

	case $option in
		1) echo "Current system time: $(date)" ;;
		2) echo "Hardware clock time: $(hwclock)" ;;
		3) 
			read -rp "Enter new date and time (YYYY-MM-DD HH:MM:SS): " new_datetime
			if date --set "$new_datetime"; then
				echo "System time updated successfully."
			else 
				echo "Failed to update system time."
			fi
			;;
		4) 
			hwclock --systohc 
			echo "Hardware clock synced to system time."
			;;
		5)
			hwclock --hctosys
			echo "System time synced to hardware clock."
			;;
		6)
			systemctl enable --now systemd-timesyncd
			echo "NTP time synchronization enabled."
			;;
		7)
			echo "Exiting..."
			exit 0
			;;
		*)
			echo "Invalid option! Please choose a valid number."
	esac

	echo -e "\nPress Enter to continue..."
	read -r
done

