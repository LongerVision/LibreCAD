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

#include "lc_actiondrawdimbaseline.h"

#include "qg_dimoptions.h"
#include "rs_constructionline.h"
#include "rs_dimaligned.h"
#include "rs_dimlinear.h"
#include "rs_preview.h"

// some functions are duplicated with DimLiner action, however, that's intentional as later we can support angular dimensions in additional to linear ones
LC_ActionDrawDimBaseline::LC_ActionDrawDimBaseline(LC_ActionContext *actionContext, RS2::ActionType type)
       :LC_ActionDimLinearBase(type == RS2::ActionDimBaseline? "Draw Dim Baseline": "Draw Dim Continue", actionContext, type),
       m_edata(std::make_unique<RS_DimLinearData>(RS_Vector(0., 0.), RS_Vector(0., 0.), 0, 0.)){
}

namespace {
    //list of entity types supported by current action - line, arc, circle
    const auto dimEntityTypes = EntityTypeList{RS2::EntityDimLinear, RS2::EntityDimAligned};
}

void LC_ActionDrawDimBaseline::reset(){
    RS_ActionDimension::reset();

    double oldAngle = m_edata->angle; // keep selected angle
    *m_edata = {{}, {}, oldAngle, 0.0};
}

void LC_ActionDrawDimBaseline::doTrigger() {
    preparePreview();
    auto *dim = createDim(m_container);
    setPenAndLayerToActive(dim);
    dim->update();

    bool baseline = isBaseline();

    if (baseline){
        moveRelativeZero(m_edata->extensionPoint1);
    }
    else{
        moveRelativeZero(m_edata->extensionPoint2);
    }

    undoCycleAdd(dim);

    if (baseline) {
        m_prevExtensionPointEnd = m_edata->extensionPoint2;
        // test is just in case
        auto* dimLinear = dynamic_cast<RS_DimLinear*>(dim);
        if (dimLinear != nullptr) {
            m_baseDefPoint = dimLinear->getDefinitionPoint();
        }
    }
    else{
        m_edata->extensionPoint1 = m_edata->extensionPoint2;
        m_prevExtensionPointEnd = m_edata->extensionPoint1; // todo - check whether this is necessary. Potentially - for ordnance continued
    }
    m_baseDefPoint = m_dimensionData->definitionPoint;
}

RS_Entity *LC_ActionDrawDimBaseline::createDim(RS_EntityContainer* parent){
    auto *dim = new RS_DimLinear(parent, *m_dimensionData, *m_edata);
    return dim;
}

bool LC_ActionDrawDimBaseline::isBaseline(){
    return m_actionType == RS2::ActionDimBaseline;
}

