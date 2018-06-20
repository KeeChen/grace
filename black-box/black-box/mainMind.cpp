#include "mainMind.h"
#include <QtWidgets\QMessageBox.h>
#include "connect_manage.h"
#include "posix_time.h"
#include <io.h>
#include <map>
#include <stdio.h>
#include <qlistwidget.h>
#include <qfiledialog.h>
#include <functional>
#include <QtCore\qtimer.h>
#include "warning_page.h"
#include "ui_file_schedule.h"
#include "login_wid.h"
#include <qevent.h>
#include "template_manage.h"
#include "group_tab.h"

log_select::log_select(QWidget *parent)
	: QMainWindow(parent)
{
	//setWindowFlags(Qt::FramelessWindowHint);//�ޱ߿�
	//setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowMinimizeButtonHint&~Qt::WindowCloseButtonHint);
	//showMaximized();
	ui.setupUi(this);
	//setWindowIcon(QIcon(QStringLiteral(":/log_select/icon/black_box48.ico")));
	setWindowIcon(QIcon(QStringLiteral(":black_box/icon/black_box48.ico")));
	init_box();
	sourceModel_ = new ItemModel;
	connect(sourceModel_, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(itemChanged(QStandardItem *)));

	proxyModel_ = new ProxyModel;
	proxyModel_->setSourceModel(sourceModel_);
	ui.tableView->setModel(proxyModel_);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//��񲻿ɱ༭
	ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	//connect(ui.tableView->horizontalHeader(), SIGNAL(sectionClicked(int)), ui.tableView, SLOT(sortByColumn(int)));//��ͷ����
	connect(ui.tableView->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(arrange_column(int)));//ָ����ͷ����

	ui.getIpButton->setEnabled(true);
	ui.getIpButton2->setEnabled(false);

	btn_time_ = new QButtonGroup(this);
	btn_time_->setExclusive(true);
	btn_time_->addButton(ui.tempButton);
	btn_time_->addButton(ui.userDefButton);
	sel_group_ = new QButtonGroup(this);
	sel_group_->setExclusive(true);
	sel_type_ = new QButtonGroup(this);
	sel_type_->setExclusive(true);
	sel_type_->addButton(ui.todayButton);
	sel_type_->addButton(ui.yestButton);
	sel_type_->addButton(ui.allTimeButton);
	sel_type_->addButton(ui.lastTowHourButton);
	sel_type_->addButton(ui.orderButton);
	ui.userDefButton->setChecked(true);
	ui.tabWidget->setEnabled(false);
	ui.templateButton->setEnabled(false);
}

