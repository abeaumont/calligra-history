/* This file is part of the KDE project
   Copyright (C) 1999 Werner Trobin <wtrobin@carinthia.com>

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

#include <style.h>

// Size codes for the CHPX and PAPX sprm records
// Note: 0...variable length!
// const unsigned char SPRM_Size[]= { 1, 1, 2, 4, 2, 2, 0, 3 };

//////////////////////////////////////////////////////////////////////////////
// CStyle - creates everything between <FORMAT> and </FORMAT>
//////////////////////////////////////////////////////////////////////////////
CStyle::CStyle(const myFile &main, const unsigned short &styleID, unsigned char id) {

    if(id>0 && id<6)
        _id=id;
    else
        _id=1;

    stID=styleID;
    _main=main;
    onlyLayout=false;
    _pos=0;
    _len=0;

    // init...
    if(_id==1) {
        data.text.red=0;
        data.text.green=0;
        data.text.blue=0;
        data.text.fontname=new char[6];
        data.text.fontname[0]='t';
        data.text.fontname[1]='i';
        data.text.fontname[2]='m';
        data.text.fontname[3]='e';
        data.text.fontname[4]='s';
        data.text.fontname[5]='\0';
        data.text.size=12;
        data.text.weight=50;
        data.text.italic=0;
        data.text.underline=0;
        data.text.vertalign=0;
    }
    else if(_id==2)
        data.image.filename=0L;
    else if(_id==4) {
        data.variable.type=0;
        data.variable.frameSet=1;
        data.variable.frame=1;
        data.variable.pageNum=1;
        data.variable.PGNum=1;
        data.variable.year=1999;
        data.variable.month=1;
        data.variable.day=1;
        data.variable.fix=0;
        data.variable.hour=0;
        data.variable.minute=0;
        data.variable.second=0;
        data.variable.msecond=0;
        data.variable.red=0;
        data.variable.green=0;
        data.variable.blue=0;
        data.variable.fontname=new char[6];
        data.variable.fontname[0]='t';
        data.variable.fontname[1]='i';
        data.variable.fontname[2]='m';
        data.variable.fontname[3]='e';
        data.variable.fontname[4]='s';
        data.variable.fontname[5]='\0';
        data.variable.size=12;
        data.variable.weight=50;
        data.variable.italic=0;
        data.variable.underline=0;
        data.variable.vertalign=0;
    }
    else if(_id==5) {
        data.footnote.from=0;
        data.footnote.to=0;
        data.footnote.space='-';
        data.footnote.start=0;
        data.footnote.end=0;
        data.footnote.before=new char[3];
        data.footnote.before[0]='[';
        data.footnote.before[1]=' ';
        data.footnote.before[2]='\0';
        data.footnote.after=new char[3];
        data.footnote.after[0]=' ';
        data.footnote.after[1]=']';
        data.footnote.after[2]='\0';
        data.footnote.ref=0L;
        data.footnote.red=0;
        data.footnote.green=0;
        data.footnote.blue=0;
        data.footnote.fontname=new char[6];
        data.footnote.fontname[0]='t';
        data.footnote.fontname[1]='i';
        data.footnote.fontname[2]='m';
        data.footnote.fontname[3]='e';
        data.footnote.fontname[4]='s';
        data.footnote.fontname[5]='\0';
        data.footnote.size=12;
        data.footnote.weight=50;
        data.footnote.italic=0;
        data.footnote.underline=0;
        data.footnote.vertalign=0;
    }
}

CStyle::CStyle(const CStyle &rhs) {

    _id=rhs.id();
    _pos=rhs.pos();
    _len=rhs.len();

    stID=rhs.styleID();
    _main=rhs.main();
    onlyLayout=rhs.layout();

    // init...
    if(_id==1) {
        data.text.red=rhs.textRed();
        data.text.green=rhs.textGreen();
        data.text.blue=rhs.textBlue();
        int len=strlen(rhs.textFontname());
        data.text.fontname=new char[len+1];
        strcpy(data.text.fontname, rhs.textFontname());
        data.text.fontname[len]='\0';  // just in case...
        data.text.size=rhs.textSize();
        data.text.weight=rhs.textWeight();
        data.text.italic=rhs.textItalic();
        data.text.underline=rhs.textUnderline();
        data.text.vertalign=rhs.textVertalign();
    }
    else if(_id==2) {
        int len=strlen(rhs.imageFilename());
        data.image.filename=new char[len+1];
        strcpy(data.image.filename, rhs.imageFilename());
        data.image.filename[len]='\0';
    }
    else if(_id==4) {
        data.variable.type=rhs.variableType();
        data.variable.frameSet=rhs.variableFrameSet();
        data.variable.frame=rhs.variableFrame();
        data.variable.pageNum=rhs.variablePageNum();
        data.variable.PGNum=rhs.variablePGNum();
        data.variable.year=rhs.variableYear();
        data.variable.month=rhs.variableMonth();
        data.variable.day=rhs.variableDay();
        data.variable.fix=rhs.variableFix();
        data.variable.hour=rhs.variableHour();
        data.variable.minute=rhs.variableMinute();
        data.variable.second=rhs.variableSecond();
        data.variable.msecond=rhs.variableMsecond();
        data.variable.red=rhs.variableRed();
        data.variable.green=rhs.variableGreen();
        data.variable.blue=rhs.variableBlue();
        int len=strlen(rhs.variableFontname());
        data.variable.fontname=new char[len+1];
        strcpy(data.variable.fontname, rhs.variableFontname());
        data.variable.fontname[len]='\0';  // just in case...
        data.variable.size=rhs.variableSize();
        data.variable.weight=rhs.variableWeight();
        data.variable.italic=rhs.variableItalic();
        data.variable.underline=rhs.variableUnderline();
        data.variable.vertalign=rhs.variableVertalign();
    }
    else if(_id==5) {
        data.footnote.from=rhs.footnoteFrom();
        data.footnote.to=rhs.footnoteTo();
        data.footnote.space=rhs.footnoteSpace();
        data.footnote.start=rhs.footnoteStart();
        data.footnote.end=rhs.footnoteEnd();
        int len=strlen(rhs.footnoteBefore());
        data.footnote.before=new char[len+1];
        strcpy(data.footnote.before, rhs.footnoteBefore());
        data.footnote.before[len]='\0';
        len=strlen(rhs.footnoteAfter());
        data.footnote.after=new char[len+1];
        strcpy(data.footnote.after, rhs.footnoteAfter());
        data.footnote.after[len]='\0';
        len=strlen(rhs.footnoteRef());
        data.footnote.ref=new char[len+1];
        strcpy(data.footnote.ref, rhs.footnoteRef());
        data.footnote.ref[len]='\0';
        data.footnote.red=rhs.footnoteRed();
        data.footnote.green=rhs.footnoteGreen();
        data.footnote.blue=rhs.footnoteBlue();
        len=strlen(rhs.footnoteFontname());
        data.footnote.fontname=new char[len+1];
        strcpy(data.footnote.fontname, rhs.footnoteFontname());
        data.footnote.fontname[len]='\0';  // just in case...
        data.footnote.size=rhs.footnoteSize();
        data.footnote.weight=rhs.footnoteWeight();
        data.footnote.italic=rhs.footnoteItalic();
        data.footnote.underline=rhs.footnoteUnderline();
        data.footnote.vertalign=rhs.footnoteVertalign();
    }
}

CStyle::~CStyle() {

    if(_id==1) {
        delete [] data.text.fontname;
        data.text.fontname=0L;
    }
    else if(_id==2) {
        delete [] data.image.filename;
        data.image.filename=0L;
    }
    else if(_id==4) {
        delete [] data.variable.fontname;
        data.variable.fontname=0L;
    }
    else if(_id==5) {
        delete [] data.footnote.fontname;
        data.footnote.fontname=0L;
        delete [] data.footnote.before;
        data.footnote.before=0L;
        delete [] data.footnote.after;
        data.footnote.after=0L;
        delete [] data.footnote.ref;
        data.footnote.ref=0L;

    }
}

/*void CStyle::applyCHPX(const int &fcGrpprl, const unsigned short &cb) {

    unsigned short offset=0;
    unsigned short value, size;

    while(cb>offset) {
        value=( *(_main.data+fcGrpprl+offset+1) << 8 ) + *(_main.data+fcGrpprl+offset);
        size=value & 0xE000;
        size=static_cast<unsigned short>(SPRM_Size[size]);

        // variable length!
        if(size==0) {
            if(value==0xd606 || value==0xd608)
                size=( *(_main.data+fcGrpprl+offset+3) << 8 ) + *(_main.data+fcGrpprl+offset+2) + 1;
            else if(value==0xc615) {
                unsigned char tmp=*(_main.data+fcGrpprl+offset+2);
                if(tmp<255)
                    size=tmp+1;
                else {
                    kDebugInfo(31000, "Sigh - don't know the length of that sprm! Guessing :)");
                    size=3;  // don't know really, but the algorithm should be quite ok >:)
                    tmp=*(_main.data+fcGrpprl+offset+3);
                    tmp*=4;
                    size+=tmp;
                    size+=*(_main.data+fcGrpprl+offset+3+tmp)*3;
                }
            }
            else
                size=*(_main.data+fcGrpprl+offset+2)+1;
        }

        switch(value) {
            case 0x6a03:
                // picture! I'll have to handle this via a SIGNAL!
                // will implement that later :)
                // Note: Style will have to be changed - impossible -
                // create new Style(s) :)
                break;
            case 0x0806:
                // also via SIGNAL?
                break;
            case 0x6a09:
                break;
            case 0x800a:
                break;
            case 0x680e:
                break;
            case 0x6865:
                break;
            case 0x4866:
                break;
            default:
                break;
        }
        offset+=2+size;      // offset to next sprm
    }
}
*/

