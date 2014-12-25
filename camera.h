#ifndef CAMERA_H
#define CAMERA_H

/* 
Base implementation of Camera
- Buffer current frame as QImage
- Capture to file and to buffer
- Set and get resolution
*/

#include <QObject>

class Camera : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QSize sourceSize READ size NOTIFY sourceSizeChanged)
public:
	Camera(QObject *parent = 0) : QObject(parent) { }
	~Camera() { }

public slots:
	virtual void setResolution(int res) = 0;

	virtual QSize& size() const = 0;

	virtual void capture(int resolution, const QString &fileName) = 0;

signals :
	void frameReady(const QImage& frame);
	void sourceSizeChanged(const QSize& sz);

protected:
	virtual void initialize() = 0;
	virtual void deinitialize() = 0;
};

class MockCamera : public Camera
{
    Q_OBJECT
    QImage m_buffer;
    QTimer* emitter;
    int state;
public:
	MockCamera(QObject *parent = 0);
	~MockCamera();

public slots:
    void setResolution(int res) { Q_UNUSED(res) }

    QSize& size() const { return m_buffer.size(); }

    void capture(int resolution, const QString &fileName);
    
    void imageProc();

protected:
    void initialize() { }
    void deinitialize() { }
};

/*
QuickCam - QQuickItem to render camera stream
- Image stream is stretched, aspect ratio calculation is done elsewhere
- Can selectively render a part of image
- Can process image in rendering thread

To use simply connect Camera::frameReady signals to QuickCam::updateImage
*/

#include <qquickitem.h>

class QuickCam : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(QImage source READ currentFrame WRITE updateImage NOTIFY sourceChanged)
	Q_PROPERTY(bool blocked READ isBlocked WRITE block NOTIFY blockedChanged)
	Q_PROPERTY(RenderParams renderParams MEMBER renderParams)
	Q_ENUMS(RenderParams)
	QImage m_frame;
	bool m_blocked;
public:
	QuickCam(QQuickItem* parent = 0);
	~QuickCam();

	enum RenderParams {
		OriginalSize, ScaledToItem, Halved
	} renderParams;

	bool isBlocked();
	void block(bool bl);

	QImage currentFrame() const;

public slots:
	void updateImage(const QImage &frame);

signals:
	void blockedChanged(bool block);
	void sourceChanged(const QImage &nframe);

protected:
	QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);
};

#endif // CAMERA_H
