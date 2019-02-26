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

#include "snorenotificationbackend.h"

#include <iostream>

#include <QMetaObject>
#include <QtGlobal>
#include <QtGui>

#include <libsnore/notification/notification.h>
#include <libsnore/snore.h>

SnoreNotificationBackend::SnoreNotificationBackend(QObject *parent)
    : m_icon(QIcon(":/client/theme/amp/icon_c_good.png"))
{
    Snore::SnoreCore::instance().loadPlugins(Snore::SnorePlugin::Backend);
    m_application = Snore::Application("AMPCare", m_icon);
    m_application.hints().setValue("windows-app-id", "AMP.AMPCare");

    connect(&Snore::SnoreCore::instance(), &Snore::SnoreCore::actionInvoked, this, &SnoreNotificationBackend::actionInvoked);

    m_alert = Snore::Alert(tr("New Message"), m_icon);
    m_application.addAlert(m_alert);
    Snore::SnoreCore::instance().setDefaultApplication(m_application);
}

SnoreNotificationBackend::~SnoreNotificationBackend()
{
    Snore::SnoreCore::instance().deregisterApplication(m_application);
}

void SnoreNotificationBackend::notify(const Notification &n)
{
    QString title = QString(n.sender);
    QString message = QString(n.message);
    Snore::Notification noti(m_application, m_alert, title, message, m_icon, 0, Snore::Notification::Emergency);
    noti.hints().setValue("AMP_ID", n.notificationId);
    m_notificationIds.insert(n.notificationId, noti.id());
    Snore::SnoreCore::instance().broadcastNotification(noti);
}

void SnoreNotificationBackend::close(uint notificationId)
{
    Snore::Notification n = Snore::SnoreCore::instance().getActiveNotificationByID(m_notificationIds.take(notificationId));
    if (n.isValid()) { // Don't close the notification if it no longer exists.
        Snore::SnoreCore::instance().requestCloseNotification(n, Snore::Notification::Closed);
    }
}

void SnoreNotificationBackend::actionInvoked(Snore::Notification n)
{
    //emit activated(n.hints().value("AMP_ID").toUInt());
}

void SnoreNotificationBackend::setTraybackend(const QVariant &b)
{
    if (b.toBool()) {
        if (!Snore::SnoreCore::instance().aplications().contains(m_application.name())) {
            Snore::SnoreCore::instance().registerApplication(m_application);
        }
    } else {
        if (Snore::SnoreCore::instance().aplications().contains(m_application.name())) {
            Snore::SnoreCore::instance().deregisterApplication(m_application);
        }
    }
}