void CStyle::setLayout(bool l) {
    onlyLayout=l;
}

const QString CStyle::format() {

    QString format;

    if(_id==1 && onlyLayout)
        format=QString("<FORMAT>\n");
    else {
        format=QString("<FORMAT id=\"");
        format+=QString::number(static_cast<int>(_id));
        format+="\" pos=\"";
        format+=QString::number(static_cast<int>(_pos));
        if(_id==1) {
            format+="\" len=\"";
            format+=QString::number(static_cast<int>(_len));
        }
        format+="\">\n";
    }

    if(_id==1) {
        format+="<COLOR red=\"";
        format+=QString::number(static_cast<int>(data.text.red));
        format+="\" green=\"";
        format+=QString::number(static_cast<int>(data.text.green));
        format+="\" blue=\"";
        format+=QString::number(static_cast<int>(data.text.blue));
        format+="\"/>\n<FONT name=\"";
        format+=QString(data.text.fontname);
        format+="\"/>\n<SIZE value=\"";
        format+=QString::number(static_cast<int>(data.text.size));
        format+="\"/>\n<WEIGHT value=\"";
        format+=QString::number(static_cast<int>(data.text.weight));
        format+="\"/>\n<ITALIC value=\"";
        format+=QString::number(static_cast<int>(data.text.italic));
        format+="\"/>\n<UNDERLINE value=\"";
        format+=QString::number(static_cast<int>(data.text.underline));
        format+="\"/>\n<VERTALIGN value=\"";
        format+=QString::number(static_cast<int>(data.text.vertalign));
        format+="\"/>\n";
    }
    else if(_id==2) {
        format+="<FILENAME value=\"";
        format+=QString(data.image.filename);
        format+="\"/>\n";
    }
    else if(_id==4) {
        format+="<TYPE type=\"";
        format+=QString::number(static_cast<int>(data.variable.type));
        format+="\"/>\n<POS frameSet=\"";
        format+=QString::number(static_cast<int>(data.variable.frameSet));
        format+="\" frame=\"";
        format+=QString::number(static_cast<int>(data.variable.frame));
        format+="\" pageNum=\"";
        format+=QString::number(static_cast<int>(data.variable.pageNum));
        format+="\"/>\n";
        if(data.variable.type==0 || data.variable.type==1) {
            format+="<DATE year=\"";
            format+=QString::number(static_cast<int>(data.variable.year));
            format+="\" month=\"";
            format+=QString::number(static_cast<int>(data.variable.month));
            format+="\" day=\"";
            format+=QString::number(static_cast<int>(data.variable.day));
            format+="\" fix=\"";
            format+=QString::number(static_cast<int>(data.variable.fix));
            format+="\"/>\n<FRMAT>\n";
        }
        else if(data.variable.type==2 || data.variable.type==3) {
            format+="<TIME hour=\"";
            format+=QString::number(static_cast<int>(data.variable.hour));
            format+="\" minute=\"";
            format+=QString::number(static_cast<int>(data.variable.minute));
            format+="\" second=\"";
            format+=QString::number(static_cast<int>(data.variable.second));
            format+="\" msecond=\"";
            format+=QString::number(static_cast<int>(data.variable.msecond));
            format+="\" fix=\"";
            format+=QString::number(static_cast<int>(data.variable.fix));
            format+="\"/>\n<FRMAT>\n";
        }
        else if(data.variable.type==4) {
            format+="<PGNUM value=\"";
            format+=QString::number(static_cast<int>(data.variable.PGNum));
            format+="\"/>\n<FRMAT>\n";
        }
        format+="<COLOR red=\"";
        format+=QString::number(static_cast<int>(data.variable.red));
        format+="\" green=\"";
        format+=QString::number(static_cast<int>(data.variable.green));
        format+="\" blue=\"";
        format+=QString::number(static_cast<int>(data.variable.blue));
        format+="\"/>\n<FONT name=\"";
        format+=QString(data.variable.fontname);
        format+="\"/>\n<SIZE value=\"";
        format+=QString::number(static_cast<int>(data.variable.size));
        format+="\"/>\n<WEIGHT value=\"";
        format+=QString::number(static_cast<int>(data.variable.weight));
        format+="\"/>\n<ITALIC value=\"";
        format+=QString::number(static_cast<int>(data.variable.italic));
        format+="\"/>\n<UNDERLINE value=\"";
        format+=QString::number(static_cast<int>(data.variable.underline));
        format+="\"/>\n<VERTALIGN value=\"";
        format+=QString::number(static_cast<int>(data.variable.vertalign));
        format+="\"/>\n</FRMAT>\n";
    }
    else if(_id==5) {
        format+="<INTERNAL>\n<PART from=\"";
        format+=QString::number(static_cast<int>(data.footnote.from));
        format+="\" to=\"";
        format+=QString::number(static_cast<int>(data.footnote.to));
        format+="\" space=\"";
        format+=QChar(data.footnote.space);
        format+="\"/>\n</INTERNAL>\n<RANGE start=\"";
        format+=QString::number(static_cast<int>(data.footnote.start));
        format+="\" end=\"";
        format+=QString::number(static_cast<int>(data.footnote.end));
        format+="\"/>\n<TEXT before=\"";
        format+=QString(data.footnote.before);
        format+="\" after=\"";
        format+=QString(data.footnote.after);
        format+="\"/>\n<DESCRIPT ref=\"";
        format+=QString(data.footnote.ref);
        format+="\"/>\n<FRMAT>\n";
        format+="<COLOR red=\"";
        format+=QString::number(static_cast<int>(data.footnote.red));
        format+="\" green=\"";
        format+=QString::number(static_cast<int>(data.footnote.green));
        format+="\" blue=\"";
        format+=QString::number(static_cast<int>(data.footnote.blue));
        format+="\"/>\n<FONT name=\"";
        format+=QString(data.footnote.fontname);
        format+="\"/>\n<SIZE value=\"";
        format+=QString::number(static_cast<int>(data.footnote.size));
        format+="\"/>\n<WEIGHT value=\"";
        format+=QString::number(static_cast<int>(data.footnote.weight));
        format+="\"/>\n<ITALIC value=\"";
        format+=QString::number(static_cast<int>(data.footnote.italic));
        format+="\"/>\n<UNDERLINE value=\"";
        format+=QString::number(static_cast<int>(data.footnote.underline));
        format+="\"/>\n<VERTALIGN value=\"";
        format+=QString::number(static_cast<int>(data.footnote.vertalign));
        format+="\"/>\n</FRMAT>\n";
    }
    format+="</FORMAT>";
    return format;
}

