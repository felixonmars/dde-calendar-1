﻿/*
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
#include "monthschceduleview.h"
#include <QAction>
#include <QMenu>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include "dbmanager.h"
#include "schceduledlg.h"
#include "myschceduleview.h"
#include "scheduledatamanage.h"
#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>
#include "schcedulectrldlg.h"
DGUI_USE_NAMESPACE
CMonthSchceduleWidgetItem::CMonthSchceduleWidgetItem( QWidget *parent /*= nullptr*/, int edittype): DPushButton(parent)
{
    m_editType = edittype;
    //setMargin(0);
    m_editAction = new QAction(tr("Edit"), this);
    m_deleteAction = new QAction(tr("Delete"), this);
    connect(m_editAction, SIGNAL(triggered(bool)), this, SLOT(slotEdit()));
    connect(m_deleteAction, SIGNAL(triggered(bool)), this, SLOT(slotDelete()));
}

void CMonthSchceduleWidgetItem::setColor( QColor color1, QColor color2, bool GradientFlag /*= false*/ )
{
    m_color1 = color1;
    m_color2 = color2;
    m_GradientFlag = GradientFlag;
}

void CMonthSchceduleWidgetItem::setText( QColor tcolor, QFont font, QPoint pos)
{
    m_textcolor = tcolor;
    m_font = font;
    m_pos = pos;
}

void CMonthSchceduleWidgetItem::getColor(QColor &color1, QColor &color2, bool &GradientFlag)
{
    color2 = m_color2;
    color1 = m_color1;
    GradientFlag = m_GradientFlag;
}

void CMonthSchceduleWidgetItem::getText(QColor &tcolor, QFont &font, QPoint &pos)
{
    tcolor = m_textcolor;
    font = m_font;
    pos = m_pos;
}

void CMonthSchceduleWidgetItem::setTransparentB(bool t, QColor tcolor)
{
    m_transparentcolor = tcolor;
    m_transparentf = t;
}

void CMonthSchceduleWidgetItem::setData( ScheduleDtailInfo vScheduleInfo )
{
    m_ScheduleInfo = vScheduleInfo;
    setToolTip(m_ScheduleInfo.titleName);
    update();
}
void CMonthSchceduleWidgetItem::slotEdit()
{
    CSchceduleDlg dlg(0, this);
    dlg.setData(m_ScheduleInfo);
    if (dlg.exec() == DDialog::Accepted) {

        emit signalsEdit(this, 1);
    }
}