void log_select::arrange_column(int column)
{
	if (column == 1){
		ui.tableView->sortByColumn(column);
	}
}
void log_select::itemChanged(QStandardItem *item)
{
	if (item->column() == 0){

		if (item->checkState() == Qt::Unchecked){
			ui.checkBox->setCheckState(Qt::Unchecked);
			return;
		}
		
		int iCount = 0;
		for (int row = 0; row < proxyModel_->rowCount(); ++row){
			if (proxyModel_->data(proxyModel_->index(row, 0), Qt::CheckStateRole) == Qt::Checked){
				iCount++;
			}
			/*if (sourceModel_->item(row, 0)->checkState() == Qt::Checked){
				iCount++;
			}*/
		}
		if (iCount == proxyModel_->rowCount()){
			ui.checkBox->setCheckState(Qt::Checked);
		}
		else{
			ui.checkBox->setCheckState(Qt::Unchecked);
		}
	}
}
log_select::~log_select()
{

}
//���ӳ�
void log_select::on_connectButton_clicked()
{
	nsp::toolkit::singleton<connect_manage>::instance()->disconnect_regiset_callback(std::bind(&log_select::connect_state, this, std::placeholders::_1,std::placeholders::_2));
	nsp::toolkit::singleton<connect_manage>::instance()->schedule_regiset_callback(std::bind(&log_select::display_schedule_a, this, std::placeholders::_1, std::placeholders::_2));
	//std::string strip = ui.ipComboBox->currentText().toStdString();//DHCP��IP�Ͷ˿�
	//std::string strport = ui.portEdit->text().toStdString();	
	int count = 0;
	select_ep_.clear();
	for (int row = 0; row < ui.iplistWidget->count(); ++row){
		if (ui.iplistWidget->item(row)->checkState()==Qt::Checked){
			size_t pos = ui.iplistWidget->item(row)->text().toStdString().find_last_of("_");
			size_t length = ui.iplistWidget->item(row)->text().toStdString().length();
			std::string sel_ep = ui.iplistWidget->item(row)->text().toStdString().substr(pos+1,length-pos);
			auto iter = all_agv_ep_.find(sel_ep);
			//auto iter = all_agv_ep_.find(ui.iplistWidget->item(row)->text().toStdString());
			if (iter != all_agv_ep_.end()){
				select_ep_.insert(std::make_pair(iter->first, iter->second));
				count++;
			}
			
		}
	}

	if (count == 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("No vehicle was selected."), QMessageBox::Ok);//û��ѡ����
		return;
	}
	ui.stackedWidget->setCurrentIndex(1);//һ�����ӣ�����ת���ڶ���ҳ�棬����һ���б���ʾһ��
	if (connect_to_agvs_share(select_ep_) < 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Connection failed ��"), QMessageBox::Ok);
		ui.connectButton->setEnabled(true);
		ui.disconnectButton->setEnabled(false);
		return;
	}

	proxyModel_->removeRows(0, proxyModel_->rowCount());//�����־���ͱ�
	ui.checkAllBox->setCheckState(Qt::Unchecked);
	ui.checkBox->setCheckState(Qt::Unchecked);
	ui.positionBox->setCheckState(Qt::Unchecked);
	ui.repositionBox->setCheckState(Qt::Unchecked);
	ui.systemLogBox->setCheckState(Qt::Unchecked);
	//input_ep_.insert(std::make_pair(strip, strport));
	//nsp::toolkit::singleton<rw_xml_file>::instance()->write_input_ep(input_ep_);
	/*QMessageBox::information(this, QStringLiteral("info��"), QStringLiteral("Connection success!"), QMessageBox::Ok);
	ui.connectButton->setEnabled(false);*/
	ui.disconnectButton->setEnabled(true);
	
	init_log_group();//��־ģ��


	//ui.selectEdit->clear();
	//nsp::toolkit::singleton<connect_manage>::instance()->disconnect_regiset_callback(std::bind(&log_select::connect_state, this, std::placeholders::_1));
	//if (ui.ipComboBox->currentText() == "" || ui.portEdit->text() == ""){
	//	QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Enter a valid IP or port"), QMessageBox::Ok);
	//	return ;
	//}
	//std::string strip = ui.ipComboBox->currentText().toStdString();
	//std::string strport = ui.portEdit->text().toStdString();	
	//if (connect_agv_share(strip, strport) < 0){
	//	QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Connection failed ��"), QMessageBox::Ok);
	//	ui.connectButton->setEnabled(true);
	//	ui.disconnectButton->setEnabled(false);
	//}
	//else {
	//	input_ep_.insert(std::make_pair(strip, strport));
	//	nsp::toolkit::singleton<rw_xml_file>::instance()->write_input_ep(input_ep_);
	//	QMessageBox::information(this, QStringLiteral("info��"), QStringLiteral("Connection success!"), QMessageBox::Ok);
	//	ui.connectButton->setEnabled(false);
	//	ui.disconnectButton->setEnabled(true);
	//}
	
}
//�ӱ��ػ�ȡip�б�
void log_select::on_radioButton_clicked()
{
	ui.getIpButton->setEnabled(true);
	ui.getIpButton2->setEnabled(false);
}
void log_select::on_getIpButton_clicked()
{
	all_agv_ep_.clear();
	ui.iplistWidget->clear();

	//agvs_ = new agv_info;
	if (load_agvinfo(&agvs_, LAM_Local) < 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("failed to load agvinfo."), QMessageBox::Ok);
		return;
	}
	agv_info *tmp_info = agvs_;

	do{
		id_ep tmp;
		tmp.car_id_ = tmp_info->vhid;
		tmp.port_ = tmp_info->shport;
		all_agv_ep_.insert(std::make_pair(tmp_info->inet, tmp));//map��ȡ��ȡ���������г�����IP��port
		char id[8];
		itoa(tmp_info->vhid, id, 10);
		std::string content = (std::string)id + "_";
		content+=tmp_info->inet;
		QListWidgetItem *item = new QListWidgetItem(QString::fromLocal8Bit(content.c_str()));//�б�ֻ��ʾIP
		item->setCheckState(Qt::Unchecked);
		ui.iplistWidget->addItem(item);
		if (tmp_info->next){
			tmp_info = tmp_info->next;
		}
		else{
			break;
		}
	} while (tmp_info);

	//ui.iplistWidget->sortItems(Qt::AscendingOrder);
}
//ѡ���DHCP��������ȡIP�б�
void log_select::on_radioButton2_clicked()
{
	//delete agvs_;
	ui.getIpButton->setEnabled(false);
	ui.getIpButton2->setEnabled(true);
}
void log_select::on_getIpButton2_clicked()
{
	if (ui.ipComboBox->currentText() == "" || ui.portEdit->text() == ""){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Enter a valid IP or port."), QMessageBox::Ok);
		return;
	}

	if (connect_agvinfo_server(ui.ipComboBox->currentText().toLocal8Bit(), ui.portEdit->text().toInt()) < 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("failed to connect to DHCP server."), QMessageBox::Ok);
		return;

	}
	//ui.getIpButton2->setEnabled(false);
	all_agv_ep_.clear();
	ui.iplistWidget->clear();

	//agvs_ = new agv_info;
	if (load_agvinfo(&agvs_, LAM_Real) < 0)
	{
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("failed to load DHCP agvinfo."), QMessageBox::Ok);
		return;
	}
	agv_info *tmp_info = agvs_;	
	do{
		if (tmp_info->status == AS_ONLINE){
			id_ep tmp;
			tmp.car_id_ = tmp_info->vhid;
			tmp.port_ = tmp_info->shport;
			all_agv_ep_.insert(std::make_pair(tmp_info->inet, tmp));//map��ȡ��ȡ���������г�����IP��port
			char id[8];
			itoa(tmp_info->vhid, id, 10);
			std::string content = (std::string)id + "_";
			content += tmp_info->inet;
			QListWidgetItem *item = new QListWidgetItem(QString::fromLocal8Bit(content.c_str()));//�б�ֻ��ʾIP
			item->setCheckState(Qt::Unchecked);
			ui.iplistWidget->addItem(item);
		}	

		if (tmp_info->next){
			tmp_info = tmp_info->next;
		}
		else{
			break;
		}
	} while (tmp_info);



	//ui.iplistWidget->sortItems(Qt::AscendingOrder);
}
//ȫѡIP�б�
void log_select::on_AllcheckBox_clicked(bool state)
{
	for (int row = 0; row < ui.iplistWidget->count(); ++row){
		ui.iplistWidget->item(row)->setCheckState(state ? Qt::Checked : Qt::Unchecked);
	}
}
void log_select::on_iplistWidget_itemClicked(QListWidgetItem * item)
{
	if (item->checkState() == Qt::Unchecked){
		ui.AllcheckBox->setCheckState(Qt::Unchecked);
		return;
	}
	if (ui.iplistWidget->count() <= 0)return;
	int count = 0;
	for (int row = 0; row < ui.iplistWidget->count(); ++row){
		if (ui.iplistWidget->item(row)->checkState() == Qt::Checked){
			count++;
		}
	}
	if (count == ui.iplistWidget->count()){
		ui.AllcheckBox->setCheckState(Qt::Checked);
	}
}
//�����ӳ���IP�б�ȫѡ�б�
void log_select::on_checkAllBox_clicked(bool state)
{
	for (int row = 0; row < ui.connectedListWidget->count(); ++row){
		ui.connectedListWidget->item(row)->setCheckState(state?Qt::Checked:Qt::Unchecked);
	}
}
void log_select::on_connectedListWidget_itemClicked(QListWidgetItem * item)
{
	if (item->checkState() == Qt::Unchecked){
		ui.checkAllBox->setCheckState(Qt::Unchecked);
		return;
	}
	if (ui.connectedListWidget->count() <= 0){
		return;
	}
	int iCount = 0;
	for (int row = 0; row < ui.connectedListWidget->count(); ++row){
		if (ui.connectedListWidget->item(row)->checkState() == Qt::Checked){
			iCount++;
		}
	}
	if (iCount == ui.connectedListWidget->count()){
		ui.checkAllBox->setCheckState(Qt::Checked);
	}
}
//�Ͽ�����
void log_select::on_disconnectButton_clicked()
{
	ui.selectEdit->clear();
	if (nsp::toolkit::singleton<connect_manage>::instance()->disconnect_network(select_ep_) < 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Network unconnected "), QMessageBox::Ok);//δ����
		ui.connectButton->setEnabled(false);
		ui.disconnectButton->setEnabled(true);
	}
	else{
		ui.stackedWidget->setCurrentIndex(0);
		//QMessageBox::information(this, QStringLiteral("infomation��"), QStringLiteral("�Ͽ����ӳɹ�"), QMessageBox::Ok);
		ui.connectButton->setEnabled(true);
		ui.disconnectButton->setEnabled(false);
	}
}
//���Ӷ೵
int log_select::connect_to_agvs_share(std::map<std::string, id_ep>&select_ep)
{
	//login_wid login(this);//��ʾ�������ӡ�����
	//login.show();
	//qApp->processEvents();
	if (nsp::toolkit::singleton<connect_manage>::instance()->init_networks(select_ep) < 0){
		return -1;
	}
	ui.connectedListWidget->clear();
	//if (select_ep.size() > 0){
	//	//ui.stackedWidget->setCurrentIndex(1);
	//}
	//for (auto&iter : select_ep){
	//	QListWidgetItem *item = new QListWidgetItem(QString::fromLocal8Bit(iter.first.c_str()));//�б�ֻ��ʾIP
	//	item->setCheckState(Qt::Unchecked);
	//	ui.connectedListWidget->addItem(item);
	//}

	return 0;
}

