#pragma once
#include <QtWidgets/qgraphicsitem.h>
#include <QtWidgets/qstyleoption.h>
#include <QtGui/qpainter.h>
//#include "MapScene.h"
#include "BasicDefine.h"

class EdgeItem;
class MapScene;

class NodeItem : public QGraphicsItem
{
public:
	NodeItem(MapScene* mapScene);
	~NodeItem();
public:
	int type() const { return ItemType::ITEM_NODE; }
	void SetFeature();
	void SetNodeInfo(const NODEINFO& nodeInfo);
	int GetNodeId();
	int GetTshaped();
	void SetMoveable(bool bMoveable);
	void SetPosData(const QPointF& pos);
	void SetEdgeItem(EdgeItem* pEdgeItem);
	void RemoveEdge(EdgeItem* pEdgeItem);
	void RemoveAllEdge(); //ɾ�����˽ڵ���������б�
	QList<EdgeItem*>* listEdgeItem() { return &m_listEdgeItem; }

	void  UpdateNodeInfo(bool spin , double x, double y);
private:
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void DrawNode(QPainter* painter);
protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	QVariant itemChange(GraphicsItemChange change, const QVariant& value);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
private:
	//MapScene* m_pMapScene;
	NODEINFO m_nodeInfo;
	double m_dScale;
	bool m_bMouseHover;//�Ƿ������ͣ
	bool m_bMousePress;//����Ƿ���
	bool m_bAlign;//�Ƿ����ö�����

	QList<EdgeItem*> m_listEdgeItem;//�����˽ڵ�����ı�
	MapScene* m_mapScene;
};