void CMonthSchceduleWidgetItem::slotDelete()
{
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    if (m_ScheduleInfo.rpeat == 0) {
        CSchceduleCtrlDlg msgBox;
        //msgBox.setWindowFlags(Qt::FramelessWindowHint);
        //msgBox.setIconPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg").scaled(QSize(34, 34) * devicePixelRatioF()));

        msgBox.setText(tr("You are deleted schedule."));
        msgBox.setInformativeText(tr("Are you sure you want to delete this schedule?"));
        DPushButton *noButton = msgBox.addPushButton(tr("Cancel"));
        DPushButton *yesButton = msgBox.addPushButton(tr("Delete Schedule"));
        msgBox.updatesize();
        DPalette pa = yesButton->palette();
        if (themetype == 0 || themetype == 1) {
            pa.setColor(DPalette::ButtonText, Qt::red);

        } else {
            pa.setColor(DPalette::ButtonText, "#FF5736");

        }
        yesButton->setPalette(pa);
        msgBox.exec();

        if (msgBox.clickButton() == noButton) {
            return;
        } else if (msgBox.clickButton() == yesButton) {
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(m_ScheduleInfo.id);
        }
    } else {
        if (m_ScheduleInfo.RecurID == 0) {
            CSchceduleCtrlDlg msgBox;
            //msgBox.setWindowFlags(Qt::FramelessWindowHint);
            //msgBox.setIconPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg").scaled(QSize(34, 34) * devicePixelRatioF()));

            msgBox.setText(tr("You are deleted schedule."));
            msgBox.setInformativeText(tr("You want to delete all repeat of the schedule, or just delete the selected repeat?"));
            DPushButton *noButton = msgBox.addPushButton(tr("Cancel"));
            DPushButton *yesallbutton = msgBox.addPushButton(tr("All Deleted"));
            DPushButton *yesButton = msgBox.addPushButton(tr("Just Delete Schedule"));
            msgBox.updatesize();
            DPalette pa = yesButton->palette();
            if (themetype == 0 || themetype == 1) {
                pa.setColor(DPalette::ButtonText, Qt::white);
                pa.setColor(DPalette::Dark, QColor("#25B7FF"));
                pa.setColor(DPalette::Light, QColor("#0098FF"));
            } else {
                pa.setColor(DPalette::ButtonText, "#B8D3FF");
                pa.setColor(DPalette::Dark, QColor("#0056C1"));
                pa.setColor(DPalette::Light, QColor("#004C9C"));
            }
            yesButton->setPalette(pa);
            msgBox.exec();

            if (msgBox.clickButton() == noButton) {
                return;
            } else if (msgBox.clickButton() == yesallbutton) {
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(m_ScheduleInfo.id);
            } else if (msgBox.clickButton() == yesButton) {

                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(m_ScheduleInfo.id, newschedule);
                newschedule.ignore.append(m_ScheduleInfo.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            }
        } else {
            CSchceduleCtrlDlg msgBox(this);
            //msgBox.setWindowFlags(Qt::FramelessWindowHint);
            //msgBox.setIconPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-logo.svg").scaled(QSize(34, 34) * devicePixelRatioF()));
            msgBox.setText(tr("You are deleted schedule."));
            msgBox.setInformativeText(tr("You want to delete the schedule of this repetition and all repeat in the future, or just delete all repeat?"));
            DPushButton *noButton = msgBox.addPushButton(tr("Cancel"));
            DPushButton *yesallbutton = msgBox.addPushButton(tr("Delete all schedule in the future"));
            DPushButton *yesButton = msgBox.addPushButton(tr("Just Delete Schedule"));
            msgBox.updatesize();
            DPalette pa = yesButton->palette();
            if (themetype == 0 || themetype == 1) {
                pa.setColor(DPalette::ButtonText, Qt::white);
                pa.setColor(DPalette::Dark, QColor("#25B7FF"));
                pa.setColor(DPalette::Light, QColor("#0098FF"));
            } else {
                pa.setColor(DPalette::ButtonText, "#B8D3FF");
                pa.setColor(DPalette::Dark, QColor("#0056C1"));
                pa.setColor(DPalette::Light, QColor("#004C9C"));
            }
            yesButton->setPalette(pa);
            msgBox.exec();

            if (msgBox.clickButton() == noButton) {
                return;
            } else if (msgBox.clickButton() == yesallbutton) {
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(m_ScheduleInfo.id, newschedule);
                newschedule.enddata.type = 2;
                newschedule.enddata.date = m_ScheduleInfo.beginDateTime;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);

            } else if (msgBox.clickButton() == yesButton) {

                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(m_ScheduleInfo.id, newschedule);
                newschedule.ignore.append(m_ScheduleInfo.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            }
        }
    }
    emit signalsDelete(this);
}
//有问题
void CMonthSchceduleWidgetItem::slotDoubleEvent(int type)
{
    emit signalsEdit(this, 1);
}

void CMonthSchceduleWidgetItem::paintEvent( QPaintEvent *e )
{
    int labelwidth = width();
    int labelheight = height();
    float avge = 1;
    QPainter painter(this);
    if (m_GradientFlag) {
        QLinearGradient linearGradient(0, 0, labelwidth, 0);
        linearGradient.setColorAt(0, m_color1);
        linearGradient.setColorAt(1, m_color2);

        QRect fillRect = QRect(2, 2 * avge, labelwidth - 2, labelheight - 2 * avge);
        painter.save();
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter.setRenderHints(QPainter::HighQualityAntialiasing);
        painter.setBrush(linearGradient);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(fillRect, 3, 3 * avge);
        painter.restore();
        painter.setFont(m_font);
        painter.setPen(m_textcolor);
        QFontMetrics fm = painter.fontMetrics();

        QString str =  "-" + m_ScheduleInfo.titleName;
        QString tstr;
        for (int i = 0; i < str.count(); i++) {
            tstr.append(str.at(i));
            int widthT = fm.width(tstr) + 5;
            if (widthT >= labelwidth - m_pos.x()) {
                tstr.chop(2);
                break;
            }
        }
        if (tstr != str) {
            tstr = tstr + "...";
        }
        painter.drawText(QRect(m_pos.x(), m_pos.y(), labelwidth - m_pos.x(), labelheight - m_pos.y() + 2 * avge), Qt::AlignLeft, tstr);

        if (m_transparentf) {
            painter.setBrush(m_transparentcolor);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(fillRect, 3, 3 * avge);
        }
    } else {
        QRect fillRect = QRect(2, 2 * avge, labelwidth - 2, labelheight - 2 * avge);
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter.setRenderHints(QPainter::HighQualityAntialiasing);
        painter.setBrush(m_color1);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(fillRect, 3, 3);

        painter.setFont(m_font);
        painter.setPen(m_textcolor);
        QFontMetrics fm = painter.fontMetrics();
        QString str = m_ScheduleInfo.titleName;
        if (fm.width(str) > width()) {
            int widthT = fm.width(str);
            int singlecharw = widthT * 1.0 / str.count();
            int rcharcount = width() * 1.0 / singlecharw;
            QString tstr;
            for (int i = 0; i < rcharcount - 8; i++) {
                tstr.append(str.at(i));
            }
            str = tstr + "...";
        }
        painter.drawText(QRect(m_pos.x(), m_pos.y(), labelwidth - m_pos.x(), labelheight - m_pos.y() + 4 * avge), Qt::AlignLeft, str);
    }
}
void CMonthSchceduleWidgetItem::contextMenuEvent( QContextMenuEvent *event )
{
    DMenu Context(this);
    Context.addAction(m_editAction);
    Context.addAction(m_deleteAction);
    Context.exec(QCursor::pos());
}

void CMonthSchceduleWidgetItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    //if (m_editType == 0) return;
    CMySchceduleView dlg(this);
    dlg.setSchedules(m_ScheduleInfo);
    connect(&dlg, &CMySchceduleView::signalsEditorDelete, this, &CMonthSchceduleWidgetItem::slotDoubleEvent);
    dlg.exec();
    disconnect(&dlg, &CMySchceduleView::signalsEditorDelete, this, &CMonthSchceduleWidgetItem::slotDoubleEvent);
}
CMonthSchceduleNumButton::CMonthSchceduleNumButton(QWidget *parent /*= nullptr*/): DPushButton(parent)
{

}