void CStyle::textFontname(const QString &name) {
    if(_id==1) {
        if(data.text.fontname!=0)
            delete [] data.text.fontname;
        int len=name.length();
        data.text.fontname=new char[len+1];
        strncpy(data.text.fontname, static_cast<const char*>(name), len);
        data.text.fontname[len]='\0';
    }
}

void CStyle::imageFilename(const QString &name) {
    if(_id==2) {
        if(data.image.filename!=0)
            delete [] data.image.filename;
        int len=name.length();
        data.image.filename=new char[len+1];
        strncpy(data.image.filename, static_cast<const char*>(name), len);
        data.image.filename[len]='\0';
    }
}

void CStyle::variableFontname(const QString &name) {
    if(_id==4) {
        if(data.variable.fontname!=0)
            delete [] data.variable.fontname;
        int len=name.length();
        data.variable.fontname=new char[len+1];
        strncpy(data.variable.fontname, static_cast<const char*>(name), len);
        data.variable.fontname[len]='\0';
    }
}

void CStyle::footnoteFontname(const QString &name) {
    if(_id==5) {
        if(data.footnote.fontname!=0)
            delete [] data.footnote.fontname;
        int len=name.length();
        data.footnote.fontname=new char[len+1];
        strncpy(data.footnote.fontname, static_cast<const char*>(name), len);
        data.footnote.fontname[len]='\0';
    }
}

