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

#include "messageobject.h"
#include <sharee.h>

#include <cmath>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace OCC {

MessageObject::MessageObject()
    : messageId(QUuid::createUuid())
    , priority(InfoPriority)
    , status(DraftStatus)
    , bpSys(0)
    , bpDia(0)
    , pulse(0)
    , temp(std::numeric_limits<double>::quiet_NaN())
    , sugar(std::numeric_limits<double>::quiet_NaN())
    , weight(std::numeric_limits<double>::quiet_NaN())
{
}

MessageObject::MessageObject(const QString &_path)
    : priority(InfoPriority)
    , status(DraftStatus)
    , bpSys(0)
    , bpDia(0)
    , pulse(0)
    , temp(std::numeric_limits<double>::quiet_NaN())
    , sugar(std::numeric_limits<double>::quiet_NaN())
    , weight(std::numeric_limits<double>::quiet_NaN())
{
    path = QFileInfo(_path).absolutePath();
    QFile file(_path);
    file.open(QIODevice::ReadOnly);
    setJson(QJsonDocument::fromJson(file.readAll()).object());
}

QIcon MessageObject::priorityIcon() const
{
    QString _priorityIcon = "icon_d_info.png";

    // set correct icon for priority
    switch (priority) {
    case InfoPriority:
        _priorityIcon = "icon_d_info.png";
        break;
    case GoodPriority:
        _priorityIcon = "icon_c_good.png";
        break;
    case UrgentPriority:
        _priorityIcon = "icon_b_urgent.png";
        break;
    case CriticalPriority:
        _priorityIcon = "icon_a_critical.png";
        break;
    }

    return QIcon(QString(":/client/theme/amp/%1").arg(_priorityIcon));
}

QIcon MessageObject::statusIcon() const
{
    // set image for message status
    QString _statusIcon = "icon_a_empty";

    switch (status) {
    case DraftStatus:
        _statusIcon = "icon_a_empty";
        break;
    case SentStatus:
        _statusIcon = "icon_b_sent.png";
        break;
    case ReadStatus:
        _statusIcon = "icon_c_read.png";
        break;
    case ResentStatus:
        _statusIcon = "icon_d_resent.png";
        break;
    case RereadStatus:
        _statusIcon = "icon_e_reread.png";
        break;
    case ResolvedStatus:
        _statusIcon = "icon_f_resolved.png";
        break;
    }

    return QIcon(QString(":/client/theme/amp/%1").arg(_statusIcon));
}

QString MessageObject::shortTitle() const
{
    // trim messageBody to generate a preview
    QString _shortTitle = note;
    // TODO refine regex
    QRegularExpression re(".*<td class='messageBody'>([0-9a-zA-Z]*)</td>.*");
    QRegularExpressionMatch match = re.match(_shortTitle);
    _shortTitle = match.captured(1);

    if (_shortTitle.length() > TEXT_PREVIEW_LENGTH)
        _shortTitle = _shortTitle.left(TEXT_PREVIEW_LENGTH) + " ...";
    return QString("<b>%1</b><br/><span>%2</span>").arg(title, _shortTitle);
}

QString MessageObject::longTitle() const
{
    return QString("<table border-spacing=15px>"
                   "<tr><td><b>%1</b></td></tr>"
                   "<tr><td style='padding-left:10px'>%2</td></tr>"
                   "</table>")
        .arg(title, note);
}

