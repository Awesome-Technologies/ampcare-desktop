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
#include <QFileDialog>
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
    ui->listWidget_attachments->clear();
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

    // process attachement list
    for (int i = 0; i < ui->listWidget_attachments->count(); ++i) {
        const auto &currentItem = ui->listWidget_attachments->item(i);
        // get filename and full file path
        if (currentItem) {
            if (currentItem->data(Roles::PathRole).toString().endsWith("pdf", Qt::CaseInsensitive)) {
                message.newDocumentsList.append({ currentItem->text(), currentItem->data(Roles::PathRole).toString(), model->currentUser().shareWith() });
            } else {
                message.newImagesList.append({ currentItem->text(), currentItem->data(Roles::PathRole).toString(), model->currentUser().shareWith() });
            }
        }
    }

    model->setData(modelIndex, QVariant::fromValue(message), MessageModel::MessageObjectRole);
}

// add an image to the message
void AnswerMessageDialog::on_button_addAttachment_clicked()
{
    // restrict to images (jpg, png) and pdf documents
    QString filePath = QFileDialog::getOpenFileName(this, tr("Attach image or document"), model->rootPath(), "Images and Documents (*.jpg *.png *.pdf)");

    // check if filepath is empty, e.g. by cancelling the open file dialog
    if (filePath.isEmpty())
        return;

    // add name to listbox
    QListWidgetItem *newItem = new QListWidgetItem;
    QVariant fullFilePathData(filePath);
    QFileInfo fileinfo = QFileInfo(filePath);
    newItem->setData(Roles::PathRole, fullFilePathData);
    newItem->setData(Roles::IsNewRole, true);
    newItem->setText(fileinfo.fileName());
    int row = ui->listWidget_attachments->row(ui->listWidget_attachments->currentItem());
    ui->listWidget_attachments->insertItem(row, newItem);
}

// remove image from message
void AnswerMessageDialog::on_button_deleteAttachment_clicked()
{
    QList<QListWidgetItem *> items = ui->listWidget_attachments->selectedItems();
    for (const QListWidgetItem *item : items) {
        delete ui->listWidget_attachments->takeItem(ui->listWidget_attachments->row(item));
    }
}

} //end namespace