void CStyle::footnoteBefore(const QString &name) {
    if(_id==5) {
        if(data.footnote.before!=0)
            delete [] data.footnote.before;
        int len=name.length();
        data.footnote.before=new char[len+1];
        strncpy(data.footnote.before, static_cast<const char*>(name), len);
        data.footnote.before[len]='\0';
    }
}

void CStyle::footnoteAfter(const QString &name) {
    if(_id==5) {
        if(data.footnote.after!=0)
            delete [] data.footnote.after;
        int len=name.length();
        data.footnote.after=new char[len+1];
        strncpy(data.footnote.after, static_cast<const char*>(name), len);
        data.footnote.after[len]='\0';
    }
}

void CStyle::footnoteRef(const QString &name) {
    if(_id==5) {
        if(data.footnote.ref!=0)
            delete [] data.footnote.ref;
        int len=name.length();
        data.footnote.ref=new char[len+1];
        strncpy(data.footnote.ref, static_cast<const char*>(name), len);
        data.footnote.ref[len]='\0';
    }
}

//////////////////////////////////////////////////////////////////////////////
// PStyle - creates everything between <LAYOUT> and </LAYOUT>
//////////////////////////////////////////////////////////////////////////////
PStyle::PStyle(const myFile &main, const unsigned short &styleID, const CStyle &cstyle) :
               _cstyle(cstyle) {

    _styleID=styleID;
    _main=main;
    _layoutTag=true;
    _cstyle.setLayout();   // just to be sure!

    _name="Standard";
    _following="Standard";
    _flow=0;
    _ohead_pt=0;
    _ofoot_pt=0;
    _ifirst_pt=0;
    _ileft_pt=0;
    _linespace_pt=0;
    counter.type=0;
    counter.depth=0;
    counter.bullet=176;
    counter.start=1;
    counter.nmbType=1;
    counter.lefttext="";
    counter.righttext="";
    counter.bulletfont="times";
    _left.red=255;
    _left.green=255;
    _left.blue=255;
    _left.style=0;
    _left.width=0;
    _right=_top=_bottom=_left;
}

