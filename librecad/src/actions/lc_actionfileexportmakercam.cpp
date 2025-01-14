/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2014 Christian Luginbühl (dinkel@pimprecords.com)
** Copyright (C) 2018 Andrey Yaromenok (ayaromenok@gmail.com)
**
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
**********************************************************************/

#include "lc_actionfileexportmakercam.h"

#include <QAction>
#include <QFile>

#include "lc_makercamsvg.h"
#include "lc_xmlwriterqxmlstreamwriter.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_graphic.h"
#include "rs_settings.h"

namespace {

// create an SVG generator
std::unique_ptr<LC_MakerCamSVG> getGenerator()
{
    auto groupGuard = RS_SETTINGS->beginGroupGuard("/ExportMakerCam");

    return std::make_unique<LC_MakerCamSVG>(std::make_unique<LC_XMLWriterQXmlStreamWriter>(),
                                            (bool)RS_SETTINGS->readNumEntry("/ExportInvisibleLayers"),
                                            (bool)RS_SETTINGS->readNumEntry("/ExportConstructionLayers"),
                                            (bool)RS_SETTINGS->readNumEntry("/WriteBlocksInline"),
                                            (bool)RS_SETTINGS->readNumEntry("/ConvertEllipsesToBeziers"),
                                            (bool)RS_SETTINGS->readNumEntry("/ExportImages"),
                                            (bool)RS_SETTINGS->readNumEntry("/BakeDashDotLines"),
                                            RS_SETTINGS->readEntry("/DefaultElementWidth", "1.0").toDouble(),
                                            RS_SETTINGS->readEntry("/DefaultDashLinePatternLength").toDouble());
}
}

LC_ActionFileExportMakerCam::LC_ActionFileExportMakerCam(RS_EntityContainer& container,
                                                         RS_GraphicView& graphicView)
    : RS_ActionInterface("Export as CAM/plain SVG...", container, graphicView)
{
    setActionType(RS2::ActionFileExportMakerCam);
}


void LC_ActionFileExportMakerCam::init(int status) {

    RS_ActionInterface::init(status);
    trigger();
}

bool LC_ActionFileExportMakerCam::writeSvg(const QString& fileName, RS_Graphic& graphic)
{
    if (fileName.isEmpty()) {
        LC_ERR<<__func__<<"(): empty file name, no SVG is generated";
        return false;
    }

    auto generator = getGenerator();
    if (generator->generate(&graphic)) {
        QFile file{fileName};
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            LC_ERR<<__func__<<"(): failed in creating file "<<fileName<<", no SVG is generated";
            return false;
        }

        QTextStream out(&file);
        out << QString::fromStdString(generator->resultAsString());
    }
    return true;
}


void LC_ActionFileExportMakerCam::trigger() {

	RS_DEBUG->print("LC_ActionFileExportMakerCam::trigger()");

    if (graphic != nullptr) {

        bool accepted = RS_DIALOGFACTORY->requestOptionsMakerCamDialog();

        if (accepted) {

            QString filename = RS_DIALOGFACTORY->requestFileSaveAsDialog(tr("Export as"),
                                                                         "",
                                                                         "Scalable Vector Graphics (*.svg)");
            writeSvg(filename, *graphic);
        }
    }

    finish(false);
}
