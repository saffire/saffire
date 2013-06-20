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
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <unistd.h>
#include <string.h>
#include "debugger/dbgp/args.h"
#include "debugger/dbgp/xml.h"

/**
 *
 */
void dbgp_xml_init(void) {
    xmlKeepBlanksDefault(1);
}

/**
 *
 */
void dbgp_xml_fini(void) {
    xmlCleanupParser();
}

/**
 * Send out the XML
 * @param sockfd
 * @param doc
 */
void dbgp_xml_send(int sockfd, xmlNodePtr root_node) {
    xmlChar *xmlbuf;
    int xmlbuf_size;

    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlDocSetRootElement(doc, root_node);

    // Dump XML to stdout
    xmlDocDumpFormatMemory(doc, &xmlbuf, &xmlbuf_size, 1);
    //printf("BUFSIZE: %d\n", xmlbuf_size);
    xmlDocFormatDump(stdout, doc, 1);


    // Write to socket
    char buf[100];
    sprintf(buf, "%d%c", xmlbuf_size, 0x0);

    write(sockfd, buf, strlen(buf));
    write(sockfd, "\0", 1);
    write(sockfd, xmlbuf, xmlbuf_size);
    write(sockfd, "\0", 1);

    // Free buffers
    xmlFree(xmlbuf);
    xmlFreeDoc(doc);
}

/**
 *
 */
xmlNodePtr dbgp_xml_create_response(t_debuginfo *di) {
//    // Find transaction ID
//    int i = dbgp_args_find("-i", argc, argv);
//    char *transaction_id_str = (i != -1) ? argv[i+1] : "0";

    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "response");
    xmlNewProp(root_node, BAD_CAST "xmlns", BAD_CAST "urn:debugger_protocol_v1");
    xmlNewProp(root_node, BAD_CAST "command", BAD_CAST di->cur_cmd);
    xmlNewProp(root_node, BAD_CAST "transaction_id", BAD_CAST di->cur_txid);

    return root_node;
}


/**
 *
 */
xmlNodePtr dbgp_xml_create_init_node(void) {
    xmlNodePtr node = NULL;


    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "init");

    char buf[100];
    sprintf(buf, "%d", getpid());
    xmlNewProp(root_node, BAD_CAST "appid", BAD_CAST buf);
    xmlNewProp(root_node, BAD_CAST "idekey", BAD_CAST "PHPStorm1");
    xmlNewProp(root_node, BAD_CAST "thread", BAD_CAST "1");
    xmlNewProp(root_node, BAD_CAST "language", BAD_CAST "Saffire");
    xmlNewProp(root_node, BAD_CAST "protocol_version", BAD_CAST "1.0");
    xmlNewProp(root_node, BAD_CAST "fileuri", BAD_CAST "/home/jthijssen/saffire/hello.sf");

    node = xmlNewChild(root_node, NULL, BAD_CAST "author", BAD_CAST "Joshua Thijssen");
    xmlAddChild(root_node, node);
    node = xmlNewChild(root_node, NULL, BAD_CAST "company", BAD_CAST "Saffire Group");
    xmlAddChild(root_node, node);
    node = xmlNewChild(root_node, NULL, BAD_CAST "license", BAD_CAST "BSD 3-Clause");
    xmlAddChild(root_node, node);
    node = xmlNewChild(root_node, NULL, BAD_CAST "url", BAD_CAST "http://saffire-lang.org");
    xmlAddChild(root_node, node);

    return root_node;
}