CMonthSchceduleNumButton::~CMonthSchceduleNumButton()
{

}

void CMonthSchceduleNumButton::setColor( QColor color1, QColor color2, bool GradientFlag /*= false*/ )
{
    m_color1 = color1;
    m_color2 = color2;
    m_GradientFlag = GradientFlag;
}

void CMonthSchceduleNumButton::setText( QColor tcolor, QFont font, QPoint pos)
{
    m_textcolor = tcolor;
    m_font = font;
    m_pos = pos;
}

void CMonthSchceduleNumButton::setTransparentB(bool t, QColor tcolor)
{
    m_transparentcolor = tcolor;
    m_transparentf = t;
}

void CMonthSchceduleNumButton::paintEvent(QPaintEvent *e)
{
    int labelwidth = width();
    int labelheight = height();

    QPainter painter(this);
    if (m_GradientFlag) {
        QLinearGradient linearGradient(0, 0, labelwidth, 0);
        linearGradient.setColorAt(0, m_color1);
        linearGradient.setColorAt(1, m_color2);
        QRect fillRect = QRect(2, 1, labelwidth - 2, labelheight - 1);
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter.setRenderHints(QPainter::HighQualityAntialiasing);
        painter.setBrush(linearGradient);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(fillRect, 3, 2);

        painter.setFont(m_font);
        painter.setPen(m_textcolor);
        painter.drawText(QRect(m_pos.x(), m_pos.y(), labelwidth - m_pos.x(), labelheight), Qt::AlignCenter, QString(tr("There is %1 schedule")).arg(m_num) + "...");
        if (m_transparentf) {
            painter.setBrush(m_transparentcolor);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(fillRect, 3, 2);
        }
    } else {
        QRect fillRect = QRect(2, 1, labelwidth - 2, labelheight - 1);
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter.setRenderHints(QPainter::HighQualityAntialiasing);
        painter.setBrush(m_color1);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(fillRect, 3, 2);

        painter.setFont(m_font);
        painter.setPen(m_textcolor);
        painter.drawText(QRect(m_pos.x(), m_pos.y(), labelwidth - m_pos.x(), labelheight), Qt::AlignLeft, QString(tr("There is %1 schedule")).arg(m_num));
    }
}

