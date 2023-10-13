/*
    SPDX-FileCopyrightText: 2015-2018 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDialog>
#include <QMap>
#include <QPointer>

class QDialogButtonBox;
class EwsResource;
class EwsClient;
class KConfigDialogManager;
namespace Ui
{
class SetupServerView;
}
class KJob;
class EwsAbstractAuth;
class EwsAutodiscoveryJob;
class EwsGetFolderRequest;
class EwsProgressDialog;
class EwsSubscriptionWidget;
class EwsSettings;

class EwsConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EwsConfigDialog(EwsResource *parentResource, EwsClient &client, WId windowId, EwsSettings *settings);
    ~EwsConfigDialog() override;

private:
    void save();
    void autoDiscoveryFinished(KJob *job);
    void tryConnectFinished(KJob *job);
    void performAutoDiscovery();
    void autoDiscoveryCancelled();
    void tryConnectCancelled();
    void setAutoDiscoveryNeeded();
    void dialogAccepted();
    void enableTryConnect();
    void tryConnect();
    void userAgentChanged(int index);
    EwsAbstractAuth *prepareAuth();

    QString fullUsername() const;
    EwsResource *mParentResource = nullptr;
    KConfigDialogManager *mConfigManager = nullptr;
    Ui::SetupServerView *mUi = nullptr;

    QDialogButtonBox *mButtonBox = nullptr;
    EwsAutodiscoveryJob *mAutoDiscoveryJob = nullptr;
    EwsGetFolderRequest *mTryConnectJob = nullptr;
    bool mTryConnectJobCancelled = false;
    bool mAutoDiscoveryNeeded = false;
    bool mTryConnectNeeded = false;
    EwsProgressDialog *mProgressDialog = nullptr;
    EwsSubscriptionWidget *mSubWidget = nullptr;
    QPointer<EwsSettings> mSettings;
    QMap<QString, QString> mAuthMap;
};
