/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef INFOWIDGET_H
#define INFOWIDGET_H


#include "infowidgetbase.h"

class KTorrentMonitor;
class IWFileTreeDirItem;
class KPopupMenu; 
class QString;
class PeerView;
class ChunkDownloadView;
class QWidget;
class QHBoxLayout;


namespace kt
{
	class TorrentInterface;
}

class InfoWidget : public InfoWidgetBase
{
	Q_OBJECT

public:
	InfoWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
	virtual ~InfoWidget();
	
	///Show PeerView in main window
	void showPeerView(bool show);
	///Show ChunkDownloadView in main window
	void showChunkView(bool show);

public slots:
	void changeTC(kt::TorrentInterface* tc);
	void update();
    void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p); 

    ///preview slot 
    void contextItem(int id); 

private:
	void fillFileTree();
	void readyPreview(); 
	
private:
	KTorrentMonitor* monitor;
	kt::TorrentInterface* curr_tc;
	IWFileTreeDirItem* multi_root;
    KPopupMenu* context_menu; 
    QString preview_path; 
    int preview_id;
	QWidget* peer_page;
	PeerView* peer_view;
	QWidget* cd_page;
	ChunkDownloadView* cd_view;
};

#endif

