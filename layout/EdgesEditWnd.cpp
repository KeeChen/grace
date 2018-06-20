#include "EdgesEditWnd.h"
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qlabel.h>
#include "WopAttrWnd.h"
#include "SelectWopWnd.h"
#include "DataManage.h"
#include <QtWidgets/qmessagebox.h>

EdgesEditWnd::EdgesEditWnd()
{
	InitWnd();
	InitCtrl();
	InitForm();
	InitSlot();
}


EdgesEditWnd::~EdgesEditWnd()
{

}

void EdgesEditWnd::InitWnd()
{
	resize(450, 500);
	setWindowTitle(QStringLiteral("�����༭������"));
}

void EdgesEditWnd::InitCtrl()
{
	m_pWopTable = new QTableWidget;  //wop�б�
	if (m_pWopTable)
	{
		m_pWopTable->setColumnCount(4);
		QStringList headerList;
		headerList.clear();
		headerList << (QStringLiteral("ID")) << QStringLiteral("�˶�����") <<QStringLiteral("��������") <<  QStringLiteral("����");
		m_pWopTable->setHorizontalHeaderLabels(headerList);
		m_pWopTable->setContextMenuPolicy(Qt::CustomContextMenu);
		m_pWopTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_pWopTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_pWopTable->setSelectionMode(QAbstractItemView::SingleSelection);
	}

	m_pBtnAddWop = new QPushButton(QStringLiteral("���"));
	m_pBtnSaveInfo = new QPushButton( QStringLiteral("����"));
	m_pBtnExit = new QPushButton( QStringLiteral("ȡ��"));
	
	m_pEditSpeed = new QLineEdit;

}

void EdgesEditWnd::InitForm()
{
	QGridLayout* pAttrLayout = new QGridLayout;
	if (pAttrLayout)
	{
		QLabel* pSpeed = new QLabel(QStringLiteral("�ٶȣ�"));
		pAttrLayout->addWidget(pSpeed, 0, 0, 1, 1);
		pAttrLayout->addWidget(m_pEditSpeed, 0, 1, 1, 1);
	}
	QGridLayout* pWopTitleLayout = new QGridLayout;
	if (pWopTitleLayout)
	{
		QLabel* pLabel = new QLabel(QStringLiteral("WOP�б�"));
		pWopTitleLayout->addWidget(pLabel, 0, 0, 1, 1);
		pWopTitleLayout->addWidget(m_pBtnAddWop, 0, 4, 1, 1);
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
		pWopLayout->addLayout(pWopTitleLayout);
		pWopLayout->addWidget(m_pWopTable);
		pWopLayout->addLayout(pSaveLayout);
	}
	setLayout(pWopLayout);
}

void EdgesEditWnd::InitSlot()
{
	connect(m_pBtnAddWop, &QPushButton::clicked, this, &EdgesEditWnd::SlotBtnAddWop);
	connect(m_pBtnSaveInfo, &QPushButton::clicked, this, &EdgesEditWnd::SlotBtnSaveData);
	connect(m_pBtnExit, &QPushButton::clicked, this, &QDialog::reject);
	connect(m_pWopTable, &QTableWidget::itemDoubleClicked, this, &EdgesEditWnd::SlotWopItemDClicked);
}

void EdgesEditWnd::SlotBtnAddWop()
{
	SelectWopWnd selectWopWnd;
	if (selectWopWnd.exec() == QDialog::Accepted)
	{
		selectWopWnd.GetSelectedWop(m_edgeInfo.wop_list);
		UpdateWopTable();
	}
}

void EdgesEditWnd::UpdateWopTable()
{
	if (!m_pWopTable)
	{
		return;
	}
	m_pWopTable->clearContents();
	m_pWopTable->setRowCount(0);
	for (auto wopId : m_edgeInfo.wop_list)
	{
		WOP wop;
		if (DATAMNG->GetWopById(wopId, wop))
		{
			AddWopToTableWidget(wop);
		}
	}
}