void CMonthSchceduleView::setTheMe(int type)
{
    updateData();
}

CMonthSchceduleView::CMonthSchceduleView(QWidget *parent) : QObject (parent), m_parernt(parent)
{

}

CMonthSchceduleView::~CMonthSchceduleView()
{

}

void CMonthSchceduleView::setallsize(int w, int h, int left, int top, int buttom)
{
    m_width = w;
    m_height = h;
    m_buttommagin = buttom;
    m_leftMagin = left;
    m_topMagin = top;
    m_cNum = ((m_height - m_topMagin - m_buttommagin) / 6.0  - 27) / 23;
}

void CMonthSchceduleView::setData(QVector<ScheduleDateRangeInfo> &data, int currentMonth)
{
    m_data = data;
    m_currentMonth = currentMonth;
    updateData();
}

void CMonthSchceduleView::slotdeleteitem( CMonthSchceduleWidgetItem *item)
{
    emit signalsUpdateShcedule(item->getData().id);
}

void CMonthSchceduleView::slotedititem(CMonthSchceduleWidgetItem *item, int type)
{
    emit signalsUpdateShcedule(item->getData().id);
}
bool MScheduleDateThan(const MScheduleDateRangeInfo &s1, const MScheduleDateRangeInfo &s2)
{
    return s1.bdate < s2.bdate;
}
bool MScheduleDaysThan(const MScheduleDateRangeInfo &s1, const MScheduleDateRangeInfo &s2)
{
    return s1.bdate.daysTo(s1.edate) > s2.bdate.daysTo(s2.edate);
}
void CMonthSchceduleView::updateData()
{
    if (m_data.count() != 42 || m_cNum < 1) return;
    QDate begindate = m_data.begin()->date;
    QDate enddate = m_data[m_data.count() - 1].date;
    m_beginDate = begindate;
    m_endDate  = enddate;
    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    for (int i = 0; i < m_data.count(); i++) {
        const QVector<ScheduleDtailInfo> &vData = m_data.at(i).vData;
        for (int j = 0; j < vData.size(); j++) {
            QDate tbegindate = vData.at(j).beginDateTime.date();
            QDate tenddate = vData.at(j).endDateTime.date();
            //if (tbegindate == tenddate) continue;
            if (tbegindate <  begindate) tbegindate = begindate;
            if (tenddate > enddate) tenddate = enddate;
            //if (tbegindate == tenddate) continue;

            MScheduleDateRangeInfo info;
            info.bdate = tbegindate;
            info.edate = tenddate;
            info.tData = vData.at(j);
            info.state = false;
            int k = 0;
            for (; k < vMDaySchedule.count(); k++) {
                if (vData.at(j).id == vMDaySchedule.at(k).tData.id && vData.at(j).RecurID == vMDaySchedule.at(k).tData.RecurID) {
                    break;
                }
            }
            if (k == vMDaySchedule.count())
                vMDaySchedule.append(info);
        }
    }
    qSort(vMDaySchedule.begin(), vMDaySchedule.end(), MScheduleDaysThan);
    qSort(vMDaySchedule.begin(), vMDaySchedule.end(), MScheduleDateThan);


    QVector<QVector<int> > vCfillSchedule;
    QVector<QVector<MScheduleDateRangeInfo> >vCMDaySchedule;
    vCfillSchedule.resize(m_cNum);
    vCMDaySchedule.resize(m_cNum);
    //设置日程状态
    for (int c = 0; c < m_cNum; c++) {
        vCfillSchedule[c].resize(42);
        for (int sd = 0; sd < 42; sd++) {
            vCfillSchedule[c][sd] = -1;
        }
        for (int i = 0; i < vMDaySchedule.count(); i++) {
            if (vMDaySchedule[i].state) continue;
            int bindex = begindate.daysTo(vMDaySchedule[i].bdate);
            int eindex = begindate.daysTo(vMDaySchedule[i].edate);
            bool flag = false;
            for (int sd = bindex; sd <= eindex; sd++) {
                if (vCfillSchedule[c][sd] != -1) continue;
                vCfillSchedule[c][sd] = i;
                flag = true;
            }
            if (flag)
                vMDaySchedule[i].state = true;
        }
    }

    QVector<int> vId, vRid; //用于删除日程显示项目保证正确
    //重新组装数据
    for (int c = 0; c < m_cNum; c++) {
        int tnum = -1;
        int tsid = -1;
        int tbindex = 0;
        for (int sd = 0; sd < 42; sd++) {
            if (vCfillSchedule[c][sd] == -1) {
                if (tsid != -1) {
                    MScheduleDateRangeInfo info;
                    info.bdate = m_data[tbindex].date;
                    info.edate = info.bdate.addDays(tnum);
                    info.tData = vMDaySchedule[tsid].tData;
                    info.state = false;
                    vCMDaySchedule[c].append(info);
                    if (c < m_cNum - 1) {
                        vId.append(info.tData.id);
                        vRid.append(info.tData.RecurID);
                    }
                    tsid = -1;
                    tbindex = 0;
                    tnum = -1;
                }
                if (c == m_cNum - 1) continue;
                if (m_data[sd].vData.count() < c + 1) continue;
                MScheduleDateRangeInfo info;
                info.bdate = m_data[sd].date;
                info.edate = m_data[sd].date;
                info.tData = m_data[sd].vData.at(c);
                info.state = false;
                vCMDaySchedule[c].append(info);
                vId.append(info.tData.id);
                vRid.append(info.tData.RecurID);

            } else {
                if (tsid  == -1) {
                    tsid = vCfillSchedule[c][sd];
                    tbindex = sd;
                    tnum++;
                } else {
                    if (tsid == vCfillSchedule[c][sd]) {
                        tnum++;
                    } else {
                        MScheduleDateRangeInfo info;
                        info.bdate = m_data[tbindex].date;
                        info.edate = info.bdate.addDays(tnum);
                        info.tData = vMDaySchedule[tsid].tData;
                        info.state = false;
                        vCMDaySchedule[c].append(info);
                        if (c < m_cNum - 1) {
                            vId.append(info.tData.id);
                            vRid.append(info.tData.RecurID);
                        }
                        tsid = vCfillSchedule[c][sd];
                        tbindex = sd;
                        tnum = 0;
                    }
                }
            }
        }
        if (tsid != -1) {
            MScheduleDateRangeInfo info;
            info.bdate = m_data[tbindex].date;
            info.edate = info.bdate.addDays(tnum);
            info.tData = vMDaySchedule[tsid].tData;
            info.state = false;
            vCMDaySchedule[c].append(info);
            if (c < m_cNum - 1) {
                vId.append(info.tData.id);
                vRid.append(info.tData.RecurID);
            }
        }
    }

    QVector<ScheduleDateRangeInfo> listdata = m_data;
    for (int did  = 0; did < vId.count(); did++) {
        for (int  i = 0; i < listdata.count(); i++) {
            for (int j = 0; j < listdata.at(i).vData.count(); j++) {
                if (listdata.at(i).vData.at(j).id == vId[did] && listdata.at(i).vData.at(j).RecurID == vRid[did]) {
                    listdata[i].vData.remove(j);
                }
            }
        }
    }
    vId.clear();
    vRid.clear();
    //先判断是否有多余日程
    for (int i = 0; i < vCMDaySchedule[m_cNum - 1].count(); i++) {
        int bindex = begindate.daysTo(vCMDaySchedule[m_cNum - 1][i].bdate);
        int eindex = begindate.daysTo(vCMDaySchedule[m_cNum - 1][i].edate);
        int sd = bindex;
        for ( ; sd <= eindex; sd++) {
            if (listdata[sd].vData.count() > 1) break;
        }
        if (sd != eindex + 1) {
            vCMDaySchedule[m_cNum - 1].remove(i);
            i--;
        } else {
            vId.append(vCMDaySchedule[m_cNum - 1][i].tData.id);
            vRid.append(vCMDaySchedule[m_cNum - 1][i].tData.RecurID);
        }
    }
    //再次删除多与数据
    for (int did  = 0; did < vId.count(); did++) {
        for (int  i = 0; i < listdata.count(); i++) {
            for (int j = 0; j < listdata.at(i).vData.count(); j++) {
                if (listdata.at(i).vData.at(j).id == vId[did] && listdata.at(i).vData.at(j).RecurID == vRid[did]) {
                    listdata[i].vData.remove(j);
                }
            }
        }
    }
    QVector<int>  vlastfillSchedule;
    vlastfillSchedule.resize(42);
    for (int sd = 0; sd < 42; sd++) {
        vlastfillSchedule[sd] = -1;
    }

    for (int i = 0; i < vCMDaySchedule[m_cNum - 1].count(); i++) {
        int bindex = begindate.daysTo(vCMDaySchedule[m_cNum - 1][i].bdate);
        int eindex = begindate.daysTo(vCMDaySchedule[m_cNum - 1][i].edate);
        for ( int sd = bindex; sd <= eindex; sd++) {
            vlastfillSchedule[sd] = 1;
        }
    }
    //删除展示了的数据
    for (int i = 0; i < vCMDaySchedule.count(); i++) {
        for (int j = 0; j < vCMDaySchedule[i].count(); j++) {
            ScheduleDtailInfo info = vCMDaySchedule[i][j].tData;
            for (int  m = 0; m < listdata.count(); m++) {
                for (int n = 0; n < listdata.at(m).vData.count(); n++) {
                    if (listdata.at(m).vData.at(n).id == info.id && listdata.at(m).vData.at(n).RecurID == info.RecurID) {
                        listdata[m].vData.remove(n);
                    }
                }
            }
        }

    }

    //处理最后一层的数据
    for (int sd = 0; sd < 42; sd++) {
        if (vlastfillSchedule[sd] == -1) {
            if (listdata[sd].vData.isEmpty()) continue;
            if (listdata[sd].vData.count() > 1) {
                MScheduleDateRangeInfo info;
                info.bdate = listdata[sd].date;
                info.edate = listdata[sd].date;
                info.num = listdata[sd].vData.count();
                info.state = true;
                vCMDaySchedule[m_cNum - 1].append(info);
            } else {
                MScheduleDateRangeInfo info;
                info.bdate = listdata[sd].date;
                info.edate = listdata[sd].date;
                info.tData = listdata[sd].vData.at(0);
                info.state = false;
                vCMDaySchedule[m_cNum - 1].append(info);
            }
        }
    }


    updateDateShow(vCMDaySchedule);

}
void CMonthSchceduleView::updateDateShow(QVector<QVector<MScheduleDateRangeInfo> > &vCMDaySchedule)
{
    for (int i = 0; i < m_scheduleShowItem.count(); i++) {

        m_scheduleShowItem[i]->deleteLater();
    }
    m_scheduleShowItem.clear();

    for (int i = 0; i < vCMDaySchedule.count(); i++) {
        for (int j = 0; j < vCMDaySchedule[i].count(); j++) {
            QVector<MScheduleDateRangeInfo> newData;
            splitSchedule(vCMDaySchedule[i][j], newData);
            for (int k = 0; k < newData.count(); k++) {
                if (newData[k].state) {
                    createScheduleNumWidget(newData[k], i + 1);
                } else {
                    createScheduleItemWidget(newData[k], i + 1);
                }
            }
        }
    }
}

