/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewsitem.h"
#include "ewsitemshape.h"
#include "ewsrequest.h"
#include "ewstypes.h"

class EwsFindItemRequest : public EwsRequest
{
    Q_OBJECT
public:
    explicit EwsFindItemRequest(EwsClient &client, QObject *parent);
    ~EwsFindItemRequest() override;

    void setFolderId(const EwsId &id);
    void setItemShape(const EwsItemShape &shape);
    void setTraversal(EwsTraversalType traversal)
    {
        mTraversal = traversal;
    }

    void setPagination(EwsIndexedViewBasePoint basePoint, unsigned offset, int maxItems = -1)
    {
        mFractional = false;
        mMaxItems = maxItems;
        mPageBasePoint = basePoint;
        mPageOffset = offset;
        mPagination = true;
    }

    void setFractional(unsigned numerator, unsigned denominator, int maxItems = -1)
    {
        mPagination = false;
        mMaxItems = maxItems;
        mFracNumerator = numerator;
        mFracDenominator = denominator;
        mFractional = true;
    }

    void start() override;

    bool includesLastItem() const
    {
        return mIncludesLastItem;
    }

    int nextOffset() const
    {
        return mNextOffset;
    }

    int nextNumerator() const
    {
        return mNextNumerator;
    }

    int nextDenominator() const
    {
        return mNextDenominator;
    }

    const QList<EwsItem> items() const
    {
        return mItems;
    }

protected:
    bool parseResult(QXmlStreamReader &reader) override;
    bool parseItemsResponse(QXmlStreamReader &reader);

private:
    EwsId mFolderId;
    EwsItemShape mShape;
    EwsTraversalType mTraversal;
    bool mPagination;
    EwsIndexedViewBasePoint mPageBasePoint;
    unsigned mPageOffset;
    bool mFractional;
    int mMaxItems;
    unsigned mFracNumerator;
    unsigned mFracDenominator;
    QList<EwsItem> mItems;
    unsigned mTotalItems;
    int mNextOffset;
    int mNextNumerator;
    int mNextDenominator;
    bool mIncludesLastItem;
};
