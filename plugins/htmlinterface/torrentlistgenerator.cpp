/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
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

#include <QChar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMap>
#include <QString>
#include <iostream>

#include "torrentlistgenerator.h"
#include <util/sha1hash.h>
#include <torrent/queuemanager.h>
#include <torrent/magnetmanager.h>
#include <interfaces/coreinterface.h>

using namespace bt;
using namespace std;

namespace kt
{

    TorrentListGenerator::TorrentListGenerator(CoreInterface* core): core(core)
    {
    }


    TorrentListGenerator::~TorrentListGenerator()
    {
    }


    QByteArray TorrentListGenerator::get()
    {
        kt::QueueManager* qman = core->getQueueManager();
        kt::QueueManager::iterator i = qman->begin();
        QJsonArray array;
        while (i != qman->end())
        {
            bt::TorrentInterface* ti = *i;
            const bt::TorrentStats& s = ti->getStats();

            QJsonObject json;
            json.insert(QString::fromLatin1("name"), ti->getDisplayName());
            json.insert(QString::fromLatin1("infoHash"), ti->getInfoHash().toString());
            json.insert(QString::fromLatin1("status"), s.status);
            json.insert(QString::fromLatin1("timeAdded"), s.time_added.toMSecsSinceEpoch());            
            json.insert(QString::fromLatin1("bytesDownloaded"), (double) s.bytes_downloaded);
            json.insert(QString::fromLatin1("bytesUploaded"), (double) s.bytes_uploaded);
            json.insert(QString::fromLatin1("totalBytes"), (double) s.total_bytes);
            json.insert(QString::fromLatin1("totalBytesToDownload"), (double) s.total_bytes_to_download);
            json.insert(QString::fromLatin1("downloadRate"), (double) s.download_rate);
            json.insert(QString::fromLatin1("uploadRate"), (double) s.upload_rate);
            json.insert(QString::fromLatin1("numPeers"), (double) s.num_peers);
            json.insert(QString::fromLatin1("seeders"), (double) s.seeders_connected_to);
            json.insert(QString::fromLatin1("seedersTotal"), (double) s.seeders_total);
            json.insert(QString::fromLatin1("leechers"), (double) s.leechers_connected_to);
            json.insert(QString::fromLatin1("leechersTotal"), (double) s.leechers_total);
            json.insert(QString::fromLatin1("running"), s.running);
            json.insert(QString::fromLatin1("numFiles"), (double) ti->getNumFiles());
            array.append(json);
            i++;
        }
        QJsonObject message;
        message.insert(QString::fromLatin1("torrents"), array);
        return QJsonDocument(message).toJson();
    }

    void TorrentListGenerator::post(const char * json)
    {
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray((const char *) json));
        QMap<QString, QVariant> qJson = doc.toVariant().toMap();
        
        QString action = qJson[QString::fromLatin1("type")].toString();

        if (action.isEmpty())
        {
            return;
        }
        
        if (action == QString::fromLatin1("magnet"))
        {
            QString magnet = qJson[QString::fromLatin1("magnet")].toString();
            if (magnet.isEmpty())
            {
                return;
            }
            QUrl url = QUrl(magnet);
            if (!url.isValid())
            {
                return;
            }
            core->loadSilently(url, QString());
            return;
        }
        
        QString hash = qJson[QString::fromLatin1("hash")].toString();
        if (hash.isEmpty())
        {
            return;
        }

        TorrentInterface * ti = getTorrentFromHash(hash);
        if (action == QString::fromLatin1("start"))
        {
            if (ti->getStats().paused)
            {
                ti->unpause();
            }
            else
            {
                ti->start();
            }
        }
        else if (action == QString::fromLatin1("stop"))
        {
            core->stop(ti);
        }
        else if (action == QString::fromLatin1("remove"))
        {
            core->remove(ti, false);
        }
    }
    
    /**
     * From: https://github.com/KDE/libktorrent/blob/master/src/dht/tests/keytest.cpp
     */
    static Uint8 HexCharToUint8(char c)
    {
        if (c >= 'a' && c <= 'f')
            return 10 + (c - 'a');
        else if (c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
        else if (c >= '0' && c <= '9')
            return c - '0';
        else
            return 0;
    }

    /**
     * Based on: https://github.com/KDE/libktorrent/blob/master/src/dht/tests/keytest.cpp
     */
    static Uint8 * HashFromHexString(QString & str)
    {
        bt::Uint8 * result = new Uint8[20];
        std::fill(result, result + 20, 0);
        
        QString s = str.toLower();
        if (s.size() % 2 != 0)
        {
            s.prepend(QChar::fromLatin1('0'));
        }
        
        int j = 19;
        
        for (int i = s.size() - 1; i >= 0; i -= 2)
        {
            char left = s[i - 1].toLatin1();
            char right = s[i].toLatin1();
            result[j--] = (HexCharToUint8(left) << 4) | HexCharToUint8(right);
        }
        return result;
    }
    
    static const SHA1Hash * QStringToSHA1Hash(QString qString) {
        Uint8 * hashNum = HashFromHexString(qString);
        const SHA1Hash * hash = new SHA1Hash(hashNum);
        delete hashNum;
        return hash;
    }
        
    TorrentInterface * TorrentListGenerator::getTorrentFromHash(QString qHash)
    {
        const SHA1Hash * hash = QStringToSHA1Hash(qHash);
        kt::QueueManager* qman = core->getQueueManager();
        kt::QueueManager::iterator i = qman->begin();
        while (i != qman->end())
        {
            bt::TorrentInterface* ti = *i;
            if (ti->getInfoHash() == *hash)
            {
                return ti;
            }
            i++;
        }
        return NULL;
    }
}
