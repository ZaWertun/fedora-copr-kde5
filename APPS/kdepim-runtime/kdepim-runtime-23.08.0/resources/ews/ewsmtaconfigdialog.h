/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDialog>

class QDialogButtonBox;
namespace Akonadi
{
}
namespace Ui
{
class SetupServerView;
}
class EwsMtaResource;

class EwsMtaConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EwsMtaConfigDialog(EwsMtaResource *parentResource, WId windowId);
    ~EwsMtaConfigDialog() override;
private Q_SLOTS:
    void save();

private:
    QDialogButtonBox *const mButtonBox;
    EwsMtaResource *const mParentResource;
    Ui::SetupServerView *mUi = nullptr;
};
