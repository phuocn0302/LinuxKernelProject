#!/bin/bash

show_pwd() { pwd; }

list_files() { ls -l; }

change_directory() { read -p "Enter directory path: " dir; cd "$dir" || echo "Invalid directory"; }

create_file() { read -p "Enter file name: " file; touch "$file"; echo "File $file created successfully"; }

delete_file() { read -p "Enter file name: " file; rm -i "$file"; echo "File deleted"; }

create_directory() {
	read -p "Enter directory name: " dir; 
	mkdir "$dir";
	echo "Directory created successfully";
}

remove_directory() {
	read -p "Enter directory name: " dir;
	rmdir "$dir" && echo "Directory removed" || echo "Failed to remove directory (might not be empty)";
}

show_file_content() {
	read -p "Enter file name: " file;
	cat "$file";
}

copy_file() {
	read -p "Enter source file: " src; 
	read -p "Enter destination: " dest;
	cp "$src" "$dest";
	echo "File copied";
}

move_file() {
	read -p "Enter source file: " src; 
	read -p "Enter destination: " dest;
	mv "$src" "$dest";
	echo "File moved/renamed";
}

search_file() {
	read -p "Enter file name to search: " file; 
	find / -name "$file" 2>/dev/null;
}

show_disk_usage() {
	df -h;
}


while true; do
	clear
	echo "==================================="
	echo "        LINUX FILE MANAGER         "
	echo "==================================="
	echo "1. Show current directory"
	echo "2. List files"
	echo "3. Change directory"
	echo "4. Create a file"
	echo "5. Delete a file"
	echo "6. Create a directory"
	echo "7. Remove a directory"
	echo "8. Show file content"
	echo "9. Copy file"
	echo "10. Move/Rename file"
	echo "11. Search for a file"
	echo "12. Show disk usage"
	echo "13. Exit"
	echo "===================================="

	read -p "Choose an option (1-13): " choice

	case $choice in
		1) show_pwd ;;
		2) list_files ;;
		3) change_directory ;;
		4) create_file ;;
		5) delete_file ;;
		6) create_directory ;;
		7) remove_directory ;;
		8) show_file_content ;;
		9) copy_file ;;
		10) move_file ;;
		11) search_file ;;
		12) show_disk_usage ;;
		13) echo "Exiting..."; exit 0 ;;
		*) echo "Invalid option, please try again." ;;
	esac

	read -p "Press Enter to continue..."

done



