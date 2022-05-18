/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#ifndef CTIMELINEEDIT_H
#define CTIMELINEEDIT_H

#include <DSpinBox>

DWIDGET_USE_NAMESPACE

//时间数字编辑器
class CTimeLineEdit : public DSpinBox
{
    Q_OBJECT
public:
    explicit CTimeLineEdit(int id = 0, QWidget *parent = nullptr);

    //设置控件数字，值将被限制在临界值范围内
    void setNum(int num);
    //设置控件数字，如果canCaryy为true且超过临界值将进行自动跳转时间
    void setNum(int num, bool canCarry);
    //设置数字显示范围
    void setNumberRange(int min, int max);

signals:
    //数字改变信号
    void signalNumChange(int id, int num);
    //时间跳转信号
    void signalDateJump(int id, int num);

public slots:
    //编辑完成事件
    void slotEditingFinished();

protected:
    //重写步状态
    StepEnabled stepEnabled() const override;
    //重写步事件
    void stepBy(int steps) override;

private:
    void initView();

private:
    int m_num = 0;      //当前显示的数字

    const int m_id = 0; //控件id

};

#endif // CTIMELINEEDIT_H
