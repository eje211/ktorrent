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

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QUuid>
#define GET             0
#define POST            1
#define POSTBUFFERSIZE  65536

using namespace std;

namespace kt {
    WebServer::WebServer(CoreInterface * core): core(core)
    {
        
    }
    
    WebServer::~WebServer()
    {
        MHD_stop_daemon (daemon);
    }

    static QString BASE_DIRECTORY = QString::fromLatin1("/usr/share/ktorrent/html/");

    TorrentListGenerator * WebServer::listGenerator;

    QString WebServer::sessionToken;

    const char * WebServer::cookie;

    static int respond404(MHD_Connection * connection)
    {
        int ret;
        struct MHD_Response *response;

        const char * message = "<html><body>404: The requested resource was not found.</body></html>";

        response = MHD_create_response_from_buffer (strlen(message), (void *) message, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
        MHD_add_response_header (response, MHD_HTTP_HEADER_CONTENT_ENCODING, "text/html");
        MHD_destroy_response (response);
        return ret;
    }

    const char * getMimeType(QString url)
    {
        if (url.endsWith(QString::fromLatin1(".html")))
        {
            return "text/html";
        }
        else if (url.endsWith(QString::fromLatin1(".css")))
        {
            return "text/css";
        }
        else if (url.endsWith(QString::fromLatin1(".js")))
        {
            return "text/javascript"; // It's "application/javascript" but this is more compatible.
        }
        return "text/plain";
    }

    static int respond500(MHD_Connection * connection)
    {
        int ret;
        struct MHD_Response *response;

        const char * message = "<html><body>500: An internal server error has occured.</body></html>";

        response = MHD_create_response_from_buffer (strlen(message), (void *) message, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response (connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_add_response_header (response, MHD_HTTP_HEADER_CONTENT_ENCODING, "text/html");
        MHD_destroy_response (response);
        return ret;
    }

    struct Json
    {
        QString json;
        bool firstPass;
    };
    
    int WebServer::answer_to_connection(void * cls, struct MHD_Connection * connection,
                      const char * url, const char * method,
                      const char * version, const char * upload_data,
                      size_t * upload_data_size, void ** con_cls)
    {
        int ret;
        struct MHD_Response *response;

        if (strcmp("/ktorrentdata", url) == 0 && strcmp("GET", method) == 0)
        {
            QByteArray json = listGenerator->get();
            response = MHD_create_response_from_buffer (json.size(), (void *) json.data(), 
                MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
            MHD_destroy_response (response);
            return ret;
        }
        if (strcmp("/ktorrentaction", url) == 0 && strcmp("POST", method) == 0)
        {
            Json * json;
            if (*con_cls == NULL)
            {
                json = new Json();
                *con_cls = (void *) json;
                json->firstPass = true;
            }
            else
            {
                json = (Json *) *con_cls;
            }

            if (json->firstPass) {
                json->firstPass = false;
                return MHD_YES;
            }
            if (*upload_data_size > 0 && !json->firstPass)
            {
                QString qJson = QString::fromLatin1(upload_data, *upload_data_size);
                json->json.append(qJson);
                if (checkForToken(connection))
                {
                    listGenerator->post(json->json);
                }
                *upload_data_size = 0;
                return MHD_YES;
            }
            const char * validation = "{\"status\":\"Request received.\"}";
            response = MHD_create_response_from_buffer(strlen(validation), (void *) validation,
                MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
            return ret;
        }
        struct stat sbuf;
        int fd;
        if (0 == strcmp(url, "/") || 0 == strcmp(url, "/index"))
        {
            url = "index.html";
        }
        QString qUrl = QString::fromLatin1(url);
        const char * file = (BASE_DIRECTORY + qUrl).toLatin1().data();
        if ((-1 == (fd = open(file, O_RDONLY))) || (0 != fstat(fd, &sbuf)))
        {
            return respond404(connection);
        }
        response = MHD_create_response_from_fd_at_offset64(sbuf.st_size, fd, 0);
        MHD_add_response_header(response, "Content-Type", getMimeType(qUrl));
        MHD_add_response_header(response, "Set-Cookie", makeCookie());
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response (response);
        return ret;
    }
    
    static void request_completed (void *cls, struct MHD_Connection *connection,
        void **con_cls, enum MHD_RequestTerminationCode toe)
    {
        Json * json = (Json *) *con_cls;

        delete json;
    }
    
    void WebServer::process()
    {
        listGenerator = new TorrentListGenerator(core);
        sessionToken = QUuid::createUuid().toString();
        cookie = makeCookie();

        daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8880, NULL, NULL,
                                &answer_to_connection, NULL,
                                MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                                NULL, MHD_OPTION_END);
    }
    
    const char * WebServer::makeCookie()
    {
        QString tempString = QString::fromLatin1("token=") + sessionToken;
        return tempString.toLatin1().data();
    }

    bool WebServer::checkForToken(MHD_Connection * connection)
    {
        const char * token = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "session-Token");
        if (token == NULL)
        {
            return false;
        }
        if (strcmp(token, sessionToken.toLatin1().data()) == 0)
        {
            return true;
        }
        return false;
    }
}
