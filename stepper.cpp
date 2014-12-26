#include "stdafx.h"
#include "stepper.h"

//#include "api_wrapper.h"

//Stepper
Stepper::Stepper(QObject *parent)
	: QObject(parent), m_working(false)
{
	QSettings s("Miconos", "Optilab");
	m_xLim = s.value("X_LIMIT", 100).toDouble();
	m_yLim = s.value("Y_LIMIT", 100).toDouble();
	m_zLim = s.value("Z_LIMIT", 100).toDouble();
	m_speed = s.value("SPEED", 100).toDouble();
}

Stepper::~Stepper()
{

}

void Stepper::setWorking(bool w) {
	if (m_working != w) {
		m_working = w;
		emit isWorkingChanged(m_working);
	}
}

void Stepper::setXLimit(double lim) {
	m_xLim = lim;
	QSettings s("Miconos", "Optilab");
	s.setValue("X_LIMIT", lim);
}

void Stepper::setYLimit(double lim) {
	m_yLim = lim;
	QSettings s("Miconos", "Optilab");
	s.setValue("Y_LIMIT", lim);
}

void Stepper::setZLimit(double lim) {
	m_zLim = lim;
	QSettings s("Miconos", "Optilab");
	s.setValue("Z_LIMIT", lim);
}

void Stepper::setSpeed(double spd) {
	m_speed = spd;
	QSettings s("Miconos", "Optilab");
	s.setValue("SPEED", spd);
}

void Stepper::addCommand(Command cmd) {
	commandPool.push(cmd);
}

//Call this function to start all command sequence
void Stepper::nextCommand() {
	disconnect(this, &Stepper::bufferFull, this, &Stepper::nextCommand);
    if (!commandPool.empty()) {
        auto cmd = commandPool.front();
        cmd();
        commandPool.pop();
    }
    connect(this, &Stepper::bufferFull, this, &Stepper::nextCommand);
}

//Convenient functions
void Stepper::addMoveCommand(int code, double dist) {
	switch (code) {
	case 0:
		addCommand([=]() { moveX(dist); });
		break;
	case 1:
		addCommand([=]() { moveY(dist); });
		break;
	case 2:
		addCommand([=]() { moveZ(dist); });
		break;
	}
}

void Stepper::addMoveToCommand(const QPointF &target) {
    addCommand([=]() { moveTo(target); });
}

//Currently only support code 2 (X Axis) and code 0 (Y Axis)
void Stepper::addWaitLimitCommand(int code, int movement) {
    Q_UNUSED(movement)
	if (code == 2) {
		auto cmd1 = [this]() {
			moveX(-1000);
			connect(this, &Stepper::limit2Changed, this, &Stepper::nextCommand);
		};
		auto cmd2 = [this]() { stop(0); };
		addCommand(cmd1);
		addCommand(cmd2);
	}
	else if (code == 0) {
		auto cmd1 = [this]() {
			moveY(-1000);
			connect(this, &Stepper::limit0Changed, this, &Stepper::nextCommand);
		};
		auto cmd2 = [this]() { stop(0); };
		addCommand(cmd1);
		addCommand(cmd2);
	}
}

void Stepper::addBlockCommand(int msecond) {
    auto cmd = [=]() {
        QTimer::singleShot(msecond, this, SLOT(nextCommand()));
    };
    addCommand(cmd);
}

enum Movement {
	Idle, Up, Right, Down, Left, ZUp, ZDown, UpRight, DownRight, DownLeft, UpLeft
};

// MockStepper
MockStepper::MockStepper(QObject* parent)
	: Stepper(parent), m_z(0), movementCode(Idle), m_bufferSize(14),
	m_bufferFree(14), m_ztarget(0)
{
	m_limit = std::bitset<8>(false);
	eventDriver = new QTimer(this);
	eventDriver->setInterval(1);
	connect(eventDriver, &QTimer::timeout, this, &MockStepper::updateStatus);
	eventDriver->start();
}

MockStepper::~MockStepper() {}

