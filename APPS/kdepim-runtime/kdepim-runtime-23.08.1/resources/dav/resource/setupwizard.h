/*
    SPDX-FileCopyrightText: 2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KDAV/Enums>

#include <QWizard>
#include <QWizardPage>

class KJob;
class QLineEdit;
class QTextBrowser;

class QLabel;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QRadioButton;
class KPasswordLineEdit;

class SetupWizard : public QWizard
{
    Q_OBJECT

public:
    explicit SetupWizard(QWidget *parent = nullptr);

    enum {
        W_CredentialsPage,
        W_PredefinedProviderPage,
        W_ServerTypePage,
        W_ConnectionPage,
        W_CheckPage,
    };

    class Url
    {
    public:
        using List = QVector<Url>;

        KDAV::Protocol protocol;
        QString url;
        QString userName;
        QString password;
    };

    Url::List urls() const;
    QString displayName() const;
};

class PredefinedProviderPage : public QWizardPage
{
public:
    explicit PredefinedProviderPage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;

private:
    QLabel *const mLabel;
    QButtonGroup *const mProviderGroup;
    QRadioButton *const mUseProvider;
    QRadioButton *const mDontUseProvider;
};

class CredentialsPage : public QWizardPage
{
public:
    explicit CredentialsPage(QWidget *parent = nullptr);
    int nextId() const override;

private:
    QLineEdit *const mUserName;
    KPasswordLineEdit *const mPassword;
};

class ServerTypePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ServerTypePage(QWidget *parent = nullptr);

    bool validatePage() override;

private:
    void manualConfigToggled(bool toggled);
    QButtonGroup *mServerGroup = nullptr;
    QComboBox *mProvidersCombo = nullptr;
};

class ConnectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ConnectionPage(QWidget *parent = nullptr);

    void initializePage() override;
    void cleanupPage() override;

private:
    void urlElementChanged();
    QFormLayout *mLayout = nullptr;
    QLineEdit *mHost = nullptr;
    QLineEdit *mPath = nullptr;
    QCheckBox *mUseSecureConnection = nullptr;
    QFormLayout *mPreviewLayout = nullptr;
    QLabel *mCalDavUrlLabel = nullptr;
    QLabel *mCalDavUrlPreview = nullptr;
    QLabel *mCardDavUrlLabel = nullptr;
    QLabel *mCardDavUrlPreview = nullptr;
    QLabel *mGroupDavUrlLabel = nullptr;
    QLabel *mGroupDavUrlPreview = nullptr;
};

class CheckPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit CheckPage(QWidget *parent = nullptr);

private:
    void checkConnection();
    void onFetchDone(KJob *);
    QTextBrowser *const mStatusLabel;
};
