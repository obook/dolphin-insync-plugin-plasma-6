/*****************************************************************************
 *   Copyright (C) 2026 by Olivier Booklage <obooklagek@gmail.com>           *
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
 * insyncfileitemaction.hpp
 * Insync context-menu plugin for Dolphin.
 *
 * Implements the KAbstractFileItemActionPlugin interface so that
 * Dolphin calls into this plugin whenever the user right-clicks on
 * a file or directory. The plugin asks the Insync daemon (through
 * InsyncDolphinPluginHelper) for the list of menu items applicable
 * to the selected path and exposes them in a KActionMenu submenu.
 *
 * Only single-selection is supported, since the Insync client does
 * not yet handle multi-file context menu queries.
 */

#ifndef INSYNCFILEITEMACTION_H
#define INSYNCFILEITEMACTION_H

#include <KAbstractFileItemActionPlugin>
#include <QPointer>

#include "insyncdolphinpluginhelper.hpp"

/**
 * @brief Insync implementation of KAbstractFileItemActionPlugin.
 *
 * Provides the right-click context-menu entries (Add to Insync,
 * Share, etc.) by querying the Insync daemon at right-click time
 * and building a KActionMenu submenu from its reply.
 */
class InsyncFileItemAction : public KAbstractFileItemActionPlugin
{
    Q_OBJECT

private:
    InsyncDolphinPluginHelper helper;
    QPointer<QLocalSocket> controlSocket;

public:
    /**
     * @brief Construct the plugin and open the control socket.
     *
     * @param parent  Parent QObject (passed by the KDE plugin loader).
     * @param args    Plugin arguments (unused).
     */
    InsyncFileItemAction(QObject *parent, const QVariantList &args);

    ~InsyncFileItemAction() override;

    /**
     * @brief Return the context-menu actions for the selected items.
     *
     * Called by Dolphin every time the user right-clicks. Only
     * single-item selections produce actions; otherwise an empty
     * list is returned and no Insync entries are added.
     *
     * @param fileItemInfos  Information about the selected items.
     * @param parentWidget   The widget that owns the menu (unused).
     * @returns A list containing the Insync KActionMenu, or empty.
     */
    QList<QAction *> actions(const KFileItemListProperties &fileItemInfos,
                             QWidget *parentWidget) override;

private Q_SLOTS:
    /**
     * @brief Forward a context-menu activation to the Insync daemon.
     *
     * @param action  The JSON command associated with the chosen menu entry.
     */
    void handleContextAction(const QJsonObject &action);

private:
    /**
     * @brief Build the Insync submenu for a single selected path.
     *
     * Sends a CONTEXT-MENU-ITEMS command to the daemon and converts
     * the returned title and entries into a KActionMenu.
     *
     * @param url  Local path of the selected file or directory.
     * @returns A list containing the KActionMenu, or empty if the
     *          daemon is not running or has nothing to offer.
     */
    QList<QAction *> getContextMenuActions(const QString &url);
};

#endif // INSYNCFILEITEMACTION_H