int log_select::connect_agv_share(std::string&ip, std::string&port)
{
	login_wid login(this);//��ʾ�������ӡ�����
	//login.move((width() - login.width()) / 2, (height() - login.height()) / 2);
	login.show();
	qApp->processEvents();
	if (nsp::toolkit::singleton<connect_manage>::instance()->init_network(ip, port) < 0){
		return -1;
	}
	return 0;
}
//���ͻ�ȡ��־���͵�����
void log_select::on_selectButton_clicked()
{
	use_ep_.clear();
	map_log_types_.clear();//ÿ�λ�ȡ���б����
	list_.clear();
	ui.selectEdit->clear();
	nsp::toolkit::singleton<connect_manage>::instance()->regiset_callback(std::bind(&log_select::display_log_types,this,std::placeholders::_1));
	//nsp::toolkit::singleton<connect_manage>::instance()->schedule_regiset_callback(std::bind(&log_select::display_schedule_a, this, std::placeholders::_1, std::placeholders::_2));
	int count = 0;
	for (int row = 0; row < ui.connectedListWidget->count(); ++row){
		if (ui.connectedListWidget->item(row)->checkState() == Qt::Checked){
			size_t pos = ui.connectedListWidget->item(row)->text().toStdString().find_last_of("_");
			size_t length = ui.connectedListWidget->item(row)->text().toStdString().length();
			std::string sel_ep = ui.connectedListWidget->item(row)->text().toStdString().substr(pos + 1, length - pos);
			auto iter = select_ep_.find(sel_ep);
			if (iter != select_ep_.end()){
				use_ep_.insert(std::make_pair(iter->first, iter->second.port_));
				count++;
			}

		}
	}
	if (count == 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("No vehicle was selected."), QMessageBox::Ok);//û��ѡ����
		return;
	}

	if (nsp::toolkit::singleton<connect_manage>::instance()->query_log_types(use_ep_) < 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Send request failure"), QMessageBox::Ok);//��������ʧ��
		return;
	}
}
//ȫѡ
void log_select::on_checkBox_clicked(bool state)
{
	
	for (int row = 0; row < proxyModel_->rowCount(); ++row){		
		//sourceModel_->item(row, 0)->setCheckState(state?Qt::Checked:Qt::Unchecked);	
		int stat = state ? 2 : 0;
		proxyModel_->setData(proxyModel_->index(row, 0), stat, Qt::CheckStateRole);
		
	}
}