QString MessageObject::details() const
{
    static const char *GREEN = "#27AE60";
    static const char *RED = "#E74C3C";
    static const char *YELLOW = "#F1C40F";
    static const char *BLUE = "#3498DB";

    QString genderText = QObject::tr(gender == FEMALE ? "female" : "male");

    // urgencyColor sets interface color corresponding to the messages priority
    QString urgencyColor = BLUE;

    switch (priority) {
    case InfoPriority:
        urgencyColor = BLUE;
        break;
    case GoodPriority:
        urgencyColor = GREEN;
        break;
    case UrgentPriority:
        urgencyColor = YELLOW;
        break;
    case CriticalPriority:
        urgencyColor = RED;
        break;
    }

    // build html with data contents
    QString style = QString("<style>"
                            "html {font-family:Segoe UI,Frutiger,Frutiger Linotype,Dejavu Sans,Helvetica Neue,Arial,sans-serif;font-size:14px;font-weight:400}"
                            "h1 {font-size:24px;font-weight:700}"
                            "h3 {font-weight:700}"
                            "table.basicTable {width:100%;border-collapse:collapse}"
                            "table.basicTable td:nth-child(n+2) {border-left:1px solid rgba(0,0,0,.15);padding-left:4px}"
                            "table.basicTable tr:first-child td {font-weight:700;border-left:0}"
                            "div.header {float:left;margin-top:10px}"
                            "div.header h1 {margin:0}"
                            "div.headerRight {margin-top:10px;padding-top:6px;float:right}"
                            "div.patientInfo h3 {margin:0}"
                            "hr.seperator {background-color:rgba(0,0,0,.15);height:1px;border:0}"
                            "div.topBar {position:fixed;top:0;left:0;width:100%;height:10px;background-color:%1}"
                            ".segmentBody {-webkit-border-radius:9px;padding:10px;background-color:#f8f8f8}"
                            ".segmentBodyColoredBorder {-webkit-border-radius:9px;padding-left:5px;padding-right:5px;background-color:%1}"
                            ".messageTable td {vertical-align:top;}"
                            ".messageSender, .messageRecipient {-webkit-border-radius:2px;padding:0px 3px 3px 3px;white-space:nowrap;color:#fff;}"
                            ".messageSender {background-color:#9b59b6;}"
                            ".messageRecipient {background-color:#34495e;}"
                            ".messageBody {padding:0px 10px;width:100%;}"
                            ".messageDate {padding:0px;color:#858585;font-style:italic;white-space:nowrap;}"
                            "img.zoom {transition: transform 0.2s; -webkit-transition: transform 0.2s;}"
                            "img.zoom:hover {cursor: zoom-in; cursor: -webkit-zoom-in; transform: scale(4.0,4.0); -webkit-transform: scale(4.0,4.0);}"
                            "</style>")
                        .arg(urgencyColor);

    QString html = "<html><head>" + style + "</head><body>";

    // add patients data
    html += "<div class='topBar'></div>";
    html += "<div class='header'><h1>" + title + "</h1>";
    html += "<a>" + QObject::tr("Created") + ": " + authoredOn.toString("dd.MM.yyyy hh:mm") + " " + QObject::tr("o'clock") + "</a></div>";
    html += "<div class='headerRight'><a>" + recipient + "<br>" + senderName + " / " + initials + "</a></div>";
    html += "<div style='clear:both;'></div>"
            "<hr class='seperator' noshade />";
    html += "<div class='patientInfo'><h3>" + patientName + "</h3><a>" + genderText + "<br>" + QObject::tr("Birthdate") + " " + birthday.toString("dd.MM.yyyy") + "</a></div>";
    html += "<hr class='seperator' noshade />";
    html += "<h1>" + QObject::tr("Messages") + "</h1>";
    html += "<div class='segmentBodyColoredBorder'><div class='segmentBody'><table class='messageTable'>" + note + "</table></div></div>";

    // add images
    if (imagesList.size() > 0) {
        html += "<h1>" + QObject::tr("Images") + "</h1><div class='segmentBodyColoredBorder'><div class='segmentBody'>";
        html += "<div class='imgcontainer'>";
        for (const auto &image : imagesList) {
            html += QString("<img class='zoom' width='200' src='file://%1'>").arg(image.path);
        }
        html += "</div></div></div>";
    }

    // add vital data / observations
    html += "<div style='float: left; width: 49%;'>";
    html += "<h1>" + QObject::tr("Vital parameter") + "</h1>";
    html += "<div class='segmentBodyColoredBorder'><div class='segmentBody'><table class='basicTable'><tbody>";
    html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr><tr><td>%4</td>")
                .arg(QObject::tr("Parameter"), QObject::tr("Value"), QObject::tr("Timestamp"), QObject::tr("Bloodpressure"));

    QString _bpSys = "-", _bpDia = "-", _dtBp = "-";
    if (bpSys && bpDia) {
        _bpSys = QString::number(bpSys);
        _bpDia = QString::number(bpDia);
        _dtBp = dtBp.toString("dd.MM.yyyy hh:mm");
    }
    html += QString("<td>%1/%2 mmHg</td><td>%3 %4</td></tr>").arg(_bpSys, _bpDia, _dtBp, QObject::tr("o'clock"));

    QString _pulse = "-", _dtPulse = "-", _temp = "-", _dtTemp = "-";
    if (pulse) {
        _pulse = QString::number(pulse);
        _dtPulse = dtPulse.toString("dd.MM.yyyy hh:mm");
    }
    if (!std::isnan(temp)) {
        _temp = QString::number(temp);
        _dtTemp = dtTemp.toString("dd.MM.yyyy hh:mm");
    }
    html += QString("<tr><td>%2</td><td>%4 bpm</td><td>%5 %1</td></tr><tr><td>%3</td><td>%6 °C</td><td>%7 %1</td></tr>")
                .arg(QObject::tr("o'clock"), QObject::tr("Pulse"), QObject::tr("Body temperature"))
                .arg(_pulse, _dtPulse, _temp, _dtTemp);

    QString _sugar = "-", _dtSugar = "-", _weight = "-", _dtWeight = "-";
    if (!std::isnan(sugar)) {
        _sugar = QString::number(sugar);
        _dtSugar = dtSugar.toString("dd.MM.yyyy hh:mm");
    }
    if (!std::isnan(weight)) {
        _weight = QString::number(weight);
        _dtWeight = dtWeight.toString("dd.MM.yyyy hh:mm");
    }
    html += QString("<tr><td>%2</td><td>%4 mg/dl</td><td>%5 %1</td></tr><tr><td>%3</td><td>%6 kg</td><td>%7 %1</td></tr>")
                .arg(QObject::tr("o'clock"), QObject::tr("Blood sugar"), QObject::tr("Weight"))
                .arg(_sugar, _dtSugar, _weight, _dtWeight);
    html += "</tbody></table></div></div></div><div style='float: right; width: 49%'>";

    html += "<h1>" + QObject::tr("Observations") + "</h1>";
    html += "<div class='segmentBodyColoredBorder'><div class='segmentBody'>"
            "<table class='basicTable'><tbody>";
    html += "<tr><td>" + QObject::tr("Parameter") + "</td><td>" + QObject::tr("Value") + "</td></tr>";
    QString _response = response != "" ? response : "-";
    html += "<tr><td>" + QObject::tr("Responsiveness") + "</td><td>" + _response + "</td></tr>";
    QString _pain = pain != "" ? pain : "-";
    html += "<tr><td>" + QObject::tr("Pain") + "</td><td>" + _pain + "</td></tr>";
    html += "<tr><td>" + QObject::tr("Last defecation") + "</td><td>" + dtDefac.toString("dd.MM.yyyy hh:mm") + " " + QObject::tr("o'clock") + "</td></tr>";
    QString _misc = misc != "" ? misc : "-";
    html += "<tr><td>" + QObject::tr("Other") + "</td><td>" + _misc + "</td></tr>";
    html += "</tbody></table></div></div></div><div style='clear:both;'></div>";

    // add medications
    if (medicationList.size() > 0) {
        html += "<h1>" + QObject::tr("Medication") + "</h1>";
        html += "<div class='segmentBodyColoredBorder'><div class='segmentBody'><table class='basicTable'><tbody>";
        html += "<tr><td>" + QObject::tr("Active ingredient") + "</td>";
        html += "<td>" + QObject::tr("Brand") + "</td>";
        html += "<td style='width:70px;'>" + QObject::tr("Strength") + "</td>";
        html += "<td style='width:70px;'>" + QObject::tr("Form") + "</td>";
        html += "<td style='width:30px;'>" + QObject::tr("mo") + "</td>";
        html += "<td style='width:30px;'>" + QObject::tr("no") + "</td>";
        html += "<td style='width:30px;'>" + QObject::tr("ev") + "</td>";
        html += "<td style='width:30px;'>" + QObject::tr("ni") + "</td>";
        html += "<td style='width:80px;'>" + QObject::tr("Unit") + "</td>";
        html += "<td>" + QObject::tr("Note") + "</td>";
        html += "<td>" + QObject::tr("Patient instruction") + "</td></tr>";

        for (const QStringList &medication : medicationList) {
            html += "<tr><td>" + medication.at(0) + "</td>";
            html += "<td>" + medication.at(1) + "</td>";
            html += "<td style='width:70px;'>" + medication.at(2) + "</td>";
            html += "<td style='width:70px;'>" + medication.at(3) + "</td>";
            html += "<td style='width:30px;'>" + medication.at(4) + "</td>";
            html += "<td style='width:30px;'>" + medication.at(5) + "</td>";
            html += "<td style='width:30px;'>" + medication.at(6) + "</td>";
            html += "<td style='width:30px;'>" + medication.at(7) + "</td>";
            html += "<td style='width:80px;'>" + medication.at(8) + "</td>";
            html += "<td>" + medication.at(9) + "</td>";
            html += "<td>" + medication.at(10) + "</td></tr>";
        }
        html += "</tbody></table></div></div>";
    }

    html += "</body></html>";
    return html;
}

