/*
    SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2020 Igor Poboiko <igor.poboiko@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "googlesettingswidget.h"
#include "googleresource.h"
#include "googleresource_debug.h"
#include "googlescopes.h"
#include "googlesettings.h"

#include <QDialogButtonBox>

#include <KGAPI/Account>
#include <KGAPI/AuthJob>
#include <KGAPI/Calendar/Calendar>
#include <KGAPI/Calendar/CalendarFetchJob>
#include <KGAPI/Tasks/TaskList>
#include <KGAPI/Tasks/TaskListFetchJob>
#include <KMessageBox>
#include <KWindowSystem>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <qt5keychain/keychain.h>
#else
#include <qt6keychain/keychain.h>
#endif

using namespace QKeychain;
using namespace KGAPI2;

GoogleSettingsWidget::GoogleSettingsWidget(GoogleSettings &settings, const QString &identifier, QWidget *parent)
    : QWidget(parent)
    , m_settings(settings)
    , m_identifier(identifier)
{
    qDebug() << m_settings.account();
    auto mainLayout = new QVBoxLayout(this);

    auto mainWidget = new QWidget(this);
    mainLayout->addWidget(mainWidget);
    setupUi(mainWidget);

    refreshSpinBox->setSuffix(ki18np(" minute", " minutes"));
    enableRefresh->setChecked(m_settings.enableIntervalCheck());
    refreshSpinBox->setEnabled(m_settings.enableIntervalCheck());
    refreshSpinBox->setValue(m_settings.intervalCheckTime());

    eventsLimitCombo->setMaximumDate(QDate::currentDate());
    eventsLimitCombo->setMinimumDate(QDate::fromString(QStringLiteral("2000-01-01"), Qt::ISODate));
    eventsLimitCombo->setOptions(KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker | KDateComboBox::WarnOnInvalid);
    if (m_settings.eventsSince().isEmpty()) {
        const QString ds = QStringLiteral("%1-01-01").arg(QString::number(QDate::currentDate().year() - 3));
        eventsLimitCombo->setDate(QDate::fromString(ds, Qt::ISODate));
    } else {
        eventsLimitCombo->setDate(QDate::fromString(m_settings.eventsSince(), Qt::ISODate));
    }
    connect(reloadCalendarsBtn, &QPushButton::clicked, this, &GoogleSettingsWidget::slotReloadCalendars);
    connect(reloadTaskListsBtn, &QPushButton::clicked, this, &GoogleSettingsWidget::slotReloadTaskLists);
    connect(configureBtn, &QPushButton::clicked, this, &GoogleSettingsWidget::loadSettings);
    if (m_settings.isReady()) {
        m_account = m_settings.accountPtr();
    }
    connect(&m_settings, &GoogleSettings::accountReady, this, [this](bool ready) {
        if (ready) {
            m_account = m_settings.accountPtr();
            accountChanged();
        }
    });
    QMetaObject::invokeMethod(this, &GoogleSettingsWidget::accountChanged, Qt::QueuedConnection);
}

GoogleSettingsWidget::~GoogleSettingsWidget()
{
}

bool GoogleSettingsWidget::handleError(KGAPI2::Job *job)
{
    if ((job->error() == KGAPI2::NoError) || (job->error() == KGAPI2::OK)) {
        return true;
    }

    if (job->error() == KGAPI2::Unauthorized) {
        qCDebug(GOOGLE_LOG) << job << job->errorString();
        const QList<QUrl> resourceScopes = googleScopes();
        for (const QUrl &scope : resourceScopes) {
            if (!m_account->scopes().contains(scope)) {
                m_account->addScope(scope);
            }
        }

        auto authJob = new AuthJob(m_account, m_settings.clientId(), m_settings.clientSecret(), this);
        authJob->setProperty(JOB_PROPERTY, QVariant::fromValue(job));
        connect(authJob, &AuthJob::finished, this, &GoogleSettingsWidget::slotAuthJobFinished);

        return false;
    }

    KMessageBox::error(this, job->errorString());
    return false;
}

void GoogleSettingsWidget::accountChanged()
{
    if (!m_account) {
        accountLabel->setText(i18n("<b>not configured</b>"));
        calendarsList->setDisabled(true);
        reloadCalendarsBtn->setDisabled(true);
        calendarsList->clear();
        taskListsList->setDisabled(true);
        reloadTaskListsBtn->setDisabled(true);
        taskListsList->clear();
        return;
    }
    accountLabel->setText(QStringLiteral("<b>%1</b>").arg(m_account->accountName()));
    slotReloadCalendars();
    slotReloadTaskLists();
}

void GoogleSettingsWidget::loadSettings()
{
    const QString username = m_account && !m_account->accountName().isEmpty() ? m_account->accountName() : QString();
    m_account = AccountPtr(new Account());
    const QList<QUrl> resourceScopes = googleScopes();
    for (const QUrl &scope : resourceScopes) {
        if (!m_account->scopes().contains(scope)) {
            m_account->addScope(scope);
        }
    }
    auto authJob = new AuthJob(m_account, m_settings.clientId(), m_settings.clientSecret());
    authJob->setUsername(username);
    connect(authJob, &AuthJob::finished, this, &GoogleSettingsWidget::slotAuthJobFinished);
}

void GoogleSettingsWidget::slotAuthJobFinished(KGAPI2::Job *job)
{
    auto authJob = qobject_cast<AuthJob *>(job);
    m_account = authJob->account();
    if (authJob->error() != KGAPI2::NoError) {
        KMessageBox::error(this, authJob->errorString());
        return;
    }
    accountChanged();

    auto otherJob = job->property(JOB_PROPERTY).value<KGAPI2::Job *>();
    if (otherJob) {
        otherJob->setAccount(m_account);
        otherJob->restart();
    }
}

void GoogleSettingsWidget::saveSettings()
{
    auto reset = [this] {
        m_settings.setAccount({});
        m_settings.setEnableIntervalCheck(enableRefresh->isChecked());
        m_settings.setIntervalCheckTime(refreshSpinBox->value());
        m_settings.setCalendars({});
        m_settings.setTaskLists({});
        m_settings.setEventsSince({});
        m_settings.save();
    };

    if (!m_account) {
        reset();
        return;
    }

    auto writeJob = m_settings.storeAccount(m_account);
    connect(writeJob, &WritePasswordJob::finished, this, [this, reset, writeJob]() {
        if (writeJob->error()) {
            qCWarning(GOOGLE_LOG) << "Failed to store account's password in secret storage" << writeJob->errorString();
            reset();
            return;
        }

        m_settings.setAccount(m_account->accountName());
        m_settings.setEnableIntervalCheck(enableRefresh->isChecked());
        m_settings.setIntervalCheckTime(refreshSpinBox->value());

        QStringList calendars;
        for (int i = 0; i < calendarsList->count(); i++) {
            QListWidgetItem *item = calendarsList->item(i);

            if (item->checkState() == Qt::Checked) {
                calendars.append(item->data(Qt::UserRole).toString());
            }
        }
        m_settings.setCalendars(calendars);

        if (eventsLimitCombo->isValid()) {
            m_settings.setEventsSince(eventsLimitCombo->date().toString(Qt::ISODate));
        }

        QStringList taskLists;
        for (int i = 0; i < taskListsList->count(); i++) {
            QListWidgetItem *item = taskListsList->item(i);

            if (item->checkState() == Qt::Checked) {
                taskLists.append(item->data(Qt::UserRole).toString());
            }
        }
        m_settings.setTaskLists(taskLists);
        m_settings.save();
    });
}

void GoogleSettingsWidget::slotReloadCalendars()
{
    calendarsList->setDisabled(true);
    reloadCalendarsBtn->setDisabled(true);
    calendarsList->clear();

    if (!m_account) {
        return;
    }

    auto fetchJob = new CalendarFetchJob(m_account, this);
    connect(fetchJob, &CalendarFetchJob::finished, this, [this](KGAPI2::Job *job) {
        if (!handleError(job) || !m_account) {
            calendarsList->setEnabled(false);
            reloadCalendarsBtn->setEnabled(false);
            return;
        }

        const ObjectsList objects = qobject_cast<FetchJob *>(job)->items();

        QStringList activeCalendars;
        if (m_account->accountName() == m_settings.account()) {
            activeCalendars = m_settings.calendars();
        }
        calendarsList->clear();
        for (const ObjectPtr &object : objects) {
            const CalendarPtr calendar = object.dynamicCast<Calendar>();

            auto item = new QListWidgetItem(calendar->title());
            item->setData(Qt::UserRole, calendar->uid());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
            item->setCheckState((activeCalendars.isEmpty() || activeCalendars.contains(calendar->uid())) ? Qt::Checked : Qt::Unchecked);
            calendarsList->addItem(item);
        }

        calendarsList->setEnabled(true);
        reloadCalendarsBtn->setEnabled(true);
    });
}

void GoogleSettingsWidget::slotReloadTaskLists()
{
    if (!m_account) {
        return;
    }

    taskListsList->setDisabled(true);
    reloadTaskListsBtn->setDisabled(true);
    taskListsList->clear();

    auto job = new TaskListFetchJob(m_account, this);
    connect(job, &TaskListFetchJob::finished, this, [this](KGAPI2::Job *job) {
        if (!handleError(job) || !m_account) {
            taskListsList->setDisabled(true);
            reloadTaskListsBtn->setDisabled(true);
            return;
        }

        const ObjectsList objects = qobject_cast<FetchJob *>(job)->items();

        QStringList activeTaskLists;
        if (m_account->accountName() == m_settings.account()) {
            activeTaskLists = m_settings.taskLists();
        }
        taskListsList->clear();
        for (const ObjectPtr &object : objects) {
            const TaskListPtr taskList = object.dynamicCast<TaskList>();

            auto item = new QListWidgetItem(taskList->title());
            item->setData(Qt::UserRole, taskList->uid());
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
            item->setCheckState((activeTaskLists.isEmpty() || activeTaskLists.contains(taskList->uid())) ? Qt::Checked : Qt::Unchecked);
            taskListsList->addItem(item);
        }

        taskListsList->setEnabled(true);
        reloadTaskListsBtn->setEnabled(true);
    });
}
