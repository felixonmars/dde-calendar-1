/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "graphicsview.h"
#include "graphicsItem/scheduleitem.h"
#include "schedulecoormanage.h"
#include "scheduledlg.h"
#include "scheduledatamanage.h"
#include "schedulectrldlg.h"
#include "myscheduleview.h"
#include "constants.h"

#include <DHiDPIHelper>
#include <DPalette>

#include <QMimeData>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QShortcut>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QtMath>
#include <QDebug>

DGUI_USE_NAMESPACE
CGraphicsView::CGraphicsView(QWidget *parent, ViewPosition Type)
    : CWeekDayGraphicsview(parent, Type, ViewType::PartTimeView)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_timeInterval = height() / 24.0;

    setSceneRect(0, 0, width(), height());
    m_LRPen.setColor(QColor(255, 255, 255));
    m_LRPen.setStyle(Qt::SolidLine);
    m_TBPen.setColor(QColor(255, 255, 255));
    m_TBPen.setStyle(Qt::SolidLine);
    m_LRFlag = true;
    m_TBFlag = true;
    m_margins = QMargins(0, 0, 0, 0);

    setLineWidth(0);
    //TODO 界面刷新?
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(scrollBarValueChangedSlot()));
    m_timer->start(60000);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarValueChangedSlot()));
    connect(this->verticalScrollBar(), &QScrollBar::sliderPressed, this, &CGraphicsView::slotScrollBar);

    //如果为周视图
    if (m_viewType == 0) {
        //设置显示右下角圆角
        setShowRadius(false, true);
    }
}

CGraphicsView::~CGraphicsView()
{
    m_timer->stop();
    m_timer->deleteLater();
    clearSchdule();
}

void CGraphicsView::setMargins(int left, int top, int right, int bottom)
{
    Q_UNUSED(top)
    m_margins = QMargins(left, 0, right, bottom);
    setViewportMargins(m_margins);
}

void CGraphicsView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_weekcolor = "#00429A";
        m_weekcolor.setAlphaF(0.05);
        QColor linecolor = "#000000";
        //ui图透明度为5%
        linecolor.setAlphaF(0.05);
        m_LRPen.setColor(linecolor);
        m_TBPen.setColor(linecolor);
        m_LRPen.setStyle(Qt::SolidLine);
        m_TBPen.setStyle(Qt::SolidLine);
    } else if (type == 2) {
        m_weekcolor = "#4F9BFF";
        m_weekcolor.setAlphaF(0.1);
        QColor linecolor = "#000000";
        //ui图透明度为5%
        linecolor.setAlphaF(0.05);
        m_LRPen.setColor(linecolor);
        m_TBPen.setColor(linecolor);
    }
    CWeekDayGraphicsview::setTheMe(type);
}

void CGraphicsView::slotCreate(const QDateTime &date)
{
    emit signalViewtransparentFrame(1);
    CScheduleDlg dlg(1, this);
    dlg.setDate(date);

    if (dlg.exec() == DDialog::Accepted) {
        emit signalsUpdateShcedule();
    }
    emit signalViewtransparentFrame(0);
}

bool CGraphicsView::MeetCreationConditions(const QDateTime &date)
{
    return qAbs(date.daysTo(m_PressDate) < 7);
}

void CGraphicsView::updateHigh()
{
    scene()->update();
    update();
}

void CGraphicsView::setCurrentDate(const QDateTime &currentDate)
{
    m_currentDateTime = currentDate;
    scrollBarValueChangedSlot();
}

void CGraphicsView::setInfo(const QVector<ScheduleDataInfo> &info)
{
    m_scheduleInfo = info;
}

bool MScheduleTimeThan(const ScheduleDataInfo &s1, const ScheduleDataInfo &s2)
{
    if (s1.getBeginDateTime().date().daysTo(s1.getEndDateTime().date()) == s2.getBeginDateTime().date().daysTo(s2.getEndDateTime().date())) {
        if (s1.getBeginDateTime() == s2.getBeginDateTime()) {
            return s1.getBeginDateTime().secsTo(s1.getEndDateTime()) > s2.getBeginDateTime().secsTo(s2.getEndDateTime());
        } else {
            return s1.getBeginDateTime() < s2.getBeginDateTime();
        }
    } else {
        return s1.getBeginDateTime().date().daysTo(s1.getEndDateTime().date()) > s2.getBeginDateTime().date().daysTo(s2.getEndDateTime().date());
    }
}

