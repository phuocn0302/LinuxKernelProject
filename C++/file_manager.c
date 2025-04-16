#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

void log_history(const char *command) {
    FILE *log = fopen("history.log", "a");
    if (log) {
        fprintf(log, "%s\n", command);
        fclose(log);
    }
}

void get_input(char *buffer, int size, const char *prompt) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        log_history(buffer);
    }
}

void list_files(int show_all) {
    char path[256];
    get_input(path, sizeof(path), "Enter directory path (or .): ");

    DIR *d = opendir(path);
    if (!d) {
        perror("opendir");
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (!show_all && dir->d_name[0] == '.')
            continue;
        printf("%s\n", dir->d_name);
    }
    closedir(d);
}

void show_current_directory() {
    char cwd[512];
    if (getcwd(cwd, sizeof(cwd)))
        printf("Current directory: %s\n", cwd);
    else
        perror("getcwd");
}

void change_directory() {
    char path[256];
    get_input(path, sizeof(path), "Enter path to change directory: ");

    if (chdir(path) == 0)
        printf("Changed directory to %s\n", path);
    else
        perror("chdir");
}

void make_directory() {
    char name[256];
    get_input(name, sizeof(name), "Enter directory name: ");

    if (mkdir(name, 0755) == 0)
        printf("Directory created.\n");
    else
        perror("mkdir");
}

void remove_directory() {
    char name[256];
    get_input(name, sizeof(name), "Enter directory name to remove: ");

    if (rmdir(name) == 0)
        printf("Directory removed.\n");
    else
        perror("rmdir");
}

void copy_recursive(const char *src, const char *dest) {
    struct stat st;
    if (stat(src, &st) == -1) {
        perror("stat");
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        mkdir(dest, 0755);
        DIR *d = opendir(src);
        if (!d) return;

        struct dirent *entry;
        while ((entry = readdir(d)) != NULL) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            char src_path[512], dest_path[512];
            snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
            snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);
            copy_recursive(src_path, dest_path);
        }
        closedir(d);
    } else {
        FILE *fs = fopen(src, "rb");
        FILE *fd = fopen(dest, "wb");
        if (!fs || !fd) return;

        char buf[1024];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fs)) > 0)
            fwrite(buf, 1, n, fd);

        fclose(fs);
        fclose(fd);
    }
}

void copy_file() {
    char src[256], dest[256];
    get_input(src, sizeof(src), "Enter source path: ");
    get_input(dest, sizeof(dest), "Enter destination path: ");

    copy_recursive(src, dest);
    printf("Copied from %s to %s\n", src, dest);
}

void move_file() {
    char oldname[256], newname[256];
    get_input(oldname, sizeof(oldname), "Enter current name: ");
    get_input(newname, sizeof(newname), "Enter new name/path: ");

    if (rename(oldname, newname) == 0)
        printf("Moved/Renamed successfully.\n");
    else
        perror("rename");
}

void delete_recursive(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(path);
        if (!d) return;

        struct dirent *entry;
        while ((entry = readdir(d)) != NULL) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
            delete_recursive(fullpath);
        }
        closedir(d);
        rmdir(path);
    } else {
        remove(path);
    }
}

void remove_file() {
    char name[256];
    get_input(name, sizeof(name), "Enter file or directory to remove: ");

    delete_recursive(name);
    printf("Removed: %s\n", name);
}

void create_file() {
    char name[256];
    get_input(name, sizeof(name), "Enter file name to create: ");

    FILE *fp = fopen(name, "w");
    if (!fp)
        perror("fopen");
    else {
        printf("File created.\n");
        fclose(fp);
    }
}

void find_file() {
    char name[256];
    get_input(name, sizeof(name), "Enter name to search: ");

    char command[300];
    snprintf(command, sizeof(command), "find . -name \"%s\"", name);
    system(command);
}

int main() {
    int choice;
    do {
        printf("\n--- Linux File Manager ---\n");
        printf("1.  List files (ls)\n");
        printf("2.  List all files (ls -a)\n");
        printf("3.  Show current directory (pwd)\n");
        printf("4.  Change directory (cd)\n");
        printf("5.  Make directory (mkdir)\n");
        printf("6.  Remove empty directory (rmdir)\n");
        printf("7.  Copy file/directory (cp -r)\n");
        printf("8.  Move/Rename file/directory (mv)\n");
        printf("9.  Remove file/directory (rm -r)\n");
        printf("10. Create file (touch)\n");
        printf("11. Find file (find)\n");
        printf("12. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        getchar(); // clear newline

        switch (choice) {
            case 1: list_files(0); break;
            case 2: list_files(1); break;
            case 3: show_current_directory(); break;
            case 4: change_directory(); break;
            case 5: make_directory(); break;
            case 6: remove_directory(); break;
            case 7: copy_file(); break;
            case 8: move_file(); break;
            case 9: remove_file(); break;
            case 10: create_file(); break;
            case 11: find_file(); break;
            case 12: printf("Exiting...\n"); break;
            default: printf("Invalid option.\n");
        }

    } while (choice != 12);

    return 0;
}

