#ifndef SERIALCAPTURE_H
#define SERIALCAPTURE_H

#include <QObject>
#include "qqmlapplicationengine.h"
#include "qqmlcontext.h"

class MockCamera;
class MockStepper;
class CameraModel;
class SMInterface;

class SerialCapture : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QSize cellSize READ cellSize NOTIFY cellSizeChanged)
	Q_PROPERTY(bool block MEMBER m_block NOTIFY blockChanged)
    Q_PROPERTY(int selectCounter MEMBER selectCounter NOTIFY selectCounterChanged)
	
public:
	SerialCapture(QObject *parent = 0);
	~SerialCapture();

	QSize cellSize() const;
	void setCellSize(const QSize &size);

public slots:
	void zoomIn();
	void zoomOut();

	void blockStream();
	void unblockStream();
	void redirectImage(const QImage& img);

	void moveToSelected();

	void show();
    
    void procSelect(const QPoint& pos);
    void procHighlight(const QPoint& pos);
    
    void beginMultiSelect(const QPoint& pos);
    void endMultiSelect(const QPoint& pos);
    
    void boxFill();
    void autoFill();

signals:
	void cellSizeChanged(const QSize &size);
	void blockChanged(bool);
    void selectCounterChanged(int nv);
    void autoFillFailed();

private:
	MockCamera* m_camera;
	MockStepper* m_stepper;
	CameraModel* m_model;
	SMInterface* m_interface;
	QQmlApplicationEngine engine;
	QQmlContext* rootContext;

	QSize m_size;	// cell size, i.e for implementing zoom
	int m_zoom;
	bool m_block;
    
    QPoint lastHighlight, select1;
    int selectCounter = 0;
};

#endif // SERIALCAPTURE_H
