/*
 * DiskProvision - Allows the creation, management, and updating of disk images for use with QEMU.
 * DiskProvision.c - The source code of the DiskProvision program itself.
 * BSD 3-Clause "New" or "Revised" License
 * Copyright (c) 2023 RoyalGraphX
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  // For stat() function
#include <sys/statvfs.h>  // For statvfs() function
#include <dirent.h>    // For directory listing
#include <unistd.h> // For sleep function
#include <ctype.h> // Include ctype.h for toupper()

// Function to check if a directory exists
int directoryExists(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0) {
        return 0;  // Directory does not exist
    }
    if (S_ISDIR(info.st_mode)) {
        return 1;  // Directory exists
    }
    return 0;  // Path exists but is not a directory
}

// Function to check if a file exists
int fileExists(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0) {
        return 0;  // File does not exist
    }
    if (S_ISREG(info.st_mode)) {
        return 1;  // File exists
    }
    return 0;  // Path exists but is not a regular file
}

// Function to get available free space on the current directory
unsigned long long getFreeSpace() {
    struct statvfs buf;
    if (statvfs(".", &buf) != 0) {
        return 0;  // Error occurred while getting file system information
    }
    return buf.f_bsize * buf.f_bavail;
}

// Function to check if an executable file exists in PATH
int isExecutableAvailable(const char *executable) {
    char command[512];
    snprintf(command, sizeof(command), "command -v %s > /dev/null", executable);
    return system(command) == 0;
}

// Function to convert a string to uppercase
void stringToUpper(char *str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

int main() {
    // Check if required packages are installed
    if (!isExecutableAvailable("qemu-img") || !isExecutableAvailable("mkfs.fat") || !isExecutableAvailable("modprobe")) {
        printf("Please install the required packages: qemu-utils, dosfstools, and modprobe capability.\n");
        return 1;
    }

    char image_name[256];
    char image_size[256];
    char image_format[10];
    char image_path[512];
    char command[512];
    int choice;

    do {
        // Clear the screen by issuing the "clear" command
        system("clear");

        // Display welcome message
        printf("Welcome to DiskProvision!\n");
        printf("Copyright (c) 2023 RoyalGraphX\n\n");

        // Display menu options
        printf("Menu:\n");
        printf("1. Create New Disk Image\n");
        printf("2. Delete Disk Image\n");
        printf("3. Mount Disk Image\n");
        printf("4. Unmount Disk Image\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);  // Read user's choice

        // Handle user's choice
        switch (choice) {
            case 1:

                system("clear");

               // Create New Disk Image logic

                // Check if the "images" subfolder already exists
                if (!directoryExists("images")) {
                    // Create the "images" subfolder if it doesn't exist
                    if (system("mkdir images") != 0) {
                        printf("Failed to create the 'images' subfolder.\n");
                        break;
                    }
                }

                // Display available free space before creating the image
                unsigned long long free_space_before = getFreeSpace();
                printf("Available free space before creating the image: %.2f GB\n", free_space_before / (1024.0 * 1024.0 * 1024.0));

                // Display menu for image format selection
                printf("Choose the image format:\n");
                printf("1. Raw\n");
                printf("2. QCOW2\n");
                printf("Enter your choice (1 or 2): ");
                int format_choice;
                scanf("%d", &format_choice);

                if (format_choice == 1) {
                    strcpy(image_format, "raw");
                } else if (format_choice == 2) {
                    strcpy(image_format, "qcow2");
                } else {
                    printf("Invalid format choice. Using Raw format by default.\n");
                    strcpy(image_format, "raw");
                }

                // Prompt user for image name
                printf("Enter the name for the disk image (without .img extension): ");
                scanf("%s", image_name);

                // Construct the path to create the disk image in the "images" subfolder
                snprintf(image_path, sizeof(image_path), "images/%s.img", image_name);

                // Check if the image file already exists
                if (fileExists(image_path)) {
                    printf("Disk image already exists! Please choose another name.\n");
                    break;
                }

                // Prompt user for image size in gigabytes
                printf("Enter the size (in GB) for the disk image (e.g., 1): ");
                scanf("%s", image_size);

                // Convert image_size to a double for comparison
                double requested_size = atof(image_size);

                // Get available free space on the current directory
                unsigned long long free_space = getFreeSpace();

                if (free_space == 0) {
                    printf("Failed to get available free space on the current directory.\n");
                    break;
                }

                // Check if the requested image size is greater than available free space
                if (requested_size > (free_space / (1024.0 * 1024.0 * 1024.0))) {
                    printf("Error: Not enough free space to create the disk image. Available space: %.2f GB\n", free_space / (1024.0 * 1024.0 * 1024.0));
                    break;
                }

                // Construct the command to create the disk image
                snprintf(command, sizeof(command), "qemu-img create -f %s %s %sG", image_format, image_path, image_size);

                // Execute the command to create the disk image
                int result = system(command);
                if (result == 0) {
                    printf("Disk image 'images/%s' created successfully.\n", image_name);
                } else {
                    printf("Failed to create disk image.\n");
                    break;
                }

                // Construct the command to format the disk image
                snprintf(command, sizeof(command), "sudo qemu-nbd --connect=/dev/nbd0 -f %s %s", image_format, image_path);

                // Execute the command to connect the image
                result = system(command);
                if (result == 0) {
                    printf("Disk image '%s' connected to /dev/nbd0.\n", image_name);
                } else {
                    printf("Failed to connect disk image to /dev/nbd0.\n");
                    break;
                }

                // Convert image_name to uppercase
                stringToUpper(image_name);

                // Format the disk image using mkfs.fat
                snprintf(command, sizeof(command), "sudo mkfs.fat -F 32 -n \"%s\" -I /dev/nbd0", image_name);

                // Execute the command to format the image
                result = system(command);
                if (result == 0) {
                    printf("Disk image '%s' formatted successfully.\n", image_name);
                } else {
                    printf("Failed to format disk image '%s'.\n", image_name);
                }

                // Disconnect the image from /dev/nbd0
                if (system("sudo qemu-nbd --disconnect /dev/nbd0") == 0) {
                    printf("Disk image disconnected from /dev/nbd0.\n");
                } else {
                    printf("Failed to disconnect disk image from /dev/nbd0.\n");
                }
                sleep(4);
                break;
            case 2:

                system("clear");

                // Delete Disk Image logic

                // Check if the "images" subfolder exists
                if (!directoryExists("images")) {
                    printf("No disk images found in 'images' subfolder. Create some images first.\n");
                    break;
                }

                // List and allow the user to select a disk image for deletion
                printf("Disk images available for deletion:\n");

                int image_count = 0;
                struct dirent *entry;
                DIR *dp = opendir("images");
                if (dp == NULL) {
                    printf("Failed to open 'images' subfolder.\n");
                    break;
                }

                while ((entry = readdir(dp))) {
                    if (entry->d_type == DT_REG &&
                        (strstr(entry->d_name, ".img") || strstr(entry->d_name, ".qcow2"))) {
                        printf("%d. %s\n", ++image_count, entry->d_name);
                    }
                }
                closedir(dp);

                if (image_count == 0) {
                    printf("No disk images found in 'images' subfolder. Create some images first.\n");
                    sleep(4);
                    break;
                }

                int selected_image;
                printf("Enter the number of the image to delete (1-%d): ", image_count);
                scanf("%d", &selected_image);

                if (selected_image < 1 || selected_image > image_count) {
                    printf("Invalid selection.\n");
                    break;
                }

                // Get the name of the selected image
                dp = opendir("images");
                if (dp == NULL) {
                    printf("Failed to open 'images' subfolder.\n");
                    break;
                }

                int current_image = 0;
                char selected_image_name[256];
                while ((entry = readdir(dp))) {
                    if (entry->d_type == DT_REG &&
                        (strstr(entry->d_name, ".img") || strstr(entry->d_name, ".qcow2"))) {
                        current_image++;
                        if (current_image == selected_image) {
                            strcpy(selected_image_name, entry->d_name);
                            break;
                        }
                    }
                }
                closedir(dp);

                // Confirm deletion
                char confirm;
                printf("Are you sure you want to delete '%s'? (y/n): ", selected_image_name);
                scanf(" %c", &confirm);

                if (confirm == 'y' || confirm == 'Y') {
                    char image_path[512];
                    snprintf(image_path, sizeof(image_path), "images/%s", selected_image_name);

                    // Delete the selected image
                    if (remove(image_path) == 0) {
                        printf("Disk image '%s' deleted successfully.\n", selected_image_name);
                    } else {
                        printf("Failed to delete disk image '%s'.\n", selected_image_name);
                    }
                } else {
                    printf("Deletion canceled.\n");
                }
                sleep(2);
                break;
            case 3:
                {
                    system("clear");

                    // Mount Disk Image logic

                    // Check if the "images" subfolder exists
                    if (!directoryExists("images")) {
                        printf("No disk images found in 'images' subfolder. Create some images first.\n");
                        sleep(3);
                        break;
                    }

                    // Declare variables here
                    int image_count = 0;
                    struct dirent *entry;
                    DIR *dp = opendir("images");
                    int selected_image;
                    char selected_image_name[256];

                    if (dp == NULL) {
                        printf("Failed to open 'images' subfolder.\n");
                        sleep(3);
                        break;
                    }

                    while ((entry = readdir(dp))) {
                        if (entry->d_type == DT_REG &&
                            (strstr(entry->d_name, ".img") || strstr(entry->d_name, ".qcow2"))) {
                            printf("%d. %s\n", ++image_count, entry->d_name);
                        }
                    }
                    closedir(dp);

                    if (image_count == 0) {
                        printf("No disk images found in 'images' subfolder. Create some images first.\n");
                        sleep(3);
                        break;
                    }

                    printf("Enter the number of the image to mount (1-%d): ", image_count);
                    scanf("%d", &selected_image);

                    if (selected_image < 1 || selected_image > image_count) {
                        printf("Invalid selection.\n");
                        sleep(2);
                        break;
                    }

                    // Get the name of the selected image
                    dp = opendir("images");
                    if (dp == NULL) {
                        printf("Failed to open 'images' subfolder.\n");
                        sleep(3);
                        break;
                    }

                    int current_image = 0;
                    while ((entry = readdir(dp))) {
                        if (entry->d_type == DT_REG &&
                            (strstr(entry->d_name, ".img") || strstr(entry->d_name, ".qcow2"))) {
                            current_image++;
                            if (current_image == selected_image) {
                                strcpy(selected_image_name, entry->d_name);
                                break;
                            }
                        }
                    }
                    closedir(dp);

                    // Check if the nbd module is loaded
                    if (system("lsmod | grep -q '^nbd'") != 0) {
                        printf("nbd module is not loaded. Loading...\n");
                        if (system("sudo modprobe nbd max_part=8") != 0) {
                            printf("Failed to load the nbd module.\n");
                            sleep(3);
                            break;
                        }
                        printf("nbd module loaded successfully.\n");
                        sleep(3);
                    } else {
                        printf("nbd module is already loaded. Proceeding...\n");
                    }

                    // Check if /dev/nbd0 is already connected
                    if (system("lsblk -o NAME | grep -q '^nbd0$'") == 0) {
                        printf("/dev/nbd0 is already connected. Disconnecting...\n");
                        if (system("sudo qemu-nbd --disconnect /dev/nbd0") != 0) {
                            printf("Failed to disconnect /dev/nbd0.\n");
                            sleep(3);
                            break;
                        }
                    }

                    // Use qemu-nbd to connect the image
                    char nbd_command[512];
                    snprintf(nbd_command, sizeof(nbd_command), "sudo qemu-nbd --connect=/dev/nbd0 -f raw images/%s", selected_image_name);

                    if (system(nbd_command) != 0) {
                        printf("Failed to connect /dev/nbd0 to the image.\n");
                        sleep(3);
                        break;
                    }

                    printf("Image '%s' mounted as /dev/nbd0.\n", selected_image_name);

                    // Create a mount point if it doesn't exist
                    struct stat st;
                    if (stat("mnt", &st) == -1) {
                        if (mkdir("mnt", 0755) != 0) {
                            printf("Failed to create 'mnt' directory.\n");
                            sleep(3);
                            break;
                        }
                        printf("Created 'mnt' directory.\n");
                        sleep(3);
                    }

                    // Mount /dev/nbd0 to the "mnt" directory
                    if (system("sudo mount /dev/nbd0 mnt") != 0) {
                        printf("Failed to mount /dev/nbd0 to 'mnt' directory.\n");
                        sleep(3);
                        break;
                    }

                    printf("Image mounted to 'mnt' directory successfully.\n");
                    sleep(3);
                }
                break;
            case 4:
                {
                    system("clear");

                    // Unmount Disk Image logic

                    // Check if the "mnt" directory exists
                    if (directoryExists("mnt")) {
                        // Unmount the image from the "mnt" directory
                        if (system("sudo umount mnt") != 0) {
                            printf("Failed to unmount the image.\n");
                            sleep(3);
                            break;
                        }
                        printf("Image unmounted.\n");

                        // Disconnect the NBD device
                        if (system("sudo qemu-nbd --disconnect /dev/nbd0") != 0) {
                            printf("Failed to disconnect NBD device.\n");
                            sleep(3);
                            break;
                        }
                        printf("NBD device disconnected.\n");

                        // Remove the "mnt" directory
                        if (system("rm -rf mnt") != 0) {
                            printf("Failed to remove 'mnt' directory.\n");
                            sleep(3);
                            break;
                        }
                        printf("Directory 'mnt' removed.\n");
                    } else {
                        printf("No mounted image found in 'mnt' directory.\n");
                    }

                    sleep(3);
                }
                break;
            case 5:
                // Exit the program
                printf("Exiting DiskProvision. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please select a valid option.\n");
        }

        // Clear the input buffer
        while (getchar() != '\n');

    } while (choice != 5);

    return 0;
}