void MockStepper::updateStatus() {
	auto incr = [](const double &now, const double &target) -> double {
		double res = now;
		if (target > now)
			return res + .5;
		return res - .5;
	};
	if (m_target != m_pos) {
		if (m_target.x() != m_pos.x()) {
			auto npos = incr(m_pos.x(), m_target.x());
			m_pos.setX(npos);
			emit xChanged(npos);
		}
		if (m_target.y() != m_pos.y()) {
			auto npos = incr(m_pos.y(), m_target.y());
			m_pos.setY(npos);
			emit yChanged(npos);
		}
		emit xyChanged(m_pos);

		if (fabs(m_pos.x() - m_target.x()) <= .00001 &&
			fabs(m_pos.y() - m_target.y()) <= .00001) {
			stop(0);
			++m_bufferFree;
			emit bufferFreeChanged(m_bufferFree);
			emit bufferFull();
			setWorking(false);
		}
	}
	if (m_ztarget != m_z) {
		auto npos = incr(m_z, m_ztarget);
		m_z = npos;
		emit zChanged(npos);
	}
}

void MockStepper::setTarget(const QPointF& ntarget) {
	eventDriver->stop();
	m_target = ntarget;
	if (m_target != m_pos) {
		--m_bufferFree;
		emit bufferFreeChanged(m_bufferFree);
		setWorking(true);
	}
	eventDriver->start();
}

void MockStepper::setTarget(double x, double y) {
	double xx, yy;
	xx = (x < 0) ? m_pos.x() : x;
	yy = (y < 0) ? m_pos.y() : y;
	setTarget(QPointF(xx, yy));
}

void MockStepper::jogUp() {
	if (movementCode != Idle) return;
	setTarget(-1, 0.0);
	movementCode = Up;
}

void MockStepper::jogRight() {
	if (movementCode != Idle) return;
	setTarget(m_xLim, -1);
	movementCode = Right;
}

void MockStepper::jogDown() {
	if (movementCode != Idle) return;
	setTarget(-1, m_yLim);
	movementCode = Down;
}

void MockStepper::jogLeft() {
	if (movementCode != Idle) return;
	setTarget(0.0, -1.0);
	movementCode = Left;
}

void MockStepper::jogZUp() {
	if (movementCode != Idle) return;
	m_ztarget = m_zLim;
	movementCode = ZUp;
}

void MockStepper::jogZDown() {
	if (movementCode != Idle) return;
	m_ztarget = 0;
	movementCode = ZDown;
}

void MockStepper::jogUR() {
	if (movementCode != Idle) return;
	double DIST_X = m_xLim - m_pos.x();
	double DIST_Y = m_pos.y();
	auto DIST = (DIST_X < DIST_Y) ? DIST_X : DIST_Y;
	setTarget(m_pos.x() + DIST, m_pos.y() - DIST);
	movementCode = UpRight;
}

void MockStepper::jogDR() {
	if (movementCode != Idle) return;
	double DIST_X = m_xLim - m_pos.x();
	double DIST_Y = m_yLim - m_pos.y();
	auto DIST = (DIST_X < DIST_Y) ? DIST_X : DIST_Y;
	setTarget(m_pos.x() + DIST, m_pos.y() + DIST);
	movementCode = DownRight;
}

void MockStepper::jogDL() {
	if (movementCode != Idle) return;
	double DIST_X = m_pos.x();
	double DIST_Y = m_yLim - m_pos.y();
	auto DIST = (DIST_X < DIST_Y) ? DIST_X : DIST_Y;
	setTarget(m_pos.x() - DIST, m_pos.y() + DIST);
	movementCode = DownLeft;
}

void MockStepper::jogUL() {
	if (movementCode != Idle) return;
	double DIST_X = m_pos.x();
	double DIST_Y = m_pos.y();
	auto DIST = (DIST_X < DIST_Y) ? DIST_X : DIST_Y;
	setTarget(m_pos.x() - DIST, m_pos.y() - DIST);
	movementCode = UpLeft;
}

void MockStepper::stop(int code) {
	if (code == movementCode || code == 0) {
		setTarget(m_pos);
		movementCode = Idle;
	}
}

void MockStepper::moveX(double dist) {
	if (movementCode != Idle) return;
	setTarget(m_pos.x() + dist, -1);
}

void MockStepper::moveY(double dist) {
	if (movementCode != Idle) return;
	setTarget(-1, m_pos.y() + dist);
}

void MockStepper::moveZ(double dist) {
	if (movementCode != Idle) return;
	m_ztarget = m_z + dist;
}

void MockStepper::moveTo(const QPointF& npos) {
	setTarget(npos);
}
