/***************************************************************************
 *   Copyright (C) 2011-2019 by Hannah von Reth                            *
 *   vonreth@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SNORENOTIFICATIONBACKEND_H_
#define SNORENOTIFICATIONBACKEND_H_

#include <libsnore/notification/notification.h>
#include <libsnore/snore.h>

#include "abstractnotificationbackend.h"

class SystrayNotificationBackend;

class SnoreNotificationBackend : public QObject
{
    Q_OBJECT
public:
    enum NotificationType {
        Highlight = 0x01,
        PrivMsg = 0x02,
        HighlightFocused = 0x11,
        PrivMsgFocused = 0x12
    };

    struct Notification
    {
        uint notificationId;
        NotificationType type;
        QString sender;
        QString message;

        Notification(uint id_, NotificationType type_, QString sender_, QString msg_)
            : notificationId(id_)
            , type(type_)
            , sender(std::move(sender_))
            , message(std::move(msg_))
        {
        }
    };

    SnoreNotificationBackend(QObject *parent);
    ~SnoreNotificationBackend();

    void notify(const Notification &);
    void close(uint notificationId);

public slots:
    void actionInvoked(Snore::Notification);

private slots:
    void setTraybackend(const QVariant &b);

private:
#ifndef HAVE_KDE
    SystrayNotificationBackend *m_systrayBackend = nullptr;
#endif
    QHash<uint, uint> m_notificationIds;
    Snore::Icon m_icon;
    Snore::Application m_application;
    Snore::Alert m_alert;
};

#endif // SNORENOTIFICATIONBACKEND_H
