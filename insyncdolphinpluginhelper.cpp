/*****************************************************************************
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

#include <insyncdolphinpluginhelper.hpp>

#include <unistd.h>

#include <QDir>
#include <QPointer>
#include <QLocalSocket>
#include <QStringBuilder>
#include <QJsonDocument>
#include <QJsonObject>

bool InsyncDolphinPluginHelper::connectWithInsync(const QPointer<QLocalSocket> &socket,
                                                  SendCommandTimeout timeout) const
{
    QString socketFileName = QLatin1String("insync") % QString::number(getuid()) % QLatin1String(".sock");
    QString insyncSocketPath = QDir::tempPath() % QDir::separator() % socketFileName;
    QString controlSocketPath = QDir::toNativeSeparators(insyncSocketPath);

    if (socket->state() != QLocalSocket::ConnectedState)
    {
        socket->connectToServer(controlSocketPath);

        if (!socket->waitForConnected(timeout == ShortTimeout ? 100 : 500))
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
    if (!connectWithInsync(socket, timeout))
    {
        return QVariant();
    }

    const QJsonDocument *request = new QJsonDocument(command);

    socket->readAll();
    socket->write(request->toJson());
    socket->flush();

    if (mode == SendCommandOnly)
    {
        return QVariant();
    }

    QString reply;
    while (socket->waitForReadyRead(timeout == ShortTimeout ? 100 : 500))
    {
        reply.append(QString::fromUtf8(socket->readAll()));
        break;
    }

    QJsonDocument jsonReply = QJsonDocument::fromJson(reply.toUtf8());
    if (jsonReply.toVariant().isNull())
    {
        // Response wasn't JSON serializable

        // Trim quotes
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
