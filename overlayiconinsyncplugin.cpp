/*****************************************************************************
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

#include "overlayiconinsyncplugin.h"
#include "insyncdolphinpluginhelper.h"

#include <KFileItem>
#include <KPluginFactory>

#include <QFileInfo>
#include <QPointer>
#include <QLocalSocket>
#include <QJsonObject>

QStringList OverlayIconInsyncPlugin::getOverlays(const QUrl &url)
{
    if (!url.isLocalFile())
    {
        return QStringList();
    }

    QString status = getFileStatus(url.toLocalFile());

    QStringList overlays;
    if (status == "SYNCED")
    {
        overlays << "emblem-insync-synced";
    }
    else if (status == "SYNCING")
    {
        overlays << "emblem-insync-syncing";
    }
    else if (status == "ERROR")
    {
        overlays << "emblem-insync-error";
    }

    return overlays;
}

QString OverlayIconInsyncPlugin::getFileStatus(const QString &url) const
{
    QJsonObject command = QJsonObject();
    command.insert("command", "GET-FILE-STATUS");
    command.insert("full_path", QFileInfo(url).canonicalFilePath());

    QPointer<QLocalSocket> itemStateSocket = new QLocalSocket;
    const QVariant reply = helper->sendCommand(command, itemStateSocket, InsyncDolphinPluginHelper::WaitForReply);
    delete itemStateSocket;

    return reply.toString();
}

#include "overlayiconinsyncplugin.moc"
