/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KIO/TransferJob>

#include "faketransferjob.h"

namespace KIO
{
inline TransferJob *http_post(const QUrl &url, const QByteArray &postData, JobFlags flags)
{
    Q_UNUSED(url)
    Q_UNUSED(flags)

    FakeTransferJob::Verifier vfy = FakeTransferJob::getVerifier();
    auto job = new FakeTransferJob(postData, vfy.fn, vfy.object);
    return reinterpret_cast<TransferJob *>(job);
}
}