//ȫѡ
//void log_select::on_checkBox_stateChanged(int state)
//{
//	for (int row = 0; row < proxyModel_->rowCount(); ++row)
//	{
//		proxyModel_->setData(proxyModel_->index(row, 0), state, Qt::CheckStateRole);
//
//	}
//}

//ģ������
void log_select::on_selectEdit_textChanged(const QString &arg)
{
	proxyModel_->setText(arg);

	int iCount = 0;
	if (proxyModel_->rowCount() == 0){
		ui.checkBox->setCheckState(Qt::Unchecked);
		return;
	}
	for (int row = 0; row < proxyModel_->rowCount(); ++row){
		if (proxyModel_->data(proxyModel_->index(row, 0), Qt::CheckStateRole) == Qt::Checked){
			iCount++;
		}
	}
	if (iCount == proxyModel_->rowCount()){
		ui.checkBox->setCheckState(Qt::Checked);
	}
	else{
		ui.checkBox->setCheckState(Qt::Unchecked);
	}

	//ui.tableView->resizeColumnsToContents();
	//ui.tableView->resizeRowsToContents();
}

//��ʾ��־�����б�
void log_select::display_log_types(const std::vector<std::string>&log_vct)
{
	std::lock_guard<decltype(list_mutex_)> lock(list_mutex_);	
	list_.clear();
	for (auto&iter : log_vct){ //�೵����ȥ��
		if (iter != ""){
			map_log_types_.insert(std::make_pair(iter, iter));
		}
	}
	for (auto&iter : map_log_types_){
		list_.push_back(QPair<bool, QString>(false, QString::fromLocal8Bit(iter.first.c_str())));
	}

	//����
	//for (auto&iter : log_vct){	
	//	list_.push_back(QPair<bool, QString>(false, QString::fromLocal8Bit(iter.c_str())));
	//}
	

	QTimer::singleShot(0, this, SLOT(update_table_view()));
}
void log_select::update_table_view()
{
	sourceModel_->setData(list_);
	ui.tableView->resizeColumnsToContents();
	ui.tableView->resizeRowsToContents();
}
//��ʼ��ip�б�ʱ���
void log_select::init_box()
{
	ui.selectEdit->setMaxLength(12);
	//ui.disconnectButton->setEnabled(false);
	QRegExp regExp("(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])");//ֻ������IP��ʽ
	ui.ipComboBox->setValidator(new QRegExpValidator(regExp, this));

	ui.portEdit->setValidator(new QIntValidator(1, 65535, this));
	ui.ftsPortEdit->setValidator(new QIntValidator(1, 65535, this));
	//ip�б�
	/*input_ep_.clear();
	nsp::toolkit::singleton<rw_xml_file>::instance()->read_input_ep(input_ep_);
	int row = 0;
	for (auto &iter : input_ep_){
		ui.ipComboBox->insertItem(row, QString::fromLocal8Bit(iter.first.c_str()));
		row++;
	}*/

	//���ó�ʼʱ��
	/*QDateTime* stime = new QDateTime(QDateTime::currentDateTime().addSecs(-3600));
	ui.startTimeEdit->setDateTime(*stime);*/
	QDate sdata = QDate::currentDate();
	QTime stime = QTime(0, 0, 0);
	ui.startTimeEdit->setDate(sdata);
	ui.startTimeEdit->setTime(stime);

	QDateTime etime = QDateTime::currentDateTime();
	ui.endTimeEdit->setDateTime(etime);
	
}
//��ȡ��־���ļ��Ĵ���ʱ��
int log_select::get_file_time(std::string& file_path,uint64_t &file_time)
{
	HANDLE hFile = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	FILETIME c_time, m_time, a_time;
	if (!GetFileTime(hFile, &c_time, &a_time, &m_time)){
		CloseHandle(hFile);
		return -1;
	}
	SYSTEMTIME sysTime;
	FileTimeToSystemTime(&c_time, &sysTime);//���ļ�ʱ��ת��Ϊ��׼ϵͳʱ��
	
	struct tm gm = { sysTime.wSecond, sysTime.wMinute, sysTime.wHour, sysTime.wDay, sysTime.wMonth - 1, sysTime.wYear - 1900, sysTime.wDayOfWeek, 0, 0 };
	file_time = mktime(&gm);//ʱ���

	char b_time[32];//�ļ�����ʽ
	sprintf(b_time, "%04d%02d%02d_%02d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour + 8, sysTime.wMinute, sysTime.wSecond);
	
	return 0;
}
//���ͻ�ȡ��־�ļ�������
void log_select::on_sendLogButton_clicked()
{	
	is_warn_ = true;
	if ( ui.ftsPortEdit->text() == ""){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Enter fts Port"), QMessageBox::Ok);
		return;
	}
	//if (list_.size() == 0){
	//	QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("There is no log type list yet��"), QMessageBox::Ok);//û�л�ȡ��־�����б�
	//	return;
	//}
	//��ȡʱ��
	uint16_t fts_port = ui.ftsPortEdit->text().toInt();
	QDateTime btime = ui.startTimeEdit->dateTime();
	QDateTime etime = ui.endTimeEdit->dateTime();
	//uint64_t etime = ui.endTimeEdit->dateTime().toTime_t();//ʱ�����ʽ
	std::string strat_time = btime.toString("yyyy").toStdString() + btime.toString("MM").toStdString() + btime.toString("dd").toStdString() +
		"_" + btime.toString("HH").toStdString() + btime.toString("mm").toStdString() + btime.toString("ss").toStdString();
	std::string end_time = etime.toString("yyyy").toStdString() + etime.toString("MM").toStdString() + etime.toString("dd").toStdString() +
		"_" + etime.toString("HH").toStdString() + etime.toString("mm").toStdString() + etime.toString("ss").toStdString();
	if (ui.newTimeBox->checkState() == Qt::Checked) end_time = "";
	if (ui.allTimeButton->isChecked()){
		strat_time = "";
		end_time = "";
	}
	if (strat_time != ""&&end_time != ""){
		if (strat_time >= end_time){
			QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("The start time should be less than the end time ��"), QMessageBox::Ok);//��ʼʱ��ӦС�ڽ���ʱ��
			return;
		}
		//std::string current_time;
		//get_current_time(current_time);
		//if (strat_time > current_time || end_time > current_time){
		//	QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("The time is greater than current time, unreasonable ��"), QMessageBox::Ok);//ʱ����ڵ�ǰʱ�䣬������
		//	return;
		//}
	}
	//��ȡ��־����
	std::vector<std::string>selected;
	if (ui.userDefButton->isChecked()){
		for (int row = 0; row < proxyModel_->rowCount(); ++row){
			if (proxyModel_->data(proxyModel_->index(row, 0), Qt::CheckStateRole).toInt() != 0){
				std::string t = proxyModel_->data(proxyModel_->index(row, 1), Qt::DisplayRole).toString().toStdString();
				selected.push_back(t);
			}
		}
		if (ui.positionBox->checkState() == Qt::Checked){//��ȡ��λͼ
			selected.push_back("get_position_image");
		}
		if (ui.repositionBox->checkState() == Qt::Checked){//��ȡ���ζ�λͼ
			selected.push_back("get_reposition_image");
		}
		if (ui.systemLogBox->checkState() == Qt::Checked){
			selected.push_back("system_log");
		}
	}
	else if (ui.tempButton->isChecked()){
		if (select_group_ == ""){
			QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("You did not select any template��"), QMessageBox::Ok);//û�й�ѡ��־����
			return;
		}
		auto iter = template_type_.find(select_group_);
		if (iter != template_type_.end()){
			for (auto &typeiter : iter->second){
				if ("localization" == typeiter){
					selected.push_back("get_position_image");
				}else if ("deviation" == typeiter){
					selected.push_back("get_reposition_image");
				}
				else{
					selected.push_back(typeiter);
				}
				//selected.assign(iter->second.begin(), iter->second.end());
			}
		}

	}
	if (selected.size() == 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("You did not select any log types��"), QMessageBox::Ok);//û�й�ѡ��־����
		return;
	}
	//��Ҫ��ȡ��־�ĳ����б�
	std::vector<std::string>ip_vct;
	for (int row = 0; row < ui.connectedListWidget->count(); row++){
		if (ui.connectedListWidget->item(row)->checkState() == Qt::Checked){
			size_t pos = ui.connectedListWidget->item(row)->text().toStdString().find_last_of("_");
			size_t length = ui.connectedListWidget->item(row)->text().toStdString().length();
			std::string sel_ep = ui.connectedListWidget->item(row)->text().toStdString().substr(pos + 1, length - pos);
			ip_vct.push_back(sel_ep);
		}
	}
	if (ip_vct.size() == 0){
		QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("You did not select any vehicle��"), QMessageBox::Ok);//û�й�ѡ��־����
		return;
	}
	//���浯��
	warning_page page(this);
	if(page.exec()){
		if (page.state_){
			//�����ļ�
			if (selected.size()>0){
				QString filePath = QFileDialog::getExistingDirectory(
					this,
					QStringLiteral("Save the logs path"),//������־·��
					"");
				/*QString fileName = QFileDialog::getSaveFileName(this,
					QStringLiteral("������־ѹ���ļ�"),
					"",
					tr("Config Files(*.tar)"));*/
				if (filePath == "")return;
				nsp::toolkit::singleton<connect_manage>::instance()->query_log_info(ip_vct,fts_port, strat_time, end_time, selected, (std::string)filePath.toLocal8Bit());
				is_cancel_ = true;
				schedule_close_ = false;
				file_schedule_ = new file_schedule;
				file_schedule_->ui.progressBar->setRange(0, 99);
				file_schedule_->ui.progressBar->setValue(0);
				if (file_schedule_->exec()){
					if (is_cancel_){
						nsp::toolkit::singleton<connect_manage>::instance()->post_cancel_command();//��agv_shellȡ��
					}
					else{
						nsp::toolkit::singleton<connect_manage>::instance()->cancel_query_logs();//����ȡ��
					}
					schedule_close_ = true;
					file_schedule_->deleteLater();
				}
			}
		}

	}

}
//����
void log_select::on_todayButton_clicked()
{
	QDate sdata =QDate::currentDate();
	QTime stime = QTime(0,0,0);
	ui.startTimeEdit->setDate(sdata);
	ui.startTimeEdit->setTime(stime);

	QDateTime etime = QDateTime::currentDateTime();
	ui.endTimeEdit->setDateTime(etime);

	ui.startTimeEdit->setEnabled(false);
	ui.endTimeEdit->setEnabled(false);
	ui.newTimeBox->setCheckState(Qt::Checked);
	ui.newTimeBox->setEnabled(false);
}
//����
void log_select::on_yestButton_clicked()
{
	QDate sdata = QDate::currentDate();
	sdata = sdata.addDays(-1);
	QTime stime = QTime(0, 0, 0);
	ui.startTimeEdit->setDate(sdata);
	ui.startTimeEdit->setTime(stime);

	QDate edata = QDate::currentDate();
	edata = edata.addDays(-1);
	QTime etime = QTime(23, 59, 0);
	ui.endTimeEdit->setDate(edata);
	ui.endTimeEdit->setTime(etime);

	ui.startTimeEdit->setEnabled(false);
	ui.endTimeEdit->setEnabled(false);
	ui.newTimeBox->setCheckState(Qt::Unchecked);
	ui.newTimeBox->setEnabled(false);
}
//�Զ���ʱ��
void log_select::on_orderButton_clicked()
{
	QDate sdata = QDate::currentDate();
	QTime stime = QTime(0, 0, 0);
	ui.startTimeEdit->setDate(sdata);
	ui.startTimeEdit->setTime(stime);

	QDateTime etime = QDateTime::currentDateTime();
	ui.endTimeEdit->setDateTime(etime);

	ui.startTimeEdit->setEnabled(true);
	ui.endTimeEdit->setEnabled(true);
	ui.newTimeBox->setCheckState(Qt::Unchecked);
	ui.newTimeBox->setEnabled(true);
}
//ȫ��ʱ��
void log_select::on_allTimeButton_clicked()
{
	ui.startTimeEdit->setEnabled(false);
	ui.endTimeEdit->setEnabled(false);
	ui.newTimeBox->setEnabled(false);
}
//�����Сʱ
void log_select::on_lastTowHourButton_clicked()
{
	QDate sdata = QDate::currentDate();
	QTime stime = QTime::currentTime();
	stime = stime.addSecs(-7200);
	ui.startTimeEdit->setDate(sdata);
	ui.startTimeEdit->setTime(stime);

	QDateTime etime = QDateTime::currentDateTime();
	ui.endTimeEdit->setDateTime(etime);

	ui.startTimeEdit->setEnabled(false);
	ui.endTimeEdit->setEnabled(false);
	ui.newTimeBox->setCheckState(Qt::Checked);

}
void log_select::on_newTimeBox_clicked()
{
	ui.endTimeEdit->setEnabled(ui.newTimeBox->checkState() == Qt::Checked?false:true);
}
void log_select::display_schedule_a(int total_count, int finish_count)
{
	if (total_count == 0){//û���ļ�
		count_ = 100;
		QTimer::singleShot(0, this, SLOT(display_schedule()));
		QTimer::singleShot(0, this, SLOT(no_files()));
		return;
	}
	else if (total_count==-1){//����ʧ��
		count_ = 100;
		//QTimer::singleShot(0, this, SLOT(display_schedule()));
		QTimer::singleShot(0, this, SLOT(wrong_fts_port()));
		return;
	}

	is_cancel_ = false;//����ȡ��
	//count_ = finish_count * 100 / (total_count * 2);
	count_ = finish_count * 100 / total_count;
	QTimer::singleShot(0, this, SLOT(display_schedule()));
	total_count_ = total_count;
	if (is_warn_){
		QTimer::singleShot(0, this, SLOT(logs_count_info()));
		is_warn_ = false;
	}
}

