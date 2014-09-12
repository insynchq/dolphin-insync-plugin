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

#ifndef DOLPHININSYNCPLUGIN_H
#define DOLPHININSYNCPLUGIN_H

#include <kversioncontrolplugin.h>
#include <qjson/parser.h>
#include <qjson/serializer.h>

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QSignalMapper>
#include <QTimer>

/**
 * @brief Insync implementation for the KVersionControlPlugin interface.
 */
class DolphinInsyncPlugin : public KVersionControlPlugin
{
    Q_OBJECT

public:
    DolphinInsyncPlugin(QObject* parent, const QList<QVariant>& args);
    virtual ~DolphinInsyncPlugin();
    virtual QString fileName() const;
    virtual bool beginRetrieval(const QString& directory);
    virtual void endRetrieval();
    virtual KVersionControlPlugin::VersionState versionState(const KFileItem& item);
    virtual QList<QAction*> contextMenuActions(const KFileItemList& items);
    virtual QList<QAction*> contextMenuActions(const QString& directory);

private slots:
    void checkIfUpdateNeeded();
    void doAction(QString args);

private:
    QVariant sendCommand(QByteArray string, bool inGuiThread = true);
    QList<QAction*> getActions(QString url);
    QDateTime getLastUpdate();

    QHash<QString, VersionState> m_versionInfoHash;

    QDir m_dbDir;
    QDateTime m_lastUpdate;
    QString m_socketpath;
    QTimer* m_timer;
    QJson::Serializer m_serializer;
    QJson::Parser m_parser;
};
#endif // DOLPHININSYNCPLUGIN_H
