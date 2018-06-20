#include "WopsEditWnd.h"
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include "DataManage.h"
#include "rw_layout_data.h"
#include <QtWidgets/qapplication.h>

WopsEditWnd::WopsEditWnd()
: m_bOpera(false)
, m_strFilePath("")
{
	en = qApp->property("language").toInt() == 1;
	InitWnd();
	InitCtrl();
	InitForm();
	InitSlot();
}


WopsEditWnd::~WopsEditWnd()
{
}

void WopsEditWnd::InitWnd()
{
	resize(450, 500);
	setWindowTitle(en ? "Edit Wop" : QStringLiteral("�༭WOP"));
}

void WopsEditWnd::InitForm()
{
	QGridLayout* pParamLayout = new QGridLayout;
	if (pParamLayout)
	{
		QLabel* pDirectionLabel = new QLabel(en ? "Position" : QStringLiteral("�˶�����:"));
		QLabel* pAngleTypeLabel = new QLabel(en ? "AnchorAngleType" : QStringLiteral("��������:"));
		QLabel* pAngleLabel = new QLabel(en ? "AnchorAngle" : QStringLiteral("����:"));
		pParamLayout->addWidget(pDirectionLabel, 0, 0, 1, 1);
		pParamLayout->addWidget(m_pComboDirection, 0, 1, 1, 2);
		pParamLayout->addWidget(pAngleTypeLabel, 1, 0, 1, 1);
		pParamLayout->addWidget(m_pComboAngleType, 1, 1, 1, 2);
		pParamLayout->addWidget(pAngleLabel, 2, 0, 1, 1);
		pParamLayout->addWidget(m_pEditAngle, 2, 1, 1, 2);
		pParamLayout->addWidget(m_pBtnAddWop, 2, 3, 1, 1);
		pParamLayout->addWidget(m_pBtnSave, 3, 2, 1, 1);
		pParamLayout->addWidget(m_pBtnCancel, 3, 3, 1, 1);
	}
	QVBoxLayout* pWopLayout = new QVBoxLayout;
	if (pWopLayout)
	{
		pWopLayout->addWidget(m_pWopTable);
		pWopLayout->addLayout(pParamLayout);
	}
	this->setLayout(pWopLayout);
}

void WopsEditWnd::InitCtrl()
{
	m_pWopTable = new QTableWidget;  //wop�б�
	if (m_pWopTable)
	{
		m_pWopTable->setColumnCount(4);
		QStringList headerList;
		headerList.clear();
		headerList << (en ? "Id" : QStringLiteral("ID")) << (en ? "Position" : QStringLiteral("�˶�����")) << (en ? "AnchorAngleType" : QStringLiteral("��������")) << (en ? "AnchorAngle" : QStringLiteral("����"));
		m_pWopTable->setHorizontalHeaderLabels(headerList);
		m_pWopTable->setContextMenuPolicy(Qt::CustomContextMenu);
		m_pWopTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_pWopTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_pWopTable->setSelectionMode(QAbstractItemView::SingleSelection);
	}

	m_pComboDirection = new QComboBox;
	if (m_pComboDirection)
	{
		m_pComboDirection->addItem(en ? "Forward:" : QStringLiteral("����"));
		m_pComboDirection->addItem(en ? "Backward:" : QStringLiteral("����"));
	}

	m_pComboAngleType = new QComboBox;
	if (m_pComboAngleType)
	{
		m_pComboAngleType->addItem(en ? "Const Angle:" : QStringLiteral("����㶨��"));
		m_pComboAngleType->addItem(en ? "Global Const Angle:" : QStringLiteral("ȫ�ֺ㶨��"));
	}

	m_pEditAngle = new QLineEdit;
	if (m_pEditAngle)
	{
		m_pEditAngle->setText("0");
	}
	m_pBtnAddWop = new QPushButton(en ? "AddWop" : QStringLiteral("���"));
	m_pBtnCancel = new QPushButton(en ? "Cancel" : QStringLiteral("�˳�"));
	m_pBtnSave = new QPushButton(en ? "Save" : QStringLiteral("���浽�ļ�"));
}

void WopsEditWnd::InitSlot()
{
	connect(m_pBtnAddWop, &QPushButton::clicked, this, &WopsEditWnd::SlotBtnClickedAddWop);
	connect(m_pBtnCancel, &QPushButton::clicked, this, &WopsEditWnd::SlotBtnClickedExit);
	connect(m_pBtnSave, &QPushButton::clicked, this, &WopsEditWnd::SlotBtnClickedSave);
}

void WopsEditWnd::SlotBtnClickedAddWop()
{
	WOP wop;
	if (GetWopInfo(wop) < 0)
	{
		return;
	}
	if (IsWopExist(wop))
	{
		QMessageBox::warning(this, en ? "Warning" : QStringLiteral("��ʾ"), en ? "Wop Existed!" : QStringLiteral("��wop�Ѵ��ڣ�"), QMessageBox::Ok);
		return;
	}
	AddWopInfo(wop);
	AddWopToTableWidget(wop);
	m_bOpera = true;
}