void CMonthSchceduleView::splitSchedule(MScheduleDateRangeInfo &old, QVector<MScheduleDateRangeInfo> &newData)
{
    int brow = (m_beginDate.daysTo(old.bdate) + 1) / 7;
    int erow = (m_beginDate.daysTo(old.edate) + 1) / 7;
    int bcol = (m_beginDate.daysTo(old.bdate) + 1) % 7;
    int ecol = (m_beginDate.daysTo(old.edate) + 1) % 7;
    if (bcol != 0) brow = brow + 1;
    if (ecol != 0) erow = erow + 1;
    if (brow > erow) {
        return;
    }
    if (brow == erow) {
        newData.append(old);
    } else {
        bcol =  bcol ? bcol : 7;
        ecol =  ecol ? ecol : 7;
        //处理开始
        MScheduleDateRangeInfo bfirst = old;
        bfirst.edate = bfirst.bdate.addDays(7 - bcol);
        newData.append(bfirst);
        //处理中间数据
        for (int i = brow + 1; i < erow; i++) {
            MScheduleDateRangeInfo info = old;
            info.bdate = m_beginDate.addDays((i - 1) * 7);
            info.edate = m_beginDate.addDays((i - 1) * 7 + 6);
            newData.append(info);
        }

        //处理结束
        MScheduleDateRangeInfo second = old;
        second.bdate = second.edate.addDays(-ecol + 1);
        newData.append(second);
    }
}

