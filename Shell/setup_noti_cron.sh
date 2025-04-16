#!/bin/bash

install_dependencies() {
	if [[ -f /etc/os-release ]]; then
		. /etc/os-release
		case "$ID" in
			arch)
				echo "Detected Arch Linux. Installing dependencies..."
				sudo pacman -S --noconfirm libnotify cronie
				sudo systemctl enable --now cronie ;;
			debian|ubuntu)
				echo "Detected Ubuntu/Debian. Installing dependencies..."
				sudo apt update && sudo apt install -y libnotify-bin cron
				sudo systemctl enable --now cron ;;
			fedora)
				echo "Detected Fedora. Installing dependencies..."
				sudo dnf install -y libnotify cronie
				sudo systemctl enable --now crond ;;
			*)
				echo "Unsupported OS: $ID"
				exit 1 ;;
		esac
	else 
		echo "Cannot detect OS. Exiting."
		exit 1
	fi
}

check_dependencies() {
	if ! command -v notify-send &>/dev/null || ! command -v crontab &>/dev/null; then
		install_dependencies
	else 
		echo "All dependencies are already installed"
	fi
}

generate_notify_script() {
	local title="$1"
	local message="$2"
	local script_dir="$(cd "$(dirname "$0")" && pwd)"
	local timestamp=$(date +%s)
	local notify_script="$script_dir/notify_${timestamp}.sh"

	echo "#!/bin/bash" > "$notify_script"
	echo "DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/\$(id -u)/bus notify-send \"$title\" \"$message\"" >> "$notify_script"
	chmod +x "$notify_script"

	echo "$notify_script"
}

add_cron_job() {
	read -rp "Enter notification title: " title
	read -rp "Enter notification message: " message

	local notify_script
	notify_script=$(generate_notify_script "$title" "$message")

	echo ""
	echo "Choose notification interval:"
	echo "1) Every minute"
	echo "2) Every hour"
	echo "3) Every day at specific time"
	echo "4) Custom cron expression"
	read -rp "Enter your choice [1-4]: " choice

	case $choice in
		1) CRON_EXPR="* * * * *" ;;
		2) CRON_EXPR="0 * * * *" ;;
		3)
			read -rp "Enter hour (0-23): " hour
			read -rp "Enter minute (0-59): " minute
			CRON_EXPR="$minute $hour * * *"
			;;
		4)
			read -rp "Enter full cron expression (e.g., */5 * * * *): " CRON_EXPR
			;;
		*)
			echo "Invalid choice."
			return
			;;
	esac

	(crontab -l 2>/dev/null; echo "$CRON_EXPR $notify_script") | crontab -
	echo "Job scheduled: [$title] at $CRON_EXPR"
}

list_cron_jobs() {
	echo ""
	echo "Current crontab entries:"
	crontab -l 2>/dev/null || echo "No cron jobs found."
}

remove_cron_job() {
	crontab -l 2>/dev/null | grep -v '^\s*$' > /tmp/current_cron || { echo "No cron jobs found."; return; }

	mapfile -t jobs < /tmp/current_cron

	if [ ${#jobs[@]} -eq 0 ]; then
		echo "No cron jobs to remove."
		return
	fi

	echo ""
	echo "Select a job to remove:"
	for i in "${!jobs[@]}"; do
		printf "%d) %s\n" $((i+1)) "${jobs[$i]}"
	done

	read -rp "Enter job number to delete: " index
	if [[ $index =~ ^[0-9]+$ ]] && (( index >= 1 && index <= ${#jobs[@]} )); then
		script_path=$(echo "${jobs[index-1]}" | awk '{print $6}')
		if [[ -f "$script_path" ]]; then
			rm -f "$script_path"
			echo "Deleted script: $script_path"
		fi
		unset 'jobs[index-1]'
		printf "%s\n" "${jobs[@]}" | grep -v '^\s*$' | crontab -
		echo "Job removed successfully."
	else
		echo "Invalid selection."
	fi
}


main_menu() {
	check_dependencies

	while true; do
		echo ""
		echo "============  Cron Job Manager  ============ "
		echo "1) Create notification cron job"
		echo "2) List cron jobs"
		echo "3) Remove a cron job"
		echo "4) Exit"
		read -rp "Choose an option [1-4]: " opt

		case $opt in
			1) add_cron_job ;;
			2) list_cron_jobs ;;
			3) remove_cron_job ;;
			4) echo "Exit"; exit 0 ;;
			*) echo "Invalid option!" ;;
		esac
	done
}

main_menu

