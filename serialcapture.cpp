#include "stdafx.h"
#include "serialcapture.h"
#include "camera.h"
#include "stepper.h"
#include "cameramodel.h"
#include "sminterface.h"
using namespace std;

SerialCapture::SerialCapture(QObject *parent)
	: QObject(parent), engine{ this }, m_zoom(10), m_block(false)
{
	// Initialize
	m_camera = new MockCamera(this);
	m_stepper = new MockStepper(this);
	m_interface = new SMInterface(0, this);
	m_model = new CameraModel(m_interface->rows(), m_interface->cols(), this);
	qmlRegisterType<QuickCam>("QuickCam", 1, 0, "CameraItem");

	auto sz = m_camera->size();
    m_size = QSize(sz.width() / m_zoom, sz.height() / m_zoom);

	// Connections
	connect(m_stepper, &Stepper::xyChanged, m_interface, &SMInterface::updatePos);
	connect(m_stepper, &Stepper::bufferFull, this, &SerialCapture::unblockStream);
	connect(m_camera, &MockCamera::frameReady, this, &SerialCapture::redirectImage);
	connect(m_interface, &SMInterface::stepperMoveTo, m_stepper, &Stepper::moveTo);
}

SerialCapture::~SerialCapture()
{

}

void SerialCapture::blockStream() {
	if (!m_block) {
		m_block = true;
		emit blockChanged(true);
	}
}

void SerialCapture::unblockStream() {
	if (m_block) {
		m_block = false;
		emit blockChanged(false);
	}
}

void SerialCapture::redirectImage(const QImage& img) {
	if (!m_stepper->isWorking())
		m_model->updateBuffer(img, m_interface->currentPos());
}

QSize SerialCapture::cellSize() const {
	return m_size;
}

void SerialCapture::setCellSize(const QSize& size) {
	if (m_size != size) {
		m_size = size;
		emit cellSizeChanged(m_size);
	}
}

void SerialCapture::zoomIn() {
	if (m_zoom > 1) {
		--m_zoom;
		setCellSize(m_camera->size() / m_zoom);
	}
}

void SerialCapture::zoomOut() {
	if (m_zoom < 10) {
		++m_zoom;
		setCellSize(m_camera->size() / m_zoom);
	}
}

void SerialCapture::moveToSelected() {
	blockStream();
	m_interface->moveTo(m_model->selectedCell());
}

void SerialCapture::show() {
	// Start UI
	rootContext = engine.rootContext();
	rootContext->setContextProperty("serialcapture", this);
	rootContext->setContextProperty("camera", m_camera);
	rootContext->setContextProperty("stepper", m_stepper);
	rootContext->setContextProperty("cammodel", m_model);
	rootContext->setContextProperty("istep", m_interface);
	auto now = std::chrono::steady_clock::now();
	engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
	auto then = std::chrono::steady_clock::now();
	auto diff = then - now;
	qDebug() << "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " ms";
}

void SerialCapture::procSelect(const QPoint &pos) {
    m_model->clearSelection();
    int col = pos.x() / m_size.width();
    int row = pos.y() / m_size.height();
    m_model->select(QPoint(col, row));
}

void SerialCapture::procHighlight(const QPoint &pos) {
    int col = pos.x() / m_size.width();
    int row = pos.y() / m_size.height();
    QPoint index{ col, row };
    if (index != lastHighlight) {
        m_model->unhighlight();
        m_model->highlight(QPoint(col, row));
        lastHighlight = index;
    }
}

void SerialCapture::beginMultiSelect(const QPoint &pos) {
    int col = pos.x() / m_size.width();
    int row = pos.y() / m_size.height();
    select1 = QPoint(col, row);
}

void SerialCapture::endMultiSelect(const QPoint &pos) {
    m_model->clearSelection();
    int col = pos.x() / m_size.width();
    int row = pos.y() / m_size.height();
    auto select2 = QPoint(col, row);
    int tlx = (select1.x() < select2.x()) ? select1.x() : select2.x();
    int tly = (select1.y() < select2.y()) ? select1.y() : select2.y();
    int brx = (select1.x() > select2.x()) ? select1.x() : select2.x();
    int bry = (select1.y() > select2.y()) ? select1.y() : select2.y();
    m_model->multiselect(QPoint(tlx, tly), QPoint(brx, bry));
    selectCounter = m_model->selectedCount();
    emit selectCounterChanged(selectCounter);
}

void SerialCapture::boxFill() {
    auto ts = m_model->boxFill();
    vector<QPointF> targets;
    for (const auto& t : ts)
        targets.push_back(m_interface->indexToCoord(t));
    for (auto f = begin(targets); f != end(targets); ++f) {
        m_stepper->addMoveToCommand(*f);
        m_stepper->addBlockCommand(100);
    }
    m_stepper->nextCommand();
}