void LC_ActionDrawDimBaseline::onMouseMoveEvent(int status, LC_MouseEvent *e) {
    RS_Vector mouse = e->snapPoint; // snap on entity?
    switch (status){
        case SetExtPoint1: {
            auto dimCandidate = RS_Snapper::catchEntity(mouse, dimEntityTypes, RS2::ResolveNone);
            if (dimCandidate != nullptr) {
                highlightHover(dimCandidate);

                RS_Vector extPoint1;
                RS_Vector extPoint2;
                RS_Vector dim1;
                RS_Vector dim2;

                int rtti = dimCandidate->rtti();
                switch (rtti){
                    case RS2::EntityDimLinear:{
                        auto dimLinear = dynamic_cast<RS_DimLinear *>(dimCandidate);
                        extPoint1 = dimLinear->getExtensionPoint1();
                        extPoint2 = dimLinear->getExtensionPoint2();
                        dimLinear->getDimPoints(dim1, dim2);
                        break;
                    }
                    case RS2::EntityDimAligned:{
                        auto dimAligned = dynamic_cast<RS_DimAligned *>(dimCandidate);
                        extPoint1 = dimAligned->getExtensionPoint1();
                        extPoint2 = dimAligned->getExtensionPoint2();
                        dimAligned->getDimPoints(dim1, dim2);
                        break;
                    }
                    default:
                        break;
                }
                // we define which dim point is closer to mouse - based on this, extension 1 or extension 2 point will be selected as start extension point
                double dist1 = mouse.distanceTo(dim1);
                double dist2 = mouse.distanceTo(dim2);

                bool dim1CloserToMouse = dist1 < dist2;
                if (e->isControl){
                    dim1CloserToMouse = !dim1CloserToMouse;
                }
                if (dim1CloserToMouse){
                    previewRefSelectablePoint(extPoint1);
                }
                else {
                    previewRefSelectablePoint(extPoint2);
                }
            }
            break;
        }
        case SetExtPoint2:{
            const RS_Vector &extPoint1 = getExtensionPoint1();
            if (extPoint1.valid){
                mouse = getSnapAngleAwarePoint(e, extPoint1, mouse, true);

                // make a projection for new def point
                // vector in direction controlled by angle of dimension
                const RS_Vector &dimVector = RS_Vector::polar(100.0, m_edata->angle);
                // infinite line from extPoint1 to angle direction
                RS_ConstructionLine dimLine(nullptr, RS_ConstructionLineData(extPoint1, extPoint1 + dimVector));
                // projection of mouse on infinite line
                RS_Vector ext2Candidate = dimLine.getNearestPointOnEntity(mouse, false);
                // projection of previous definition point on infinite line
                RS_Vector originalDefPointProjection = dimLine.getNearestPointOnEntity(m_baseDefPoint, false);
                // distance vector between projection of old definition point projection and mouse projections
                RS_Vector defPointsDistance = m_baseDefPoint - originalDefPointProjection;
                // coordinate of definition point on base dimension line
                RS_Vector newDefPointInline = ext2Candidate + defPointsDistance;
                RS_Vector newDefPoint;

                // complete calculation of new definition point for dimension
                if (isBaseline()) {
                    // for base line, we need to offset definition point on infinite line that corresponds old dim line to specified distance
                    double dimAngle = m_dimDirectionAngle; // angle we use for offset vector - it's the same as angle from base dimension
                    if (e->isControl) { // swap direction of the offset, if needed, so we'll offset in opposite direction
                        dimAngle = M_PI + dimAngle;
                    }
                    double previewDistance = m_baselineDistance;
                    if (m_freeBaselineDistance){
                        previewDistance = 16; // of it's better to use zero there? Just a placeholder for preview
                    }
                    // find new definition point for this dimension
                    newDefPoint = newDefPointInline + RS_Vector::polar(previewDistance, dimAngle);
                } else {
                    // for continue mode, new definition point will be on the same line as previous definition point
                    newDefPoint = newDefPointInline;
                }

                setExtensionPoint2(mouse);
                m_dimensionData->definitionPoint = newDefPoint;

                if (m_previewShowsFullDimension) {
                    auto dim = dynamic_cast<RS_DimLinear *>(createDim(m_preview.get()));
                    dim->update();
                    previewEntity(dim);
                }

                if (m_showRefEntitiesOnPreview) {
                    previewRefPoint(extPoint1);
                    previewRefSelectablePoint(newDefPoint);
                    previewRefLine(extPoint1, mouse);
                    previewRefPoint(newDefPoint);
                }
                else{
                    previewLine(extPoint1, mouse);
                }
            }
            break;
        }
        case SetDefPoint:{
            const RS_Vector &extPoint1 = getExtensionPoint1();
            const RS_Vector &extPoint2 = getExtensionPoint2();
            if (extPoint1.valid && extPoint2.valid){
                // less restrictive snap
                mouse = getFreeSnapAwarePoint(e, mouse);
                mouse = adjustByAdjacentDim(mouse, true);

                const RS_Vector &dimVector = RS_Vector::polar(100.0, m_dimDirectionAngle);
                // infinite line in normal direction to dimension angle
                RS_ConstructionLine dimLine(nullptr, RS_ConstructionLineData(extPoint1, extPoint1 + dimVector));

                // projection of old definition point on infinite line
                RS_Vector originalDefPointProjection = dimLine.getNearestPointOnEntity(m_baseDefPoint, false);
                // projection of mouse on infinite line
                RS_Vector newDefPointProjection = dimLine.getNearestPointOnEntity(mouse, false);

                // distance between projections of old definition point and new one
                m_currentDistance = originalDefPointProjection.distanceTo(newDefPointProjection);
                updateOptionsUI(QG_DimOptions::UI_UPDATE_BASELINE_DISTANCE);

                m_dimensionData->definitionPoint = mouse;
                preparePreview();
                previewRefSelectablePoint(m_dimensionData->definitionPoint);
                previewRefPoint(extPoint1);
                previewRefPoint(extPoint2);
                RS_Entity* dim = createDim(m_preview.get());
                previewEntity(dim);
                dim->update();
                break;
            }
            break;
        }
        default:
            break;

    }
}

