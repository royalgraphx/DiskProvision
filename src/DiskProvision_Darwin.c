/*
 * DiskProvision_Darwin - Allows the creation, management, and updating of disk images for use with QEMU on macOS.
 * DiskProvision_Darwin.c - The source code of the DiskProvision program for macOS.
 * BSD 3-Clause "New" or "Revised" License
 * Copyright (c) 2023 RoyalGraphX
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For sleep function
#include <ctype.h>  // Include ctype.h for toupper()
#include <dirent.h> // For directory listing
#include <sys/stat.h> // For stat() function

// Function to convert a string to uppercase
void stringToUpper(char *str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

// Function to check if a file exists
int fileExists(const char *path) {
    struct stat info;
    return stat(path, &info) == 0 && S_ISREG(info.st_mode);
}

int main() {
    char image_name[256];
    char image_size[256];
    char image_path[512];
    int choice;

    do {
        system("clear");

        // Display welcome message
        printf("Welcome to DiskProvision!\n");
        printf("Copyright (c) 2023 RoyalGraphX\n");
        printf("Darwin x86_64 Pre-Release 0.0.1\n\n");

        // Display menu options
        printf("Menu:\n");
        printf("1. Create New Disk Image\n");
        printf("2. Mount Disk Image\n");
        printf("3. Unmount Disk Image\n");
        printf("4. Delete Disk Image\n");
        printf("5. Exit\n\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Handle user's choice
        switch (choice) {
            case 1:
                system("clear");

                // Create New Disk Image logic

                // Prompt user for image size in gigabytes
                printf("Enter the size (in GB) for the disk image (e.g., 1): ");
                scanf("%s", image_size);

                // Prompt user for image name
                printf("Enter the name for the disk image (without .img extension): ");
                scanf("%s", image_name);

                // Construct the path for the .dmg image
                snprintf(image_path, sizeof(image_path), "images/%s.dmg", image_name);

                // Check if the "images" subfolder already exists
                if (access("images", F_OK) != 0) {
                    // Create the "images" subfolder if it doesn't exist
                    if (mkdir("images", 0755) != 0) {
                        printf("Failed to create the 'images' subfolder.\n");
                        break;
                    }
                }

                // Create a new disk image with specified size and format
                char create_command[512];
                snprintf(create_command, sizeof(create_command),
                         "hdiutil create -size %sG -type UDIF -fs \"FAT32\" -volname \"%s\" %s",
                         image_size, image_name, image_path);

                if (system(create_command) != 0) {
                    printf("Failed to create the disk image.\n");
                    break;
                }

                // Rename the .dmg to .img
                char rename_command[512];
                snprintf(rename_command, sizeof(rename_command),
                         "mv %s images/%s.img", image_path, image_name);

                if (system(rename_command) != 0) {
                    printf("Failed to rename the disk image to .img.\n");
                    break;
                }

                printf("Disk image 'images/%s.img' created successfully.\n", image_name);
                break;
            case 2:
                {
                    system("clear");

                    // Mount Disk Image logic

                    // Check if the "images" subfolder exists
                    if (access("images", F_OK) != 0) {
                        printf("No disk images found in 'images' subfolder. Create some images first.\n");
                        sleep(2);
                        break;
                    }

                    printf("Disk images available:\n");

                    // List all files in the "images" subfolder
                    DIR *dp;
                    struct dirent *entry;
                    dp = opendir("images");
                    if (dp == NULL) {
                        printf("Failed to open 'images' subfolder.\n");
                        sleep(2);
                        break;
                    }

                    int image_count = 0;
                    char image_names[256][256];

                    while ((entry = readdir(dp))) {
                        if (entry->d_type == DT_REG && strstr(entry->d_name, ".img")) {
                            image_count++;
                            strcpy(image_names[image_count], entry->d_name);
                            printf("%d. %s\n", image_count, entry->d_name);
                        }
                    }
                    closedir(dp);

                    if (image_count == 0) {
                        printf("No disk images found in 'images' subfolder. Create some images first.\n");
                        sleep(2);
                        break;
                    }

                    int selected_image;
                    printf("Enter the number of the image to mount (1-%d): ", image_count);
                    scanf("%d", &selected_image);

                    if (selected_image < 1 || selected_image > image_count) {
                        printf("Invalid selection.\n");
                        sleep(2);
                        break;
                    }

                    // Get the name of the selected image
                    strcpy(image_name, image_names[selected_image]);

                    // Construct the path for the .img image
                    snprintf(image_path, sizeof(image_path), "images/%s", image_name);

                    // Mount the disk image to the "mnt" directory
                    char mount_command[512];
                    snprintf(mount_command, sizeof(mount_command),
                             "hdiutil attach %s -mountpoint mnt/", image_path);

                    if (system(mount_command) != 0) {
                        printf("Failed to mount the disk image.\n");
                        sleep(2);
                        break;
                    }

                    printf("Disk image '%s' mounted to 'mnt' directory successfully.\n", image_name);
                    sleep(2);
                }
                break;
            case 3:
                {
                    system("clear");

                    // Unmount Disk Image logic

                    // Check if the "mnt" directory exists
                    if (access("mnt", F_OK) == 0) {
                        // Unmount the image from the "mnt" directory
                        if (system("hdiutil detach mnt/") != 0) {
                            printf("Failed to unmount the image.\n");
                            sleep(2);
                            break;
                        }
                        printf("Image unmounted.\n");
                    } else {
                        printf("No mounted image found in 'mnt' directory.\n");
                    }
                    sleep(2);
                }
                break;
            case 4:
                {
                    system("clear");

                    // Delete Disk Image logic

                    // Check if the "images" subfolder exists
                    if (access("images", F_OK) != 0) {
                        printf("No disk images found in 'images' subfolder. Create some images first.\n");
                        sleep(2);
                        break;
                    }

                    printf("Disk images available for deletion:\n");

                    // List all files in the "images" subfolder
                    DIR *dp;
                    struct dirent *entry;
                    dp = opendir("images");
                    if (dp == NULL) {
                        printf("Failed to open 'images' subfolder.\n");
                        sleep(2);
                        break;
                    }

                    int image_count = 0;
                    char image_names[256][256];

                    while ((entry = readdir(dp))) {
                        if (entry->d_type == DT_REG && strstr(entry->d_name, ".img")) {
                            image_count++;
                            strcpy(image_names[image_count], entry->d_name);
                            printf("%d. %s\n", image_count, entry->d_name);
                        }
                    }
                    closedir(dp);

                    if (image_count == 0) {
                        printf("No disk images found in 'images' subfolder. Create some images first.\n");
                        sleep(2);
                        break;
                    }

                    int selected_image;
                    printf("Enter the number of the image to delete (1-%d): ", image_count);
                    scanf("%d", &selected_image);

                    if (selected_image < 1 || selected_image > image_count) {
                        printf("Invalid selection.\n");
                        sleep(2);
                        break;
                    }

                    // Get the name of the selected image
                    strcpy(image_name, image_names[selected_image]);

                    // Construct the path for the .img image
                    snprintf(image_path, sizeof(image_path), "images/%s", image_name);

                    // Confirm deletion
                    char confirm;
                    printf("Are you sure you want to delete '%s'? (y/n): ", image_name);
                    scanf(" %c", &confirm);

                    if (confirm == 'y' || confirm == 'Y') {
                        // Delete the selected image
                        if (remove(image_path) == 0) {
                            printf("Disk image '%s' deleted successfully.\n", image_name);
                            sleep(2);
                        } else {
                            printf("Failed to delete disk image '%s'.\n", image_name);
                        }
                    } else {
                        printf("Deletion canceled.\n");
                    }
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