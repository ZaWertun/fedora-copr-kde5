/*
 * SPDX-FileCopyrightText: 2012 Sofia Balicka <balicka@kolabsys.com>
 * SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "kolabformat/errorhandler.h"
#include "kolabformat/kolabobject.h"
#include <QFile>
#include <QString>
#include <iostream>
#include <kolabformat.h>
#include <string>
#include <vector>
using namespace std;

KMime::Message::Ptr readMimeFile(const QString &fileName, bool &ok)
{
    QFile file(fileName);
    ok = file.open(QFile::ReadOnly);
    if (!ok) {
        cout << "failed to open file: " << fileName.toStdString() << endl;
        return {};
    }
    const QByteArray data = file.readAll();
    KMime::Message::Ptr msg = KMime::Message::Ptr(new KMime::Message);
    msg->setContent(KMime::CRLFtoLF(data));
    msg->parse();
    return msg;
}

int main(int argc, char *argv[])
{
    vector<string> inputFiles;
    inputFiles.reserve(argc - 1);
    for (int i = 1; i < argc; ++i) {
        inputFiles.emplace_back(argv[i]);
    }
    if (inputFiles.empty()) {
        cout << "Specify input-file\n";
        return -1;
    }

    int returnValue = 0;

    cout << endl;

    for (auto it = inputFiles.begin(); it != inputFiles.end(); ++it) {
        cout << "File: " << *it << endl;

        bool ok;
        KMime::Message::Ptr message = readMimeFile(QString::fromStdString(*it), ok);

        if (!ok) {
            returnValue = -1;
            cout << endl;
            continue;
        }

        Kolab::KolabObjectReader reader(message);

        if (Kolab::ErrorHandler::errorOccured()) {
            cout << "Errors occurred during parsing." << endl;
            returnValue = -1;
        } else {
            cout << "Parsed message without error." << endl;
        }

        cout << endl;
    }

    return returnValue;
}
