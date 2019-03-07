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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPushButton>
#include <QDateTime>
#include <QFileDialog>
#include <QCompleter>
#include <cmath>
#include <sharee.h>

#include "createmessagedialog.h"
#include "messagemodel.h"
#include "messageobject.h"
#include "messageswindow.h"
#include "ui_createmessagedialog.h"

namespace OCC {

CreateMessageDialog::CreateMessageDialog(const QVector<QSharedPointer<Sharee>> &recipientList,
    MessageModel *model, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateMessageDialog)
    , model(model)
{
    ui->setupUi(this);

    // fill recipient list - use displayName to show in the list, but shareWith to store messages
    ui->comboBox_recipient->clear();
    for (const auto &recipient : recipientList)
        ui->comboBox_recipient->addItem(recipient->displayName(), recipient->shareWith());

    // change button texts
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Send"));
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save as draft"));
    ui->buttonBox->button(QDialogButtonBox::Discard)->setText(tr("Don't save"));

    // connect button signals to slots
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(on_sendMessage_clicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(on_storeMessage_clicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Discard), SIGNAL(clicked()), this, SLOT(reject()));

    // restrict input fields
    // TODO validate restrictions
    ui->lineEdit_bpSys->setValidator(new QIntValidator(50, 300, this));
    ui->lineEdit_bpDia->setValidator(new QIntValidator(30, 200, this));
    ui->lineEdit_pulse->setValidator(new QIntValidator(0, 200, this));
    ui->lineEdit_temperature->setValidator(new QDoubleValidator(0, 100.0, 2, this));
    ui->lineEdit_bloodsugar->setValidator(new QIntValidator(0, 400, this));
    ui->lineEdit_bodyweight->setValidator(new QDoubleValidator(0, 400.0, 2, this));
    ui->lineEdit_oxygen->setValidator(new QDoubleValidator(0, 100.0, 2, this));

    // configure the rows and the columns on the medication table
    ui->tableWidget->setColumnCount(11);
    ui->tableWidget->setColumnWidth(0, 180); // 80 Zeichen
    ui->tableWidget->setColumnWidth(1, 140); // 50 Zeichen
    ui->tableWidget->setColumnWidth(2, 60); // 15 Zeichen
    ui->tableWidget->setColumnWidth(3, 40); // 7 Zeichen
    ui->tableWidget->setColumnWidth(4, 30); // 4 Zeichen
    ui->tableWidget->setColumnWidth(5, 30); // 4 Zeichen
    ui->tableWidget->setColumnWidth(6, 30); // 4 Zeichen
    ui->tableWidget->setColumnWidth(7, 30); // 4 Zeichen
    ui->tableWidget->setColumnWidth(8, 60); // 20 Zeichen
    ui->tableWidget->setColumnWidth(9, 180); // 80 Zeichen
    ui->tableWidget->setColumnWidth(10, 140); // 50 Zeichen
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    // TODO Item delegate for columns to add validates
    //ui->tableWidget->setItemDelegateForColumn(4, );
    //ui->tableWidget->setItemDelegateForColumn(5, );
    //ui->tableWidget->setItemDelegateForColumn(6, );
    //ui->tableWidget->setItemDelegateForColumn(7, );

    // TODO restrict 4-7 to int only
}

CreateMessageDialog::~CreateMessageDialog()
{
    delete ui;
}

void CreateMessageDialog::on_sendMessage_clicked()
{
    saveMessage(false);
}

void CreateMessageDialog::on_storeMessage_clicked()
{
    saveMessage(true);
}

void CreateMessageDialog::saveMessage(bool isDraft)
{
    MessageObject messageObject;

    messageObject.recipient = ui->comboBox_recipient->currentData().toString();
    // if no recipient was selected, save as draft
    if (messageObject.recipient == "")
        isDraft = true;

    // message metadata
    messageObject.priority = MessageObject::Priority(ui->comboBox_severity->currentIndex());
    messageObject.title = ui->lineEdit_messageTitle->text();
    messageObject.initials = ui->lineEdit_initials->text();
    // message metadata
    if (!ui->plainTextEdit_messageBody->toPlainText().isEmpty()) {
        messageObject.note = ui->plainTextEdit_messageBody->toPlainText();
        if (!isDraft) { // format message with html
            messageObject.note = "<tr><td><div class='messageSender'>" + model->currentUser().displayName() + "/" + messageObject.initials + "</div></td><td class='messageBody'>"
                + messageObject.note + "</td><td class='messageDate'>" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + "</td></tr>";
        }
    }
    if (!currentMessageId.isNull()) {
        messageObject.messageId = currentMessageId;
    }

    messageObject.recipientName = ui->comboBox_recipient->currentText();
    messageObject.sender = model->currentUser().shareWith();
    messageObject.senderName = model->currentUser().displayName();
    messageObject.status = isDraft ? MessageObject::DraftStatus : MessageObject::SentStatus;

    // patient data
    messageObject.patientName = ui->lineEdit_patientName->text();
    // check if date was set
    if (!dBirthday.isNull()) {
        messageObject.birthday = dBirthday;
    }
    messageObject.gender = MessageObject::Gender(ui->comboBox_gender->currentIndex());

    // vital data
    // bloodpressure
    if (!ui->lineEdit_bpSys->text().isEmpty() && !ui->lineEdit_bpDia->text().isEmpty()) {
        messageObject.bpSys = ui->lineEdit_bpSys->text().toInt();
        messageObject.bpDia = ui->lineEdit_bpDia->text().toInt();
        // check if date was set
        if (!dtBp.isNull()) {
            messageObject.dtBp = dtBp;
        }
    }
    // pulse
    if (!ui->lineEdit_pulse->text().isEmpty()) {
        messageObject.pulse = ui->lineEdit_pulse->text().toInt();
        // check if date was set
        if (!dtPulse.isNull()) {
            messageObject.dtPulse = dtPulse;
        }
    }
    // temperature
    if (!ui->lineEdit_temperature->text().isEmpty()) {
        // TODO convert according to locale
        messageObject.temp = ui->lineEdit_temperature->text().toDouble();
        // check if date was set
        if (!dtTemp.isNull()) {
            messageObject.dtTemp = dtTemp;
        }
    }
    // blood sugar
    if (!ui->lineEdit_bloodsugar->text().isEmpty()) {
        // TODO convert according to locale
        messageObject.sugar = ui->lineEdit_bloodsugar->text().toDouble();
        // check if date was set
        if (!dtSugar.isNull()) {
            messageObject.dtSugar = dtSugar;
        }
    }
    // weight
    if (!ui->lineEdit_bodyweight->text().isEmpty()) {
        // TODO convert according to locale
        messageObject.weight = ui->lineEdit_bodyweight->text().toDouble();
        // check if date was set
        if (!dtWeight.isNull()) {
            messageObject.dtWeight = dtWeight;
        }
    }
    // oxygen
    if (!ui->lineEdit_oxygen->text().isEmpty()) {
        // TODO convert according to locale
        messageObject.oxygen = ui->lineEdit_oxygen->text().toDouble();
        // check if date was set
        if (!dtOxy.isNull()) {
            messageObject.dtOxygen = dtOxy;
        }
    }

    if (!ui->lineEdit_responsiveness->text().isEmpty()) {
        messageObject.response = ui->lineEdit_responsiveness->text();
    }

    if (!ui->lineEdit_pain->text().isEmpty()) {
        messageObject.pain = ui->lineEdit_pain->text();
    }

    // check if date was set
    if (!dtDefac.isNull()) {
        messageObject.dtDefac = dtDefac;
    }

    if (!ui->lineEdit_misc->text().isEmpty()) {
        messageObject.misc = ui->lineEdit_misc->text();
    }

    // process attachement list
    for (int i = 0; i < ui->listWidget_attachments->count(); ++i) {
        const auto &currentItem = ui->listWidget_attachments->item(i);
        // get filename and full file path
        if (currentItem) {
            // attachment is a document
            if (currentItem->data(Roles::PathRole).toString().right(3).toLower() == "pdf") {
                if (currentItem->data(Roles::IsNewRole) == true) {
                    messageObject.newDocumentsList.append({ currentItem->text(), currentItem->data(Roles::PathRole).toString(), model->currentUser().shareWith() });
                } else {
                    messageObject.documentsList.append({ currentItem->text(), currentItem->data(Roles::PathRole).toString(), model->currentUser().shareWith() });
                }
            } else { // attachment is an image
                if (currentItem->data(Roles::IsNewRole) == true) {
                    messageObject.newImagesList.append({ currentItem->text(), currentItem->data(Roles::PathRole).toString(), model->currentUser().shareWith() });
                } else {
                    messageObject.imagesList.append({ currentItem->text(), currentItem->data(Roles::PathRole).toString(), model->currentUser().shareWith() });
                }
            }
        }
    }

    // process medication list
    QList<QStringList> medicationList;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QStringList medicationElement;
        for (int j = 0; j < ui->tableWidget->columnCount(); j++) {
            QTableWidgetItem *item(ui->tableWidget->item(i, j));
            medicationElement.append(item ? item->text() : "");
        }
        medicationList.append(medicationElement);
    }
    messageObject.medicationList = medicationList;

