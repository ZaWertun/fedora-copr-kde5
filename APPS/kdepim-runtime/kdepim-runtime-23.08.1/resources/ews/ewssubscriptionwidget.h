/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QWidget>

#include "ewsid.h"

#include <memory>

class EwsSubscriptionWidgetPrivate;
class EwsClient;
class EwsSettings;

class EwsSubscriptionWidget : public QWidget
{
    Q_OBJECT
public:
    EwsSubscriptionWidget(EwsClient &client, EwsSettings *settings, QWidget *parent);
    ~EwsSubscriptionWidget() override;

    QStringList subscribedList() const;
    bool subscribedListValid() const;
    bool subscriptionEnabled() const;

private:
    std::unique_ptr<EwsSubscriptionWidgetPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(EwsSubscriptionWidget)
};
