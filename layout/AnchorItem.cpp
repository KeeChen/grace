#include "AnchorItem.h"
#include "AnchorInfoWnd.h"
#include "point_algorithm.hpp"
#include "MapScene.h"


AnchorItem::AnchorItem()
{
	SetFeature();
}


AnchorItem::~AnchorItem()
{
}

void AnchorItem::SetFeature()
{
	//this->setFlag(ItemIsMovable, true);
	this->setFlag(ItemIsSelectable, true);
	this->setFlag(ItemSendsGeometryChanges);
	this->setCacheMode(DeviceCoordinateCache);
	this->setAcceptedMouseButtons(Qt::LeftButton);
	this->setAcceptHoverEvents(true);
	this->setZValue(0);
}

QPainterPath AnchorItem::shape() const
{
	double dScale = m_bMouseHover ? 1.5 * SHAPE_R : SHAPE_R;

	QPainterPath path;
	path.addEllipse(-dScale, -dScale, 2 * dScale, 2 * dScale);
	return path;
}

QRectF AnchorItem::boundingRect() const
{
	double shaper = 0;
	shaper = 1.5 * SHAPE_R;

	double adjust = 3;
	return QRectF(-2 * shaper - adjust, -2 * shaper - adjust, 4 * (shaper + adjust), 4 * (shaper + adjust));
}

void AnchorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	if (painter == NULL)
	{
		return;
	}
	//�˾���ת������ϵ����
	painter->setTransform(QTransform(QMatrix(1, 0, 0, -1, 0, 0.0)), true);
	DrawAnchorItem(painter);
}

void AnchorItem::DrawAnchorItem(QPainter* painter)
{
	QColor mainColor = QColor(Qt::red);
	m_dScale = m_bMouseHover ? 1.5 * SHAPE_R : SHAPE_R;

	QPen pen;
	QRadialGradient gradient(-3, -3, 10);
	if (!isSelected())
	{
		gradient.setColorAt(0, mainColor);
		gradient.setColorAt(1, mainColor);
		pen = QPen(Qt::black, 0);
	}
	else
	{
		gradient.setColorAt(1, QColor(mainColor).light(120));
		gradient.setColorAt(0, QColor(mainColor).light(120));
		pen = QPen(Qt::yellow, 2);
	}
	painter->setBrush(gradient);
	painter->setPen(pen);
	//painter->rotate(angle);
	painter->drawEllipse(-m_dScale, -m_dScale, 2 * m_dScale, 2 * m_dScale);
	mainColor = QColor(Qt::yellow);
	pen.setColor(Qt::yellow);
	painter->setPen(pen);
	painter->setBrush(QBrush(mainColor));
	painter->drawPolygon(QPolygonF()
		<< QPointF(m_dScale, 0)
		<< QPointF(0 - 0.7 * m_dScale, 0 + 0.7 * m_dScale)
		<< QPointF(0, 0)
		<< QPointF(0 - 0.7 * m_dScale, 0 - 0.7 * m_dScale));
	//��������
	pen.setColor(Qt::black);
	QRectF textRect = QRectF(QPointF(-m_dScale, -m_dScale), QSizeF(2 * m_dScale, 2 * m_dScale));
	painter->setFont(QFont("Times", m_dScale - 2, QFont::Normal));
	painter->setPen(pen);
	QString strText;
	strText = QString::number(m_anchorInfo.id);
	painter->drawText(textRect, Qt::AlignCenter, strText);

	if (m_is_target)//���ͣ���㱻��Ϊ��Ŀ���
	{
		
		pen.setColor(Qt::yellow);
		pen.setWidth(3);
		painter->setPen(pen);
		painter->setBrush(Qt::NoBrush);
		painter->drawEllipse(-m_dScale - 2, -m_dScale - 2, 2 * (m_dScale + 2), 2 * (m_dScale + 2));
	}
}

void AnchorItem::PopupInfoWnd()
{
	AnchorInfoWnd anchorInfoWnd;
	anchorInfoWnd.SetAnchorInfo(m_anchorInfo);
	anchorInfoWnd.UpdateWopTable(m_edgeInfo);
	if (anchorInfoWnd.exec() == QDialog::Accepted)
	{
		m_anchorInfo.wop_id = anchorInfoWnd.GetSelectWopId();
		anchorInfoWnd.GetAnchorInfo(m_anchorInfo);
		if (anchorInfoWnd.GetRelocFlag())//��ͣ���㶨λ����·��
		{
			position_t point_t;
			point_t.x_ = m_anchorInfo.pos_x;
			point_t.y_ = m_anchorInfo.pos_y;
			point_t.angle_ = m_anchorInfo.angle;
			position_t point_on;
			PT_ALG::point_on_edge(m_edgeInfo.id, point_t, point_on);

			m_anchorInfo.pos_x = point_on.x_;
			m_anchorInfo.pos_y = point_on.y_;
			m_anchorInfo.angle = CalAnchorAngle(point_on.angle_ * 180 / PI, m_anchorInfo);

			setPos(point_on.x_, point_on.y_);
		}
		else{
			setPos(m_anchorInfo.pos_x, m_anchorInfo.pos_y);
		}
		QString strAngle = anchorInfoWnd.GetAnchorAngle();
		if (!strAngle.isEmpty())
		{
			m_anchorInfo.angle = strAngle.toDouble();
			if (m_anchorInfo.angle < 0.0)//����Ƕ��Ǹ�ֵ����ת��Ϊ��ֵ
			{
				m_anchorInfo.angle = PT_ALG::angle_to_normal(m_anchorInfo.angle);
			}
		}
		setRotation(m_anchorInfo.angle);
		DATAMNG->AddAnchorInfo(m_anchorInfo);
		update();
	}
}

void AnchorItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
	if (static_cast<MapScene*>(scene())->GetActionType() == ActionType::ACTION_SELECT ||
		static_cast<MapScene*>(scene())->GetActionType() == ActionType::ACTION_HAND_DRAG ||
		static_cast<MapScene*>(scene())->GetActionType() == ActionType::ACTION_ENTER_NAV)
	{
		PopupInfoWnd();
	}
	QGraphicsItem::mouseDoubleClickEvent(event);
}

void AnchorItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (static_cast<MapScene*>(scene())->GetActionType() == ActionType::ACTION_ENTER_NAV)
	{
		setSelected(true);
	}
	QGraphicsItem::mousePressEvent(event);
}

int AnchorItem::CalAnchorAngle(double posAngle, const ANCHORINFO& anchorInfo)
{
	double angle = 0.0;
	WOP wop;
	DATAMNG->GetWopById(anchorInfo.wop_id, wop);
	if (wop.angle_type == CONST_ANGLE)
	{
		angle = posAngle + wop.angle;
	}
	else if (wop.angle_type == GLOBAL_CONST_ANGLE)
	{
		angle = wop.angle;
	}
	angle = (angle >= 0.0) ? ((angle >= 360.0) ? (angle - 360) : angle) : (angle + 360.0);//���Ƕ�ת����0-360��Χ��
	return angle;
}

void AnchorItem::ChangeToTarget(bool bTarget)
{
	if (m_is_target && bTarget)//����Ѿ��Ǳ���Ϊ��Ŀ��㣬�򲻲���
	{
		return;
	}
	if (bTarget)
	{
		m_is_target = true;
		DATAMNG->ChangeAnchorToTarget(GetAnchorId());
	}
	else
	{
		m_is_target = false;
	}
	update();
}