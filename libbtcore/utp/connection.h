/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef UTP_CONNECTION_H
#define UTP_CONNECTION_H

#include <QPair>
#include <QMutex>
#include <QWaitCondition>
#include <btcore_export.h>
#include <net/address.h>
#include <utp/utpprotocol.h>
#include <util/circularbuffer.h>
#include <util/timer.h>
#include <utp/remotewindow.h>




namespace utp
{;
	class LocalWindow;
	
	/**
		Interface class for transmitting packets
	*/
	class BTCORE_EXPORT Transmitter
	{
	public:
		virtual ~Transmitter() {}
		
		/// Send a packet to some host
		virtual bool sendTo(const QByteArray & data,const net::Address & addr) = 0;
		
		/// Send a packet to some host
		virtual bool sendTo(const bt::Uint8* data,const bt::Uint32 size,const net::Address & addr) = 0;
	};

	/**
		Keeps track of a single UTP connection
	*/
	class BTCORE_EXPORT Connection : public Retransmitter
	{
	public:
		enum Type
		{
			INCOMING,
			OUTGOING
		};
		
		struct Stats
		{
			Type type;
			net::Address remote;
			ConnectionState state;
			bt::Uint16 send_connection_id;
			bt::Uint32 reply_micro;
			bt::Uint16 recv_connection_id;
			bt::Uint16 seq_nr;
			int eof_seq_nr;
			bt::Uint32 timeout;
			bt::Uint32 rtt;
			bt::Uint32 rtt_var;
			bt::Uint32 packet_size;
		};
		
		Connection(bt::Uint16 recv_connection_id,Type type,const net::Address & remote,Transmitter* transmitter);
		virtual ~Connection();
		
		/// Get the connection stats
		const Stats & connectionStats() const {return stats;}
		
		/// Handle a single packet
		ConnectionState handlePacket(const QByteArray & packet);
		
		/// Get the remote address
		const net::Address & remoteAddress() const {return stats.remote;}
		
		/// Get the receive connection id
		bt::Uint16 receiveConnectionID() const {return stats.recv_connection_id;}
		
		/// Send some data, returns the amount of bytes sent (or -1 on error)
		int send(const bt::Uint8* data,bt::Uint32 len);
		
		/// Read available data from local window, returns the amount of bytes read
		bt::Uint32 recv(bt::Uint8* buf,bt::Uint32 max_len);
		
		/// Get the connection state
		ConnectionState connectionState() const {return stats.state;}
		
		/// Get the type of connection
		Type connectionType() const {return stats.type;}
		
		/// Send a reset packet
		void sendReset();
		
		/// Get the number of bytes available
		bt::Uint32 bytesAvailable() const;
		
		/// Wait until the connectTo call fails or succeeds
		bool waitUntilConnected();
		
		/// Wait until there is data ready or the socket is closed
		bool waitForData();
		
		/// Close the socket
		void close();
		
		/// Update the RTT time
		virtual void updateRTT(const Header* hdr,bt::Uint32 packet_rtt);
		
		/// Retransmit a packet
		virtual int retransmit(const QByteArray & packet,bt::Uint16 p_seq_nr);
		
		/// Check for a timeout
		void checkTimeout();
		
	private:
		void sendSYN();
		void sendState();
		void sendFIN();
		void updateDelayMeasurement(const Header* hdr);
		void sendStateOrData();
		void sendPackets();
		int sendPacket(bt::Uint32 type,bt::Uint16 p_ack_nr);
		int sendDataPacket(const QByteArray & packet);
		
		/** 
			Parses the packet, and retrieves pointer to the header, the SelectiveAck extension (if present)
			@param packet The packet
			@param hdr The header pointer
			@param selective_ack The SelectiveAck pointer
			@return The offset of the data or -1 if there is no data
		*/
		int parsePacket(const QByteArray & packet,Header** hdr,SelectiveAck** selective_ack);
		
		
	private:
		Transmitter* transmitter;
		LocalWindow* local_wnd;
		RemoteWindow* remote_wnd;
		QList<QPair<bt::Uint32,bt::TimeStamp> > delay_window;
		bt::CircularBuffer output_buffer;
		bt::Timer timer;
		mutable QMutex mutex;
		QWaitCondition connected;
		QWaitCondition data_ready;
		Stats stats;
	};
}

#endif // UTP_CONNECTION_H