void CMonthSchceduleView::createScheduleItemWidget(MScheduleDateRangeInfo info, int cnum)
{
    ScheduleDtailInfo gd = info.tData;
    CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(gd.type.ID);
    CMonthSchceduleWidgetItem *gwi = new CMonthSchceduleWidgetItem(m_parernt);
    QPoint pos;
    int fw;
    int fh;
    computePos(cnum, info.bdate, info.edate, pos, fw, fh);
    gwi->setColor(gdcolor.gradientFromC, gdcolor.gradientToC, true);
    QFont font("PingFangSC-Light");

    font.setPixelSize(12);

    gwi->setData(gd);
    gwi->setFixedSize(fw, fh);
    gwi->setText(gdcolor.textColor, font, QPoint(13, 3));
    gwi->move(pos);
    if (m_currentMonth != info.bdate.month() && m_currentMonth != info.edate.month()) {
        QColor TransparentC = "#000000";
        TransparentC.setAlphaF(0.05);
        gwi->setTransparentB(true, TransparentC);
    }
    gwi->setWindowFlags(gwi->windowFlags() | Qt::WindowStaysOnTopHint);
    gwi->show();
    connect(gwi, &CMonthSchceduleWidgetItem::signalsDelete, this, &CMonthSchceduleView::slotdeleteitem);
    connect(gwi, &CMonthSchceduleWidgetItem::signalsEdit, this, &CMonthSchceduleView::slotedititem);

    m_scheduleShowItem.append(gwi);
}