PStyle::PStyle(const PStyle &rhs) : _cstyle(rhs.cstyle()) {

    _styleID=rhs.styleID();
    _layoutTag=rhs.layoutTag();
    _main=rhs.main();

    _name=rhs.name();
    _following=rhs.following();
    _flow=rhs.flow();
    _ohead_pt=rhs.oheadPT();
    _ofoot_pt=rhs.ofootPT();
    _ifirst_pt=rhs.ifirstPT();
    _ileft_pt=rhs.ileftPT();
    _linespace_pt=rhs.linespacePT();
    counter.type=rhs.counterType();
    counter.depth=rhs.counterDepth();
    counter.bullet=rhs.counterBullet();
    counter.start=rhs.counterStart();
    counter.nmbType=rhs.counterNmbType();
    counter.lefttext=rhs.counterLefttext();
    counter.righttext=rhs.counterRighttext();
    counter.bulletfont=rhs.counterBulletfont();
    _left=rhs.left();
    _right=rhs.right();
    _top=rhs.top();
    _bottom=rhs.bottom();
    _tabList=rhs.tabList();
}

PStyle::~PStyle() {
    _tabList.clear();
}

const QString PStyle::layout() {

    QString layout;

    if(_layoutTag)
        layout="<LAYOUT>\n<NAME value=\"";
    else
        layout="<STYLE>\n<NAME value=\"";

    layout+=_name;
    layout+="\"/>\n<FOLLOWING name=\"";
    layout+=_following;
    layout+="\"/>\n<FLOW value=\"";
    layout+=QString::number(static_cast<int>(_flow));
    layout+="\"/>\n<OHEAD ";
    layout+=unit(_ohead_pt);
    layout+="/>\n<OFOOT ";
    layout+=unit(_ofoot_pt);
    layout+="/>\n<IFIRST ";
    layout+=unit(_ifirst_pt);
    layout+="/>\n<ILEFT ";
    layout+=unit(_ileft_pt);
    layout+="/>\n<LINESPACE ";
    layout+=unit(_linespace_pt);
    layout+="/>\n<COUNTER type=\"";
    layout+=QString::number(static_cast<int>(counter.type));
    layout+="\" depth=\"";
    layout+=QString::number(static_cast<int>(counter.depth));
    layout+="\" bullet=\"";
    layout+=QString::number(static_cast<int>(counter.bullet));
    layout+="\" start=\"";
    layout+=QString::number(static_cast<int>(counter.start));
    layout+="\" numberingtype=\"";
    layout+=QString::number(static_cast<int>(counter.nmbType));
    layout+="\" lefttext=\"";
    layout+=counter.lefttext;
    layout+="\" righttext=\"";
    layout+=counter.righttext;
    layout+="\" bulletfont=\"";
    layout+=counter.bulletfont;
    layout+="\"/>\n<LEFTBORDER ";
    layout+=border(_left);
    layout+="/>\n<RIGHTBORDER ";
    layout+=border(_right);
    layout+="/>\n<TOPBORDER ";
    layout+=border(_top);
    layout+="/>\n<BOTTOMBORDER ";
    layout+=border(_bottom);
    layout+="/>\n";

    layout+=_cstyle.format();

    unsigned int count=_tabList.count();
    for(unsigned int i=0; i<count; ++i)
        layout+=tab(_tabList[i]);

    if(_layoutTag)
        layout+="</LAYOUT>";
    else
        layout+="</STYLE>";
    return layout;
}

