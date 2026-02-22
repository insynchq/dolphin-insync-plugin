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

#ifndef INSYNCDOLPHINPLUGINHELPER_H
#define INSYNCDOLPHINPLUGINHELPER_H

#include "insyncdolphinpluginhelper_export.h"

#include <QObject>
#include <QLocalSocket>

class INSYNCDOLPHINPLUGINHELPER_EXPORT InsyncDolphinPluginHelper : public QObject
{
    Q_OBJECT

private:
    QString controlSocketPath;

public:
    enum SendCommandMode
    {
        WaitForReply,
        SendCommandOnly
    };

    enum SendCommandTimeout
    {
        ShortTimeout,
        LongTimeout
    };

    bool connectWithInsync(const QPointer<QLocalSocket> &socket, SendCommandTimeout timeout = ShortTimeout) const;
    QVariant sendCommand(const QJsonObject &command,
                         const QPointer<QLocalSocket> &socket,
                         SendCommandMode mode = SendCommandOnly,
                         SendCommandTimeout timeout = ShortTimeout) const;
};

#endif // INSYNCDOLPHINPLUGINHELPER_H
