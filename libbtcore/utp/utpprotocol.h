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

#ifndef UTP_UTPPROTOCOL_H
#define UTP_UTPPROTOCOL_H

#include <QtGlobal>
#include <QString>
#include <btcore_export.h>
#include <util/constants.h>


namespace utp
{
	/*
	UTP header:
	
	0       4       8               16              24              32
	+-------+-------+---------------+---------------+---------------+
	| ver   | type  | extension     | connection_id                 |
	+-------+-------+---------------+---------------+---------------+
	| timestamp_microseconds                                        |
	+---------------+---------------+---------------+---------------+
	| timestamp_difference_microseconds                             |
	+---------------+---------------+---------------+---------------+
	| wnd_size                                                      |
	+---------------+---------------+---------------+---------------+
	| seq_nr                        | ack_nr                        |
	+---------------+---------------+---------------+---------------+
	*/
	
	struct Header
	{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		unsigned int type:4;
		unsigned int version:4;
#elif Q_BYTE_ORDER == Q_BIG_ENDIAN
		unsigned int version:4;
		unsigned int type:4;
#else
#error "Endiannes not defined"
#endif
		bt::Uint8 extension;
		bt::Uint16 connection_id;
		bt::Uint32 timestamp_microseconds;
		bt::Uint32 timestamp_difference_microseconds;
		bt::Uint32 wnd_size;
		bt::Uint16 seq_nr;
		bt::Uint16 ack_nr;
	};
	
	struct SelectiveAck
	{
		bt::Uint8 extension;
		bt::Uint8 length;
		bt::Uint8 bitmask[4];
	};
	
	struct ExtensionBits
	{
		bt::Uint8 extension;
		bt::Uint8 length;
		bt::Uint8 extension_bitmask[8];
	};
	
	struct UnknownExtension
	{
		bt::Uint8 extension;
		bt::Uint8 length;
	};
	
	const bt::Uint8 SELECTIVE_ACK_ID = 1;
	const bt::Uint8 EXTENSION_BITS_ID = 2;
	
	// type field values
	const bt::Uint8 ST_DATA = 0;
	const bt::Uint8 ST_FIN = 1;
	const bt::Uint8 ST_STATE = 2;
	const bt::Uint8 ST_RESET = 3;
	const bt::Uint8 ST_SYN = 4;
	
	QString TypeToString(bt::Uint8 type);
	
	enum ConnectionState
	{
		CS_IDLE,
		CS_SYN_SENT,
		CS_CONNECTED,
		CS_FINISHED,
		CS_CLOSED
	};
	
	const bt::Uint32 MIN_PACKET_SIZE = 150;
	
	const bt::Uint32 DELAY_WINDOW_SIZE = 2*60*1000; // 2 minutes
	const bt::Uint32 CCONTROL_TARGET = 100;
	const bt::Uint32 MAX_CWND_INCREASE_PACKETS_PER_RTT = 8;
	
	// Test if a bit is acked
	BTCORE_EXPORT bool Acked(const SelectiveAck* sack,bt::Uint16 bit);
	
	// Turn on a bit in the SelectiveAck
	BTCORE_EXPORT void Ack(SelectiveAck* sack,bt::Uint16 bit);
}

#endif // UTP_UTPPROTOCOL_H