void log_select::display_schedule()
{
	if (!schedule_close_){
		file_schedule_->ui.progressBar->setValue(count_);
		if (count_ == 100){
			file_schedule_->deleteLater();
			schedule_close_ = true;
		}
	}

}
void log_select::no_files(){
	QMessageBox::information(this, QStringLiteral("infomation��"), QStringLiteral("No log files..."), QMessageBox::Ok);//û����־�ļ�
}
void log_select::wrong_fts_port(){
	QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("Unable to get log file, possibly FTS port error."), QMessageBox::Ok);//�޷���ȡ��־�ļ�������fts �˿ڴ���
}
void log_select::logs_count_info(){
	if (!schedule_close_){
		char Count[8];
		itoa(total_count_, Count, 10);
		file_schedule_->ui.countLabel->setText(QString::fromLocal8Bit(Count));
	}
	
	//loinfo("black_box") << "total logs file count is " << total_count_ ;
	//char buf[100] = { 0 };
	//sprintf(buf, "logs count: %d", total_count_);
	//QMessageBox::information(this, QStringLiteral("infomation��"), buf, QMessageBox::Ok);//��־������
}
void log_select::connect_state(const int state,const std::string&ip)
{
	if (state == 0){
		//QMessageBox::information(this, QStringLiteral("info��"), QStringLiteral("Connection success!"), QMessageBox::Ok);
		//ui.connectButton->setEnabled(false);
		QTimer::singleShot(0, this, SLOT(connected_list()));
	}
	if (state == -1){
		broken_ip_ = ip;
		QTimer::singleShot(0, this, SLOT(change_state()));
	}
	if (state == -2){
		QTimer::singleShot(0, this, SLOT(change_state_back()));
	}
}
void log_select::connected_list()
{
	//QMessageBox::information(this, QStringLiteral("info��"), QStringLiteral("Connection success!"), QMessageBox::Ok);
	ui.connectedListWidget->clear();
	
	std::vector<std::pair<std::string, id_session>>tVector;
	for (auto&iter : nsp::toolkit::singleton<connect_manage>::instance()->map_ip_se_connected_){
		tVector.push_back(iter);
	}
	if (tVector.size() > 1){
		std::sort(tVector.begin(), tVector.end(), [&](
			const std::pair<std::string, id_session>&first, const std::pair<std::string, id_session>&second)->bool{
			return first.second.car_id_ < second.second.car_id_; });
	}

	for (auto&iter : tVector){
		char id[8];
		itoa(iter.second.car_id_, id, 10);
		std::string content = (std::string)id + "_";
		content += iter.first;
		QListWidgetItem *item = new QListWidgetItem(QString::fromLocal8Bit(content.c_str()));//�б�ֻ��ʾIP
		item->setCheckState(Qt::Unchecked);
		ui.connectedListWidget->addItem(item);
	}

	//for (auto&iter : nsp::toolkit::singleton<connect_manage>::instance()->map_ip_se_connected_){
	//	char id[8];
	//	itoa(iter.second.car_id_, id, 10);
	//	std::string content = (std::string)id + "_";
	//	content += iter.first;
	//	QListWidgetItem *item = new QListWidgetItem(QString::fromLocal8Bit(content.c_str()));//�б�ֻ��ʾIP
	//	item->setCheckState(Qt::Unchecked);
	//	ui.connectedListWidget->addItem(item);
	//}
	//ui.connectedListWidget->sortItems(Qt::AscendingOrder);
}
void log_select::change_state()
{
	for (int row = 0; row < ui.connectedListWidget->count(); row++){
		size_t pos = ui.connectedListWidget->item(row)->text().toStdString().find_last_of("_");
		size_t length = ui.connectedListWidget->item(row)->text().toStdString().length();
		std::string sel_ep = ui.connectedListWidget->item(row)->text().toStdString().substr(pos + 1, length - pos);
		if (sel_ep == broken_ip_){
			ui.connectedListWidget->takeItem(row);
		}
	}	
	//QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("The network has been disconnected."), QMessageBox::Ok);//�����ѶϿ�
}
void log_select::change_state_back()
{
	ui.stackedWidget->setCurrentIndex(0);
	QMessageBox::information(this, QStringLiteral("warning��"), QStringLiteral("The network has been disconnected."), QMessageBox::Ok);//�����ѶϿ�
	ui.connectButton->setEnabled(true);
	ui.disconnectButton->setEnabled(false);
	if (!schedule_close_){
		file_schedule_->deleteLater();
		schedule_close_ = true;		
	}
}
//�رմ�������
void log_select::closeEvent(QCloseEvent * event)
{
	switch (QMessageBox::information(this, QStringLiteral("Warning"),
		QStringLiteral("Do you want to close black_box ?"),
		QStringLiteral("Yes"), QStringLiteral("No"),
		0, 1))
	{
	case 0:
		event->accept();
		break;
	case 1:
	default:
		event->ignore();
		break;
	}
}

