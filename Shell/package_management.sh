#!/bin/bash

# Detect OS and set package manager
detect_os() {
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        case "$ID" in
            ubuntu|debian)
                PKG_MANAGER="apt"
                INSTALL_CMD="apt install -y"
                REMOVE_CMD="apt remove -y"
                UPDATE_CMD="apt update"
                ;;
            arch)
                PKG_MANAGER="pacman"
                INSTALL_CMD="pacman -S --noconfirm"
                REMOVE_CMD="pacman -R --noconfirm"
                UPDATE_CMD="pacman -Sy"
                ;;
            fedora)
                PKG_MANAGER="dnf"
                INSTALL_CMD="dnf install -y"
                REMOVE_CMD="dnf remove -y"
                UPDATE_CMD="dnf check-update"
                ;;
            *)
                echo "Unsupported OS: $ID"
                exit 1
                ;;
        esac
    else
        echo "Could not detect the OS."
        exit 1
    fi
}

# Install a package
install_package() {
    read -rp "Enter package name to install: " package
    echo "Updating package list..."
    sudo $UPDATE_CMD
    echo "Installing $package..."
    sudo $INSTALL_CMD "$package"
}

uninstall_package() {
    read -rp "Enter package name to uninstall: " package
    echo "Removing $package..."
    sudo $REMOVE_CMD "$package"
}

main_menu() {
    while true; do
        clear
        echo "====================================="
        echo "      Automatic Package Manager"
        echo "====================================="
        echo "Detected OS: $ID ($PKG_MANAGER)"
        echo "1) Install a package"
        echo "2) Uninstall a package"
        echo "3) Exit"
        echo "====================================="
        read -rp "Choose an option: " choice

        case $choice in
            1) install_package ;;
            2) uninstall_package ;;
            3) echo "Exiting..."; exit 0 ;;
            *) echo "Invalid choice. Try again." ;;
        esac

        echo -e "\nPress Enter to continue..."
        read -r
    done
}

detect_os
main_menu

