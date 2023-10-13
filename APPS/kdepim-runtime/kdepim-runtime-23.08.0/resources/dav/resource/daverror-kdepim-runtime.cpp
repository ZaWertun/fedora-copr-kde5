/*
    SPDX-FileCopyrightText: 2016 Sandro Knauß <sknauss@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "daverror-kdepim-runtime.h"

#include <KLocalizedString>

using namespace KDAV;

QString translateErrorString(const Error &error)
{
    QString result;

    ErrorNumber errorNumber = error.errorNumber();
    const int responseCode = error.responseCode();
    const QString errorText = error.errorText();
    QString err = error.translatedJobError();

    switch (errorNumber) {
    case ERR_PROBLEM_WITH_REQUEST:
        // User-side error
        if (responseCode == 401) {
            err = i18n("Invalid username/password");
        } else if (responseCode == 403) {
            err = i18n("Access forbidden");
        } else if (responseCode == 404) {
            err = i18n("Resource not found");
        } else {
            err = i18n("HTTP error");
        }
        result = i18n(
            "There was a problem with the request.\n"
            "%1 (%2).",
            err,
            responseCode);
        break;
    case ERR_NO_MULTIGET:
        result = i18n("Protocol for the collection does not support MULTIGET");
        break;
    case ERR_SERVER_UNRECOVERABLE:
        result = i18n("The server encountered an error that prevented it from completing your request: %1 (%2)", err, responseCode);
        break;
    case ERR_COLLECTIONDELETE:
        result = i18n(
            "There was a problem with the request. The collection has not been deleted from the server.\n"
            "%1 (%2).",
            err,
            responseCode);
        break;
    case ERR_COLLECTIONFETCH:
        result = i18n("Invalid responses from backend");
        break;
    case ERR_COLLECTIONFETCH_XQUERY_SETFOCUS:
        result = i18n("Error setting focus for XQuery");
        break;
    case ERR_COLLECTIONFETCH_XQUERY_INVALID:
        result = i18n("Invalid XQuery submitted by DAV implementation");
        break;
    case ERR_COLLECTIONMODIFY:
        result = i18n(
            "There was a problem with the request. The collection has not been modified on the server.\n"
            "%1 (%2).",
            err,
            responseCode);
        break;
    case ERR_COLLECTIONMODIFY_NO_PROPERITES:
        result = i18n("No properties to change or remove");
        break;
    case ERR_COLLECTIONMODIFY_RESPONSE:
        result = i18n("There was an error when modifying the properties");
        if (!errorText.isEmpty()) {
            result.append(i18n("\nThe server returned more information:\n%1", errorText));
        }
        break;
    case ERR_ITEMCREATE:
        result = i18n(
            "There was a problem with the request. The item has not been created on the server.\n"
            "%1 (%2).",
            err,
            responseCode);
        break;
    case ERR_ITEMDELETE:
        result = i18n(
            "There was a problem with the request. The item has not been deleted from the server.\n"
            "%1 (%2).",
            err,
            responseCode);
        break;
    case ERR_ITEMMODIFY:
        result = i18n(
            "There was a problem with the request. The item was not modified on the server.\n"
            "%1 (%2).",
            err,
            responseCode);
        break;
    case ERR_ITEMLIST: {
        result = i18n("There was a problem with the request.");
        break;
    };
    case ERR_ITEMLIST_NOMIMETYPE:
        result = i18n("There was a problem with the request. The requested MIME types are not supported.");
        break;
    case NO_ERR:
        break;
    }
    return result;
}