void LC_ActionDrawDimBaseline::onMouseLeftButtonRelease(int status, LC_MouseEvent *e) {
    RS_Vector snap = e->snapPoint;
    switch (status){
        case SetExtPoint1:
        case SetExtPoint2:{
            m_alternateMode = e->isControl;
            break;
        }
        case SetDefPoint: {
            snap = getFreeSnapAwarePoint(e, snap);
            snap = adjustByAdjacentDim(snap, false);
            break;
        }
        default:
            break;
    }
    fireCoordinateEvent(snap);
}

void LC_ActionDrawDimBaseline::onCoordinateEvent(int status, [[maybe_unused]] bool isZero, const RS_Vector &mouse) {
    switch (status){
        case SetExtPoint1: {
            auto dimCandidate = RS_Snapper::catchEntity(mouse, dimEntityTypes, RS2::ResolveNone);
            if (dimCandidate != nullptr) {
                RS_Vector extPoint1;
                RS_Vector extPoint2;
                RS_Vector dim1;
                RS_Vector dim2;
                double dimAngle;

                int rtti = dimCandidate->rtti();
                switch (rtti){
                    case RS2::EntityDimLinear:{
                        auto dimLinear = dynamic_cast<RS_DimLinear *>(dimCandidate);
                        extPoint1 = dimLinear->getExtensionPoint1();
                        extPoint2 = dimLinear->getExtensionPoint2();
                        dimLinear->getDimPoints(dim1, dim2);
                        m_baseDefPoint = dimLinear->getDefinitionPoint();
                        dimAngle = dimLinear->getAngle();
                        m_dimDirectionAngle = extPoint1.angleTo(dim1);
                        break;
                    }
                    case RS2::EntityDimAligned:{
                        auto dimAligned = dynamic_cast<RS_DimAligned *>(dimCandidate);
                        extPoint1 = dimAligned->getExtensionPoint1();
                        extPoint2 = dimAligned->getExtensionPoint2();
                        dimAligned->getDimPoints(dim1, dim2);
                        m_baseDefPoint = dimAligned->getDefinitionPoint();
                        dimAngle = extPoint1.angleTo(extPoint2);
                        m_dimDirectionAngle = extPoint2.angleTo(dim2);
                        break;
                    }
                    default:
                        dimAngle = 0.0; // just to avoid warning
                        break;
                }

                double dist1 = mouse.distanceTo(dim1);
                double dist2 = mouse.distanceTo(dim2);

                bool dim1CloserToMouse = dist1 < dist2;
                if (m_alternateMode){
                    dim1CloserToMouse = !dim1CloserToMouse;
                }
                if (dim1CloserToMouse){
                    m_edata->extensionPoint1 = extPoint1;
                    m_prevExtensionPointEnd = extPoint2;
                } else {
                    m_edata->extensionPoint1 = extPoint2;
                    m_prevExtensionPointEnd = extPoint1;
                }

                m_edata->angle =  dimAngle;
                moveRelativeZero(m_edata->extensionPoint1);
                setStatus(SetExtPoint2);
            }
            break;
        }
        case SetExtPoint2:{
            const RS_Vector &extPoint1 = getExtensionPoint1();
            const RS_Vector &dimVector = RS_Vector::polar(100.0, m_edata->angle);
            RS_ConstructionLine dimLine(nullptr, RS_ConstructionLineData(extPoint1, extPoint1 + dimVector));
            RS_Vector ext2Candidate = dimLine.getNearestPointOnEntity(mouse, false);
            RS_Vector originalDefPointProjection = dimLine.getNearestPointOnEntity(m_baseDefPoint, false);
            RS_Vector defPointsDistance = m_baseDefPoint - originalDefPointProjection;

            RS_Vector newDefPoint = ext2Candidate + defPointsDistance;
            if (isBaseline()) {
                double dimAngle = m_dimDirectionAngle;

                if (m_alternateMode){
                    dimAngle = M_PI+dimAngle;
                }
                newDefPoint = newDefPoint +RS_Vector::polar(m_baselineDistance, dimAngle);
            }
            m_dimensionData->definitionPoint = newDefPoint;

            if (m_freeBaselineDistance && isBaseline()){
                m_currentDistance = 0.0;
                setStatus(SetDefPoint);
            }
            else{
                trigger();
                setStatus(SetExtPoint2);
            }
            break;
        }
        case SetDefPoint:{
            m_dimensionData->definitionPoint = mouse;
            trigger();
            setStatus(SetExtPoint2);
            break;
        }
        default:
            break;
    }
}

