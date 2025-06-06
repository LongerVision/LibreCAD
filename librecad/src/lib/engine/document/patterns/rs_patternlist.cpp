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

#include<iostream>
#include "rs_patternlist.h"

#include <QFileInfo>
#include <QStringList>

#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_pattern.h"
#include "rs_system.h"

RS_PatternList* RS_PatternList::instance() {
	static RS_PatternList instance;
	return &instance;
}

RS_PatternList::~RS_PatternList() = default;

/**
 * Initializes the pattern list by creating empty RS_Pattern 
 * objects, one for each pattern that could be found.
 */
void RS_PatternList::init() {
    RS_DEBUG->print("RS_PatternList::initPatterns");

	QStringList list = RS_SYSTEM->getPatternList();

	patterns.clear();

    foreach(auto const& s, list) {
        RS_DEBUG->print("pattern: %s:", s.toLatin1().data());

        QString const name = QFileInfo(s).baseName().toLower();
        patterns.emplace(name, std::unique_ptr<RS_Pattern>{});

        RS_DEBUG->print("base: %s", name.toLatin1().data());
    }
    if (patterns.empty())
        RS_DIALOGFACTORY->commandMessage(QObject::tr("Hatch:: no pattern found. Please set pattern path in application preferences"));
}


/**
 * @return Pointer to the pattern with the given name or
 * \p NULL if no such pattern was found. The pattern will be loaded into
 * memory if it's not already.
 */
std::unique_ptr<RS_Pattern> RS_PatternList::requestPattern(const QString& name) {
    RS_DEBUG->print("RS_PatternList::requestPattern %s", name.toLatin1().data());

    QString name2 = name.toLower();
    RS_DEBUG->print("Pattern: name2: %s", name2.toLatin1().data());
    if (patterns.count(name2) == 0 || patterns.at(name2) == nullptr) {
        auto p = std::make_unique<RS_Pattern>(name2);
        if (p!=nullptr) {
            if (p->loadPattern()) {
                patterns.emplace(name2,  std::unique_ptr<RS_Pattern>{});
                patterns[name2].swap(p);
            }
            else {
                patterns.erase(name2);
            }
        }
        else {
            LC_ERR<<"RS_PatternList::"<<__func__<<"(): loading pattern failed: "<<name2;
            RS_DIALOGFACTORY->commandMessage(QObject::tr("Hatch:: loading pattern failed: %1").arg(name2));
            return {};
        }
    }

    if (patterns.count(name2) == 1) {
        RS_DEBUG->print("name2: %s, size= %d", name2.toLatin1().data(),
                        patterns[name2]->countDeep());
        return std::unique_ptr<RS_Pattern>{static_cast<RS_Pattern*>(patterns[name2]->clone())};
	}

    return {};

}

	
bool RS_PatternList::contains(const QString& name) const {

	return patterns.count(name.toLower());

}


/**
 * Dumps the patterns to stdout.
 */
std::ostream& operator << (std::ostream& os, RS_PatternList& l) {

    os << "Patternlist: \n";
	for (auto const& pa: l.patterns)
		if (pa.second)
			os<< *pa.second << '\n';

    return os;
}
