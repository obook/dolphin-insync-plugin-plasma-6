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
 * insyncdolphinpluginhelper.cpp
 * Implementation of the connection and JSON command protocol used by
 * both Insync Dolphin plugins to talk to the local Insync daemon.
 */

#include <insyncdolphinpluginhelper.hpp>

#include <unistd.h>

#include <QDir>
#include <QPointer>
#include <QLocalSocket>
#include <QStringBuilder>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
    /* Timeout (ms) for short, interactive socket waits. */
    constexpr int SHORT_TIMEOUT_MS = 100;
    /* Timeout (ms) used when the daemon may be slow to respond. */
    constexpr int LONG_TIMEOUT_MS = 500;

    /*
     * Pick the actual millisecond value for a given timeout category.
     */
    int timeoutToMs(InsyncDolphinPluginHelper::SendCommandTimeout timeout)
    {
        return (timeout == InsyncDolphinPluginHelper::ShortTimeout)
            ? SHORT_TIMEOUT_MS
            : LONG_TIMEOUT_MS;
    }
}

bool InsyncDolphinPluginHelper::connectWithInsync(const QPointer<QLocalSocket> &socket,
                                                  SendCommandTimeout timeout) const
{
    /*
     * Defensive check: a QPointer becomes null automatically when
     * the underlying QObject is destroyed. Dereferencing a null
     * QPointer would crash the host process (Dolphin).
     */
    if (socket.isNull())
    {
        return false;
    }

    /*
     * The Insync daemon listens on a per-user socket named
     * insync<uid>.sock under the system temp directory.
     */
    QString socketFileName = QLatin1String("insync") % QString::number(getuid()) % QLatin1String(".sock");
    QString insyncSocketPath = QDir::tempPath() % QDir::separator() % socketFileName;
    QString socketPath = QDir::toNativeSeparators(insyncSocketPath);

    if (socket->state() != QLocalSocket::ConnectedState)
    {
        socket->connectToServer(socketPath);

        if (!socket->waitForConnected(timeoutToMs(timeout)))
        {
            socket->abort();
            return false;
        }
    }

    return true;
}

QVariant InsyncDolphinPluginHelper::sendCommand(const QJsonObject &command,
                                                const QPointer<QLocalSocket> &socket,
                                                SendCommandMode mode,
                                                SendCommandTimeout timeout) const
{
    /* Defensive null check before any pointer dereference. */
    if (socket.isNull())
    {
        return QVariant();
    }

    if (!connectWithInsync(socket, timeout))
    {
        return QVariant();
    }

    const QJsonDocument request(command);

    /*
     * Drain any leftover data from a previous exchange before
     * sending so that the next read returns only the new reply.
     */
    socket->readAll();
    socket->write(request.toJson());
    socket->flush();

    if (mode == SendCommandOnly)
    {
        return QVariant();
    }

    /*
     * Insync replies in a single chunk, so as soon as the socket
     * signals data ready we read everything and break out of the
     * loop. The while form is used to apply the timeout on the
     * initial wait.
     */
    QString reply;
    while (socket->waitForReadyRead(timeoutToMs(timeout)))
    {
        reply.append(QString::fromUtf8(socket->readAll()));
        break;
    }

    QJsonDocument jsonReply = QJsonDocument::fromJson(reply.toUtf8());
    if (jsonReply.toVariant().isNull())
    {
        /* Response was not JSON serializable, trim surrounding quotes */
        if (reply.startsWith(QStringLiteral("\"")))
        {
            reply.remove(0, 1);
        }
        if (reply.endsWith(QStringLiteral("\"")))
        {
            reply.remove(reply.size() - 1, 1);
        }

        return QVariant(reply.toUtf8());
    }
    return jsonReply.toVariant();
}
