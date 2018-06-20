#include "dbgvardialog.h"
#include "common_impls.hpp"
#include "dbg_vars_impls.h"
#include "net_task.h"
#include "common/watcherwaiter.hpp"
#include "common/MessageBox.h"

namespace
{
	static const int WIDTH = 600;
	static const int HEIGHT = 500;
}

DbgVarDialog::DbgVarDialog(QWidget *parent)
: QDialog(parent),
m_dbgVarModel{ QSharedPointer<DbgVarModel>::create() },
m_headerView{QSharedPointer<HeaderView>::create(Qt::Horizontal)}
{
	ui.setupUi(this);
	InitUi();
	InitSlots();
}

DbgVarDialog::~DbgVarDialog()
{

}

void DbgVarDialog::InitUi()
{
	if (m_dbgVarModel.isNull() || m_headerView.isNull())
	{
		return;
	}

	QString headerText = (IS_EN ? "ID:Type" : "ID:Type");
	m_headerView->SetHeaderTexts(QStringList() << headerText);
	m_headerView->SetCheckColumns(QList<int>() << DVD_ID_Type);
	m_headerView->setStretchLastSection(true);
	m_headerView->SetStandardItemModel(m_dbgVarModel.data());
	ui.treeView->setHeader(m_headerView.data());
	ui.treeView->setModel(m_dbgVarModel.data());
	ui.okBtn->setText(IS_EN ? "Ok" : "ȷ��");
	ui.cancelBtn->setText(IS_EN ? "Cancel" : "ȡ��");
	setWindowTitle(IS_EN ? "Choose the monitor var" : "ѡ������");
	resize(QSize(WIDTH, HEIGHT));

	QPalette palette;
	palette.setColor(QPalette::Background, QColor(221, 225, 230));
	setAutoFillBackground(true);
	setPalette(palette);
}

void DbgVarDialog::InitSlots()
{
	connect(m_headerView.data(), &HeaderView::AfterDrawCheckBox, m_dbgVarModel.data(), &DbgVarModel::UpdateAllCheckState);
	connect(ui.okBtn, &QPushButton::clicked, this, &DbgVarDialog::ChoosedFinished);
	connect(ui.cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void DbgVarDialog::PostDdgVarFinished(bool successed)
{
	if (successed)
	{
		if (m_dbgVarModel.isNull())
		{
			return;
		}

		m_dbgVarModel->SetReportItems(m_items);
		m_dbgVarModel->InitObjects();
	}
	else
	{
		CMessageBox::Critical(this, IS_EN ? "Critical" : "����", IS_EN ? "Post dbg var failed!" : "��ȡ�ڴ����ʧ�ܣ�", CMessageBox::Ok);
	}
}

void DbgVarDialog::ChoosedFinished()
{
	accept();

	if (m_dbgVarModel.isNull())
	{
		return;
	}

	m_dbgVarModel->AdjustChoosedItems();
}

void DbgVarDialog::SetChoosedMap(const QMultiMap<QPair<int, uint32_t>, QString> &choosedMap)
{
	if (m_dbgVarModel.isNull())
	{
		return;
	}

	m_dbgVarModel->SetCheckedMap(choosedMap);
}

void DbgVarDialog::showEvent(QShowEvent *)
{
	bool(net_task::*func)(uint32_t, std::vector<mn::var_item> &) = &net_task::post_dbg_varls_request_;
	std::function<void(bool)> cb = std::bind(&DbgVarDialog::PostDdgVarFinished, this, std::placeholders::_1);
	net_task nt;
	nsp::toolkit::singleton<WatcherDialog>::instance()->run(&nt, func, m_robotId, std::ref(m_items), cb, getTopWidget());
}