void log_select::get_current_time(std::string&current_t)
{
	QDateTime time = QDateTime::currentDateTime();
	current_t = time.toString("yyyy").toStdString() + time.toString("MM").toStdString() + time.toString("dd").toStdString() +
		"_" + time.toString("HH").toStdString() + time.toString("mm").toStdString() + time.toString("ss").toStdString();
}
//������־ģ��
void log_select::on_templateButton_clicked()
{
	nsp::toolkit::singleton<rw_xml_file>::instance()->get_template_type(template_type_);
	template_manage manage_page(this);
	manage_page.map_log_types_ = map_log_types_;
	manage_page.template_type_ = template_type_;
	manage_page.init_page();
	if (manage_page.exec()){
		//delete tab_page;
		template_type_ = manage_page.template_type_;
		nsp::toolkit::singleton<rw_xml_file>::instance()->write_template_type(template_type_);
		int count = ui.tabWidget->count();

		init_log_group();
	}
}

void log_select::init_log_group()
{
	ui.tabWidget->clear();
	nsp::toolkit::singleton<rw_xml_file>::instance()->get_template_type(template_type_);
	int i = 0;
	for (auto&iter : template_type_){
		group_tab*tab_page = new group_tab(this);
		tab_page->set_group_data(iter.second);
		tab_page->set_group_check(false);
		tab_page->group_name_ = iter.first;
		ui.tabWidget->insertTab(i, tab_page, QString::fromLocal8Bit(iter.first.c_str()));
		i++;
		sel_group_->addButton(tab_page->ui.radioButton);
	}
}
void log_select::on_tempButton_clicked()
{
	ui.tabWidget->setEnabled(true);
	ui.templateButton->setEnabled(true);
	ui.positionBox->setEnabled(false);
	ui.repositionBox->setEnabled(false);
	ui.systemLogBox->setEnabled(false);
}

void log_select::on_userDefButton_clicked()
{
	ui.tabWidget->setEnabled(false);
	ui.templateButton->setEnabled(false);
	ui.positionBox->setEnabled(true);
	ui.repositionBox->setEnabled(true);
	ui.systemLogBox->setEnabled(false);
}