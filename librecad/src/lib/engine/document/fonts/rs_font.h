/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/


#ifndef RS_FONT_H
#define RS_FONT_H

#include <QMap>
#include <QStringList>

#include "rs_blocklist.h"


class RS_BlockList;
/**
 * Class for representing a font. This is implemented as a RS_Graphic
 * with a name (the font name) and several blocks, one for each letter
 * in the font.
 *
 * @author Andrew Mustun
 */
class RS_Font {
public:
    RS_Font(const QString& name, bool owner=true);
    //RS_Font(const char* name);

    /** @return the fileName of this font. */
    QString getFileName() const {
        return m_fileName;
    }

    /** @return the fileLicense of this font. */
    QString getFileLicense() const {
        return fileLicense;
    }

    /** @return the creation date of this font. */
    QString getFileCreate() const {
        return fileCreate;
    }

    /** @return the encoding of this font. */
    QString getEncoding() const {
        return encoding;
    }

    /** @return the alternative names of this font. */
    const QStringList& getNames() const {
        return names;
    }

    /** @return the author(s) of this font. */
    const QStringList& getAuthors() const {
        return authors;
    }

    /** @return Default letter spacing for this font */
    double getLetterSpacing() {
        return letterSpacing;
    }

    /** @return Default word spacing for this font */
    double getWordSpacing() const {
        return wordSpacing;
    }

    /** @return Default line spacing factor for this font */
    double getLineSpacingFactor() const {
        return lineSpacingFactor;
    }

    bool loadFont();

    void generateAllFonts();

    // Wrappers for block list (letters) functions
    RS_BlockList* getLetterList() {
        return &letterList;
    }
    RS_Block* findLetter(const QString& name);
    //    RS_Block* findLetter(const QString& name) {
    //		return letterList.find(name);
    //	}
    unsigned countLetters() const;
    RS_Block* letterAt(unsigned i);

    friend std::ostream& operator << (std::ostream& os, const RS_Font& l);

    friend class RS_FontList;

private:
    void readCXF(const QString& path);
    void readLFF(const QString& path);
    RS_Block* generateLffFont(const QString& key);

private:
    //raw lff font file list, not processed into blocks yet
    QMap<QString, QStringList> rawLffFontList;

    //! block list (letters)
    RS_BlockList letterList;

    //! Font file name
    QString m_fileName;

    //! Font file license
    QString fileLicense;

    //! Font file license
    QString fileCreate;

    //! Font encoding (see docu for QTextCodec)
    QString encoding;

    //! Font names
    QStringList names;

    //! Authors
    QStringList authors;

    //! Is this font currently loaded into memory?
    bool loaded = false;

    //! Default letter spacing for this font
    double letterSpacing = 0.;

    //! Default word spacing for this font
    double wordSpacing = 0.;

    //! Default line spacing factor for this font
    double lineSpacingFactor = 0.;
};

#endif
