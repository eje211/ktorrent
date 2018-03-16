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

#ifndef KHTMLINTERFACEWEBSERVER_H
#define KHTMLINTERFACEWEBSERVER_H

#include <QtGlobal>
#include <QObject>

#include <interfaces/coreinterface.h>
#include <microhttpd.h>
#include "torrentlistgenerator.h"

namespace kt
{
    class WebServer : public QObject
    {
        Q_OBJECT
    public:
        WebServer(CoreInterface * core);
        ~WebServer();
    public slots:
        void process();
    signals:
        void finished();
        void error(QString err);
    private:
        static int answer_to_connection(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method,
                    const char *version, const char *upload_data,
                    size_t *upload_data_size, void **con_cls);
        struct MHD_Daemon *daemon;
        CoreInterface * core;
        static TorrentListGenerator * listGenerator;
        static QString sessionToken;
        static const char * makeCookie();
        static const char * cookie;
        static bool checkForToken(MHD_Connection * connection);
    };
}

#endif
