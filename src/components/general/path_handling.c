/*
 Copyright (c) 2012-2013, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
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
#include "general/path_handling.h"
#include "general/definitions.h"
#include "general/smm.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>


/**
 * Replaces or adds an extension on a path. Returns path with new extension.
 */
char *replace_extension(const char *path, const char *source_ext, const char *dest_ext) {
    char *dest_path, *ptr;
    int len;

    // Allocate enough room for path + complete extension
    dest_path = smm_malloc(strlen(path) + sizeof(dest_ext));
    bzero(dest_path, strlen(path) + sizeof(dest_ext));

    // Seek last .
    ptr = strrchr(path, '.');

    // Check if the last part is the actual source extension
    if (ptr != NULL && ! strcmp(ptr, source_ext)) {
        // Only copy until the extension
        len = ptr - path;
    } else {
        // Copy whole source
        len = strlen(path);
    }

    // Copy required part of path and add extension
    strncpy(dest_path, path, len);
    strcat(dest_path, dest_ext);

    return dest_path;
}

/**
 * Checks if target is a file
 */
int is_file(const char *target) {
	struct stat s;

	if(stat(target,&s) == 0) {
	    if(s.st_mode & S_IFREG) {
			return 1;
		}
	}
	return 0;
}

/**
 * Checks if target is a directory
 */
int is_directory(const char *target) {
	struct stat s;

	if(stat(target,&s) == 0) {
		if(s.st_mode & S_IFDIR) {
			return 1;
		}
	}
	return 0;
}

/**
 * Checks if filename is a saffire file
 */
int is_saffire_file(const char *filename) {
	char *extension = strrchr(filename, '.');

	if (extension && strcmp(filename, extension) != 0 && strcmp(extension, SAFFIRE_EXTENSION) == 0) {
		return 1;
	}
	return 0;
}
