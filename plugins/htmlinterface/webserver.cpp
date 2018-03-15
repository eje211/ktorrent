/***************************************************************************
 *   Copyright (C) 2018 by Emmanuel Eytan                                  *
 *   eje211@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include <iostream>
#include "webserver.h"

using namespace std;

#include <stdio.h>
#include <string.h>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QUuid>

using namespace std;

namespace kt {
    WebServer::WebServer(CoreInterface * core): core(core)
    {
        
    }
    
    WebServer::~WebServer()
    {
    }

    TorrentListGenerator * WebServer::listGenerator;

    QString WebServer::sessionToken;

    const char * WebServer::cookie;

    void WebServer::process()
    {
        listGenerator = new TorrentListGenerator(core);
        sessionToken = QUuid::createUuid().toString();
        cookie = makeCookie();

        struct mg_connection *c;

        mg_mgr_init(&mgr, NULL);
        c = mg_bind(&mgr, "8880", WebServer::httpeventhandler);
        mg_set_protocol_http_websocket(c);
        while (true) {
            mg_mgr_poll(&mgr, 1000);
        }
    }

    void WebServer::httpeventhandler(struct mg_connection * c, int ev, void * ev_data) {
        if (ev == MG_EV_HTTP_REQUEST)
        {
            struct mg_serve_http_opts opts;

            memset(&opts, 0, sizeof(opts));  // Reset all options to defaults
            opts.document_root = "/usr/share/ktorrent/html/";
                      
            struct http_message * message = (struct http_message *) ev_data;
            
            char * uri = new char[message->uri.len + 1];
            strncpy(uri, message->uri.p, message->uri.len);
            uri[message->uri.len] = '\0';

            if (strcmp("/ktorrentdata", uri) == 0
                && strncmp("GET", message->method.p, message->method.len) == 0)
            {
                QByteArray json = listGenerator->get();
                mg_send_head(c, 200, json.size(), "Content-Type: application/json\nAccess-Control-Allow-Origin: *");
                mg_printf(c, "%s", json.data());
                return;
            }
            if (strcmp("/ktorrentaction", uri) == 0)
            {
                char * body = new char[message->body.len + 1];
                strncpy(body, message->body.p, message->body.len);
                body[message->body.len] = '\0';

                if (strncmp(message->method.p, "POST", message->method.len) == 0) {
                    listGenerator->post(body);
                }
                const char * validation = "{\"status\":\"Request received.\"}";
                int len = strlen(validation);
                mg_send_head(c, 200, len, "Content-Type: application/json\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Headers: X-ACCESS_TOKEN, Access-Control-Allow-Origin, Authorization, Origin, x-requested-with, Content-Type, Content-Range, Content-Disposition, Content-Description\nAccess-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
                mg_printf(c, "%s", validation);
                return;
            }
            opts.extra_headers = cookie;
            mg_serve_http(c, message, opts);
        }
    }
    
    const char * WebServer::makeCookie() {
        QString tempString = QString::fromLatin1("Set-Cookie:token=") + sessionToken;
        return tempString.toLatin1().data();
    }
    
    bool WebServer::checkForToken(http_message * message) {
        for (int i = 0; i < MG_MAX_HTTP_HEADERS; i++) {
            if (message->header_names[i].len == 0) {
                return false;
            }
            char * header_name = new char[message->header_names[i].len + 1];
            strncpy(header_name, message->header_names[i].p, message->header_names[i].len);
            header_name[message->header_names[i].len] = '\n';
            if (strcmp(header_name, "Session-Token") == 0) {
                char * token = new char[message->header_values[i].len + 1];
                strncpy(token, message->header_values[i].p, message->header_values[i].len);
                token[message->header_values[i].len] = '\n';
                if (strcmp(token, sessionToken.toLatin1().data()) == 0) {
                    return true;
                }
            }
        }
        return false;
    }
}