const QString PStyle::unit(const unsigned short &u) {

    QString ret="pt=\"";
    ret+=QString::number(static_cast<int>(u));
    ret+="\" mm=\"";
    ret+=QString::number(static_cast<float>(u/2.83465));
    ret+="\" inch=\"";
    ret+=QString::number(static_cast<float>(u/72.0));
    ret+="\"";
    return ret;
}

const QString PStyle::border(const myBORDER &b) {

    QString ret="red=\"";
    ret+=QString::number(static_cast<int>(b.red));
    ret+="\" green=\"";
    ret+=QString::number(static_cast<int>(b.green));
    ret+="\" blue=\"";
    ret+=QString::number(static_cast<int>(b.blue));
    ret+="\" style=\"";
    ret+=QString::number(static_cast<int>(b.style));
    ret+="\" width=\"";
    ret+=QString::number(static_cast<int>(b.width));
    ret+="\"";
    return ret;
}

const QString PStyle::tab(const myTAB &t) {

    QString ret="<TABULATOR mmpos=\"";
    ret+=QString::number(static_cast<float>(t.pos_pt/2.83465));
    ret+="\" ptpos=\"";
    ret+=QString::number(static_cast<int>(t.pos_pt));
    ret+="\" inchpos=\"";
    ret+=QString::number(static_cast<float>(t.pos_pt/72.0));
    ret+="\"/>\n";
    return ret;
}