bool LC_ActionDrawDimBaseline::doProcessCommand(int status, const QString &c) {
    bool accept = false;
    switch (status) {
        case SetText: {
            setText(c);
            updateOptions();
            enableCoordinateInput();
            setStatus(m_lastStatus);
            accept = true;
            break;
        }
        default:
            m_lastStatus = (Status) getStatus();
            deletePreview();
            if (checkCommand("text", c)) {
                disableCoordinateInput();
                setStatus(SetText);
                accept = true;
            }
            break;
    }
    return accept;
}

QStringList LC_ActionDrawDimBaseline::getAvailableCommands() {
    QStringList cmd;

    switch (getStatus()) {
        case SetExtPoint1:
        case SetExtPoint2:
        case SetDefPoint: {
            cmd += command("text");
            break;
        }
        default:
            break;
    }
    return cmd;
}

void LC_ActionDrawDimBaseline::updateMouseButtonHints() {
    int status = getStatus();
    switch (status) {
        case SetExtPoint1:
            updateMouseWidgetTRCancel(tr("Select base linear/aligned dimension"), MOD_CTRL(tr("Select distant extension point")));
            break;
        case SetExtPoint2:
            updateMouseWidgetTRBack(tr("Specify second extension line origin"), isBaseline() && !m_freeBaselineDistance ? MOD_CTRL(tr("Mirror offset direction")): MOD_NONE);
            break;
        case SetDefPoint:
            updateMouseWidgetTRBack(tr("Specify dimension line location"), MOD_SHIFT_LC(tr("Snap to Adjacent Dim")));
            break;
        case SetText:
            updateMouseWidget(tr("Enter dimension text:"));
            break;
        default:
            updateMouseWidget();
            break;
    }
}

bool LC_ActionDrawDimBaseline::isFreeBaselineDistance() const {
    return m_freeBaselineDistance;
}

void LC_ActionDrawDimBaseline::setFreeBaselineDistance(bool freeDistance) {
    this->m_freeBaselineDistance = freeDistance;
    if (!m_freeBaselineDistance && getStatus() == SetDefPoint){
        setStatus(SetExtPoint2);
    }
}

double LC_ActionDrawDimBaseline::getBaselineDistance() const {
    return m_baselineDistance;
}

void LC_ActionDrawDimBaseline::setBaselineDistance(double distance) {
    LC_ActionDrawDimBaseline::m_baselineDistance = distance;
}

double LC_ActionDrawDimBaseline::getCurrentBaselineDistance() const {
    return m_currentDistance;
}

RS_Vector LC_ActionDrawDimBaseline::getExtensionPoint1(){
    return m_edata->extensionPoint1;
}

RS_Vector LC_ActionDrawDimBaseline::getExtensionPoint2(){
    return m_edata->extensionPoint2;
}

double LC_ActionDrawDimBaseline::getDimAngle(){
    return m_edata->angle;
}

void LC_ActionDrawDimBaseline::setExtensionPoint1(RS_Vector p){
    m_edata->extensionPoint1 = p;
}

void LC_ActionDrawDimBaseline::setExtensionPoint2(RS_Vector p){
    m_edata->extensionPoint2 = p;
}

void LC_ActionDrawDimBaseline::preparePreview() {
    RS_Vector dirV = RS_Vector::polar(100., m_edata->angle + M_PI_2);
    RS_ConstructionLine cl(nullptr,RS_ConstructionLineData(m_edata->extensionPoint2,m_edata->extensionPoint2 + dirV));
    m_dimensionData->definitionPoint = cl.getNearestPointOnEntity(m_dimensionData->definitionPoint);
}
