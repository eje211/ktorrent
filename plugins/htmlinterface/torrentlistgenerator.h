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

#ifndef KTTORRENTLISTXMLGENERATOR_H
#define KTTORRENTLISTXMLGENERATOR_H

#include <util/constants.h>
#include <interfaces/torrentinterface.h>

class QXmlStreamWriter;

using namespace bt;

namespace kt
{
    class CoreInterface;

    /**
        Content generator which generates XML with all torrents in it.
    */
    class TorrentListGenerator
    {
    public:
        TorrentListGenerator(CoreInterface* core);
        virtual ~TorrentListGenerator();

        QByteArray get();
        virtual void post(QString json);

    private:
        CoreInterface* core;        
        TorrentInterface * getTorrentFromHash(QString hash);

    };

}

#endif