void CGraphicsView::upDateInfoShow(const CGraphicsView::DragStatus &status, const ScheduleDataInfo &info)
{
    clearSchdule();
    QVector<ScheduleDataInfo> vListData;
    vListData = m_scheduleInfo;

    switch (status) {
    case NONE:
        Q_UNUSED(info);
        break;
    case ChangeBegin:
    case ChangeEnd: {
        int index = vListData.indexOf(info);
        if (index >= 0)
            vListData[index] = info;
    } break;
    case ChangeWhole:
        vListData.append(info);
        break;
    case IsCreate:
        vListData.append(info);
        break;
    }
    QMap<QDate, QVector<ScheduleDataInfo>> m_InfoMap;
    QDate currentDate;
    qint64 count = m_beginDate.daysTo(m_endDate);
    qint64 beginoffset = 0, endoffset = 0;
    QVector<ScheduleDataInfo> currentInfo;

    for (int i = 0; i <= count; ++i) {
        currentDate = m_beginDate.addDays(i);
        currentInfo.clear();

        for (int j = 0; j < vListData.size(); ++j) {
            beginoffset = vListData.at(j).getBeginDateTime().date().daysTo(currentDate);
            endoffset = currentDate.daysTo(vListData.at(j).getEndDateTime().date());

            if (beginoffset < 0 || endoffset < 0) {
                continue;
            }
            if (vListData.at(j).getEndDateTime().date() == currentDate && vListData.at(j).getBeginDateTime().daysTo(vListData.at(j).getEndDateTime()) > 0 && vListData.at(j).getEndDateTime().time() == QTime(0, 0, 0)) {
                continue;
            }
            currentInfo.append(vListData.at(j));
        }
        std::sort(currentInfo.begin(), currentInfo.end(), MScheduleTimeThan);
        if (currentInfo.size() > 0) {
            m_InfoMap[currentDate] = currentInfo;
            QList<ScheduleclassificationInfo> info;
            scheduleClassificationType(currentInfo, info);

            for (int m = 0; m < info.count(); m++) {
                int tnum = info.at(m).vData.count();
                //如果为周视图则要显示一个位置显示日程的数目
                if (m_viewPos == WeekPos) {
                    if (tnum > m_sMaxNum) {
                        tnum = m_sMaxNum;
                        for (int n = 0; n < tnum - 1; n++) {
                            addSchduleItem(info.at(m).vData.at(n), currentDate, n + 1,
                                           tnum, 0, m_viewType, m_sMaxNum);
                        }
                        //添加“...”item
                        int index = tnum - 2;
                        if (index < 0) {
                            qWarning() << "week view create error,tnum -2 :" << index;
                            index = 1;
                        }
                        ScheduleDataInfo tdetaliinfo = info.at(m).vData.at(index);
                        tdetaliinfo.setTitleName("...");
                        tdetaliinfo.setType(3);
                        addSchduleItem(tdetaliinfo, currentDate, tnum, tnum, 1,
                                       m_viewType, m_sMaxNum);
                    } else {
                        for (int n = 0; n < tnum; n++) {
                            addSchduleItem(info.at(m).vData.at(n), currentDate, n + 1,
                                           tnum, 0, m_viewType, m_sMaxNum);
                        }
                    }
                } else {
                    for (int n = 0; n < tnum; n++) {
                        addSchduleItem(info.at(m).vData.at(n), currentDate, n + 1,
                                       tnum, 0, m_viewType, m_sMaxNum);
                    }
                }
            }
        }
    }
    //更新每个背景上的日程标签
    updateBackgroundShowItem();
}

QDateTime CGraphicsView::getPosDate(const QPoint &p)
{
    return TimeRounding(m_coorManage->getDate(mapToScene(p)));
}

void CGraphicsView::ShowSchedule(DragInfoItem *infoitem)
{
    CScheduleItem *scheduleitem = dynamic_cast<CScheduleItem *>(infoitem);
    if (scheduleitem->getType() == 1)
        return;
    DragInfoGraphicsView::ShowSchedule(infoitem);
}

