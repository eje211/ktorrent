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

#include <KActionCollection>
#include <KLocalizedString>
#include <KMainWindow>
#include <KPluginFactory>
#include <QThread>

#include "htmlinterfaceplugin.h"
// #include <util/fileops.h>
// #include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
// #include <interfaces/torrentfileinterface.h>
#include <torrent/queuemanager.h>
#include "mongoose.h"
// #include "htmlinterfacemanager.h"
// #include "htmlinterfacedialog.h"
#include "webserver.h"



K_PLUGIN_FACTORY_WITH_JSON(ktorrent_htmlinterface, "ktorrent_htmlinterface.json", registerPlugin<kt::HtmlInterfacePlugin>();)

using namespace bt;
using namespace std;

namespace kt
{
    HtmlInterfacePlugin::HtmlInterfacePlugin(QObject* parent, const QVariantList &args): Plugin(parent)
    {
        Q_UNUSED(args);
        qInfo("I've still been constructed!");
//         download_order_action = new QAction(QIcon::fromTheme(QStringLiteral("view-sort-ascending")), i18n("File Download Order"), this);
//         connect(download_order_action, &QAction::triggered, this, &HtmlInterfacePlugin::showHtmlInterfaceDialog);
//         actionCollection()->addAction(QStringLiteral("download_order"), download_order_action);
//         setXMLFile(QStringLiteral("ktorrent_htmlinterfaceui.rc"));
//         managers.setAutoDelete(true);
    }


    HtmlInterfacePlugin::~HtmlInterfacePlugin()
    {
    }


    bool HtmlInterfacePlugin::versionCheck(const QString& version) const
    {
        return version == QStringLiteral(KT_VERSION_MACRO);
    }

    void HtmlInterfacePlugin::load()
    {
        qInfo("I've been loaded");
        WebServer * ws = new WebServer(getCore());
        ws->process();
//         HtmlInterfacePlugin::instance = this;
//         thread = new QThread;
//         worker = new WebServer(getCore());
//         worker->moveToThread(thread);
// //         connect(worker, SIGNAL (error(QString)), this, SLOT (errorString(QString)));
//         connect(thread, SIGNAL (started()), worker, SLOT (process()));
//         connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
//         connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
//         connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
//         thread->start();
    }

    void HtmlInterfacePlugin::unload()
    {
    }
 
    void HtmlInterfacePlugin::currentTorrentChanged(bt::TorrentInterface* tc)
    {
    }

}



#include <htmlinterfaceplugin.moc>
