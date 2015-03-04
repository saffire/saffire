"""
 This script creates a vmopcodes.h include file from our vm_codes template. This is needed because we need to have
 a few different kind of accessing the opcodes. Either by a define, or through an array (by hexidecimal code).
"""

import textwrap
import sys
import re

#
# Read opcodes
#
opcodes = {}
for line in open(sys.argv[1]):
    line = line.strip()

    if not line or line[0] == ';' :
        continue

    match = re.match('[\t ]*([A-Z_0-9]+)[\t ]+([0-9A-Fx]+)', line, re.IGNORECASE)
    if match is None:
        print "Encountered incorrect formated opcode at '%s'\n" % line
    opcodes[match.group(2)] = match.group(1)

opcode_keys = opcodes.keys()
opcode_keys.sort()


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
#ifndef __VM_GENERATED_OPCODES_H__
#define __VM_GENERATED_OPCODES_H__


/*
* WARNING: THIS FILE IS AUTOGENERATED! PLEASE USE CREATE_VMOPCODES_H.PY TO REGENERATE!
*/
'''
fp.write(header)

for hex_code,str_code in opcodes.iteritems():
    fp.write("    #define VM_%-20s%s\n" % (str_code.upper(), hex_code))
fp.write("\n\n")


fp.write("    int vm_codes_offset[256];\n")
fp.write("    char *vm_code_names[%d];\n" % len(opcodes))

footer = '''
#endif
'''
fp.write(footer)
fp.close()





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
* WARNING: THIS FILE IS AUTOGENERATED! PLEASE USE CREATE_VMOPCODES_H.PY TO REGENERATE!
*/

'''
fp.write(header)

# Do indexes
fp.write("int vm_codes_index[%d] = {\n" % len(opcodes))
for line in textwrap.wrap(", ".join(opcode_keys), 70, initial_indent='    ', subsequent_indent="\n    ") :
    fp.write(line)
fp.write("\n};\n")
fp.write("\n\n")


# Do offsets
fp.write("int vm_codes_offset[256] = {\n")

s = []
for i in range(0,256) :
    opcode = "0x%02X" % i
    if opcode in opcode_keys :
        idx = opcode_keys.index(opcode)
    else :
        idx = -1
    s.append(str(idx))
#fp.write(", ".join(s))
for line in textwrap.wrap(", ".join(s), 70, initial_indent='    ', subsequent_indent="\n    ") :
    fp.write(line)
fp.write("\n};\n")
fp.write("\n\n")


# Do names
fp.write("char *vm_code_names[%d] = {\n" % len(opcodes))
s = []
for i in opcode_keys :
    s.append('"%s"' % opcodes[i])
for line in textwrap.wrap(", ".join(s), 70, initial_indent='    ', subsequent_indent="\n    ") :
    fp.write(line)
fp.write("\n};\n")
fp.write("\n\n")



fp.close()