void MessageObject::setJson(const QJsonObject &json)
{
    // TODO sanitize inputs
    static const char *DATETIMEFORMAT = "yyyy-MM-ddThh:mm:ss"; //example: 2018-04-03T15:30:10+01:00

    // message meta data
    if (json.contains("id") && json["id"].isString())
        messageId = QUuid(json["id"].toString());

    if (json.contains("title") && json["title"].isString())
        title = json["title"].toString();

    if (json.contains("note") && json["note"].isString())
        note = json["note"].toString();

    if (json.contains("status") && json["status"].isString()) {
        QString _status = json["status"].toString();
        if (_status == "preparation") {
            status = DraftStatus;
        } else if (_status == "sent") {
            status = SentStatus;
        } else if (_status == "read") {
            status = ReadStatus;
        } else if (_status == "resent") {
            status = ResentStatus;
        } else if (_status == "reread") {
            status = RereadStatus;
        } else if (_status == "resolved") {
            status = ResolvedStatus;
        }
    }

    if (json.contains("archivedFor") && json["archivedFor"].isArray()) {
        foreach (const QJsonValue &v, json["archivedFor"].toArray()) {
            archivedFor.append({ v.toObject().value("user").toString(), v.toObject().value("date").toString() });
        }
    }

    if (json.contains("authoredOn") && json["authoredOn"].isString())
        authoredOn = QDateTime::fromString(json["authoredOn"].toString(), DATETIMEFORMAT);

    // TODO check range
    if (json.contains("priority") && json["priority"].isDouble())
        priority = Priority(json["priority"].toInt());

    if (json.contains("requester")) {
        QStringList requester = json["requester"].toObject().value("agent").toObject().value("reference").toString().split("/");
        initials = requester.value(1);
        sender = requester.value(0);
        senderName = json["requester"].toObject().value("agent").toObject().value("display").toString();
        recipient = json["requester"].toObject().value("onBehalfOf").toObject().value("reference").toString();
        recipientName = json["requester"].toObject().value("onBehalfOf").toObject().value("display").toString();
    }

    // message payload
    if (json.contains("payload")) {
        QJsonObject payload = json["payload"].toObject();

        // patient data
        if (payload.contains("patient")) {
            patientName = payload.value("patient").toObject().value("name").toObject().value("text").toString();
            gender = payload.value("patient").toObject().value("gender").toString() == "male" ? MALE : FEMALE;
            birthday = QDate::fromString(payload.value("patient").toObject().value("birthDate").toString(), "yyyy-MM-dd");
        }

        // patients vital data
        if (payload.contains("observations")) {
            for (const QJsonValue &v : payload.value("observations").toArray()) {
                QString payloadId = v.toObject().value("id").toString();
                if (payloadId == "heart-rate") {
                    pulse = v.toObject().value("valueQuantity").toObject().value("value").toInt();
                    dtPulse = (QDateTime::fromString(v.toObject().value("effectiveDateTime").toString(), DATETIMEFORMAT));
                } else if (payloadId == "glucose") {
                    sugar = v.toObject().value("valueQuantity").toObject().value("value").toDouble();
                    dtSugar = (QDateTime::fromString(v.toObject().value("effectiveDateTime").toString(), DATETIMEFORMAT));
                } else if (payloadId == "body-temperature") {
                    temp = v.toObject().value("valueQuantity").toObject().value("value").toDouble();
                    dtTemp = (QDateTime::fromString(v.toObject().value("effectiveDateTime").toString(), DATETIMEFORMAT));
                } else if (payloadId == "blood-pressure") {
                    bpSys = v.toObject().value("component")[0].toObject().value("valueQuantity").toObject().value("value").toInt();
                    bpDia = v.toObject().value("component")[1].toObject().value("valueQuantity").toObject().value("value").toInt();
                    dtBp = (QDateTime::fromString(v.toObject().value("effectiveDateTime").toString(), DATETIMEFORMAT));
                } else if (payloadId == "body-weight") {
                    weight = v.toObject().value("valueQuantity").toObject().value("value").toDouble();
                    dtWeight = (QDateTime::fromString(v.toObject().value("effectiveDateTime").toString(), DATETIMEFORMAT));
                } else if (payloadId == "last-defecation") {
                    dtDefac = (QDateTime::fromString(v.toObject().value("effectiveDateTime").toString(), DATETIMEFORMAT));
                } else if (payloadId == "pain") {
                    pain = (v.toObject().value("valueQuantity").toObject().value("value").toString());
                } else if (payloadId == "responsiveness") {
                    response = (v.toObject().value("valueQuantity").toObject().value("value").toString());
                } else if (payloadId == "misc") {
                    misc = (v.toObject().value("valueQuantity").toObject().value("value").toString());
                }
            }
        }

        // medication
        if (payload.contains("medicationRequests")) {
            for (const QJsonValue &v : payload.value("medicationRequests").toArray()) {
                QStringList medicationItem;
                medicationItem << v.toObject().value("medication").toObject().value("ingredient").toObject().value("itemCodeableConcept").toString();
                medicationItem << v.toObject().value("medication").toObject().value("manufacturer").toString();
                medicationItem << v.toObject().value("medication").toObject().value("ingredient").toObject().value("amount").toString();
                medicationItem << v.toObject().value("medication").toObject().value("form").toString();

                int mo = 0;
                int mi = 0;
                int ab = 0;
                int na = 0;
                for (const QJsonValue &w : v.toObject().value("dosageInstruction").toArray()) {
                    const int doseQuantity = w.toObject().value("doseQuantity").toInt();
                    switch (w.toObject().value("sequence").toInt()) {
                    case 1:
                        mo = doseQuantity;
                        break;
                    case 2:
                        mi = doseQuantity;
                        break;
                    case 3:
                        ab = doseQuantity;
                        break;
                    case 4:
                        na = doseQuantity;
                        break;
                    }
                }

                medicationItem << QString::number(mo);
                medicationItem << QString::number(mi);
                medicationItem << QString::number(ab);
                medicationItem << QString::number(na);

                medicationItem << v.toObject().value("unit").toString();
                medicationItem << v.toObject().value("note").toString();
                medicationItem << v.toObject().value("patientInstruction").toString();

                medicationList << medicationItem;
            }
        }

        // images
        if (payload.contains("media")) {
            QDir imgPath = QDir(path);
            imgPath.cdUp();
            for (const QJsonValue &v : payload.value("media").toArray()) {
                imagesList.append({ v.toObject().value("content").toString(), imgPath.absolutePath() + "/assets/" + v.toObject().value("content").toString() });
            }
        }
    }
}

