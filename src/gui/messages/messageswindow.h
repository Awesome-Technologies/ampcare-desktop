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
#include <sharee.h>

class QSortFilterProxyModel;

namespace OCC {

class AnswerMessageDialog;
class CreateMessageDialog;
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
    MessagesWindow(const Sharee &_currentUser,
        const QString &localPath,
        const QVector<QSharedPointer<Sharee>> &recipientList,
        QWidget *parent = 0);
    virtual ~MessagesWindow();

private:
    Ui::MessagesWindow *ui;
    MessageModel *messageModel;
    QSortFilterProxyModel *filterProxy;
    AnswerMessageDialog *_answerMessageDialog;
    CreateMessageDialog *_createMessageDialog;

    const Sharee currentUser;
    const QString localPath;

private slots:
    /** model data changed */
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    /** shows content of the message in the detailView */
    void slotShowDetails(const QModelIndex &current, const QModelIndex &previous);

    /** item at @p index in message list got double clicked */
    void on_messageList_doubleClicked(const QModelIndex &index);

    /** show dialog to create a new message */
    void on_createMessageButton_clicked();
    void on_resolvedButton_clicked();
    void on_archiveButton_clicked();
    void on_videocallButton_clicked();

signals:
    void callRecipientChanged(QString callRecipient);
};

} // namespace OCC

#endif
