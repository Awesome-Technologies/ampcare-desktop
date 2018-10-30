/*
 * Copyright (C) by Michael Albert <michael.albert@awesome-technologies.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#ifndef AMP_THEME_H
#define AMP_THEME_H

#include "config.h"
#include "theme.h"
#include "version.h"

#include <QColor>
#include <QPixmap>
#include <QVariant>

namespace OCC {

/**
 * @brief The AMPTheme class
 * @ingroup libsync
 */
class AMPTheme : public Theme
{
public:
    AMPTheme()
        : Theme()
    {
    }

#ifndef TOKEN_AUTH_ONLY
    QVariant customMedia(CustomMediaType) Q_DECL_OVERRIDE
    {
        return QVariant();
    }
    QColor wizardHeaderBackgroundColor() const Q_DECL_OVERRIDE
    {
        return QColor("#0082c9");
    }
    QColor wizardHeaderTitleColor() const Q_DECL_OVERRIDE
    {
        return QColor("#ffffff");
    }
    QPixmap wizardHeaderLogo() const Q_DECL_OVERRIDE
    {
        return QPixmap(hidpiFileName(":/client/theme/amp/wizard_logo.png"));
    }
#endif

    QIcon folderOfflineIcon(bool sysTray, bool sysTrayMenuVisible) const Q_DECL_OVERRIDE
    {
        return themeIcon(QLatin1String("state-amp-offline"), sysTray, sysTrayMenuVisible);
    }

    // the client only supports one folder to sync. The Add-Button is removed
    // accordingly.
    bool singleSyncFolder() const Q_DECL_OVERRIDE { return true; }

    // the client is limited to one account only
    bool multiAccount() const Q_DECL_OVERRIDE { return false; }

    QString helpUrl() const Q_DECL_OVERRIDE
    {
        return QString::fromLatin1("https://docs.amp.care/desktop/%1.%2/")
            .arg(MIRALL_VERSION_MAJOR)
            .arg(MIRALL_VERSION_MINOR);
    }
    QString overrideServerUrl() const Q_DECL_OVERRIDE
    {
        return QLatin1String("https://oo.amp.care");
    }
    QString updateCheckUrl() const Q_DECL_OVERRIDE
    {
        return QLatin1String("https://updates.amp.care/client/");
    }

    QString about() const Q_DECL_OVERRIDE
    {
        QString devString;
        devString = tr("<p>Version %1. For more information please visit <a "
                       "href='%2'>%3</a>.</p>")
                        .arg(MIRALL_VERSION_STRING)
                        .arg("http://" MIRALL_STRINGIFY(APPLICATION_DOMAIN))
                        .arg(MIRALL_STRINGIFY(APPLICATION_DOMAIN));

        devString += tr("<p><small>By Klaas Freitag, Daniel Molkentin, "
                        "Jan-Christoph Borchardt, "
                        "Olivier Goffart, Markus Götz and others.</small></p>");

        devString += tr("<p>This release was supplied by the Awesome Technologies"
                        " Innovationslabor GmbH<br />"
                        "Copyright 2017-2018 Nextcloud GmbH<br />"
                        "Copyright 2012-2018 ownCloud GmbH</p>");

        devString += tr("<p>Licensed under the GNU General Public License (GPL) "
                        "Version 2.0 or any later version.</p>");

        devString += gitSHA1();

        return devString;
    }

    QString wizardUrlHint() const Q_DECL_OVERRIDE
    {
        return QLatin1String("https://oo.amp.care");
    }
};

} // namespace OCC
#endif // AMP_THEME_H
