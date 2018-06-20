#include "connectstatewidget.h"
#include "dbg_vars_impls.h"

namespace
{
	static const int WIDGET_HEIGHT = 24;
}

ConnectStateWidget::ConnectStateWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	InitUi();
}

void ConnectStateWidget::InitUi()
{
	setFixedHeight(WIDGET_HEIGHT);
}

void ConnectStateWidget::OnUpdateConnectState(bool connect, const QString &ipAddr, int port)
{
	QString title = IS_EN ? "Viewer" : "�ڴ�����";

	if (connect)
	{
		title += QString("(%1:%2)").arg(ipAddr).arg(QString::number(port));
	}
	else
	{
		title += QString("(%1)").arg(IS_EN ? "No connect" : "δ����");
	}

	ui.connectStateLabel->setText(title);
}
