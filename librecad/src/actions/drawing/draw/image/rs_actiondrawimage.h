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

#ifndef RS_ACTIONDRAWIMAGE_H
#define RS_ACTIONDRAWIMAGE_H

#include "rs_previewactioninterface.h"

struct RS_ImageData;
class QImage;

/**
 * This action class can handle user events for inserting bitmaps into the
 * current drawing.
 *
 * @author Andrew Mustun
 */
class RS_ActionDrawImage : public RS_PreviewActionInterface {
Q_OBJECT
public:
    RS_ActionDrawImage(LC_ActionContext *actionContext);
    ~RS_ActionDrawImage() override;

    void init(int status) override;
    void reset();
    QStringList getAvailableCommands() override;
//    void updateToolBar() override;
    double getUcsAngleDegrees() const;
    void setUcsAngleDegrees(double ucsRelAngleDegrees);
    void setAngle(double wcsAngle) const;
    double getFactor() const;
    void setFactor(double f) const;
    double dpiToScale(double dpi) const;
    double scaleToDpi(double scale) const;
protected:
    struct ImageData;
    std::unique_ptr<ImageData> m_imageData;
/**
     * Action States.
     */
    enum Status {
        ShowDialog,        /**< Dialog. */
        SetTargetPoint,    /**< Setting the reference point. */
        SetAngle,          /**< Setting angle in the command line. */
        SetFactor,          /**< Setting factor in the command line. */
        SetDPI              /**< Setting dpi in the command line. */
//SetColumns,        /**< Setting columns in the command line. */
//SetRows,           /**< Setting rows in the command line. */
//SetColumnSpacing,  /**< Setting column spacing in the command line. */
//SetRowSpacing      /**< Setting row spacing in the command line. */
    };

/** Last status before entering option. */
    Status m_lastStatus = ShowDialog;

    RS2::CursorType doGetMouseCursor(int status) override;
    void onMouseLeftButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseRightButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseMoveEvent(int status, LC_MouseEvent *event) override;
    bool doProcessCommand(int status, const QString &command) override;
    void onCoordinateEvent(int status, bool isZero, const RS_Vector &pos) override;
    LC_ActionOptionsWidget* createOptionsWidget() override;
    void updateMouseButtonHints() override;
    void doTrigger() override;
};
#endif
