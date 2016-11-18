#include "transactiondesc.h"

#include "bitcoinunits.h"
#include "guiutil.h"

#include "base58.h"
#include "main.h"
#include "paymentserver.h"
#include "transactionrecord.h"
#include "util.h"
#include "ui_interface.h"
#include "wallet.h"
#include "txdb.h"

#include <string>

QString TransactionDesc::FormatTxStatus(const CWalletTx& wtx)
{
    AssertLockHeld(cs_main);
    if (!IsFinalTx(wtx, nBestHeight + 1))
    {
        if (wtx.nLockTime < LOCKTIME_THRESHOLD)
            return tr("Open for %n more block(s)", "", wtx.nLockTime - nBestHeight);
        else
            return tr("Open until %1").arg(GUIUtil::dateTimeStr(wtx.nLockTime));
    }
    else
    {
        int signatures = wtx.GetTransactionLockSignatures();
        QString strUsingIX = "";
        if(signatures >= 0){

            if(signatures >= INSTANTX_SIGNATURES_REQUIRED){
                int nDepth = wtx.GetDepthInMainChain();
                if (nDepth < 0)
                    return tr("conflicted");
                else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                    return tr("%1/offline (verified via instantx)").arg(nDepth);
                else if (nDepth < 10)
                    return tr("%1/confirmed (verified via instantx)").arg(nDepth);
                else
                    return tr("%1 confirmations (verified via instantx)").arg(nDepth);
            } else {
                if(!wtx.IsTransactionLockTimedOut()){
                    int nDepth = wtx.GetDepthInMainChain();
                    if (nDepth < 0)
                        return tr("conflicted");
                    else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                        return tr("%1/offline (InstantX verification in progress - %2 of %3 signatures)").arg(nDepth).arg(signatures).arg(INSTANTX_SIGNATURES_TOTAL);
                    else if (nDepth < 10)
                        return tr("%1/confirmed (InstantX verification in progress - %2 of %3 signatures )").arg(nDepth).arg(signatures).arg(INSTANTX_SIGNATURES_TOTAL);
                    else
                        return tr("%1 confirmations (InstantX verification in progress - %2 of %3 signatures)").arg(nDepth).arg(signatures).arg(INSTANTX_SIGNATURES_TOTAL);
                } else {
                    int nDepth = wtx.GetDepthInMainChain();
                    if (nDepth < 0)
                        return tr("conflicted");
                    else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                        return tr("%1/offline (InstantX verification failed)").arg(nDepth);
                    else if (nDepth < 10)
                        return tr("%1/confirmed (InstantX verification failed)").arg(nDepth);
                    else
                        return tr("%1 confirmations").arg(nDepth);
                }
            }
        } else {
            int nDepth = wtx.GetDepthInMainChain();
            if (nDepth < 0)
                return tr("conflicted");
            else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                return tr("%1/offline").arg(nDepth);
            else if (nDepth < 10)
                return tr("%1/unconfirmed").arg(nDepth);
            else
                return tr("%1 confirmations").arg(nDepth);
        }
    }
}