void CMonthSchceduleView::createScheduleNumWidget(MScheduleDateRangeInfo info, int cnum)
{
    int type = CScheduleDataManage::getScheduleDataManage()->getTheme();
    CMonthSchceduleNumButton *gwi = new CMonthSchceduleNumButton(m_parernt);
    QPoint pos;
    int fw;
    int fh;
    computePos(cnum, info.bdate, info.edate, pos, fw, fh);
    QColor gradientFromC = "#000000";
    gradientFromC.setAlphaF(0.00);
    gwi->setColor(gradientFromC, gradientFromC, true);
    QFont font("PingFangSC-Regular");
    font.setPixelSize(12);
    if (type == 0 || type == 1) {
        gwi->setText(QColor("#5E5E5E"), font, QPoint(13, 3));
    } else {
        gwi->setText(QColor("#C0C6D4"), font, QPoint(13, 3));
    }
    gwi->setFixedSize(fw, fh);
    gwi->setData(info.num);
    if (m_currentMonth != info.bdate.month() && m_currentMonth != info.edate.month()) {
        QColor TransparentC = "#000000";
        TransparentC.setAlphaF(0.05);
        gwi->setTransparentB(true, TransparentC);
    }
    gwi->move(pos);
    gwi->setWindowFlags(gwi->windowFlags() | Qt::WindowStaysOnTopHint);
    gwi->show();
    m_scheduleShowItem.append(gwi);

}

void CMonthSchceduleView::computePos(int cnum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh)
{
    int brow = (m_beginDate.daysTo(bgeindate)) / 7;

    int bcol = (m_beginDate.daysTo(bgeindate) ) % 7;
    int ecol = (m_beginDate.daysTo(enddate) ) % 7;

    fw = (ecol - bcol + 1) * ((m_width - m_leftMagin * 2) / 7.0) - 11;
    fh = 22;
    int x = m_leftMagin + bcol * ((m_width - m_leftMagin * 2)  / 7.0) + 5;
    int y = m_topMagin + (m_height - m_topMagin - m_buttommagin) / 6.0 * brow + 27 + (cnum - 1) * fh;
    pos = QPoint(x, y);
}

