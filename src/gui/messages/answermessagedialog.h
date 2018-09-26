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

#ifndef ANSWERMESSAGEDIALOG_H
#define ANSWERMESSAGEDIALOG_H

#include <QDialog>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <sharee.h>
#include "ui_answermessagedialog.h"
#include "messageobject.h"

namespace OCC {

namespace Ui {
    class AnswerMessageDialog;
}

class AnswerMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnswerMessageDialog(Sharee currentUser, QString localPath = "", QSortFilterProxyModel *proxyModel = 0, QWidget *parent = 0);
    ~AnswerMessageDialog();

    /**
     * @brief setMessageObject
     * @param message
     */
    void setMessageObject(const MessageObject &message);

    /**
     * @brief set values of dialog
     * @param HTML of detail view
     */
    void setValues(QString values);

    /**
     * @brief reset dialog fields
     */
    void reset();

private:
    Ui::AnswerMessageDialog *ui;
    MessageObject message;
    Sharee currentUser;
    QString tempFile;
    QSortFilterProxyModel *filterProxy;
    QString basePath;

private slots:
    void on_sendAnswer_clicked();
};

} // end namespace
#endif // ANSWERMESSAGEDIALOG_H