void WopsEditWnd::SlotBtnClickedExit()
{
	if (!m_bOpera)
	{
		this->reject();
		return;
	}
	if (QMessageBox::warning(this, en ? "Warning" : QStringLiteral("��ʾ"), en ? "Save Prior Operation?" : QStringLiteral("�Ƿ񱣴�֮ǰ�Ĳ������ļ���"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		if (SaveLayoutFile())
		{
			this->accept();
		}
	}
	else
	{
		this->reject();
	}
}
int WopsEditWnd::GetWopInfo(WOP& wopInfo)
{
	if (!m_pComboDirection || !m_pComboAngleType || !m_pEditAngle)
	{
		return -1;
	}
	wopInfo.direction = static_cast<Direction>(m_pComboDirection->currentIndex());
	wopInfo.angle_type = static_cast<AngleType>(m_pComboAngleType->currentIndex());
	if (m_pEditAngle->text().isEmpty())
	{
		QMessageBox::warning(this, en ? "Warning" : QStringLiteral("��ʾ"), en ? "Can not Empty" : QStringLiteral("���ǲ���Ϊ�գ�"), QMessageBox::Ok);
		return -1;
	}
	wopInfo.angle = m_pEditAngle->text().toDouble();
	if (wopInfo.angle < 0 || wopInfo.angle > 360)
	{
		QMessageBox::warning(this, en ? "Warning" : QStringLiteral("��ʾ"), en ? "0-360" : QStringLiteral("����ֵ������0-360֮�䣡"), QMessageBox::Ok);
		return -1;
	}
	return 0;
}

void WopsEditWnd::AddWopToTableWidget(const WOP& wopInfo)
{
	if (!m_pWopTable)
	{
		return ;
	}
	int iRows = m_pWopTable->rowCount();
	m_pWopTable->insertRow(iRows);

	QString qsWopId = QString::number(wopInfo.id);
	QString qsDirection;
	if (wopInfo.direction == FORWARD)
	{
		qsDirection = en ? "Forward" : QStringLiteral("����");
	}
	else if (wopInfo.direction == BACKWARD)
	{
		qsDirection = en ? "Backward" : QStringLiteral("����");
	}
	QString qsAngleType;
	if (wopInfo.angle_type == AngleType::CONST_ANGLE)
	{
		qsAngleType = en ? "Const Angle" : QStringLiteral("����㶨��");
	}
	else if (wopInfo.angle_type == AngleType::GLOBAL_CONST_ANGLE)
	{
		qsAngleType = en ? "Global Const Angle" : QStringLiteral("ȫ�ֺ㶨��");
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

void WopsEditWnd::UpdateTableWidget()
{
	DATAMNG->GetAllWopInfo(m_vecWopInfo);
	for (auto wopInfo : m_vecWopInfo)
	{
		AddWopToTableWidget(wopInfo);
	}
}

void WopsEditWnd::SlotBtnClickedSave()
{
	SaveLayoutFile();
}

bool WopsEditWnd::SaveLayoutFile()
{
	if (m_strFilePath.empty())
	{
		QMessageBox::warning(this, en ? "Warning" : QStringLiteral("��ʾ"), en ? "Save Failed!" : QStringLiteral("�ļ�����ʧ�ܣ����ȴ�layout�ļ���"), QMessageBox::Ok);
		return false;
	}
	DATAMNG->SetAllWopInfo(m_vecWopInfo);
	LayoutData layoutData;
	DATAMNG->GetLayoutData(layoutData);
	if (RWXml::write_layout_data(m_strFilePath, layoutData) == SUCCESS)
	{
		QMessageBox::warning(this, en ? "Information" : QStringLiteral("��ʾ"), en ? "Save Success!" : QStringLiteral("�ļ�����ɹ���").arg(QString::fromStdString(m_strFilePath)));
		m_bOpera = false;
		return true;
	}
	else
	{
		QMessageBox::warning(this, en ? "Warning" : QStringLiteral("��ʾ"), en ? "Save Failed!" : QStringLiteral("�ļ�����ʧ�ܣ�").arg(QString::fromStdString(m_strFilePath)));
	}
	return false;
}

void WopsEditWnd::closeEvent(QCloseEvent *event)
{
	SlotBtnClickedExit();
}

bool WopsEditWnd::IsWopExist(const WOP& wop)
{
	for (auto wopInfo : m_vecWopInfo)
	{
		if (wopInfo.direction == wop.direction && wopInfo.angle_type == wop.angle_type &&
			wopInfo.angle == wop.angle)
		{
			return true;
		}
	}
	return false;
}

int WopsEditWnd::GetMaxWopId()
{
	int iMaxId = 0;
	for (auto wopInfo : m_vecWopInfo)
	{
		if (wopInfo.id > iMaxId)
		{
			iMaxId = wopInfo.id;
		}
	}
	return iMaxId;
}

void WopsEditWnd::AddWopInfo(WOP& wop)
{
	if (wop.id == -1)
	{
		wop.id = GetMaxWopId() + 1;
	}
	m_vecWopInfo.push_back(wop);
}