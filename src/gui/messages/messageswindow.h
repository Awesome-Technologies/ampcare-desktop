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

#ifndef MESSAGESWINDOW_H
#define MESSAGESWINDOW_H

#include <QMainWindow>

class QSortFilterProxyModel;

namespace OCC {

class MessageModel;

namespace Ui {
    class MessagesWindow;
}

/**
 * @brief The MessagesWindow class allows to view messages
 * @ingroup gui
 */
class MessagesWindow : public QMainWindow
{
    Q_OBJECT

public:
    MessagesWindow(const QString &_currentUser,
        const QString &localPath,
        QWidget *parent = 0);
    virtual ~MessagesWindow();

private:
    Ui::MessagesWindow *ui;
    MessageModel *messageModel;
    QSortFilterProxyModel *filterProxy;

    const QString currentUser;
    const QString localPath;

private slots:
    /** shows content of the message in the detailView */
    void slotShowDetails(const QModelIndex &current, const QModelIndex &previous);
};

} // namespace OCC

#endif
