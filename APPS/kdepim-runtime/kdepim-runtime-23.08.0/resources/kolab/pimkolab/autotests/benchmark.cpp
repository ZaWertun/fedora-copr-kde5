/*
 * SPDX-FileCopyrightText: 2011 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "benchmark.h"
#include "conversion/kcalconversion.h"
#include "kolabformatV2/event.h"
#include "testutils.h"
#include <kolabformat.h>

#include <KMime/Message>

#include <QTest>

KMime::Message::Ptr readMimeFile(const QString &fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    const QByteArray data = file.readAll();
    Q_ASSERT(!data.isEmpty());

    auto msg = new KMime::Message;
    msg->setContent(data);
    msg->parse();
    return KMime::Message::Ptr(msg);
}

KMime::Content *findContentByType(const KMime::Message::Ptr &data, const QByteArray &type)
{
    const KMime::Content::List list = data->contents();
    for (KMime::Content *c : list) {
        if (c->contentType()->mimeType() == type) {
            return c;
        }
    }
    return nullptr;
}

void BenchmarkTests::parsingBenchmarkComparison_data()
{
    QTest::addColumn<bool>("v2Parser");
    QTest::newRow("v2") << true;
    QTest::newRow("v3") << false;
}

void BenchmarkTests::parsingBenchmarkComparison()
{
    const KMime::Message::Ptr kolabItem = readMimeFile(TESTFILEDIR + QLatin1String("/v2/event/complex.ics.mime"));
    KMime::Content *xmlContent = findContentByType(kolabItem, "application/x-vnd.kolab.event");
    QVERIFY(xmlContent);
    const QByteArray xmlData = xmlContent->decodedContent();
    //     qDebug() << xmlData;
    const QDomDocument xmlDoc = KolabV2::Event::loadDocument(QString::fromUtf8(xmlData));
    QVERIFY(!xmlDoc.isNull());
    const KCalendarCore::Event::Ptr i = KolabV2::Event::fromXml(xmlDoc, QStringLiteral("Europe/Berlin"));
    QVERIFY(i);
    const Kolab::Event &event = Kolab::Conversion::fromKCalendarCore(*i);
    const std::string &v3String = Kolab::writeEvent(event);

    QFETCH(bool, v2Parser);

    //     Kolab::readEvent(v3String, false); //init parser (doesn't really change the results it seems)
    //     qDebug() << QString::fromUtf8(xmlData);
    //     qDebug() << "------------------------------------------------------------------------------------";
    //     qDebug() << QString::fromStdString(v3String);
    if (v2Parser) {
        QBENCHMARK {
            KolabV2::Event::fromXml(KolabV2::Event::loadDocument(QString::fromUtf8(xmlData)), QStringLiteral("Europe/Berlin"));
        }
    } else {
        QBENCHMARK {
            Kolab::readEvent(v3String, false);
        }
    }
}

QTEST_MAIN(BenchmarkTests)