QString TransactionDesc::toHTML(CWallet *wallet, CWalletTx &wtx, TransactionRecord *rec, int unit)
{
    QString strHTML;

    LOCK2(cs_main, wallet->cs_wallet);
    strHTML.reserve(4000);
    strHTML += "<html><font face='verdana, arial, helvetica, sans-serif'>";

    int64_t nTime = wtx.GetTxTime();
    CAmount nCredit = wtx.GetCredit(ISMINE_ALL);
    CAmount nDebit = wtx.GetDebit(ISMINE_ALL);
    CAmount nNet = nCredit - nDebit;

    strHTML += "<b>" + tr("Status") + ":</b> " + FormatTxStatus(wtx);
    int nRequests = wtx.GetRequestCount();
    if (nRequests != -1)
    {
        if (nRequests == 0)
            strHTML += tr(", has not been successfully broadcast yet");
        else if (nRequests > 0)
            strHTML += tr(", broadcast through %n node(s)", "", nRequests);
    }
    strHTML += "<br>";

    strHTML += "<b>" + tr("Date") + ":</b> " + (nTime ? GUIUtil::dateTimeStr(nTime) : "") + "<br>";

    //
    // From
    //
    if (wtx.IsCoinBase() || wtx.IsCoinStake())
    {
        strHTML += "<b>" + tr("Source") + ":</b> " + tr("Generated") + "<br>";
    }
    else if (wtx.mapValue.count("from") && !wtx.mapValue["from"].empty())
    {
        // Online transaction
        strHTML += "<b>" + tr("From") + ":</b> " + GUIUtil::HtmlEscape(wtx.mapValue["from"]) + "<br>";
    }
    else
    {
        // Offline transaction
        if (nNet > 0)
        {
            // Credit
            if (CBitradioAddress(rec->address).IsValid())
            {
                CTxDestination address = CBitradioAddress(rec->address).Get();
                if (wallet->mapAddressBook.count(address))
                {
                    strHTML += "<b>" + tr("From") + ":</b> " + tr("unknown") + "<br>";
                    strHTML += "<b>" + tr("To") + ":</b> ";
                    strHTML += GUIUtil::HtmlEscape(rec->address);
                    QString addressOwned = (::IsMine(*wallet, address) == ISMINE_SPENDABLE) ? tr("own address") : tr("watch-only");
                    if (!wallet->mapAddressBook[address].empty())
                        strHTML += " (" + addressOwned + ", " + tr("label") + ": " + GUIUtil::HtmlEscape(wallet->mapAddressBook[address]) + ")";
                    else
                        strHTML += " (" + addressOwned + ")";
                    strHTML += "<br>";
                }
            }
        }
    }

    //
    // To
    //
    if (wtx.mapValue.count("to") && !wtx.mapValue["to"].empty())
    {
        // Online transaction
        std::string strAddress = wtx.mapValue["to"];
        strHTML += "<b>" + tr("To") + ":</b> ";
        CTxDestination dest = CBitradioAddress(strAddress).Get();
        if (wallet->mapAddressBook.count(dest) && !wallet->mapAddressBook[dest].empty())
            strHTML += GUIUtil::HtmlEscape(wallet->mapAddressBook[dest]) + " ";
        strHTML += GUIUtil::HtmlEscape(strAddress) + "<br>";
    }

    //
    // Amount
    //
    if (wtx.IsCoinBase() && nCredit == 0)
    {
        //
        // Coinbase
        //
        CAmount nUnmatured = 0;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            nUnmatured += wallet->GetCredit(txout, ISMINE_ALL);
        strHTML += "<b>" + tr("Credit") + ":</b> ";
        if (wtx.IsInMainChain())
            strHTML += BitcoinUnits::formatHtmlWithUnit(unit, nUnmatured)+ " (" + tr("matures in %n more block(s)", "", wtx.GetBlocksToMaturity()) + ")";
        else
            strHTML += "(" + tr("not accepted") + ")";
        strHTML += "<br>";
    }
    else if (nNet > 0)
    {
        //
        // Credit
        //
        strHTML += "<b>" + tr("Credit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, nNet) + "<br>";
    }
    else
    {

        isminetype fAllFromMe = ISMINE_SPENDABLE;
        BOOST_FOREACH(const CTxIn& txin, wtx.vin)
        {
            isminetype mine = wallet->IsMine(txin);
            if(fAllFromMe > mine) fAllFromMe = mine;
        }

        isminetype fAllToMe = ISMINE_SPENDABLE;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            isminetype mine = wallet->IsMine(txout);
            if(fAllToMe > mine) fAllToMe = mine;
        }

        if (fAllFromMe)
        {
            if(fAllFromMe == ISMINE_WATCH_ONLY)
                strHTML += "<b>" + tr("From") + ":</b> " + tr("watch-only") + "<br>";
            //
            // Debit
            //
            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            {
                // Ignore change
                isminetype toSelf = wallet->IsMine(txout);
                if ((toSelf == ISMINE_SPENDABLE) && (fAllFromMe == ISMINE_SPENDABLE))
                    continue;

                if (!wtx.mapValue.count("to") || wtx.mapValue["to"].empty())
                {
                    // Offline transaction
                    CTxDestination address;
                    if (ExtractDestination(txout.scriptPubKey, address))
                    {
                        strHTML += "<b>" + tr("To") + ":</b> ";
                        if (wallet->mapAddressBook.count(address) && !wallet->mapAddressBook[address].empty())
                            strHTML += GUIUtil::HtmlEscape(wallet->mapAddressBook[address]) + " ";
                        strHTML += GUIUtil::HtmlEscape(CBitradioAddress(address).ToString());
                        if(toSelf == ISMINE_SPENDABLE)
                            strHTML += " (own address)";
                        else if(toSelf == ISMINE_WATCH_ONLY)
                            strHTML += " (watch-only)";
                        strHTML += "<br>";
                    }
                }

                strHTML += "<b>" + tr("Debit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, -txout.nValue) + "<br>";
                if(toSelf)
                    strHTML += "<b>" + tr("Credit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, txout.nValue) + "<br>";
            }

            if (fAllToMe)
            {
                // Payment to self
                CAmount nChange = wtx.GetChange();
                CAmount nValue = nCredit - nChange;
                strHTML += "<b>" + tr("Total debit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, -nValue) + "<br>";
                strHTML += "<b>" + tr("Total credit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, nValue) + "<br>";
            }

            CAmount nTxFee = nDebit - wtx.GetValueOut();
            if (nTxFee > 0)
                strHTML += "<b>" + tr("Transaction fee") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, -nTxFee) + "<br>";
        }
        else
        {
            //
            // Mixed debit transaction
            //
            BOOST_FOREACH(const CTxIn& txin, wtx.vin)
                if (wallet->IsMine(txin))
                    strHTML += "<b>" + tr("Debit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, -wallet->GetDebit(txin, ISMINE_ALL)) + "<br>";
            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
                if (wallet->IsMine(txout))
                    strHTML += "<b>" + tr("Credit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, wallet->GetCredit(txout, ISMINE_ALL)) + "<br>";
        }
    }

    strHTML += "<b>" + tr("Net amount") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, nNet, true) + "<br>";

    //
    // Message
    //
    if (wtx.mapValue.count("message") && !wtx.mapValue["message"].empty())
        strHTML += "<br><b>" + tr("Message") + ":</b><br>" + GUIUtil::HtmlEscape(wtx.mapValue["message"], true) + "<br>";
    if (wtx.mapValue.count("comment") && !wtx.mapValue["comment"].empty())
        strHTML += "<br><b>" + tr("Comment") + ":</b><br>" + GUIUtil::HtmlEscape(wtx.mapValue["comment"], true) + "<br>";

    strHTML += "<b>" + tr("Transaction ID") + ":</b> " + TransactionRecord::formatSubTxId(wtx.GetHash(), rec->idx) + "<br>";

    if (wtx.IsCoinBase() || wtx.IsCoinStake())
    {
        strHTML += "<br>" + tr("Generated coins must mature 80 blocks before they can be spent. When you generated this block, it was broadcast to the network to be added to the block chain. If it fails to get into the chain, its state will change to \"not accepted\" and it won't be spendable. This may occasionally happen if another node generates a block within a few seconds of yours.") + "<br>";
    }


    //
    // Debug view
    //
    if (fDebug)
    {
        strHTML += "<hr><br>" + tr("Debug information") + "<br><br>";
        BOOST_FOREACH(const CTxIn& txin, wtx.vin)
            if(wallet->IsMine(txin))
                strHTML += "<b>" + tr("Debit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, -wallet->GetDebit(txin, ISMINE_ALL)) + "<br>";
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            if(wallet->IsMine(txout))
                strHTML += "<b>" + tr("Credit") + ":</b> " + BitcoinUnits::formatHtmlWithUnit(unit, wallet->GetCredit(txout, ISMINE_ALL)) + "<br>";

        strHTML += "<br><b>" + tr("Transaction") + ":</b><br>";
        strHTML += GUIUtil::HtmlEscape(wtx.ToString(), true);

        CTxDB txdb("r"); // To fetch source txouts

        strHTML += "<br><b>" + tr("Inputs") + ":</b>";
        strHTML += "<ul>";

        BOOST_FOREACH(const CTxIn& txin, wtx.vin)
        {
            COutPoint prevout = txin.prevout;

            CTransaction prev;
            if(txdb.ReadDiskTx(prevout.hash, prev))
            {
                if (prevout.n < prev.vout.size())
                {
                    strHTML += "<li>";
                    const CTxOut &vout = prev.vout[prevout.n];
                    CTxDestination address;
                    if (ExtractDestination(vout.scriptPubKey, address))
                    {
                        if (wallet->mapAddressBook.count(address) && !wallet->mapAddressBook[address].empty())
                            strHTML += GUIUtil::HtmlEscape(wallet->mapAddressBook[address]) + " ";
                        strHTML += QString::fromStdString(CBitradioAddress(address).ToString());
                    }
                    strHTML = strHTML + " " + tr("Amount") + "=" + BitcoinUnits::formatWithUnit(unit, vout.nValue);
                    strHTML = strHTML + " IsMine=" + (wallet->IsMine(vout) & ISMINE_SPENDABLE ? tr("true") : tr("false"));
                    strHTML = strHTML + " IsWatchOnly=" + (wallet->IsMine(vout) & ISMINE_WATCH_ONLY ? tr("true") : tr("false")) + "</li>";
                }
            }
        }

        strHTML += "</ul>";
    }

    strHTML += "</font></html>";
    return strHTML;
}
