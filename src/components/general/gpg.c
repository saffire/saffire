/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "general/smm.h"
#include "general/gpg.h"
#include "general/popen2.h"


/**
 * Verifies a buffer block. Returns 1 on valid. 0 when not valid.
 */
int gpg_verify(char *buffer, unsigned int buffer_len, char *signature, unsigned int signature_len) {
    char tmp_path[] = "/tmp/.sf_gpg_XXXXXX";

    // Open temp file (and modify tmp_path with generated filename)
    int fd = mkstemp(tmp_path);
    FILE *f = fdopen(fd, "wb");
    if (! f) return 0;
    fwrite(signature, signature_len, 1, f);
    fclose(f);

    // Arguments passed to GPG
    char *args[] = {
            "/usr/bin/gpg",
            "--no-armor",
            "--verify",
            tmp_path,
            "-",
            NULL
    };

    // open GPG
    int pipe[3];
    int pid = popenRWE(pipe, args[0], args);
    if (pid < 0) return 0;

    // Write our buffer to GPG
    write(pipe[0], buffer, buffer_len);
    close(pipe[0]);


    // Output contents from childs STDERR
    char buf[256];
    bzero(buf, 256);
    while (read(pipe[2], buf, 255) > 0) {
        printf("%s", buf);
        bzero(buf, 256);
    }

    // Close GPG
    int ret = pcloseRWE(pid, pipe);

    // Remove temp file
    if (unlink(tmp_path) != 0) return 0;

    // Test exit status of child GPG process
    if (! WIFEXITED(ret)) {
        // Status of child says it wasn't exited ?
        return 0;
    }
    ret = WEXITSTATUS(ret); // Get the exit status from the child

    // Return 1 when returncode = 0, return with 0 otherwise
    return (ret == 0) ? 1 : 0;
}

/**
 * Signs a buffer block. *signature should be NULL to allocate a new buffer, and *signature_len returns the length of the signature
 */
int gpg_sign(char *gpg_key, char *buffer, unsigned int buffer_len, char **signature, unsigned int *signature_len) {
    char *args[] = {
            "/usr/bin/gpg",
            "-bsu",
            gpg_key,
            NULL
    };

    // @TODO: use path from configuration

    int pipe[3];
    int pid = popenRWE(pipe, args[0], args);
    if (pid < 0) return 0;

    // Write data to GPG
    write(pipe[0], buffer, buffer_len);
    close(pipe[0]);

    FILE *f = fopen("/tmp/ggpsig", "w");

    // Read back the actual (binary) GPG signature
    int len;
    do {
        char buf[256];
        len = read(pipe[1], buf, 255);
        if (len == -1) continue;

        *signature = smm_realloc(*signature, *signature_len + len);
        memcpy(*signature + *signature_len, buf, len);

        fwrite(buf, len, 1, f);

        *signature_len += len;
    } while (len > 0);

    fclose(f);

    // Close all
    pcloseRWE(pid, pipe);

    return 1;
}