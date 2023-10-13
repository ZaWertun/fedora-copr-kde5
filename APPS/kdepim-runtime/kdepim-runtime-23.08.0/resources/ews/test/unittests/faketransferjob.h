/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <functional>

#include <QQueue>

#include <KIO/SpecialJob>

namespace KIO
{
class Job;
}

template<typename F>
struct Finally {
    Finally(F f)
        : cleanupf{f}
    {
    }

    ~Finally()
    {
        cleanupf();
    }

    F cleanupf;
};

template<typename F>
Finally<F> finally(F f)
{
    return Finally<F>(f);
}

class FakeTransferJob : public KIO::SpecialJob
{
    Q_OBJECT
public:
    using VerifierFn = std::function<void(FakeTransferJob *, const QByteArray &)>;

    struct Verifier {
        QObject *object;
        VerifierFn fn;
    };

    FakeTransferJob(const QByteArray &postData, const VerifierFn &fn, QObject *parent = nullptr);
    ~FakeTransferJob() override;

    static void addVerifier(QObject *obj, const VerifierFn &fn);
    static Verifier getVerifier();
public Q_SLOTS:
    void postResponse(const QByteArray &resp);
private Q_SLOTS:
    void callVerifier();
    void doEmitResult();
    void doData(const QByteArray &resp);
Q_SIGNALS:
    void requestReceived(FakeTransferJob *job, const QByteArray &req);

private:
    QByteArray mPostData;
    QByteArray mResponse;
    VerifierFn mVerifier;
    static QQueue<Verifier> mVerifierQueue;
};