void EdgesEditWnd::AddWopToTableWidget(const WOP& wopInfo)
{
	if (!m_pWopTable)
		{
			return;
		}
		int iRows = m_pWopTable->rowCount();
		m_pWopTable->insertRow(iRows);

		QString qsWopId = QString::number(wopInfo.id);
		QString qsDirection;
		if (wopInfo.direction == FORWARD)
		{
			qsDirection = (QStringLiteral("����"));
		}
		else if (wopInfo.direction == BACKWARD)
		{
			qsDirection = (QStringLiteral("����"));
		}
		QString qsAngleType;
		if (wopInfo.angle_type == AngleType::CONST_ANGLE)
		{
			qsAngleType = (QStringLiteral("����㶨��"));
		}
		else if (wopInfo.angle_type == AngleType::GLOBAL_CONST_ANGLE)
		{
			qsAngleType = (QStringLiteral("ȫ�ֺ㶨��"));
		}
		QString qsAngle = QString::number(wopInfo.angle);

		QTableWidgetItem* pItemWopId = new QTableWidgetItem(qsWopId);
		QTableWidgetItem* pItemDirection = new QTableWidgetItem(qsDirection);
		QTableWidgetItem* pItemAngleType = new QTableWidgetItem(qsAngleType);
		QTableWidgetItem* pItemAngle = new QTableWidgetItem(qsAngle);

		if (pItemWopId)
		{
			pItemWopId->setToolTip(qsWopId);
		}
		if (pItemDirection != NULL)
		{
			pItemDirection->setToolTip(qsDirection);
		}
		if (pItemAngleType)
		{
			pItemAngleType->setToolTip(qsAngleType);
		}
		if (pItemAngle)
		{
			pItemAngle->setToolTip(qsAngle);
		}

		m_pWopTable->setItem(iRows, 0, pItemWopId);
		m_pWopTable->setItem(iRows, 1, pItemDirection);
		m_pWopTable->setItem(iRows, 2, pItemAngleType);
		m_pWopTable->setItem(iRows, 3, pItemAngle);
}

void EdgesEditWnd::SlotWopItemDClicked(QTableWidgetItem* pItem)
{
	if (!pItem){
		return;
	}
	int iRow = pItem->row();
	QTableWidgetItem* pIdItem = m_pWopTable->item(iRow, 0);
	int wopId = pIdItem->text().toInt();
	WOPATTR wopAttr;
	if (m_edgeInfo.wop_attr_list.size() == 0){//�ջ���edge,û��wop����,wop_attr_list��sizeΪ0,�����޷�����wop���ԣ���Ҫ��ʼ������
		WOPATTR wop_attr;
		wop_attr.id = wopId;
		m_edgeInfo.wop_attr_list.push_back(wop_attr);
	}
	for (auto wop_attr : m_edgeInfo.wop_attr_list)
	{
		if (wop_attr.id == wopId){
			wopAttr = wop_attr;
		}
	}
	WopAttrWnd wopAttrWnd;
	wopAttrWnd.SetWopAttr(wopAttr);
	if (wopAttrWnd.exec() == QDialog::Accepted)
	{
		wopAttrWnd.GetWopAttr(wopAttr);
		for (auto &wop_attr : m_edgeInfo.wop_attr_list){
			if (wop_attr.id == wopId){
				wop_attr = wopAttr;
			}
		}
	}
}

void EdgesEditWnd::UpdateCtrlData()
{

}
int EdgesEditWnd::GetCtrlData()
{
	if (m_pEditSpeed)
	{
		if (m_pEditSpeed->text() == ""){
			QMessageBox::warning(this, QStringLiteral("��ʾ"),QStringLiteral("�ٶȲ���Ϊ��!"), QMessageBox::Ok);
			return -1;
		}
		double speed = m_pEditSpeed->text().toDouble();
		if (m_edgeInfo.speed.size() > 0)
		{
			m_edgeInfo.speed[0].speed = speed;
		}
	}
	return 0;
}
void EdgesEditWnd::SetEdgeInfo(const EDGEINFO& edgeInfo)
{
	m_edgeInfo = edgeInfo;

}
void EdgesEditWnd::GetEdgeInfo(EDGEINFO& edgeInfo)
{
	edgeInfo = m_edgeInfo;
}

void EdgesEditWnd::SlotBtnSaveData()
{
	if (GetCtrlData() < 0)return;
	this->accept();
}
