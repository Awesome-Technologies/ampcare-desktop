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

#include "ui_answermessagedialog.h"
#include "answermessagedialog.h"
#include "messagemodel.h"
#include "messageobject.h"
#include <QDir>
#include <QPushButton>

namespace OCC {


AnswerMessageDialog::AnswerMessageDialog(MessageModel *model, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AnswerMessageDialog)
    , model(model)
{
    ui->setupUi(this);

    // change button texts
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Send"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Don't save"));

    // connect button signals to slots
    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(on_sendAnswer_clicked()));
}

AnswerMessageDialog::~AnswerMessageDialog()
{
    delete ui;
}

void AnswerMessageDialog::setModelIndex(const QPersistentModelIndex &index)
{
    modelIndex = index;
    QString values = index.data(MessageModel::DetailRole).toString();
    ui->detailView->setHtml(values.toUtf8(), QUrl("qrc:/"));
}

void AnswerMessageDialog::reset()
{
    modelIndex = QPersistentModelIndex();
    ui->plainTextEdit_messageBody->clear();
    ui->lineEdit_initials->clear();
    ui->label_requester->setText(model->currentUser().displayName());
}

void AnswerMessageDialog::on_sendAnswer_clicked()
{
    QString _initials = ui->lineEdit_initials->text();
    QString _text = ui->plainTextEdit_messageBody->toPlainText();

    MessageObject message = modelIndex.data(MessageModel::MessageObjectRole).value<MessageObject>();
    // combine old text and answer
    QString _oldBodyText = message.note;

    // change css class corresponding to author of the text
    QString _messageClass = (message.sender == model->currentUser().shareWith()) ? "messageSender" : "messageRecipient";
    QString _newBodyText = QString("%1<tr><td><div class='%2'>%3/%4</div></td><td class='messageBody'>%5</td><td class='messageDate'>%6</td></tr>")
                               .arg(_oldBodyText, _messageClass, model->currentUser().displayName(), _initials, _text, QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));

    // change status of message
    message.status = (message.sender == model->currentUser().shareWith()) ? MessageObject::SentStatus : MessageObject::ResentStatus;

    // write new message body
    message.note = _newBodyText;
    model->setData(modelIndex, QVariant::fromValue(message), MessageModel::MessageObjectRole);
}

} //end namespace