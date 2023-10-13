/*
    SPDX-FileCopyrightText: 2015-2019 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCompositeJob>

#include <ewstypes.h>

class EwsJob : public KCompositeJob
{
    Q_OBJECT
public:
    explicit EwsJob(QObject *parent);
    ~EwsJob() override;

    EwsResponseCode ewsResponseCode() const
    {
        return m_ewsRespCode;
    }

protected:
    bool doKill() override;
    bool setErrorMsg(const QString &msg, int code = KJob::UserDefinedError);
    void setEwsResponseCode(EwsResponseCode code)
    {
        m_ewsRespCode = code;
    }

private:
    EwsResponseCode m_ewsRespCode;
};
