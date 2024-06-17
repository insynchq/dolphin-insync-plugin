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

#include "fileitemactioninsyncplugin.h"
#include "insyncdolphinpluginhelper.h"

#include <KFileItem>
#include <KFileItemListProperties>
#include <KActionMenu>
#include <KPluginFactory>

#include <QPointer>
#include <QLocalSocket>
#include <QStringBuilder>
#include <QJsonObject>

K_PLUGIN_FACTORY(InsyncFileItemActionPluginFactory, registerPlugin<FileItemActionInsyncPlugin>();)
K_EXPORT_PLUGIN(InsyncFileItemActionPluginFactory("fileitemactioninsyncplugin"))

FileItemActionInsyncPlugin::FileItemActionInsyncPlugin(QObject *parent, const QVariantList &args) : KAbstractFileItemActionPlugin(parent)
{
    Q_UNUSED(args);

    controlSocket = new QLocalSocket(parent);
    helper->connectWithInsync(controlSocket);
}

FileItemActionInsyncPlugin::~FileItemActionInsyncPlugin()
{
    delete controlSocket;
}

QList<QAction *> FileItemActionInsyncPlugin::actions(const KFileItemListProperties &fileItemInfos,
                                                     QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    // The FileItemActionInsyncPlugin::contextMenuActions implemented by Luis only works when
    // you right click a single file/directory. The snippet below is a part of the code
    // to handle multiple files/directories selected when opening the context menu
    // for (const KFileItem& item : fileItemInfos.items()) {
    //     d->contextFilePaths << QDir(item.localPath()).canonicalPath();
    // }

    // For simplicity, let's just handle a single file for now
    if (fileItemInfos.items().size() > 1 || fileItemInfos.items().size() == 0)
    {
        return QList<QAction *>();
    }

    KFileItem item = fileItemInfos.items().at(0);
    return getContextMenuActions(item.url().path());
}

void FileItemActionInsyncPlugin::handleContextAction(const QJsonObject &action)
{
    helper->sendCommand(action, controlSocket);
}

QList<QAction *> FileItemActionInsyncPlugin::getContextMenuActions(const QString &url)
{
    QJsonObject command = QJsonObject();
    command.insert("command", "CONTEXT-MENU-ITEMS");
    command.insert("full_path", url);

    const QVariant reply = helper->sendCommand(command, controlSocket, InsyncDolphinPluginHelper::WaitForReply);

    if (reply.isNull())
        return QList<QAction *>();

    QList<QVariant> menuinfo = reply.toList();
    QString title = menuinfo.at(0).toString();
    QList<QVariant> menuitems = menuinfo.at(1).toList();

    QPointer<KActionMenu> topContextMenu = new KActionMenu(
        QIcon::fromTheme("insync"),
        title,
        this);

    for (int i = 0; i < menuitems.size(); i++)
    {
        QList<QVariant> commandinfo = menuitems.at(i).toList();
        QString text = commandinfo.at(0).toString();
        QString method = commandinfo.at(1).toString();

        if (text == "separator")
        {
            topContextMenu->addSeparator();
        }
        else
        {
            QAction *actionItem = new QAction(text, this);

            QJsonObject actionJson = QJsonObject();
            actionJson.insert("method", method);
            actionJson.insert("full_path", url);

            connect(actionItem, &QAction::triggered, [=] {
                handleContextAction(actionJson);
            });

            topContextMenu->addAction(actionItem);
        }
    }

    return QList<QAction *>{topContextMenu};
}

#include "fileitemactioninsyncplugin.moc"