void CGraphicsView::MoveInfoProcess(ScheduleDataInfo &info, const QPointF &pos)
{
    Q_UNUSED(pos);

    if (!info.getAllDay()) {
        qint64 offset = m_PressDate.secsTo(m_MoveDate);
        info.getBeginDateTime() = info.getBeginDateTime().addSecs(offset);
        info.getEndDateTime() = info.getEndDateTime().addSecs(offset);
    } else {
        info.setAllDay(false);
        info.setRemindData(RemindData());
        info.getBeginDateTime() = m_MoveDate;
        info.getEndDateTime() = m_MoveDate.addSecs(3600);
    }
    upDateInfoShow(ChangeWhole, info);
}

void CGraphicsView::addSchduleItem(const ScheduleDataInfo &info, QDate date, int index, int totalNum, int type, int viewtype, int maxnum)
{
    CScheduleItem *item = new CScheduleItem(
        m_coorManage->getDrawRegion(date, info.getBeginDateTime(),
                                    info.getEndDateTime(), index, totalNum, maxnum,
                                    viewtype),
        nullptr, type);
    if (type == 1) {
        item->setItemType(CFocusItem::COTHER);
    }
    m_Scene->addItem(item);
    item->setData(info, date, totalNum);
    m_vScheduleItem.append(item);
}

/**
 * @brief CGraphicsView::setSelectSearchSchedule        设置搜索选中日程
 * @param info
 */
void CGraphicsView::setSelectSearchSchedule(const ScheduleDataInfo &info)
{
    DragInfoGraphicsView::setSelectSearchSchedule(info);
    setTime(info.getBeginDateTime().time());
    for (int i = 0; i < m_vScheduleItem.size(); ++i) {
        if (m_vScheduleItem.at(i)->getType() == 1)
            continue;
        //判断是否为选中日程
        if (m_vScheduleItem.at(i)->hasSelectSchedule(info)) {
            m_vScheduleItem.at(i)->setStartValue(0);
            m_vScheduleItem.at(i)->setEndValue(10);
            m_vScheduleItem.at(i)->startAnimation();
        }
    }
}

void CGraphicsView::clearSchdule()
{
    for (int i = 0; i < m_vScheduleItem.size(); i++) {
        m_Scene->removeItem(m_vScheduleItem.at(i));
        delete m_vScheduleItem[i];
        m_vScheduleItem[i] = nullptr;
    }
    m_vScheduleItem.clear();
    m_updateDflag = true;
}

void CGraphicsView::scheduleClassificationType(QVector<ScheduleDataInfo> &scheduleInfolist, QList<ScheduleclassificationInfo> &info)
{
    QVector<ScheduleDataInfo> schedulelist = scheduleInfolist;
    if (schedulelist.isEmpty())
        return;

    info.clear();
    std::sort(schedulelist.begin(), schedulelist.end(), MScheduleTimeThan);
    QVector<int> containIndex;

    for (int k = 0; k < schedulelist.count(); k++) {
        QDateTime endTime = schedulelist.at(k).getEndDateTime();
        QDateTime begTime = schedulelist.at(k).getBeginDateTime();

        if (begTime.date().daysTo(endTime.date()) == 0 && begTime.time().secsTo(endTime.time()) < m_minTime) {
            endTime = begTime.addSecs(m_minTime);
        }
        if (endTime.time().hour() == 0 && endTime.time().second() == 0) {
            endTime = endTime.addSecs(-1);
        }
        containIndex.clear();

        for (int i = 0; i < info.count(); i++) {
            if ((schedulelist.at(k).getBeginDateTime() >= info.at(i).begindate && schedulelist.at(k).getBeginDateTime() <= info.at(i).enddate) || (endTime >= info.at(i).begindate && endTime <= info.at(i).enddate)) {
                containIndex.append(i);
            }
        }
        if (containIndex.count() == 0) {
            ScheduleclassificationInfo firstschedule;
            firstschedule.begindate = schedulelist.at(k).getBeginDateTime();
            firstschedule.enddate = endTime;
            firstschedule.vData.append(schedulelist.at(k));
            info.append(firstschedule);
        } else {
            ScheduleclassificationInfo &scheduleInfo = info[containIndex.at(0)];
            int index = 0;

            for (int i = 1; i < containIndex.count(); ++i) {
                index = containIndex.at(i);
                if (info.at(index).begindate < scheduleInfo.begindate)
                    scheduleInfo.begindate = info.at(index).begindate;
                if (info.at(index).enddate > scheduleInfo.enddate)
                    scheduleInfo.enddate = info.at(index).enddate;
                scheduleInfo.vData.append(info.at(index).vData);
            }
            for (int i = containIndex.count() - 1; i > 0; --i) {
                info.removeAt(containIndex.at(i));
            }
            if (schedulelist.at(k).getBeginDateTime() < scheduleInfo.begindate)
                scheduleInfo.begindate = schedulelist.at(k).getBeginDateTime();
            if (endTime > scheduleInfo.enddate)
                scheduleInfo.enddate = endTime;
            scheduleInfo.vData.append(schedulelist.at(k));
        }
    }
}

void CGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit signalScheduleShow(false);
    DGraphicsView::mouseDoubleClickEvent(event);
    CScheduleItem *item = dynamic_cast<CScheduleItem *>(itemAt(event->pos()));

    if (item == nullptr) {
        QPointF senceposs = mapToScene(event->pos());
        emit signalViewtransparentFrame(1);
        CScheduleDlg dlg(1, this);
        QDateTime tDatatime = m_coorManage->getDate(senceposs);
        dlg.setDate(tDatatime);
        if (dlg.exec() == DDialog::Accepted) {
            emit signalsUpdateShcedule();
        }
        emit signalViewtransparentFrame(0);
        return;
    }
    if (item->getType() == 1) {
        emit signalsCurrentScheduleDate(item->getDate());
        return;
    }
    emit signalViewtransparentFrame(1);
    m_updateDflag = false;
    CMyScheduleView dlg(item->getData(), this);
    connect(&dlg, &CMyScheduleView::signalsEditorDelete, this, &CGraphicsView::slotDoubleEvent);
    dlg.exec();
    emit signalViewtransparentFrame(0);
    disconnect(&dlg, &CMyScheduleView::signalsEditorDelete, this, &CGraphicsView::slotDoubleEvent);
}

void CGraphicsView::mousePressEvent(QMouseEvent *event)
{
    CScheduleItem *item = dynamic_cast<CScheduleItem *>(itemAt(event->pos()));

    if (item != nullptr && item->getType() == 1) {
        emit signalScheduleShow(false);
        return;
    }
    DragInfoGraphicsView::mousePressEvent(event);
}

void CGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    CScheduleItem *item = dynamic_cast<CScheduleItem *>(itemAt(event->pos()));

    if (item != nullptr && item->getType() == 1) {
        setCursor(Qt::ArrowCursor);
        DGraphicsView::mouseMoveEvent(event);
        return;
    }
    DragInfoGraphicsView::mouseMoveEvent(event);
}
void CGraphicsView::slotDoubleEvent(int type)
{
    Q_UNUSED(type);
    m_updateDflag = true;
    emit signalsUpdateShcedule();
}

void CGraphicsView::slotScrollBar()
{
    emit signalScheduleShow(false);
}

void CGraphicsView::slotUpdateScene()
{
    pressScheduleInit();
    this->scene()->update();
}

#ifndef QT_NO_WHEELEVENT
/************************************************************************
Function:       wheelEvent()
Description:    鼠标滚轮事件
Input:          event 滚轮事件
Output:         无
Return:         无
Others:         无
************************************************************************/
void CGraphicsView::wheelEvent(QWheelEvent *event)
{
    emit signalScheduleShow(false);
    //非全天部分如果滚动为左右则退出
    if (event->orientation() == Qt::Orientation::Horizontal) {
        return;
    }
    int test = event->delta();
    int viewWidth = viewport()->width();
    int viewHeight = viewport()->height();
    QPoint newCenter(viewWidth / 2, viewHeight / 2 - test);
    QPointF centerpos = mapToScene(newCenter);
    centerOnScene(centerpos);
}
#endif

