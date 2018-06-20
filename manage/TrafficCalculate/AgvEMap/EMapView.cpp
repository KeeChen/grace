#include "EMapView.h"
#include <iostream>

EMapView::EMapView(QWidget *parent)
	: QGraphicsView(parent)
{
	setParent(parent);
	setCacheMode(CacheBackground);//��������ģʽ
	setViewportUpdateMode(BoundingRectViewportUpdate);//�ӿڵĸ���ģʽ��
	setRenderHint(QPainter::Antialiasing);//�����
	setTransformationAnchor(AnchorUnderMouse);
	setOptimizationFlags(QGraphicsView::DontSavePainterState);
	setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	setMouseTracking(true);//���ø���
	setMatrix( QMatrix(1.0, 0.0 , 0.0 , -1.0 , 0.0 , 0.0) , true );//ת������ͼ

	setDragMode(QGraphicsView::ScrollHandDrag);
	setWindowTitle(tr("EMap"));
}

void EMapView::FitInView()
{
	QRectF m_rect = map_rect_;
	centerOn(m_rect.center());
	fitInView(m_rect, Qt::KeepAspectRatio);
	min_scaled_ = transform().m11();
}

void EMapView::SetEMapRect(QRectF rect)
{
	map_rect_ = rect;
}

void EMapView::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0) {
		scale(1.3, 1.3);
	}
	else {
		double m_value = transform().m11();
		if (m_value * 0.7 > min_scaled_) {
			scale(0.7, 0.7);
		}
		else {
			this->FitInView();
		}
	}
	//QGraphicsView::wheelEvent(event);
}
