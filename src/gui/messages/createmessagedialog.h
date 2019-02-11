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

#ifndef MESSAGECREATEDIALOG_H
#define MESSAGECREATEDIALOG_H

#include <QDialog>
#include <QPersistentModelIndex>
#include <QUuid>
#include <sharee.h>

#include "messageobject.h"

namespace OCC {
class MessageModel;

namespace Ui {
    class CreateMessageDialog;
}

class CreateMessageDialog : public QDialog
{
    Q_OBJECT

public:
    enum Roles {
        PathRole = Qt::UserRole,
        IsNewRole
    };

    CreateMessageDialog(const QVector<QSharedPointer<Sharee>> &recipientList,
        MessageModel *model, QWidget *parent = 0);
    virtual ~CreateMessageDialog();

    /** fills fields of form with values from previous draft */
    void setModelIndex(const QPersistentModelIndex &index);

    /** resets all fields of the form */
    void reset();

private:
    Ui::CreateMessageDialog *ui;

    /**
     * @brief creates message object to write message to file
     * @param isDraft - should be saved in drafts folder
     */
    void saveMessage(bool isDraft);

    MessageModel *model;
    QPersistentModelIndex modelIndex;
    QUuid currentMessageId;
    QStringList deleteList;

    QDate dBirthday;
    QDateTime dtBp;
    QDateTime dtPulse;
    QDateTime dtTemp;
    QDateTime dtSugar;
    QDateTime dtWeight;
    QDateTime dtDefac;
    QDateTime dtOxy;

private slots:
    void on_sendMessage_clicked();
    void on_storeMessage_clicked();
    void on_button_addAttachment_clicked();
    void on_button_deleteAttachment_clicked();
    void on_deleteRow_clicked();
    void on_addRow_clicked();
    void on_dateEditBirthday_dateChanged(QDate date);
    void on_dateTime_bloodpressure_dateTimeChanged(QDateTime date);
    void on_dateTime_bloodsugar_dateTimeChanged(QDateTime date);
    void on_dateTime_bodyweight_dateTimeChanged(QDateTime date);
    void on_dateTime_lastDefecation_dateTimeChanged(QDateTime date);
    void on_dateTime_pulse_dateTimeChanged(QDateTime date);
    void on_dateTime_temperature_dateTimeChanged(QDateTime date);
    void on_dateTime_oxygen_dateTimeChanged(QDateTime date);

signals:
    void listUpdated();
};
}
#endif // MESSAGECREATEDIALOG_H