/************************************************************************
Function:       resizeEvent()
Description:    窗口大小改变事件
Input:          event 窗口大小改变事件
Output:         无
Return:         无
Others:         无
************************************************************************/
void CGraphicsView::resizeEvent(QResizeEvent *event)
{
    scrollBarValueChangedSlot();
    QGraphicsView::resizeEvent(event);
    viewport()->update();
    update();
}

/************************************************************************
Function:       paintEvent()
Description:    绘制事件
Input:          event 事件
Output:         无
Return:         无
Others:         无
************************************************************************/
void CGraphicsView::paintEvent(QPaintEvent *event)
{
    QPainter t_painter(viewport());
    int t_width = viewport()->width() + 2;
    //绘制水平线
    if (m_LRFlag) {
        if (m_cuttrnttimetype == 0) {
            t_painter.save();
            t_painter.setPen(m_LRPen);

            for (int i = 0; i < m_vLRLarge.size(); ++i)
                t_painter.drawLine(QPoint(0, m_vLRLarge[i] - 1), QPoint(t_width, m_vLRLarge[i] - 1));
            t_painter.restore();
        } else {
            t_painter.save();
            t_painter.setPen(m_LRPen);

            for (int i = 0; i < m_vLRLarge.size() - 1; ++i)
                t_painter.drawLine(QPoint(0, m_vLRLarge[i] - 1), QPoint(t_width, m_vLRLarge[i] - 1));
            t_painter.restore();

            if (m_viewType == 1 && m_coorManage->getBegindate() == m_currentDateTime.date()) {
                t_painter.save();
                QPen pen = m_LRPen;
                pen.setColor(m_currenttimecolor);
                t_painter.setPen(pen);
                int index = m_vLRLarge.count() - 1;
                t_painter.drawLine(QPoint(0, m_vLRLarge[index] - 1), QPoint(t_width, m_vLRLarge[index] - 1));
                t_painter.restore();
            }
        }
    }
    DragInfoGraphicsView::paintEvent(event);
}

void CGraphicsView::scrollBarValueChangedSlot()
{
    QMutexLocker locker(&m_Mutex);
    int viewHeight = viewport()->height();
    m_vLRLarge.clear();
    QPointF leftToprealPos = mapToScene(QPoint(0, 0));
    QPointF leftBttomrealPos = mapToScene(QPoint(0, viewHeight));
    m_timeInterval = m_Scene->height() / 24.0;
    qreal beginpos = static_cast<qreal>(qFloor(leftToprealPos.y() / m_timeInterval) * m_timeInterval);

    if (beginpos < leftToprealPos.y()) {
        beginpos = (beginpos / m_timeInterval + 1) * m_timeInterval;
    }

    QVector<int> vHours;

    for (qreal i = beginpos; i < leftBttomrealPos.y(); i = i + m_timeInterval) {
        QPoint point = mapFromScene(leftBttomrealPos.x(), i);
        m_vLRLarge.append(point.y());
        vHours.append(qFloor(i / m_timeInterval + 0.5));
    }

    qreal currentTime = static_cast<qreal>(m_coorManage->getHeight(m_currentDateTime.time()));

    if (currentTime > beginpos && currentTime < leftBttomrealPos.y()) {
        m_cuttrnttimetype = 1;
        QPoint point = mapFromScene(leftBttomrealPos.x(), currentTime);
        m_vLRLarge.append(point.y());
        vHours.append(qFloor(currentTime / m_timeInterval + 0.5));
        emit signalsPosHours(m_vLRLarge, vHours, m_cuttrnttimetype);
    } else {
        m_cuttrnttimetype = 0;
        emit signalsPosHours(m_vLRLarge, vHours, m_cuttrnttimetype);
    }
    scene()->update();
    update();
}

CGraphicsView::PosInItem CGraphicsView::getPosInItem(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    QPointF itemPos = QPointF(scenePos.x() - itemRect.x(),
                              scenePos.y() - itemRect.y());
    qreal bottomy = itemRect.height() - itemPos.y();

    if (itemPos.y() < getVerticalArea()) {
        return TOP;
    }
    if (bottomy < getVerticalArea()) {
        return BOTTOM;
    }

    return MIDDLE;
}

