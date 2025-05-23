/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef LC_CONVERT_H
#define LC_CONVERT_H

#include <QString>

namespace LC_Convert{
    QString asString(double value, int precision = 10);
    QString asStringAngle(double value, int precision = 10);
    QString asStringAngleDeg(double value, int precision = 10);
    bool toDouble(const QString &strValue, double &res, double notMeaningful, bool positiveOnly);
    bool parseToToDoubleAngleDegrees(const QString &strValue, double &res, double notMeaningful, bool positiveOnly);
    bool toDoubleAngleRad(const QString &strValue, double& res, double notMeaningful, bool positiveOnly);
    QString evaluateFraction(QString input, QRegularExpression rx, int index, int tailI);
    QString updateForFraction(QString input);
    double evalAngleValue(const QString &angleStr, bool &ok2);
};

#endif // LC_CONVERT_H
