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
				sudo systemctl enable --now crond ;;

			fedora)
				echo "Detected Fedora. Installing dependencies..."
				sudo dnf install -y libnotify cronie
				sudo systemctl enable --now crond ;;

			*)
				echo "Unsupported OS: $ID"
				exit 1 ;;
		esac
	else 
		echo "Cannot detect OS. Exitig."
		exit 1
	fi
}

if ! command -v notify-send &>/dev/null || ! command -v crontab &>/dev/null; then
	install_dependencies
else 
	echo "All dependencies are already installed"
fi


SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
NOTIFY_SCRIPT="$SCRIPT_DIR/notify.sh"

echo '#!/bin/bash' > "$NOTIFY_SCRIPT"
echo 'DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u)/bus notify-send "Reminder" "This is a notification every minute."' >> "$NOTIFY_SCRIPT"

chmod +x "$NOTIFY_SCRIPT"

sudo systemctl enable --now cron 2>/dev/null || sudo systemctl enable --now cronie 2>/dev/null || sudo systemctl enable --now crond 2>/dev/null

(crontab -l 2>/dev/null | grep -v "$NOTIFY_SCRIPT"; echo "* * * * * $NOTIFY_SCRIPT") | crontab -

echo "Notification script set up successfully! It will run every minute."
