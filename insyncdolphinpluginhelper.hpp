/*****************************************************************************
 *   Copyright (C) 2026 by Olivier Booklage <obooklage@gmail.com>           *
 *   Copyright (C) 2025 by Tomáš Hnyk <tomashnyk@gmail.com>                  *
 *   Copyright (C) 2025 by Kevin B. Burns                                    *
 *   Copyright (C) 2021 by Kurt Ko <kurt@insynchq.com>                       *
 *   Copyright (C) 2014 by Luis Manuel R. Pugoy <lpugoy@insynchq.com>        *
 *   Copyright (C) 2014 by Emmanuel Pescosta <emmanuelpescosta099@gmail.com> *
 *   Copyright (C) 2012 by Sergei Stolyarov <sergei@regolit.com>             *
 *   Copyright (C) 2010 by Thomas Richard <thomas.richard@proan.be>          *
 *   Copyright (C) 2009-2010 by Peter Penz <peter.penz19@gmail.com>          *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA              *
 *****************************************************************************/

/*
 * insyncdolphinpluginhelper.hpp
 * Shared helper library for the Insync Dolphin plugins.
 *
 * Provides the connection logic and the JSON command protocol used
 * by both Dolphin plugins to communicate with the running Insync
 * daemon over its Unix domain socket (insync<uid>.sock).
 *
 * Communication architecture:
 *   Dolphin plugin  <-->  QLocalSocket  <-->  insync<uid>.sock  <-->  Insync daemon
 *
 * Consumers:
 *   - insyncfileitemaction (context-menu entries on right-click)
 *   - insyncoverlayicon    (overlay status icons on file entries)
 */

#ifndef INSYNCDOLPHINPLUGINHELPER_H
#define INSYNCDOLPHINPLUGINHELPER_H

#include "insyncdolphinpluginhelper_export.h"

#include <QObject>
#include <QLocalSocket>

/**
 * @brief Helper that talks to the local Insync daemon.
 *
 * Both Dolphin plugins use this helper to:
 *   - establish a connection to the user's insync.sock,
 *   - send a JSON command and optionally wait for a JSON reply.
 *
 * The QLocalSocket is owned by the caller; this class only handles
 * the connection and request/response logic on top of it.
 */
class INSYNCDOLPHINPLUGINHELPER_EXPORT InsyncDolphinPluginHelper : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Whether sendCommand() should wait for a reply.
     */
    enum SendCommandMode
    {
        /** Send the command and block until the daemon replies. */
        WaitForReply,
        /** Send the command and return immediately (fire-and-forget). */
        SendCommandOnly
    };

    /**
     * @brief Timeout category for socket waits.
     *
     * Concrete millisecond values are defined in the implementation file.
     */
    enum SendCommandTimeout
    {
        /** Short timeout, suitable for interactive operations. */
        ShortTimeout,
        /** Longer timeout, used when the daemon may be slow to respond. */
        LongTimeout
    };

    /**
     * @brief Connect the given socket to the user's Insync control socket.
     *
     * If the socket is already connected, this is a no-op and returns true.
     *
     * @param socket   The socket to connect (must be non-null).
     * @param timeout  How long to wait for the connection to succeed.
     * @returns true on success, false if the connection could not be established.
     */
    bool connectWithInsync(const QPointer<QLocalSocket> &socket,
                           SendCommandTimeout timeout = ShortTimeout) const;

    /**
     * @brief Send a JSON command to the Insync daemon over @p socket.
     *
     * If @p mode is WaitForReply, the function blocks until a reply is
     * received (or the timeout elapses) and returns the parsed reply.
     * Otherwise it returns an empty QVariant immediately after writing.
     *
     * @param command  The JSON command to send.
     * @param socket   An open socket (will be connected if not already).
     * @param mode     Whether to wait for and return a reply.
     * @param timeout  Timeout used for both connection and read waits.
     * @returns The parsed reply, or an empty QVariant on failure or fire-and-forget.
     */
    QVariant sendCommand(const QJsonObject &command,
                         const QPointer<QLocalSocket> &socket,
                         SendCommandMode mode = SendCommandOnly,
                         SendCommandTimeout timeout = ShortTimeout) const;
};

#endif // INSYNCDOLPHINPLUGINHELPER_H
