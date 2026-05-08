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
 * insyncoverlayicon.hpp
 * Insync overlay-icon plugin for Dolphin.
 *
 * Implements the KOverlayIconPlugin interface so that Dolphin asks
 * this plugin for status overlays (synced, syncing, error) on every
 * file or directory it displays. The plugin queries the Insync
 * daemon for each path through InsyncDolphinPluginHelper.
 *
 * Because getOverlays() is invoked for every visible item, a fresh
 * QLocalSocket is created on every call: sharing a single socket
 * across many concurrent queries was found to cause segfaults.
 */

#ifndef INSYNCOVERLAYICON_H
#define INSYNCOVERLAYICON_H

#include <KOverlayIconPlugin>

#include "insyncdolphinpluginhelper.hpp"

/**
 * @brief Insync implementation of KOverlayIconPlugin.
 *
 * Returns the list of emblem icons (synced / syncing / error) that
 * Dolphin draws on top of file thumbnails. Called by Dolphin once
 * per visible file or directory.
 */
class InsyncOverlayIcon : public KOverlayIconPlugin
{
    Q_PLUGIN_METADATA(IID "com.insync.overlayiconplugin" FILE "insyncoverlayicon.json")
    Q_OBJECT

private:
    InsyncDolphinPluginHelper helper;

public:
    /**
     * @brief Return the list of overlay emblems for a given URL.
     *
     * Only local files are queried; remote URLs return an empty list.
     *
     * @param url  URL of the file or directory to inspect.
     * @returns The emblem names to overlay (possibly empty).
     */
    QStringList getOverlays(const QUrl &url) override;

private:
    /**
     * @brief Ask the Insync daemon for the sync status of @p url.
     *
     * @param url  Local path to query.
     * @returns The status as a string (e.g. "SYNCED", "SYNCING",
     *          "ERROR"), or empty if the daemon is not running.
     */
    QString getFileStatus(const QString &url) const;
};

#endif // INSYNCOVERLAYICON_H
