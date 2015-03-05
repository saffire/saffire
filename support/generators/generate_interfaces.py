"""
 This script generates interface classes

 Usage: generate_interfaces.py <path/to/file.dat> <path/to/file.c>

"""

import sys


cur_interface = ""
interfaces = {}
for line in open(sys.argv[1]):
    line = line.rstrip()
    if not line or line[0] == '#' :
        continue

    if line[0] <> ' ' :
        line = line.strip()
        interfaces[line] = []
        cur_interface = line
    else :
        line = line.strip()
        interfaces[cur_interface].append(line)



#
# Write C file
#
fp = open(sys.argv[2], "w")

header = '''
/*
Copyright (c) 2012-2015, The Saffire Group
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

/*
 * WARNING: THIS FILE IS AUTOGENERATED!
 */


#include <stdio.h>
#include <saffire/objects/object.h>


'''
fp.write(header)


for name, methods in interfaces.iteritems() :
    fp.write("static void object_%s_init(void) {\n" % name)
    fp.write("    Object_%s_struct.attributes = ht_create();\n" % name.title())
    for method in methods :
        fp.write("    object_add_internal_method((t_object *)&Object_%s_struct, \"%s\", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, NULL);\n" % (name.title(), method))
    fp.write("    vm_populate_builtins(\"%s\", (t_object *)&Object_%s_struct);\n" % (name, name.title()))
    fp.write("}\n")
    fp.write("\n")
    fp.write("static void object_%s_fini(void) {\n" % name)
    fp.write("    object_free_internal_object((t_object *)&Object_%s_struct);\n" % name.title())
    fp.write("}\n")
    fp.write("\n")
    fp.write("t_interface_object Object_%s_struct = {\n" % name.title())
    fp.write("    OBJECT_HEAD_INIT(\"%s\", objectTypeBase, OBJECT_TYPE_INTERFACE|OBJECT_FLAG_IMMUTABLE, NULL, 0),\n" % name.title())
    fp.write("};\n")
    fp.write("\n")
    fp.write("\n")
    fp.write("\n")

fp.write("\n\n")

fp.write("void object_interfaces_init(void) {\n")
for name, methods in interfaces.iteritems() :
    fp.write("    object_%s_init();\n" % name)

fp.write("}\n")
fp.write("\n")

fp.write("void object_interfaces_fini(void) {\n")
for name, methods in reversed(sorted(interfaces.iteritems())) :
    fp.write("    object_%s_fini();\n" % name)

fp.write("}\n")

fp.write("\n\n")

fp.close()




#
# Write H file
#
fp = open(sys.argv[3], "w")

header = '''
/*
Copyright (c) 2012-2015, The Saffire Group
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
#ifndef ___GENERATED_INTERFACES_H__
#define ___GENERATED_INTERFACES_H__


    /*
     * WARNING: THIS FILE IS AUTOGENERATED!
     */

    typedef struct {
        SAFFIRE_OBJECT_HEADER;
    } t_interface_object;


'''
fp.write(header)

for name, methods in interfaces.iteritems() :
    fp.write("    // %s\n" % name)
    fp.write("    t_interface_object Object_%s_struct;\n" % name.title())
    fp.write("    #define Object_%s ((t_object *)&Object_%s_struct)\n" % (name.title(), name.title() ))
    fp.write("\n")


footer = '''
#endif
'''
fp.write(footer)

fp.close()
