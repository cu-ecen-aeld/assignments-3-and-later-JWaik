#include <stdio.h>
#include <syslog.h>

// Planning
// 1.get user input, filename(1st-arg) and file content(2nd-arg)
// 2.open the file (if dir is not exist, do not create)
// 3.write string contents

// use syslog (syslog.h)
// Write success, File is opend - LOG_DEBUG level
// Other error - LOG_ERR level

int main(int argc, char**argv) {
    //setup syslog
    openlog(NULL, 0, LOG_USER);

    // prepare file info
    // Note: argv[0] = name, argv[argc-1]=last input
    const char *writefile = argv[1];
    const char *writestr = argv[2];
    FILE *fptr;
    fptr = fopen(writestr, "w");

    // Check if the file was successfully opened/created
    if (fptr == NULL) {
        syslog(LOG_ERR, "Could not create or open %s", writefile);
        return 1;
    } else {
        syslog(LOG_DEBUG, "File %s created successfully. ", writefile);
    }

    // Write content
    fwrite(writestr, sizeof(char), sizeof(writestr) - 1, fptr); // -1 to exclude null terminator
    syslog(LOG_DEBUG, "Writing %s to %s successfully", writestr, writefile);

    // Close the file
    fclose(fptr);

    return 0;
} 
  