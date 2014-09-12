/***************************************************************************
 *   Copyright (C) 2014 by Luis Manuel R. Pugoy <lpugoy@insynchq.com>      *
 *   Copyright (C) 2010 by Thomas Richard <thomas.richard@proan.be>        *
 *   Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>             *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include <kaction.h>
#include <kactionmenu.h>
#include <kdemacros.h>
#include <kfileitem.h>
#include <klocalizedstring.h>
#include <sys/types.h>
#include <unistd.h>

#include <QByteArray>
#include <QDebug>
#include <QFileInfo>
#include <QFileInfoList>
#include <QLocalSocket>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>

#include "dolphin-insync-plugin.h"

K_PLUGIN_FACTORY(DolphinInsyncPluginFactory, registerPlugin<DolphinInsyncPlugin>();)
K_EXPORT_PLUGIN(DolphinInsyncPluginFactory("dolphin-insync-plugin"))

DolphinInsyncPlugin::DolphinInsyncPlugin(QObject* parent, const QList<QVariant>& args) :
    KVersionControlPlugin(parent),
    m_versionInfoHash()
{
    Q_UNUSED(args);

    m_socketpath = QDir::tempPath();
    m_socketpath.append("/insync" + QString::number(getuid()) + ".sock");
    m_socketpath = QDir::toNativeSeparators(m_socketpath);

    QString dbDirPath = QDir::home().path();
    dbDirPath.append("/.config/Insync/dbs");
    dbDirPath = QDir::toNativeSeparators(dbDirPath);
    m_dbDir = QDir(dbDirPath);

    QStringList filters;
    filters << "gd2-*.db" << "gd2-*.db-wal";
    m_dbDir.setNameFilters(filters);
    m_dbDir.setSorting(QDir::Time);

    m_lastUpdate = getLastUpdate();

    m_timer = new QTimer(this);
    m_timer->start(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkIfUpdateNeeded()));
}

DolphinInsyncPlugin::~DolphinInsyncPlugin()
{
}

QString DolphinInsyncPlugin::fileName() const
{
    return QString();
}

bool DolphinInsyncPlugin::beginRetrieval(const QString& directory)
{
    Q_ASSERT(directory.endsWith(QLatin1Char('/')));

    //Clear hashmap
    m_versionInfoHash.clear();

    //Get directory listing
    QDir dir(directory);
    QStringList files = dir.entryList();

    //Start at 2 because we're not interested in . and ..
    for(int i=2; i<files.size(); ++i)
    {
        QString filename = dir.absolutePath() + QDir::separator() + files.at(i);

        QVariantMap command;
        command.insert("command", "GET-FILE-STATUS");
        command.insert("full_path", filename);

        bool ok;
        QByteArray commandjson = m_serializer.serialize(command, &ok);

        //Error somehow in serializing command
        if(!ok)
            return true;

        QVariant result;
        result = sendCommand(commandjson, false);

        //The connection failed but we still return true to avoid annoying
        // "Failed to update version information"
        if(result.isNull())
            return true;

        QString state = result.toString();

        VersionState versionstate = UnversionedVersion;

        if(state == "SYNCED")
        {
            versionstate = NormalVersion;
        }
        else if(state == "SYNCING")
        {
            versionstate = UpdateRequiredVersion;
        }
        else if(state == "ERROR")
        {
            versionstate = ConflictingVersion;
        }

        m_versionInfoHash.insert(filename, versionstate);
    }

    return true;
}

void DolphinInsyncPlugin::endRetrieval()
{
}

KVersionControlPlugin::VersionState DolphinInsyncPlugin::versionState(const KFileItem& item)
{
    const QString itemUrl = item.localPath();
    if (m_versionInfoHash.contains(itemUrl))
    {
        return m_versionInfoHash.value(itemUrl);
    }

    return UnversionedVersion;
}

QList<QAction*> DolphinInsyncPlugin::contextMenuActions(const KFileItemList& items)
{
    Q_ASSERT(!items.isEmpty());

    if(items.size() > 1 || items.size() == 0)
    {
        QList<QAction*> emptyactions;
        return emptyactions;
    }

    KFileItem item = items.at(0);
    return getActions(item.url().path());
}

QList<QAction*> DolphinInsyncPlugin::contextMenuActions(const QString& directory)
{
    return getActions(directory);
}

void DolphinInsyncPlugin::checkIfUpdateNeeded()
{
    QDateTime lastUpdate = getLastUpdate();
    if(lastUpdate != m_lastUpdate)
    {
        m_lastUpdate = lastUpdate;
        emit versionStatesChanged();
    }
}

QDateTime DolphinInsyncPlugin::getLastUpdate()
{
    m_dbDir.refresh();
    QFileInfoList dbs = m_dbDir.entryInfoList();
    if(dbs.size() > 0)
        return dbs.at(0).lastModified();
    else
        return QDateTime();
}

void DolphinInsyncPlugin::doAction(QString args)
{
    QStringList splittedArgs = args.split('\n');
    QString method = splittedArgs.at(0);
    QString fullpath = splittedArgs.at(1);

    if(splittedArgs.size() > 2)
    {
        //recreate full path (somehow there's a newline in the file name)
        for(int i=2; i<splittedArgs.size(); i++)
        {
            fullpath += '\n' + splittedArgs.at(i);
        }
    }

    QVariantMap command;
    command.insert("method", method);
    command.insert("full_path", fullpath);

    bool ok;
    QByteArray commandjson = m_serializer.serialize(command, &ok);

    if(ok)
        sendCommand(commandjson);
}

//Re-entrant
QVariant DolphinInsyncPlugin::sendCommand(QByteArray command, bool inGuiThread)
{
    QVariant result;

    //If we're in the GuiThread we can't afford to wait long
    //In the worker thread we have bigger margins
    int waitTime = 500;
    if(inGuiThread)
        waitTime = 100;

    QLocalSocket socket;
    socket.connectToServer(m_socketpath);
    if(!socket.waitForConnected(waitTime))
    {
        socket.close();
        return result;
    }

    socket.write(command);
    socket.flush();

    if(!socket.waitForReadyRead(waitTime))
    {
        //If we have to wait this long, the socket probably isn't open anymore
        // (insync died or closed)
        socket.close();
        return result;
    }

    QByteArray reply = socket.readAll().trimmed();

    bool ok;

    result = m_parser.parse(reply, &ok);

    socket.close();
    return result;
}

QList<QAction*> DolphinInsyncPlugin::getActions(QString url)
{
    QList<QAction*> actions;

    QSignalMapper *signalMapper = new QSignalMapper(this);

    QVariantMap command;
    command.insert("command", "CONTEXT-MENU-ITEMS");
    command.insert("full_path", url);

    bool ok;
    QByteArray commandjson = m_serializer.serialize(command, &ok);

    //Error somehow in serializing command
    if(!ok)
        return actions;

    QVariant reply = sendCommand(commandjson);

    if(reply.isNull())
        return actions;

    QList<QVariant> menuinfo = reply.toList();
    QString title = menuinfo.at(0).toString();
    QList<QVariant> menuitems = menuinfo.at(1).toList();

    KActionMenu* contextmenu = new KActionMenu(title, this);

    for(int i=0; i<menuitems.size(); i++)
    {
        QList<QVariant> commandinfo = menuitems.at(i).toList();
        QString text = commandinfo.at(0).toString();
        QString command = commandinfo.at(1).toString();

        if(text == "separator")
        {
            contextmenu->addSeparator();
        }
        else
        {
            QAction* newaction = new KAction(text, this);
            connect(newaction, SIGNAL(triggered()),
                    signalMapper, SLOT(map()));
            QString args = command + "\n" + url;
            signalMapper->setMapping(newaction, args);
            contextmenu->addAction(newaction);
        }
    }

    connect(signalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(doAction(QString)));

    actions.append(contextmenu);

    return actions;
}
