// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    unimplemented();
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, NULL, alphasort);
    unsigned found_eligible_device = 0;
    char* name;
    while (n--) {
        char* entry_name = namelist[n]->d_name;
        if (strstr(entry_name, ttyusb_prefixes[2]) == NULL && strstr(entry_name, ttyusb_prefixes[3]) == NULL)
            continue;
        if (found_eligible_device) {
            // free(namelist[n]);
            // free(name);
            panic("There are more than 1 eligilbe tty-usb devices\n");
        }
        name = strdupf("/dev/%s", entry_name);
        found_eligible_device = 1;
        // free(namelist[n]);
    }
    // free(namelist);
    if (found_eligible_device == 0) {
        panic("There are no eligible tty-usb devices\n");
        free(name);
    }
    return name;
}   

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    struct dirent **namelist;
    struct stat sb;
    int n = scandir("/dev", &namelist, NULL, alphasort);
    char* entry_name;
    char* name = NULL;
    long mod_time = 0;
    while (n--) {
        entry_name = namelist[n]->d_name;
        if (strstr(entry_name, ttyusb_prefixes[2]) == NULL && strstr(entry_name, ttyusb_prefixes[3]) == NULL)
            continue;
        stat(entry_name, &sb);
        // check the modification time
        if (sb.st_mtimespec.tv_nsec >= mod_time) {
            mod_time = sb.st_mtimespec.tv_nsec;
            name = entry_name;
        }
    }
    if (!name)
        printf("there are no eligible tty-usb devices\n");
    return strdupf("/dev/%s", name);
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    struct dirent **namelist;
    struct stat sb;
    int n = scandir("/dev", &namelist, NULL, alphasort);
    char* name;
    char* entry_name;
    long mod_time = INT64_MAX;
    while (n--) {
        entry_name = namelist[n]->d_name;
        if (strstr(entry_name, ttyusb_prefixes[2]) == NULL && strstr(entry_name, ttyusb_prefixes[3]) == NULL)
            continue;
        stat(entry_name, &sb);
        // check the modification time
        if (sb.st_mtimespec.tv_nsec < mod_time) {
            mod_time = sb.st_mtimespec.tv_nsec;
            name = entry_name;
        }
    }
    return strdupf("/dev/%s", name);
}
