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
#include <libutil/log.h>
#include <libutil/error.h>
#include "peermanager.h"
#include "peer.h"
#include "bnode.h"
#include "globals.h"
#include "server.h"
#include "authenticate.h"
#include "torrent.h"
#include "uploader.h"
#include "downloader.h"
#include <libutil/functions.h>
#include <qhostaddress.h> 
#include <klocale.h>

namespace bt
{
	Uint32 PeerManager::max_connections = 0;

	PeerManager::PeerManager(Torrent & tor) : tor(tor)
	{
		num_seeders = num_leechers = num_pending = 0;
		killed.setAutoDelete(true);
		started = false;
		Globals::instance().getServer().addPeerManager(this);
	}


	PeerManager::~PeerManager()
	{
		Globals::instance().getServer().removePeerManager(this);
		pending.setAutoDelete(true);
		//peer_map.setAutoDelete(true);
		peer_list.setAutoDelete(true);
	}

	void PeerManager::update()
	{
		if (!started)
			return;

		// update the speed of each peer,
		// and get ridd of some killed peers
		QPtrList<Peer>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
			if (p->isKilled())
			{
				i = peer_list.erase(i);
				killed.append(p);
				peer_map.erase(p->getID());
				peerKilled(p);
			}
			else
			{
				p->updateSpeed();
				i++;
			}
		}

		// check all pending connections, wether they authenticated
		// properly
		QPtrList<Authenticate>::iterator j = pending.begin();
		while (j != pending.end())
		{
			Authenticate* a = *j;
			if (a->isFinished())
			{
				j = pending.erase(j);
				peerAuthenticated(a,a->isSuccesfull());
				delete a;
			}
			else
			{
				j++;
			}
		}

		// connect to some new peers
		connectToPeers();
	}

	void PeerManager::killChokedPeers(Uint32 older_then)
	{
		Out() << "Getting rid of peers which have been choked for a long time" << endl;
		Uint32 now = bt::GetCurrentTime();
		QPtrList<Peer>::iterator i = peer_list.begin();
		Uint32 num_killed = 0;
		while (i != peer_list.end() && num_killed < 20)
		{
			Peer* p = *i;
			if (p->isChoked() && (now - p->getChokeTime()) > older_then)
			{
				p->kill();
				num_killed++;
			}

			i++;
		}
	}
	
	void PeerManager::setMaxConnections(Uint32 max)
	{
		max_connections = max;
	}
	
	void PeerManager::addPotentialPeer(const PotentialPeer & pp)
	{
		potential_peers.append(pp);
	}

	void PeerManager::killSeeders()
	{
		QPtrList<Peer>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
 			if ( p->isSeeder() )
			{
 				p->kill();
				i = peer_list.erase(i);
				killed.append(p);
				peer_map.erase(p->getID());
				peerKilled(p);
			}
			else
				i++;
		}
	}
	
	void PeerManager::newConnection(QSocket* sock,
									const PeerID & peer_id)
	{
		Uint32 total = peer_list.count() + pending.count();
		if (!started || (max_connections > 0 && total >= max_connections))
		{
			delete sock;
			return;
		}

		Peer* peer = new Peer(sock,peer_id,tor.getNumChunks());
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
		newPeer(peer);
	}
	
	void PeerManager::peerAuthenticated(Authenticate* auth,bool ok)
	{
		pending.remove(auth);
		num_pending--;
		if (!ok)
			return;
		
		if (connectedTo(auth->getPeerID()))
			return;
			
		Peer* peer = new Peer(
				auth->takeSocket(),auth->getPeerID(),tor.getNumChunks());
			
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
			
		//	Out() << "New peer connected !" << endl;
		newPeer(peer);
	}
		
	bool PeerManager::connectedTo(const PeerID & peer_id)
	{
		if (!started)
			return false;
		
		for (Uint32 j = 0;j < peer_list.count();j++)
		{
			Peer* p = peer_list.at(j);
			if (p->getPeerID() == peer_id)
			{
				return true;
			}
		}
		return false;
	}
	
	void PeerManager::connectToPeers()
	{
		if (peer_list.count() + pending.count() >= max_connections && max_connections > 0)
			return;
		
		Uint32 num = 0;
		if (max_connections > 0)
		{
			Uint32 available = max_connections - (peer_list.count() + pending.count());
			num = available >= potential_peers.count() ? 
					potential_peers.count() : available;
		}
		else
		{
			num = potential_peers.count();
		}

		if (pending.count() > 50)
			return;
		
		if (num > 0)
		{
			Out() << "Connecting to " << num << " peers (" 
					<< potential_peers.count() << ")" << endl;
		}
		
		for (Uint32 i = 0;i < num;i++)
		{
			if (pending.count() > 50)
				return;
			
			PotentialPeer pp = potential_peers.front();
			potential_peers.pop_front();
			
			if (connectedTo(pp.id))
				continue;
			
			Authenticate* auth = new Authenticate(pp.ip,pp.port,
					tor.getInfoHash(),tor.getPeerID());
			pending.append(auth);
			num_pending++;
		}
	}
	

	
	void PeerManager::clearDeadPeers()
	{
		killed.clear();
	}
	
	void PeerManager::closeAllConnections()
	{
		killed.clear();

		peer_map.clear();
		peer_list.setAutoDelete(true);
		peer_list.clear();
		peer_list.setAutoDelete(false);
		
		pending.setAutoDelete(true);
		pending.clear();
		pending.setAutoDelete(false);
	}
	
	void PeerManager::start()
	{
		started = true;
	}
		
	
	void PeerManager::stop()
	{
		started = false;
	}

	Peer* PeerManager::findPeer(Uint32 peer_id)
	{
		return peer_map.find(peer_id);
	}
}
#include "peermanager.moc"