void MessageObject::buildJson(QJsonObject &json, Sharee &currentUser, bool isDraft) const
{
    // TODO sanitize inputs

    // message metadata
    QString _note = "<tr><td><div class='messageSender'>" + currentUser.displayName() + "/" + initials + "</div></td><td class='messageBody'>"
        + messageBody + "</td><td class='messageDate'>" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + "</td></tr>";

    // message metadata
    json["resourceType"] = "Message";
    json["id"] = messageId.toString();

    QString jsonStatus("sent");
    switch (status) {
    case DraftStatus:
        jsonStatus = "preparation";
        break;
    case SentStatus:
        jsonStatus = "sent";
        break;
    case ReadStatus:
        jsonStatus = "read";
        break;
    case ResentStatus:
        jsonStatus = "resent";
        break;
    case RereadStatus:
        jsonStatus = "reread";
        break;
    case ResolvedStatus:
        jsonStatus = "resolved";
        break;
    }
    json["status"] = (isDraft ? "preparation" : jsonStatus);
    json["title"] = title;
    QJsonObject subject;
    subject["reference"] = patientName;
    json["subject"] = subject;
    json["authoredOn"] = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
    QJsonObject requester;
    QJsonObject agent;
    QJsonObject onBehalfOf;
    QString requesterReference = currentUser.shareWith() + "/" + initials;
    agent["reference"] = requesterReference;
    agent["display"] = currentUser.displayName();
    onBehalfOf["reference"] = recipient;
    onBehalfOf["display"] = recipientName;
    requester.insert("agent", agent);
    requester.insert("onBehalfOf", onBehalfOf);
    json["requester"] = requester;
    json["note"] = note;
    json["priority"] = priority;

    if (!archivedFor.empty()) {
        QJsonArray _archivedFor;
        QJsonObject _archived;

        for (int i = 0; i < archivedFor.length(); i++) {
            _archived["user"] = archivedFor.at(i).at(0);
            _archived["date"] = archivedFor.at(i).at(1);
            _archivedFor.append(_archived);
        }

        json["archivedFor"] = _archivedFor;
    }

    QJsonObject payload;

    // patient information
    QJsonObject patient;
    patient["resourceType"] = "Patient";
    QJsonObject meta;
    meta["profile"] = "http://fhir.de/StructureDefinition/patient-de-basis";
    patient["meta"] = meta;
    patient["id"] = 1;
    patient["active"] = true;
    QJsonObject name;
    name["use"] = "official";
    name["text"] = patientName;
    name["family"] = "";
    name["given"] = "";
    patient["name"] = name;
    patient["gender"] = (gender ? "female" : "male");
    patient["birthDate"] = birthday.toString("yyyy-MM-dd");
    QJsonObject generalPractitioner;
    generalPractitioner["reference"] = recipient;
    patient["generalPractitioner"] = generalPractitioner;
    QJsonObject managingOrganization;
    managingOrganization["reference"] = currentUser.shareWith();
    patient["managingOrganization"] = managingOrganization;
    payload.insert("patient", patient);

    // observations
    QJsonArray observationArray;
    QJsonObject category;
    QJsonObject catCoding;
    QJsonObject code;
    QJsonObject codeCoding;
    QJsonObject valueQuantity;

    // blood-pressure
    if (bpSys && bpDia) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "blood-pressure";
        meta["profile"] = "http://hl7.org/fhir/StructureDefinition/vitalsigns";
        observationObject["meta"] = meta;
        catCoding["system"] = "http://hl7.org/fhir/observation-category";
        catCoding["code"] = "vital-signs";
        catCoding["display"] = "Vital Signs";
        catCoding["text"] = "Vital Signs";
        category["coding"] = catCoding;
        observationObject["category"] = category;
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "85354-9";
        codeCoding["display"] = "Blood pressure panel with all children optional";
        code["coding"] = codeCoding;
        code["text"] = "Blood pressure systolic & diastolic";
        observationObject["code"] = code;
        observationObject["subject"] = "";
        observationObject["effectiveDateTime"] = dtBp.toString("yyyy-MM-ddThh:mm:ss");

        QJsonArray component;
        QJsonObject sys;
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "8480-6";
        codeCoding["display"] = "Systolic blood pressure";
        code["coding"] = codeCoding;
        code["text"] = "Systolic blood pressure";
        sys["code"] = code;
        valueQuantity["value"] = bpSys;
        valueQuantity["unit"] = "mmHg";
        valueQuantity["system"] = "http://unitsofmeasure.org";
        valueQuantity["code"] = "mm[Hg]";
        sys["valueQuantity"] = valueQuantity;
        component.append(sys);

        QJsonObject dia;
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "8462-4";
        codeCoding["display"] = "Diastolic blood pressure";
        code["coding"] = codeCoding;
        code["text"] = "Diastolic blood pressure";
        dia["code"] = code;
        valueQuantity["value"] = bpDia;
        valueQuantity["unit"] = "mmHg";
        valueQuantity["system"] = "http://unitsofmeasure.org";
        valueQuantity["code"] = "mm[Hg]";
        dia["valueQuantity"] = valueQuantity;
        component.append(dia);
        observationObject["component"] = component;

        observationArray.append(observationObject);
    }

    // heart-rate
    if (pulse) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "heart-rate";
        meta["profile"] = "http://hl7.org/fhir/StructureDefinition/vitalsigns";
        observationObject["meta"] = meta;
        catCoding["system"] = "http://hl7.org/fhir/observation-category";
        catCoding["code"] = "vital-signs";
        catCoding["display"] = "Vital Signs";
        catCoding["text"] = "Vital Signs";
        category["coding"] = catCoding;
        observationObject["category"] = category;
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "8867-4";
        codeCoding["display"] = "Heart rate";
        code["coding"] = codeCoding;
        code["text"] = "Heart rate";
        observationObject["code"] = code;
        observationObject["subject"] = "";
        observationObject["effectiveDateTime"] = dtPulse.toString("yyyy-MM-ddThh:mm:ss");
        valueQuantity["value"] = pulse;
        valueQuantity["unit"] = "beats/minute";
        valueQuantity["system"] = "http://unitsofmeasure.org";
        valueQuantity["code"] = "/min";
        observationObject["valueQuantity"] = valueQuantity;
        observationArray.append(observationObject);
    }

    // body-temperature
    if (!std::isnan(temp)) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "body-temperature";
        meta["profile"] = "http://hl7.org/fhir/StructureDefinition/vitalsigns";
        observationObject["meta"] = meta;
        catCoding["system"] = "http://hl7.org/fhir/observation-category";
        catCoding["code"] = "vital-signs";
        catCoding["display"] = "Vital Signs";
        catCoding["text"] = "Vital Signs";
        category["coding"] = catCoding;
        observationObject["category"] = category;
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "8310-5";
        codeCoding["display"] = "Body temperature";
        code["coding"] = codeCoding;
        code["text"] = "Body temperature";
        observationObject["code"] = code;
        observationObject["subject"] = "";
        observationObject["effectiveDateTime"] = dtTemp.toString("yyyy-MM-ddThh:mm:ss");
        valueQuantity["value"] = temp;
        valueQuantity["unit"] = "C";
        valueQuantity["system"] = "http://unitsofmeasure.org";
        valueQuantity["code"] = "Cel";
        observationObject["valueQuantity"] = valueQuantity;
        observationArray.append(observationObject);
    }

    // glucose
    if (!std::isnan(sugar)) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "glucose";
        meta["profile"] = "http://hl7.org/fhir/StructureDefinition/vitalsigns";
        observationObject["meta"] = meta;
        catCoding["system"] = "http://hl7.org/fhir/observation-category";
        catCoding["code"] = "vital-signs";
        catCoding["display"] = "Vital Signs";
        catCoding["text"] = "Vital Signs";
        category["coding"] = catCoding;
        observationObject["category"] = category;
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "15074-8";
        codeCoding["display"] = "Glucose [Milligramm/volume] in Blood";
        code["coding"] = codeCoding;
        code["text"] = "Glucose";
        observationObject["code"] = code;
        observationObject["subject"] = "";
        observationObject["effectiveDateTime"] = dtSugar.toString("yyyy-MM-ddThh:mm:ss");
        valueQuantity["value"] = sugar;
        valueQuantity["unit"] = "mg/dl";
        valueQuantity["system"] = "http://unitsofmeasure.org";
        valueQuantity["code"] = "mg/dl";
        observationObject["valueQuantity"] = valueQuantity;
        observationArray.append(observationObject);
    }

    // body-weight
    if (!std::isnan(weight)) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "body-weight";
        meta["profile"] = "http://hl7.org/fhir/StructureDefinition/vitalsigns";
        observationObject["meta"] = meta;
        catCoding["system"] = "http://hl7.org/fhir/observation-category";
        catCoding["code"] = "vital-signs";
        catCoding["display"] = "Vital Signs";
        catCoding["text"] = "Vital Signs";
        category["coding"] = catCoding;
        observationObject["category"] = category;
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "29463-7";
        codeCoding["display"] = "Body Weight";
        code["coding"] = codeCoding;
        code["text"] = "Body Weight";
        observationObject["code"] = code;
        observationObject["subject"] = "";
        observationObject["effectiveDateTime"] = dtWeight.toString("yyyy-MM-ddThh:mm:ss");
        valueQuantity["value"] = weight;
        valueQuantity["unit"] = "kg";
        valueQuantity["system"] = "http://unitsofmeasure.org";
        valueQuantity["code"] = "kg";
        observationObject["valueQuantity"] = valueQuantity;
        observationArray.append(observationObject);
    }

    // responsiveness
    if (!response.isEmpty()) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "responsiveness";
        observationObject["subject"] = "";
        valueQuantity = QJsonObject();
        valueQuantity["value"] = response;
        observationObject["valueQuantity"] = valueQuantity;
        observationArray.append(observationObject);
    }

    // pain
    if (!pain.isEmpty()) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "pain";
        codeCoding["system"] = "http://loinc.org";
        codeCoding["code"] = "28319-2";
        codeCoding["display"] = "Pain status";
        code["coding"] = codeCoding;
        code["text"] = "Pain status";
        observationObject["code"] = code;
        observationObject["subject"] = "";
        valueQuantity = QJsonObject();
        valueQuantity["value"] = pain;
        observationObject["valueQuantity"] = valueQuantity;
        observationArray.append(observationObject);
    }

    // last defecation
    if (dtDefac.isValid()) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "last-defecation";
        observationObject["subject"] = "";
        observationObject["effectiveDateTime"] = dtDefac.toString("yyyy-MM-ddThh:mm:ss");
        observationArray.append(observationObject);
    }

    // misc
    if (!misc.isEmpty()) {
        QJsonObject observationObject;
        observationObject["resourceType"] = "Observation";
        observationObject["id"] = "misc";
        valueQuantity = QJsonObject();
        valueQuantity["value"] = misc;
        observationObject["valueQuantity"] = valueQuantity;
        observationArray.append(observationObject);
    }

    // insert observations into payload
    if (!observationArray.empty())
        payload.insert("observations", observationArray);

    // medication
    QJsonArray medicationArray;

    for (const QStringList &medicationEntry : medicationList) {
        QJsonObject medicationObject;
        medicationObject["resourceType"] = "MedicationRequest";
        medicationObject["status"] = "active";

        QJsonObject subject;
        subject["reference"] = patientName;
        medicationObject["subject"] = subject;

        medicationObject["authoredOn"] = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");

        QJsonObject requester;
        QJsonObject agent;
        QJsonObject onBehalfOf;
        agent["reference"] = currentUser.shareWith();
        onBehalfOf["reference"] = recipient;
        requester.insert("agent", agent);
        requester.insert("onBehalfOf", onBehalfOf);
        medicationObject["requester"] = requester;

        QJsonObject medication;
        QJsonObject ingredient;

        ingredient["itemCodeableConcept"] = medicationEntry[0]; // Wirkstoff
        ingredient["amount"] = medicationEntry[2]; // Stärke

        medication["ingredient"] = ingredient;
        medication["manufacturer"] = medicationEntry[1]; // Handelsname
        medication["form"] = medicationEntry[3]; // Form

        medicationObject["medication"] = medication;
        medicationObject["note"] = medicationEntry[9]; // Hinweise
        medicationObject["patientInstruction"] = medicationEntry[10]; // Grund

        // dosage instruction
        // morning
        QJsonArray dosageInstruction;
        QJsonObject dosage;
        dosage["sequence"] = 1;

        QJsonObject timing;
        QJsonObject repeat;
        repeat["when"] = "mo";
        timing["repeat"] = repeat;
        dosage["timing"] = timing;
        dosage["doseQuantity"] = medicationEntry[4].toInt(); // mo
        dosageInstruction.append(dosage);

        // noon
        dosage["sequence"] = 2;
        repeat["when"] = "mi";
        timing["repeat"] = repeat;
        dosage["timing"] = timing;
        dosage["doseQuantity"] = medicationEntry[5].toInt(); // mi
        dosageInstruction.append(dosage);

        // evening
        dosage["sequence"] = 3;
        repeat["when"] = "ab";
        timing["repeat"] = repeat;
        dosage["timing"] = timing;
        dosage["doseQuantity"] = medicationEntry[6].toInt(); // ab
        dosageInstruction.append(dosage);

        // at night
        dosage["sequence"] = 4;
        repeat["when"] = "na";
        timing["repeat"] = repeat;
        dosage["timing"] = timing;
        dosage["doseQuantity"] = medicationEntry[7].toInt(); // na
        dosageInstruction.append(dosage);

        medicationObject["dosageInstruction"] = dosageInstruction;
        medicationObject["unit"] = medicationEntry[8]; // Einheit

        medicationArray.append(medicationObject);
    }

    // insert medication into payload
    if (!medicationArray.empty())
        payload.insert("medicationRequests", medicationArray);

    // process images list
    QJsonArray mediaArray;

    for (const ImageDetails &image : imagesList) {
        QString fullFilePath = image.path;
        QFileInfo fileInfo = QFileInfo(fullFilePath);

        QJsonObject mediaObject;
        mediaObject["resourceType"] = "Media";
        mediaObject["type"] = "photo";
        mediaObject["view"] = "";
        mediaObject["subject"] = "";
        mediaObject["occurrenceDateTime"] = "";
        mediaObject["operator"] = "";
        mediaObject["bodySite"] = "";
        mediaObject["height"] = "";
        mediaObject["width"] = "";
        mediaObject["content"] = fileInfo.fileName();

        mediaArray.append(mediaObject);
    }

    if (!mediaArray.empty())
        payload.insert("media", mediaArray);

    if (!payload.empty())
        json["payload"] = payload;
}

