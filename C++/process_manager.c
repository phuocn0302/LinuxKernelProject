#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>

void print_hint(const char *msg) {
    printf("\033[0;36mHint:\033[0m %s\n", msg);
}

int is_zombie(const char *pid) {
    char path[256], state[16];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    while (fgets(state, sizeof(state), f)) {
        if (strncmp(state, "State:", 6) == 0) {
            if (strchr(state, 'Z')) {
                fclose(f);
                return 1;
            }
            break;
        }
    }
    fclose(f);
    return 0;
}

int is_orphan(const char *pid) {
    char path[256], line[256];
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "PPid:", 5) == 0) {
            int ppid;
            sscanf(line + 5, "%d", &ppid);
            if (ppid == 1) {
                fclose(f);
                return 1;
            }
            break;
        }
    }
    fclose(f);
    return 0;
}

void list_processes_with_filter() {
    char filter_name[64] = "", status_filter = '\0';
    int uid_filter = -1;
    char input[64];

    printf("Filter by name (leave empty for all): ");
    fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) strcpy(filter_name, input);

    printf("Filter by UID (leave empty for all): ");
    fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) uid_filter = atoi(input);

    printf("Filter by status (R: running, S: sleeping, Z: zombie, etc.; leave empty for all): ");
    fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) status_filter = input[0];

    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("opendir /proc");
        return;
    }

    struct dirent *entry;
    printf("%-10s %-20s %-6s %-10s\n", "PID", "COMMAND", "STATE", "UID");
    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;

        char path[256], name[256], line[256];
        int match_name = 1, match_uid = 1, match_status = 1;

        snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;

        char comm[64] = "", state = '\0';
        int uid = -1;
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "Name:", 5) == 0)
                sscanf(line + 5, "%s", comm);
            else if (strncmp(line, "State:", 6) == 0)
                sscanf(line, "State:\t%c", &state);
            else if (strncmp(line, "Uid:", 4) == 0)
                sscanf(line, "Uid:\t%d", &uid);
        }
        fclose(f);

        if (strlen(filter_name) > 0 && strstr(comm, filter_name) == NULL) match_name = 0;
        if (uid_filter >= 0 && uid != uid_filter) match_uid = 0;
        if (status_filter && state != status_filter) match_status = 0;

        if (match_name && match_uid && match_status) {
            printf("%-10s %-20s %-6c %-10d\n", entry->d_name, comm, state, uid);
        }
    }

    closedir(proc);
}

void start_process() {
    print_hint("Example: `ls -l` or `gedit file.txt`");
    char input[256];
    printf("Enter command to run: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    char *args[20];
    int i = 0;
    args[i] = strtok(input, " ");
    while (args[i] && i < 19) {
        i++;
        args[i] = strtok(NULL, " ");
    }

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
	int status;
	waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Process %d finished with status %d.\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process %d was killed by signal %d.\n", pid, WTERMSIG(status));
        }

    } else {
        perror("fork");
    }
}

void kill_process() {
    print_hint("Enter a valid PID to terminate.");
    int pid;
    printf("Enter PID to kill: ");
    scanf("%d", &pid); getchar();
    if (kill(pid, SIGKILL) == 0)
        printf("Process %d killed.\n", pid);
    else
        perror("kill");
}

void send_signal() {
    print_hint("Use signal numbers like 9 (SIGKILL), 15 (SIGTERM), 19 (SIGSTOP)");
    int pid, sig;
    printf("Enter PID to signal: ");
    scanf("%d", &pid);
    printf("Enter signal number: ");
    scanf("%d", &sig); getchar();
    if (kill(pid, sig) == 0)
        printf("Signal %d sent to PID %d.\n", sig, pid);
    else
        perror("kill");
}

void show_process_info() {
    print_hint("Displays name, state, parent PID, UID, and memory.");
    int pid;
    char path[256], line[256];
    printf("Enter PID to inspect: ");
    scanf("%d", &pid); getchar();

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "Name:", 5) == 0 || 
            strncmp(line, "State:", 6) == 0 || 
            strncmp(line, "PPid:", 5) == 0 ||
            strncmp(line, "Uid:", 4) == 0 || 
            strncmp(line, "VmSize:", 7) == 0)
            printf("%s", line);
    }

    fclose(f);
}

void wait_for_process() {
    print_hint("Use this after starting a background process.");
    int pid, status;
    printf("Enter child PID to wait for: ");
    scanf("%d", &pid); getchar();
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
    } else {
        if (WIFEXITED(status))
            printf("Process %d exited with status %d.\n", pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Process %d killed by signal %d.\n", pid, WTERMSIG(status));
        else
            printf("Process %d ended unexpectedly.\n", pid);
    }
}

void find_zombies_or_orphans() {
    DIR *proc = opendir("/proc");
    if (!proc) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    printf("\nZombies and Orphans:\n");
    printf("%-10s %-20s %-8s\n", "PID", "NAME", "TYPE");

    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;
        char path[256], name[64] = "";
        snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
        FILE *f = fopen(path, "r");
        if (f) {
            fgets(name, sizeof(name), f);
            name[strcspn(name, "\n")] = 0;
            fclose(f);
        }

        if (is_zombie(entry->d_name)) {
            printf("%-10s %-20s ZOMBIE\n", entry->d_name, name);
        } else if (is_orphan(entry->d_name)) {
            printf("%-10s %-20s ORPHAN\n", entry->d_name, name);
        }
    }

    closedir(proc);
}

int main() {
    int choice;
    do {
        printf("\n--- Process Manager ---\n");
        printf("1. List processes (with filter)\n");
        printf("2. Start a new process\n");
        printf("3. Kill a process\n");
        printf("4. Send signal to process\n");
        printf("5. Show process info\n");
        printf("6. Wait for a process\n");
        printf("7. Show zombies/orphans\n");
        printf("8. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice); getchar();

        switch (choice) {
            case 1: list_processes_with_filter(); break;
            case 2: start_process(); break;
            case 3: kill_process(); break;
            case 4: send_signal(); break;
            case 5: show_process_info(); break;
            case 6: wait_for_process(); break;
            case 7: find_zombies_or_orphans(); break;
            case 8: printf("Exiting...\n"); break;
            default: printf("Invalid option.\n");
        }

    } while (choice != 8);

    return 0;
}