    model->setData(modelIndex, QVariant::fromValue(messageObject), MessageModel::MessageObjectRole);

    // delete removed attachments from assets folder
    for (QString item : deleteList) {
        if (QFileInfo::exists(item) && QFileInfo(item).isFile()) {
            QFile::remove(item);
        }
    }
    deleteList.clear();
}

// add an attachment to the message
void CreateMessageDialog::on_button_addAttachment_clicked()
{
    // restrict to images only
    QString filePath = QFileDialog::getOpenFileName(this, tr("Attach image or document"), model->rootPath(), "Images and Documents (*.jpg *.png *.pdf)");

    // check if filepath is empty, e.g. by cancelling the open file dialog
    if (!filePath.isEmpty()) {
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
}

// remove image from message
void CreateMessageDialog::on_button_deleteAttachment_clicked()
{
    QList<QListWidgetItem *> items = ui->listWidget_attachments->selectedItems();
    for (QListWidgetItem *item : items) {
        if (item->data(Roles::IsNewRole) != true) { // file was added before and has to be deleted
            deleteList.append(item->data(Roles::PathRole).toString());
        }
        delete ui->listWidget_attachments->takeItem(ui->listWidget_attachments->row(item));
    }
}

void CreateMessageDialog::setModelIndex(const QPersistentModelIndex &index)
{
    modelIndex = index;
    MessageObject message = index.data(MessageModel::MessageObjectRole).value<MessageObject>();
    // set current message id
    currentMessageId = message.messageId;

    // message metadata

    // set dropbox to correct selection
    QString _recipient = message.recipientName;
    int _index = ui->comboBox_recipient->findText(_recipient);
    if (_index != -1) { // -1 for not found
        ui->comboBox_recipient->setCurrentIndex(_index);
    }

    _index = message.priority;
    ui->comboBox_severity->setCurrentIndex(_index);

    ui->plainTextEdit_messageBody->setPlainText(message.note);
    ui->lineEdit_messageTitle->setText(message.title);

    ui->label_requester->setText(message.sender);
    ui->lineEdit_initials->setText(message.initials);

    // patient data
    ui->dateEditBirthday->setDate(message.birthday);
    dBirthday = message.birthday;
    ui->lineEdit_patientName->setText(message.patientName);
    ui->comboBox_gender->setCurrentIndex(message.gender == MessageObject::MALE ? 0 : 1);

    // vital data
    if (message.pulse) {
        ui->lineEdit_pulse->setText(QString::number(message.pulse));
        ui->dateTime_pulse->setDateTime(message.dtPulse);
        dtPulse = message.dtPulse;
    }

    if (!std::isnan(message.sugar)) {
        ui->lineEdit_bloodsugar->setText(QString::number(message.sugar));
        ui->dateTime_bloodsugar->setDateTime(message.dtSugar);
        dtBp = message.dtBp;
    }

    // TODO double _temp = v.toObject().value("valueQuantity").toObject().value("value").toDouble();
    if (!std::isnan(message.temp)) {
        ui->lineEdit_temperature->setText(QString::number(message.temp));
        ui->dateTime_temperature->setDateTime(message.dtTemp);
        dtTemp = message.dtTemp;
    }

    if (message.bpSys || message.bpDia) {
        ui->lineEdit_bpSys->setText(QString::number(message.bpSys));
        ui->lineEdit_bpDia->setText(QString::number(message.bpDia));
        ui->dateTime_bloodpressure->setDateTime(message.dtBp);
        dtBp = message.dtBp;
    }

    if (!std::isnan(message.weight)) {
        ui->lineEdit_bodyweight->setText(QString::number(message.weight));
        ui->dateTime_bodyweight->setDateTime(message.dtWeight);
        dtWeight = message.dtWeight;
    }

    if (!std::isnan(message.oxygen)) {
        ui->lineEdit_oxygen->setText(QString::number(message.oxygen));
        ui->dateTime_oxygen->setDateTime(message.dtOxygen);
        dtOxy = message.dtOxygen;
    }

    ui->dateTime_lastDefecation->setDateTime(message.dtDefac);
    dtDefac = message.dtDefac;

    ui->lineEdit_pain->setText(message.pain);

    ui->lineEdit_responsiveness->setText(message.response);

    ui->lineEdit_misc->setText(message.misc);

    // fill attachment list
    // images
    for (const MessageObject::AttachmentDetails &imageEntry : message.imagesList) {
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setData(Roles::PathRole, imageEntry.path);
        newItem->setData(Roles::IsNewRole, false);
        newItem->setText(imageEntry.name);
        int row = ui->listWidget_attachments->row(ui->listWidget_attachments->currentItem());
        ui->listWidget_attachments->insertItem(row, newItem);
    }
    // documents
    for (const MessageObject::AttachmentDetails &documentEntry : message.documentsList) {
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setData(Roles::PathRole, documentEntry.path);
        newItem->setData(Roles::IsNewRole, false);
        newItem->setText(documentEntry.name);
        int row = ui->listWidget_attachments->row(ui->listWidget_attachments->currentItem());
        ui->listWidget_attachments->insertItem(row, newItem);
    }

    // fill medication list
    for (int i = 0; i < message.medicationList.size(); i++) {
        const QStringList &_medicationEntry = message.medicationList.at(i);
        ui->tableWidget->insertRow(i);
        for (int j = 0; j <= 10; j++) {
            ui->tableWidget->setItem(i, j, new QTableWidgetItem(_medicationEntry[j]));
        }
    }
}

void CreateMessageDialog::reset()
{
    modelIndex = QPersistentModelIndex();
    // message metadata
    // reset message id
    currentMessageId = QUuid();
    ui->lineEdit_initials->clear();
    ui->label_requester->setText(model->currentUser().displayName());
    ui->comboBox_recipient->setCurrentIndex(-1);
    ui->comboBox_severity->setCurrentIndex(-1);
    ui->lineEdit_messageTitle->clear();
    ui->plainTextEdit_messageBody->clear();

    // patient data
    ui->comboBox_gender->setCurrentIndex(-1);
    ui->lineEdit_patientName->clear();

    dBirthday = QDate();
    dtBp = QDateTime();
    dtSugar = QDateTime();
    dtWeight = QDateTime();
    dtDefac = QDateTime();
    dtPulse = QDateTime();
    dtTemp = QDateTime();
    dtOxy = QDateTime();

    // vital data
    ui->dateTime_bloodpressure->setTime(QTime(0, 0));
    ui->dateTime_bloodpressure->setDate(QDate::currentDate());
    ui->dateTime_bloodsugar->setTime(QTime(0, 0));
    ui->dateTime_bloodsugar->setDate(QDate::currentDate());
    ui->dateTime_bodyweight->setTime(QTime(0, 0));
    ui->dateTime_bodyweight->setDate(QDate::currentDate());
    ui->dateTime_lastDefecation->setTime(QTime(0, 0));
    ui->dateTime_lastDefecation->setDate(QDate::currentDate());
    ui->dateTime_pulse->setTime(QTime(0, 0));
    ui->dateTime_pulse->setDate(QDate::currentDate());
    ui->dateTime_temperature->setTime(QTime(0, 0));
    ui->dateTime_temperature->setDate(QDate::currentDate());
    ui->dateTime_oxygen->setTime(QTime(0, 0));
    ui->dateTime_oxygen->setDate(QDate::currentDate());

    ui->lineEdit_bloodsugar->clear();
    ui->lineEdit_bodyweight->clear();
    ui->lineEdit_bpDia->clear();
    ui->lineEdit_bpSys->clear();
    ui->lineEdit_misc->clear();
    ui->lineEdit_pain->clear();
    ui->lineEdit_pulse->clear();
    ui->lineEdit_responsiveness->clear();
    ui->lineEdit_temperature->clear();
    ui->lineEdit_oxygen->clear();

    // images and medication list
    ui->listWidget_attachments->clear();
    ui->tableWidget->setRowCount(0);
}

// add a row to the medication table
void CreateMessageDialog::on_addRow_clicked()
{
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
}

// deletes selected row(s) of the medication table
void CreateMessageDialog::on_deleteRow_clicked()
{
    // multiple rows can be selected
    QModelIndexList selection = ui->tableWidget->selectionModel()->selectedRows();
    for (const QModelIndex &index : selection)
        ui->tableWidget->removeRow(index.row());
}


void CreateMessageDialog::on_dateEditBirthday_dateChanged(QDate date)
{
    dBirthday = date;
}

void CreateMessageDialog::on_dateTime_bloodpressure_dateTimeChanged(QDateTime date)
{
    dtBp = date;
}

void CreateMessageDialog::on_dateTime_bloodsugar_dateTimeChanged(QDateTime date)
{
    dtSugar = date;
}

void CreateMessageDialog::on_dateTime_bodyweight_dateTimeChanged(QDateTime date)
{
    dtWeight = date;
}

void CreateMessageDialog::on_dateTime_lastDefecation_dateTimeChanged(QDateTime date)
{
    dtDefac = date;
}

void CreateMessageDialog::on_dateTime_pulse_dateTimeChanged(QDateTime date)
{
    dtPulse = date;
}

void CreateMessageDialog::on_dateTime_temperature_dateTimeChanged(QDateTime date)
{
    dtTemp = date;
}

void CreateMessageDialog::on_dateTime_oxygen_dateTimeChanged(QDateTime date)
{
    dtOxy = date;
}

} //end namespace