bool MessageObject::saveMessage(const QString &basePath, Sharee &currentUser, bool isDraft)
{
    bool wasDraft = false;
    // create new uuid if none is set
    if (messageId.isNull()) {
        messageId = messageId.createUuid();
    } else {
        // if the message was a draft, delete it
        if (!isDraft) {
            QString filePath = basePath + "/drafts/messages/" + messageId.toString() + ".json";
            if (QFileInfo::exists(filePath) && QFileInfo(filePath).isFile()) {
                QFile::remove(filePath);
                wasDraft = true;
            }
        }
    }

    // create filename and correct path
    // complete path: <basePath>/<recipientid>/messages/<messageId>.json

    // save drafts to drafts folder
    QString dirPath = basePath + (isDraft ? QString("/drafts/") : QString("/" + recipient));

    // create messages directory if it doesnt exist
    QDir dir(dirPath + "/messages/");
    if (!dir.exists(dirPath + "/messages/")) {
        if (!dir.mkpath(dirPath + "/messages/")) {
            // display error
            QMessageBox msgBox;
            msgBox.setText(QObject::tr("Error on creating folder for messages!"));
            msgBox.exec();
            return false;
        }
    }

    // process images list
    for (ImageDetails &image : imagesList) {
        // get full file path
        if (!image.path.isEmpty()) {
            QString fullFilePath = image.path;

            // create assets directory if it doesn't exist
            QDir dir(dirPath + "/assets/");
            if (!dir.exists(dirPath + "/assets/")) {
                if (!dir.mkpath(dirPath + "/assets/")) {
                    // display error
                    QMessageBox msgBox;
                    msgBox.setText(QObject::tr("Error on creating folder for assets!"));
                    msgBox.exec();
                    return false;
                }
            }

            if (wasDraft) { // move assets belonging to draft to new assets folder
                // assure filename is unique
                while (QFileInfo::exists(dirPath + "/assets/" + image.name)) {
                    image.name = QUuid::createUuid().toString() + image.name;
                }
                if (!QFile::rename(fullFilePath, dirPath + "/assets/" + image.name)) {
                    // display error
                    QMessageBox msgBox;
                    msgBox.setText(QObject::tr("Error on moving assets!"));
                    msgBox.exec();
                    return false;
                }
            } else { // copy file to assets folder
                // assure filename is unique
                while (QFileInfo::exists(dirPath + "/assets/" + image.name)) {
                    image.name = QUuid::createUuid().toString() + image.name;
                }
                if (!QFile::copy(fullFilePath, dirPath + "/assets/" + image.name)) {
                    // display error
                    QMessageBox msgBox;
                    msgBox.setText(QObject::tr("Error on copying assets!"));
                    msgBox.exec();
                    return false;
                }
            }
        }
    }

    QJsonObject content;
    buildJson(content, currentUser, isDraft);

    // write contents to file
    QString filePath = dirPath + "/messages/" + messageId.toString() + ".json";
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument saveDoc(content);
        file.write(saveDoc.toJson());
        return true;
    }
    return false;
}

QString MessageObject::isArchivedFor(const QString &user) const
{
    if (status == MessageObject::ArchivedStatus) {
        for (int i = 0; i < archivedFor.length(); i++) {
            if (archivedFor.at(i).contains(user)) {
                return "true";
            }
        }
    }

    return "false";
}

} // end namespace
