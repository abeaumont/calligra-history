/*
    Copyright (C) 2000, S.R.Haque <shaheedhaque@hotmail.com>.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <msword.h>
#include <unistd.h>

static const short CP2UNI[] = { 0x20ac, 0x0000, 0x201a, 0x0192,
                                0x201e, 0x2026, 0x2020, 0x2021,
                                0x02c6, 0x2030, 0x0160, 0x2039,
                                0x0152, 0x0000, 0x017d, 0x0000,
                                0x0000, 0x2018, 0x2019, 0x201c,
                                0x201d, 0x2022, 0x2013, 0x2014,
                                0x02dc, 0x2122, 0x0161, 0x203a,
                                0x0153, 0x0000, 0x017e, 0x0178 };
const short char2unicode(const unsigned char &c) {
    if(c<=0x7f || c>=0xa0)
        return static_cast<short>(c);
    else
        return CP2UNI[c-0x80];
}

MsWord::MsWord(
        const U8 *mainStream,
        const U8 *table0Stream,
        const U8 *table1Stream,
        const U8 *dataStream)
{
    m_error = QString("");
    read(mainStream, &m_fib);
    if (m_fib.fEncrypted)
    {
        error(__LINE__, "the document is encrypted");
	return;
    }

    // Store away the streams for future use. Note that we do not
    // copy the contents of the streams, and that we rely on the storage
    // being present until we are destroyed.

    m_mainStream = mainStream;
    m_tableStream = m_fib.fWhichTblStm ? table1Stream : table0Stream;
    m_dataStream = dataStream;
    if (!m_tableStream)
    {
        error(__LINE__, "the tableStream is missing");
	return;
    }

    // Start with the grpprl and PCD.

    typedef enum
    {
        clxtGrpprl = 1,
        clxtPlcfpcd = 2
    };

    const U8 *ptr;
    const U8 *end;
    U8 clxt = 0;
    U16 cb;
    U32 lcb;

    ptr = m_tableStream + m_fib.fcClx;
    end = ptr + m_fib.lcbClx;
    m_grpprls.byteCountOffset = (ptr + 1) - m_tableStream;
    m_grpprls.count = 0;
    while (ptr < end)
    {
	ptr += MsWordGenerated::read(ptr, &clxt);
        if (clxt != clxtGrpprl)
	{
	    ptr--;
            break;
	}
	m_grpprls.count++;
	ptr += MsWordGenerated::read(ptr, &cb);
	ptr += cb;
    }

    // For the text plex, we store the start and size of the plex in the table

    struct
    {
    	const U8 *ptr;
        U32 byteCount;
    } m_textPlex;
    unsigned count = 0;

    while (ptr < end)
    {
	ptr += MsWordGenerated::read(ptr, &clxt);
        if (clxt != clxtPlcfpcd)
	{
	    ptr--;
            break;
	}
	count++;
	ptr += MsWordGenerated::read(ptr, &lcb);
        m_textPlex.byteCount = lcb;
        m_textPlex.ptr = ptr;
	ptr += lcb;
    }
    if ((clxt != clxtPlcfpcd) ||
	(count != 1))
    {
        error(__LINE__, "cannot locate the piece table");
	return;
    };
    m_pcd = new Plex<PCD>(this, m_textPlex.ptr, m_textPlex.byteCount);
}

MsWord::~MsWord()
{
}

void MsWord::error(unsigned line, const char *reason)
{
    m_error.sprintf("[" __FILE__ ":%u] %s", line, reason);
    kDebugError(area, m_error);
}

void MsWord::getParagraphs()
{
    if (m_error.length())
    {
       gotParagraph(m_error);
       return;
    }

    // Note that we test for the presence of complex structure, rather than
    // m_fib.fComplex. This allows us to treat newer files which always seem
    // to have piece tables in a consistent manner.
    //
    // There is also the implication that without the complex structures, the
    // text cannot be in unicode form.
//kDebugError(area, "pause for debug");
//sleep(5);
    if (m_fib.lcbClx)
    {
        U32 startFc;
        U32 endFc;
        PCD data;
        const U32 codepage1252mask = 0x40000000;
        bool unicode;

        m_pcd->startIteration();
        while (m_pcd->getNext(&startFc, &endFc, &data))
        {
            unicode = ((data.fc & codepage1252mask) != codepage1252mask);
            if (!unicode)
            {
	        data.fc &= ~ codepage1252mask;
	        data.fc /= 2;
            }
            getPAPXFKP(m_mainStream + data.fc, endFc - startFc, unicode);
        }
    }
    else
    {
        getPAPXFKP(
            m_mainStream + m_fib.fcMin,
            m_fib.fcMac - m_fib.fcMin,
            false);
    }
}

void MsWord::gotParagraph(const QString &text)
{
    kDebugInfo(area, QString("MsWord::gotParagraph: ") + text);
};

void MsWord::getStyles()
{
    if (m_error.length())
    {
       gotParagraph(m_error);
       return;
    }

    // STSHI.

    const U8 *ptr = m_tableStream + m_fib.fcStshf;
    U16 cbStshi;
    STSHI m_stshi;

    ptr += MsWordGenerated::read(ptr, &cbStshi);
    if (cbStshi != sizeof(m_stshi))
    {
        error(__LINE__, "unsupported STSHI size");
        return;
    };
    ptr += MsWordGenerated::read(ptr, &m_stshi);
    for (unsigned i = 0; i < m_stshi.cstd; i++)
    {
        U16 cbStd;
	STD std;

        ptr += MsWordGenerated::read(ptr, &cbStd);
        if (cbStd)
        {
            read(ptr, &std);
            gotStyle(std.xstzName);
        }
        else
        {
            kDebugInfo(
                area,
                "MsWord::Std: style %u: is unused",
                i);
        }
        ptr += cbStd;
    }
}

void MsWord::gotStyle(const QString &name)
{
    kDebugInfo(area, QString("MsWord::gotStyle: ") + name);
};

void MsWord::getPAPXFKP(const U8 *textStartFc, U32 textLength, bool unicode)
{
    // A bin table is a plex of BTEs.

    Plex<BTE> btes = Plex<BTE>(
                       this,
                       m_tableStream + m_fib.fcPlcfbtePapx,
                       m_fib.lcbPlcfbtePapx);
    U32 startFc;
    U32 endFc;
    BTE data;

    btes.startIteration();
    while (btes.getNext(&startFc, &endFc, &data))
    {
        getPAPX(
            m_mainStream + (data.pn * 512),
            textStartFc,
            textLength,
            unicode);
    }
}

void MsWord::getPAPX(
    const U8 *fkp,
    const U8 *textStartFc,
    U32 textLength,
    bool unicode)
{
    // A PAPX FKP contains PHEs.

    Fkp<PHE, PAPXFKP> papx = Fkp<PHE, PAPXFKP>(this, fkp);

    U32 startFc;
    U32 endFc;
    U8 rgb;
    PHE layoutData;
    PAPXFKP styleData;

    papx.startIteration();
    while (papx.getNext(&startFc, &endFc, &rgb, &layoutData, &styleData))
    {
        QString text;

        read(m_mainStream + startFc, &text, endFc - startFc, unicode);
        gotParagraph(text);
    }
}

template <class T1, class T2>
MsWord::Fkp<T1, T2>::Fkp(MsWord *client, const U8 *fkp) :
    m_client(client),
    m_fkp(fkp)
{
    // Get the number of entries in the FKP.

    MsWordGenerated::read(m_fkp + 511, &m_crun);
    kDebugError(area, "MsWord::Fkp::Fkp: crun: %u", (unsigned)m_crun);
};

template <class T1, class T2>
void MsWord::Fkp<T1, T2>::startIteration()
{
    U32 startFc;

    m_fcNext = m_fkp;
    m_dataNext = m_fkp + ((m_crun + 1) * sizeof(startFc));
    m_i = 0;
}

template <class T1, class T2>
bool MsWord::Fkp<T1, T2>::getNext(
    U32 *startFc,
    U32 *endFc,
    U8 *rgb,
    T1 *data1,
    T2 *data2)
{
    // Sanity check accesses beyond end of Fkp.

    kDebugError(
        m_i >= m_crun,
        area,
        "MsWord::getNext: attempt to read past end of FKP: i: %u, crun: %u",
        (unsigned)m_i,
        (unsigned)m_crun);
    m_i++;

    // Get fc range.

    m_fcNext += MsWordGenerated::read(m_fcNext, startFc);
    MsWordGenerated::read(m_fcNext, endFc);

    // Get word count, and return if the data is not explictly stored.

    m_dataNext += MsWordGenerated::read(m_dataNext, rgb);
    if (!(*rgb))
    {
        kDebugInfo(
            area,
            "MsWord::getNext: %u:%u: default PAPX/CHPX",
            *startFc,
            *endFc);
        return (m_i <= m_crun);
    }

    // Get the requested data.

    m_dataNext += MsWordGenerated::read(m_dataNext, data1);
    MsWord::read(m_fkp + (2 * (*rgb)), data2);
    return (m_i <= m_crun);
}

template <class T>
MsWord::Plex<T>::Plex(MsWord *client, const U8 *plex, const U32 byteCount) :
    m_client(client),
    m_plex(plex),
    m_byteCount(byteCount)
{
    U32 startFc;

    // Calculate the number of entries in the plex.

    m_crun = (m_byteCount - sizeof(startFc)) / (sizeof(T) + sizeof(startFc));
    kDebugError(area, "MsWord::Plex::Plex");
};

template <class T>
void MsWord::Plex<T>::startIteration()
{
    U32 startFc;

    m_fcNext = m_plex;
    m_dataNext = m_plex + ((m_crun + 1) * sizeof(startFc));
    m_i = 0;
}

template <class T>
bool MsWord::Plex<T>::getNext(U32 *startFc, U32 *endFc, T *data)
{
    if (m_i == m_crun)
        return false;
    m_fcNext += MsWordGenerated::read(m_fcNext, startFc);
    MsWordGenerated::read(m_fcNext, endFc);
    m_dataNext += MsWordGenerated::read(m_dataNext, data);
    m_i++;
    return true;
}

unsigned MsWord::read(const U8 *in, QString *out, unsigned count, bool unicode)
{
    U16 char16;
    U8 char8;
    unsigned bytes = 0;

    *out = QString("");
    if (unicode)
    {
        for (unsigned i = 0; i < count; i++)
        {
           bytes += MsWordGenerated::read(in + bytes, &char16);
           *out += QChar(char16);
        }
    }
    else
    {
        for (unsigned i = 0; i < count; i++)
        {
           bytes += MsWordGenerated::read(in + bytes, &char8);
           *out += QChar(char2unicode(char8));
        }
    }
    kDebugError(area, QString("MsWord::read: ") + *out);
    return bytes;
}

unsigned MsWord::read(const U8 *in, PAPXFKP *out, unsigned count)
{
    unsigned bytes = 0;
    U8 cw;

    for (unsigned i = 0; i < count; i++)
    {
        bytes += MsWordGenerated::read(in + bytes, &cw);
        if (!cw)
        {
            bytes += MsWordGenerated::read(in + bytes, &cw);
            out->grpprlBytes = 2 * (cw - 1);
        }
        else
        {
            out->grpprlBytes = 2 * (cw - 1) - 1;
        }
        bytes += MsWordGenerated::read(in + bytes, &out->istd);
        out->grpprl = (U8 *)(in + bytes);
        bytes += out->grpprlBytes;
        out++;
    }
    kDebugError(area, "MsWord::read: PAPXFKP grpprl bytes %u", out->grpprl);
    return bytes;
}

unsigned MsWord::read(const U8 *in, STD *out, unsigned count)
{
    U8 *ptr = (U8 *)out;
    unsigned bytes = 0;

    for (unsigned i = 0; i < count; i++)
    {
       U16 nameLength;
       U8 offset;

       offset = 0;
       offset += MsWordGenerated::read(in + offset, (U16 *)(ptr + bytes), 5);
       offset += MsWordGenerated::read(in + offset, &nameLength);
       offset += read(in + offset, &out->xstzName, nameLength, true);
       out->grupx = in + offset;
       bytes += out->bchUpe;
       out++;
    }
    return bytes;
} // STD

unsigned MsWord::read(const U8 *in, FIB *out, unsigned count)
{
    U8 *ptr = (U8 *)out;
    unsigned bytes = 0;

    for (unsigned i = 0; i < count; i++)
    {
        // What version of Word are we dealing with?
        // Word 6 writes files with nFib = 101-104.
        // Winword 97 and later products write files with nFib > 105.

        // Bytes 0 to 31 are common.

        bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes), 7);
        bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes), 1);
        bytes += MsWordGenerated::read(in + bytes, (U8 *)(ptr + bytes), 2);
        bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes), 2);
        bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes), 2);
        if (out->nFib > 105)
        {
            bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes),
16);
            bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes),
22);
            bytes += MsWordGenerated::read(in + bytes, (U16 *)(ptr + bytes),
1);
            bytes += MsWordGenerated::read(in + bytes, (U32 *)(ptr + bytes),
186);
        }
        else
        if (out->nFib > 100)
        {
            // We will convert the FIB into the same form as for Winword

            out->csw = 14;
            out->wMagicCreated = 0;
            out->wMagicRevised = 0;
            out->wMagicCreatedPrivate = 0;
            out->wMagicRevisedPrivate = 0;
            out->pnFbpChpFirst_W6 = 0;
            out->pnChpFirst_W6 = 0;
            out->cpnBteChp_W6 = 0;
            out->pnFbpPapFirst_W6 = 0;
            out->pnPapFirst_W6 = 0;
            out->cpnBtePap_W6 = 0;
            out->pnFbpLvcFirst_W6 = 0;
            out->pnLvcFirst_W6 = 0;
            out->cpnBteLvc_W6 = 0;
            out->lidFE = out->lid;
            out->clw = 22;
            bytes += MsWordGenerated::read(in + bytes, &out->cbMac);
            bytes += 16;
            out->lProductCreated = 0;
            out->lProductRevised = 0;

            // ccpText through ccpHdrTxbx.

            bytes += MsWordGenerated::read(in + bytes, &out->ccpText, 8);

            // ccpSpare2.

            bytes += 4;
            out->cfclcb = 93;

            // fcStshfOrig through lcbSttbfAtnbkmk.

            bytes += MsWordGenerated::read(in + bytes, &out->fcStshfOrig, 76);

            // wSpare4Fib.

            bytes += 2;

            // pnChpFirst through cpnBtePap.

            U16 tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->pnChpFirst = tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->pnPapFirst = tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->cpnBteChp = tmp;
            bytes += MsWordGenerated::read(in + bytes, &tmp);
            out->cpnBtePap = tmp;

            // fcPlcdoaMom through lcbSttbFnm.

            bytes += MsWordGenerated::read(in + bytes, &out->fcPlcdoaMom, 70);
        }
        else
        {
            // We don't support this.

            kDebugError(
                area,
                "unsupported version of Word (nFib=%hu)",
                out->nFib);
            break;
        }
        out++;
    }
    return bytes;
} // FIB


