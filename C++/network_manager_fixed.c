#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_hint(const char *msg) {
    printf("\033[0;36mHint:\033[0m %s\n", msg);
}

void list_network_interfaces() {
    print_hint("Shows all available network interfaces.");
    printf("Listing network interfaces...\n");
    system("nmcli device status");
}

void enable_interface() {
    print_hint("Enable a specific network interface.");
    char interface[64];
    printf("Enter network interface to enable (e.g., wlan0): ");
    fgets(interface, sizeof(interface), stdin);
    interface[strcspn(interface, "\n")] = 0;
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "nmcli device set %s managed yes", interface);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "nmcli device connect %s", interface);
    system(cmd);
}

void disable_interface() {
    print_hint("Disable a specific network interface.");
    char interface[64];
    printf("Enter network interface to disable (e.g., wlan0): ");
    fgets(interface, sizeof(interface), stdin);
    interface[strcspn(interface, "\n")] = 0;
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "nmcli device disconnect %s", interface);
    system(cmd);
}

void connect_to_wifi() {
    print_hint("Connect to a Wi-Fi network using SSID and password.");
    char ssid[64], password[64], security_type[64];
    
    printf("Enter Wi-Fi SSID: ");
    fgets(ssid, sizeof(ssid), stdin);
    ssid[strcspn(ssid, "\n")] = 0;

    printf("Enter Wi-Fi security type (e.g., WPA2, WPA3, or leave empty for open network): ");
    fgets(security_type, sizeof(security_type), stdin);
    security_type[strcspn(security_type, "\n")] = 0;

    printf("Enter Wi-Fi password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    // Construct the command
    char cmd[512];

    if (strlen(security_type) == 0) {
        // Open network (no password)
        snprintf(cmd, sizeof(cmd), "nmcli device wifi connect %s", ssid);
    } else {
        // Secure network (WPA2, WPA3, etc.)
        snprintf(cmd, sizeof(cmd), "nmcli device wifi connect %s password %s 802-11-wireless-security.key-mgmt %s", ssid, password, security_type);
    }

    // Execute the command to connect
    system(cmd);
}

void disconnect_from_network() {
    print_hint("Disconnect from the current network.");
    system("nmcli connection down id $(nmcli -t -f NAME c show --active)");
}

void show_network_configuration() {
    print_hint("Display current network configuration (IP, gateway, etc.).");
    system("ip addr show");
    system("ip route show");
}

void list_available_wifi() {
    print_hint("All available wifi: ");
    system("nmcli device wifi list");
}

void set_static_ip() {
    print_hint("Set static IP address for the network interface.");
    
    char interface[64], ip_address[64], gateway[64], dns[64];
    printf("Enter network interface (e.g., eth0, wlan0): ");
    fgets(interface, sizeof(interface), stdin);
    interface[strcspn(interface, "\n")] = 0;

    printf("Enter static IP address (e.g., 192.168.1.100): ");
    fgets(ip_address, sizeof(ip_address), stdin);
    ip_address[strcspn(ip_address, "\n")] = 0;

    printf("Enter gateway IP (e.g., 192.168.1.1): ");
    fgets(gateway, sizeof(gateway), stdin);
    gateway[strcspn(gateway, "\n")] = 0;

    printf("Enter DNS IP (e.g., 8.8.8.8): ");
    fgets(dns, sizeof(dns), stdin);
    dns[strcspn(dns, "\n")] = 0;

    char cmd[512];
    // Set the static IP address
    snprintf(cmd, sizeof(cmd), "nmcli con mod %s ipv4.addresses %s/24", interface, ip_address);
    system(cmd);
    
    // Set the gateway
    snprintf(cmd, sizeof(cmd), "nmcli con mod %s ipv4.gateway %s", interface, gateway);
    system(cmd);
    
    // Set the DNS server
    snprintf(cmd, sizeof(cmd), "nmcli con mod %s ipv4.dns %s", interface, dns);
    system(cmd);

    // Ensure IPv4 is set to manual (not automatic via DHCP)
    snprintf(cmd, sizeof(cmd), "nmcli con mod %s ipv4.method manual", interface);
    system(cmd);

    // Bring the interface down and up to apply changes
    snprintf(cmd, sizeof(cmd), "nmcli con down %s && nmcli con up %s", interface, interface);
    system(cmd);

    printf("Static IP configuration applied successfully.\n");
}

int main() {
    int choice;
    do {
        printf("\n--- Network Manager ---\n");
        printf("1. List available network interfaces\n");
        printf("2. Enable a network interface\n");
        printf("3. Disable a network interface\n");
        printf("4. Connect to a Wi-Fi network\n");
        printf("5. Disconnect from network\n");
        printf("6. Show network configuration\n");
        printf("7. List available WIFI\n");
        printf("8. Set static IP address\n");
        printf("9. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        getchar();  // Clear newline from buffer

        switch (choice) {
            case 1: list_network_interfaces(); break;
            case 2: enable_interface(); break;
            case 3: disable_interface(); break;
            case 4: connect_to_wifi(); break;
            case 5: disconnect_from_network(); break;
            case 6: show_network_configuration(); break;
            case 7: list_available_wifi(); break;
            case 8: set_static_ip(); break;
            case 9: printf("Exiting...\n"); break;
            default: printf("Invalid option.\n");
        }

    } while (choice != 9);

    return 0;
}



void connect_to_new_wifi() {
    print_hint("Connect to a new WiFi network using SSID and password.");
    char ssid[128], password[128], interface[64], name[128], cmd[512];

    printf("Enter WiFi SSID: ");
    fgets(ssid, sizeof(ssid), stdin);
    ssid[strcspn(ssid, "\n")] = 0;

    printf("Enter WiFi password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    printf("Enter network interface (e.g., wlan0): ");
    fgets(interface, sizeof(interface), stdin);
    interface[strcspn(interface, "\n")] = 0;

    printf("Enter connection name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    snprintf(cmd, sizeof(cmd), "nmcli connection add type wifi ifname %s con-name '%s' ssid '%s'", interface, name, ssid);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "nmcli connection modify '%s' wifi-sec.key-mgmt wpa-psk", name);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "nmcli connection modify '%s' wifi-sec.psk '%s'", name, password);
    system(cmd);

    snprintf(cmd, sizeof(cmd), "nmcli connection up '%s'", name);
    system(cmd);
}
