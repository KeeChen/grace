#include "NodesEditWnd.h"
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlayout.h>

NodesEditWnd::NodesEditWnd()
{
	InitWnd();
	InitForm();
	InitSlot();
}


NodesEditWnd::~NodesEditWnd()
{
}
void NodesEditWnd::InitWnd()
{
	resize(200, 200);
	setWindowTitle(QStringLiteral("�༭�ڵ�����"));
}

void NodesEditWnd::InitForm()
{
	m_pBtnSaveInfo = new QPushButton(QStringLiteral("ȷ��"));
	m_pBtnExit = new QPushButton(QStringLiteral("ȡ��"));

	QGridLayout* pAttrLayout = new QGridLayout;
	if (pAttrLayout)
	{
		m_pSpin = new QCheckBox(QStringLiteral("�Ƿ����ת��"));
		pAttrLayout->addWidget(m_pSpin, 0, 1, 1, 1);

	}
	QGridLayout* pSaveLayout = new QGridLayout;
	if (pSaveLayout)
	{
		pSaveLayout->addWidget(m_pBtnSaveInfo, 0, 3, 1, 1);
		pSaveLayout->addWidget(m_pBtnExit, 0, 4, 1, 1);
	}
	QVBoxLayout* pWopLayout = new QVBoxLayout;
	if (pWopLayout)
	{
		pWopLayout->addLayout(pAttrLayout);
		pWopLayout->addLayout(pSaveLayout);
	}
	setLayout(pWopLayout);
}
void NodesEditWnd::InitSlot()
{
	connect(m_pBtnSaveInfo, &QPushButton::clicked, this, &NodesEditWnd::SlotBtnSaveData);
	connect(m_pBtnExit, &QPushButton::clicked, this, &NodesEditWnd::reject);
}
void NodesEditWnd::SlotBtnSaveData()
{
	pSpin = m_pSpin->isChecked();
	this->accept();
}