#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    // How: 
    //    - use stat() to get the size of the file.
    //    - round up to a multiple of 4.
    //    - allocate a buffer
    //    - zero pads to a multiple of 4.
    //    - read entire file into buffer (read_exact())
    //    - fclose() the file descriptor
    //    - make sure any padding bytes have zeros.
    //    - return it.   
    struct stat sb;
    int status = stat(name, &sb);
    off_t file_size = sb.st_size;
    void* buffer = calloc(file_size + 4, 1); // each char is one byte
    int fd = open(name, O_RDONLY);
    if (file_size > 0)
        *size = read_exact(fd, buffer, file_size);
    close(fd);
    return buffer;
}
