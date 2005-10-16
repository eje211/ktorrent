/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTCOREINTERFACE_H
#define KTCOREINTERFACE_H

#include <qstring.h>
#include <util/constants.h>

namespace kt
{
	class TorrentInterface;

	/**
	 * @author Joris Guisson
	 * @brief Interface for plugins to communicate with the application's core
	 *
	 * This interface provides the plugin with the functionality to modify
	 * the applications core, the core is responsible for managing all
	 * TorrentControl objects.
	*/
	class CoreInterface
	{
	public:
		CoreInterface();
		virtual ~CoreInterface();

		/**
		 * Set the maximum number of simultanious downloads.
		 * @param max The max num (0 == no limit)
		 */
		virtual void setMaxDownloads(int max) = 0;
	
		/**
		 * Set wether or not we should keep seeding after
		 * a download has finished.
		 * @param ks Keep seeding yes or no
		 */
		virtual void setKeepSeeding(bool ks) = 0;
	
		/**
		 * Change the data dir. This involves copying
		 * all data from the old dir to the new.
		 * This can offcourse go horribly wrong, therefore
		 * if it doesn't succeed it returns false
		 * and leaves everything where it supposed to be.
		 * @param new_dir The new directory
		 */
		virtual bool changeDataDir(const QString & new_dir) = 0;

		/**
		 * Start all, takes into account the maximum number of downloads.
		 */
		virtual void startAll() = 0;

		/**
		 * Stop all torrents.
		 */
		virtual void stopAll() = 0;

		/**
		 * Start a torrent, takes into account the maximum number of downloads.
		 * @param tc The TorrentControl
	 	 */
		virtual void start(TorrentInterface* tc) = 0;

		/**
		 * Stop a torrent, may start another download if it hasn't been started.
		 * @param tc The TorrentControl
		 */
		virtual void stop(TorrentInterface* tc) = 0;

		/**
		 * Switch the port when no torrents are running.
		 * @param port The new port
		 * @return true if we can, false if there are torrents running
		 */
		virtual bool changePort(bt::Uint16 port) = 0;

		///  Get the number of torrents running (including seeding torrents).
		virtual bt::Uint32 getNumTorrentsRunning() const = 0;

		///  Get the number of torrents not running.
		virtual bt::Uint32 getNumTorrentsNotRunning() const = 0;

		/**
		 * Load a torrent file. Pops up an error dialog
		 * if something goes wrong.
		 * @param file The torrent file
		 */
		virtual void load(const QString & file) = 0;
	
		/**
		 * Remove a download.This will delete all temp
		 * data from this TorrentControl And delete the
		 * TorrentControl itself. It can also potentially
		 * start a new download (when one is waiting to be downloaded).
		 * @param tc
		 */
		virtual void remove(TorrentInterface* tc) = 0;
	};

}

#endif