ScheduleDataInfo CGraphicsView::getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate)
{
    ScheduleDataInfo info;
    if (beginDate.secsTo(endDate) > 0) {
        info.getBeginDateTime() = beginDate;

        if (beginDate.secsTo(endDate) < DDECalendar::ThirtyMinutesWithSec) {
            info.getEndDateTime() = beginDate.addSecs(DDECalendar::ThirtyMinutesWithSec);
        } else {
            info.getEndDateTime() = endDate;
        }
    } else {
        if (endDate.secsTo(beginDate) < DDECalendar::ThirtyMinutesWithSec) {
            info.getBeginDateTime() = beginDate.addSecs(-DDECalendar::ThirtyMinutesWithSec);
        } else {
            info.getBeginDateTime() = endDate;
        }
        info.getEndDateTime() = beginDate;
    }
    info.setTitleName(tr("New Event"));
    info.setAllDay(false);
    info.setRemindData(RemindData(1, QTime(9, 0)));
    return info;
}

bool CGraphicsView::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
{
    return !(qAbs(timeFirst.secsTo(timeSecond)) > 100);
}

bool CGraphicsView::JudgeIsCreate(const QPointF &pos)
{
    return qAbs(pos.x() - m_PressPos.x()) > 20 || qAbs(m_PressDate.secsTo(m_coorManage->getDate(mapToScene(pos.toPoint())))) > 300;
}

void CGraphicsView::RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos)
{
    Q_UNUSED(listItem);
    m_rightMenu->clear();
    m_rightMenu->addAction(m_createAction);
    QPointF senceposs = mapToScene(pos);
    m_createDate = m_coorManage->getDate(senceposs);
    m_rightMenu->exec(QCursor::pos());
}

QDateTime CGraphicsView::getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
{
    return moveDateTime.secsTo(m_InfoEndTime) < DDECalendar::ThirtyMinutesWithSec ? m_InfoEndTime.addSecs(-DDECalendar::ThirtyMinutesWithSec) : moveDateTime;
}

QDateTime CGraphicsView::getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
{
    return m_InfoBeginTime.secsTo(moveDateTime) < DDECalendar::ThirtyMinutesWithSec ? m_InfoBeginTime.addSecs(DDECalendar::ThirtyMinutesWithSec) : moveDateTime;
}

QDateTime CGraphicsView::TimeRounding(const QDateTime &time)
{
    int hours = time.time().hour();
    int minnutes = 0;
    minnutes = time.time().minute() / 15;

    return QDateTime(time.date(), QTime(hours, minnutes * 15, 0));
}

void CGraphicsView::centerOnScene(const QPointF &pos)
{
    // view 根据鼠标下的点作为锚点来定位 scene
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    centerOn(pos);
    // scene 在 view 的中心点作为锚点
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    scrollBarValueChangedSlot();
    setSceneHeightScale(pos);
}

void CGraphicsView::setSceneHeightScale(const QPointF &pos)
{
    m_sceneHeightScale = pos.y() / this->scene()->height();
}

void CGraphicsView::keepCenterOnScene()
{
    QPointF pos;
    pos.setX(this->viewport()->width() / 2);
    pos.setY(this->scene()->height() * m_sceneHeightScale);
    centerOnScene(pos);
}

void CGraphicsView::setTime(QTime time)
{
    int viewWidth = viewport()->width();
    int viewHeight = viewport()->height();
    QPoint newCenter(viewWidth / 2, viewHeight / 2);
    QPointF centerpos = mapToScene(newCenter);
    centerpos = QPointF(centerpos.x(), static_cast<qreal>(m_coorManage->getHeight(time)));
    centerOnScene(centerpos);
}

/**
 * @brief CGraphicsView::updateInfo         更新日程数据显示
 */
void CGraphicsView::updateInfo()
{
    //更新选择日程状态
    DragInfoGraphicsView::updateInfo();
    switch (m_DragStatus) {
    case IsCreate:
        upDateInfoShow(IsCreate, m_DragScheduleInfo);
        break;
    default:
        upDateInfoShow();
        break;
    }
}
