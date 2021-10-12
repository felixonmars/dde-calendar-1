/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#include "doaerrorwidget.h"

#include <QHBoxLayout>
#include <QEventLoop>
#include <QTimer>

bool widgetIsVisible = false; //窗口是否有效

DOAErrorWidget::DOAErrorWidget(QWidget *parent)
    : QWidget(parent)
{
    m_spinner = new DSpinner();
    m_spinner->setFixedSize(20, 20);

    //错误图标
    m_iconLabel = new QLabel(this);
    m_iconLabel->setPixmap(QIcon::fromTheme("doa_error").pixmap(QSize(20, 20)));
    //错误信息
    m_errorMessageLabel = new QLabel(tr("Network error, please check and"));
    m_errorMessageLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    //重试
    m_tryAginLink = new DCommandLinkButton(tr("try again"));
    connect(m_tryAginLink, &DCommandLinkButton::clicked, this, &DOAErrorWidget::slot_tryAgain);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_spinner);
    layout->addWidget(m_iconLabel);
    layout->addSpacing(8);
    layout->addWidget(m_errorMessageLabel);
    layout->addWidget(m_tryAginLink);
    layout->addStretch();
    this->setLayout(layout);
    widgetIsVisible = true;
}

DOAErrorWidget::~DOAErrorWidget()
{
    widgetIsVisible = false;
    m_spinner->stop();
    initData();
}

void DOAErrorWidget::setErrorMsg(const DOAAccount::AccountState accountState)
{
    //如果点击重试后，结果返回时间比较短
    if (m_tryAgainState.isClicked && m_tryAgainState.clickDateTime.msecsTo(QDateTime::currentDateTime()) < 200) {
        QEventLoop eventLoop;
        //优化用户体验设置600毫秒后退出
        QTimer::singleShot(600, &eventLoop, &QEventLoop::quit);
        //切换用户时退出
        connect(this, &DOAErrorWidget::sign_EventQuit, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
    }
    //如果窗口已被释放则退出
    if (!widgetIsVisible)
        return;
    m_spinner->stop();
    m_iconLabel->setVisible(true);
    m_spinner->setVisible(false);
    m_tryAgainState.isClicked = false;
    //如果验证成功则隐藏
    if (DOAAccount::Account_Success == accountState) {
        setHidden(true);
        return;
    }
    //如果验证失败则显示
    setHidden(false);
    switch (accountState) {
    case DOAAccount::Account_AuthenticationFailed:
        //密码异常
        m_errorMessageLabel->setText(tr("Cannot connect to the account, please check your password"));
        m_tryAginLink->setVisible(false);
        break;
    case DOAAccount::Account_NetWorkException:
        m_errorMessageLabel->setText(tr("Network error, please check and"));
        m_tryAginLink->setVisible(true);
        break;
    default:
        //服务器异常
        m_errorMessageLabel->setText(tr("Server error, please"));
        m_tryAginLink->setVisible(true);
        break;
    }
}

//初始化数据
void DOAErrorWidget::initData()
{
    m_tryAgainState.isClicked = false;
    //通知退出事件循环
    emit sign_EventQuit();
}

void DOAErrorWidget::slot_tryAgain()
{
    m_iconLabel->setVisible(false);
    m_spinner->setVisible(true);
    m_tryAginLink->setVisible(false);
    m_spinner->start();
    m_errorMessageLabel->setText(tr("Connecting to the account..."));
    //获取点击时间
    m_tryAgainState.isClicked = true;
    m_tryAgainState.clickDateTime = QDateTime::currentDateTime();
    emit this->sign_tryAgain();
}
