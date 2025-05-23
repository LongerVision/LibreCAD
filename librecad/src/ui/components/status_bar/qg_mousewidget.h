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
#ifndef QG_MOUSEWIDGET_H
#define QG_MOUSEWIDGET_H

#include "ui_qg_mousewidget.h"

class LC_ModifiersInfo;

class QG_MouseWidget : public QWidget, public Ui::QG_MouseWidget{
    Q_OBJECT
public:
    QG_MouseWidget(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = {});
    ~QG_MouseWidget() override;

    void updatePixmap(QString iconName, QLabel *label);
    void setHelp( const QString & left, const QString & right, const LC_ModifiersInfo& modifiersInfo) const;
    void setActionIcon(QIcon icon);
    void clearActionIcon();
public slots:
    void languageChange();
    void setupModifier(QLabel *btn, const QString& helpMsg) const;
    void onIconsRefreshed();
    void setCurrentQAction(QAction *a);
protected:
    int iconSize = 24;
};

#endif // QG_MOUSEWIDGET_H
