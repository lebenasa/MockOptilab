#include "stdafx.h"
#include "camera.h"

#include <qsgsimpletexturenode.h>
#include <qquickwindow.h>

#include "opencv2\opencv.hpp"

Camera::Camera(QObject *parent)
    : QObject(parent)
{
    
}

Camera::~Camera()
{
    
}

//MockCamera implementation
MockCamera::MockCamera(QObject *parent)
    : Camera(parent), state(0), m_buffer(QSize(1280, 1024), QImage::Format_RGB888)
{
    m_buffer.fill(qRgb(0, 255, 0));
    emitter = new QTimer(this);
    emitter->setInterval(1000/10);
    connect(emitter, &QTimer::timeout, this, &MockCamera::imageProc);
    emitter->start();
}

MockCamera::~MockCamera()
{
    
}

void MockCamera::capture(int resolution, const QString &fileName) {
    Q_UNUSED(resolution)
    Q_UNUSED(fileName)
}

void MockCamera::imageProc() {
    auto rgb = QColor::fromHsv(state, 255, 255);
    m_buffer.fill(rgb);
    emit frameReady(m_buffer);
    if (state >= 355)
        state = 0;
    else
        state += 5;
}

//QuickCam implementation
QuickCam::QuickCam(QQuickItem* parent)
	: QQuickItem(parent), m_frame(QSize(10, 10), QImage::Format_RGB888), m_blocked(false)
{
	m_frame.fill(Qt::white);
	renderParams = OriginalSize;
	setFlag(QQuickItem::ItemHasContents, true);
}

QuickCam::~QuickCam() {

}

bool QuickCam::isBlocked() {
	return m_blocked;
}

void QuickCam::block(bool bl) {
	if (m_blocked != bl) {
		m_blocked = bl;
		emit blockedChanged(bl);
	}
}

QImage QuickCam::currentFrame() const {
	return m_frame;
}

void QuickCam::updateImage(const QImage &frame) {
	if (!m_blocked) {
		auto src = frame;
        int w = (width() > 0) ? width() : src.width() / 10;
        int h = (height() > 0) ? height() : src.height() / 10;
		if (renderParams == ScaledToItem)
			m_frame = src.scaled(QSize(w, h));
		else if (renderParams == Halved)
			m_frame = src.scaled(QSize(src.width() / 2, src.height() / 2));
        else
            m_frame = src;
	}
	update();
	emit sourceChanged(m_frame);
}

QSGNode* QuickCam::updatePaintNode(QSGNode* node, UpdatePaintNodeData* u) {
	Q_UNUSED(u)
	QSGSimpleTextureNode* n = static_cast<QSGSimpleTextureNode*>(node);
	if (!n) {
		n = new QSGSimpleTextureNode();
	}
	n->setRect(boundingRect());

	auto texture = n->texture();
	if (texture) texture->deleteLater();
	n->setTexture(this->window()->createTextureFromImage(m_frame));
	return n;
}
