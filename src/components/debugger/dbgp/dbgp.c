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
#include <unistd.h>
#include <sys/types.h>
#include "general/config.h"


#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>



void dbgp_init(void) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.data.in.sin_family = AF_INET;
    server_addr.data.in.sin_port = htons(9000);
    server_addr.data.in.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, &server_addr, sizeof(server_addr));



    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;
    xmlNodePtr node = NULL;
    xmlNodePtr node1 = NULL;
    xmlDtdPtr dtd = NULL;


    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "root");
    xmlDocSetRootElement(doc, root_node);

    xmlNewChild(root_node, NULL, BAD_CAST "node1", BAD_CAST "content of node 1");
    node = xmlNewChild(root_node, NULL, BAD_CAST "node3", BAD_CAST "this node has attributes");
    xmlNewProp(node, BAD_CAST "attribute", BAD_CAST "yes");
    xmlNewProp(node, BAD_CAST "foo", BAD_CAST "bar");

    xmlDocDump(stdout, doc);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    exit;

    sprintf(buf, 4096, "<init appid=\"%s\" "
                       "idekey=\"%s\" "
                       "thread=\"%s\" "
                       "language=\"%s\" "
                       "protocol-version=\"%s\" "
                       "file_uri=\"%s\" "
                       "engine=\"%s\" "
                       "author=\"%s\" "
                       "url=\"%s\">",
                       pid(),
                       ide_key,
                       1,
                       "saffire",
                       "1.0",
                       "/hello.sf",
                       "saffire dbpg debugger",
                       "Joshua Thijssen",
                       "http://saffire-lang.org");

    buf, "%d\0%s\0", strlen(buf), buf);






//    char *debugger_host = config_get_string("debugger.host", "127.0.0.1");
//    long debugger_port = config_get_long("debugger.port", "9000");

}
