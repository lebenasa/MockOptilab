#include "stdafx.h"
#include "cameramodel.h"
using namespace std;

CameraModel::CameraModel(int row, int col, QObject *parent)
	: QAbstractListModel(parent), m_row(0), m_col(0)
{
	initModel(row, col);
}

CameraModel::~CameraModel()
{

}

int CameraModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
	return m_row * m_col;
}

QVariant CameraModel::data(const QModelIndex& index, int role) const {
	if (index.row() < 0 || index.row() >= m_row * m_col)
		return QVariant();
	if (role == BufferRole)
		return m_buffer.at(index.row());
	else if (role == SelectedRole)
		return m_selected.at(index.row());
    else if (role == HighlightRole)
		return m_highlight.at(index.row());

	return QVariant();
}

bool CameraModel::setData(const QModelIndex & index, const QVariant & value, int role) {
	if (role == SelectedRole) {
		m_selected[index.row()] = value.toBool();
		emit dataChanged(index, index, { SelectedRole });
		return true;
	}
    if (role == HighlightRole) {
        m_highlight[index.row()] = value.toBool();
        emit dataChanged(index, index, { HighlightRole });
        return true;
    }
	return false;
}

Qt::ItemFlags CameraModel::flags(const QModelIndex& index) const {
    Q_UNUSED(index)
	return Qt::ItemIsEditable;
}

QHash<int, QByteArray> CameraModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[BufferRole] = "buffer";
	roles[SelectedRole] = "selected";
    roles[HighlightRole] = "highlight";
	return roles;
}

int CameraModel::rows() const {
	return m_row;
}

int CameraModel::cols() const {
	return m_col;
}

QPoint CameraModel::selectedCell() const {
	auto p = find(begin(m_selected), end(m_selected), true);
	return indexToPoint(p - begin(m_selected));
}

void CameraModel::initModel(int row, int col) {
	auto index = this->createIndex(0, 0);
	beginRemoveRows(index, 0, rowCount());
	m_buffer.erase(begin(m_buffer), end(m_buffer));
	m_selected.erase(begin(m_selected), end(m_selected));
	m_hasImage.erase(begin(m_hasImage), end(m_hasImage));
	endRemoveRows();
	
	beginInsertRows(index, 0, row * col);
    
	m_selected = vector<bool>(row * col);
	fill(begin(m_selected), end(m_selected), false);
    
	m_hasImage = vector<bool>(row * col);
	fill(begin(m_hasImage), end(m_hasImage), false);
    
    m_highlight = vector<bool>(row * col);
    fill(begin(m_highlight), end(m_highlight), false);
    
	m_buffer = vector<QImage>(row * col);
    auto im = QImage(10, 10, QImage::Format_RGB888);
    im.fill(qRgb(55, 55, 55));
	fill(begin(m_buffer), end(m_buffer), im);
    
	endInsertRows();

	m_row = row;
	emit rowsChanged(m_row);
	m_col = col;
	emit colsChanged(m_col);
}

void CameraModel::clearBuffers() {
	fill(begin(m_buffer), end(m_buffer), QImage(10, 10, QImage::Format_RGB888));
	auto tl = createIndex(0, 0);
	auto br = createIndex(rowCount() - 1, 0);
	emit dataChanged(tl, br, { BufferRole });
}

void CameraModel::clearBuffersAt(const QPoint& target) {
	auto index = pointToIndex(target);
	m_buffer[index] = QImage(10, 10, QImage::Format_RGB888);
	auto mi = createIndex(index, 0);
	emit dataChanged(mi, mi, { BufferRole });
}

void CameraModel::updateBuffer(const QImage& buffer, const QPoint& target) {
	int index = target.x() + target.y() * m_row;
	if (index >= 0 && index < rowCount()) {
		m_buffer[index] = buffer;
		m_hasImage[index] = true;
		auto modelIndex = createIndex(index, 0);
		emit dataChanged(modelIndex, modelIndex, { BufferRole });
	}
}

void CameraModel::saveBuffers(const QString& baseDir) {
	// We'll implement this later
    baseDir;
}

void CameraModel::clearSelection() {
	fill(begin(m_selected), end(m_selected), false);
	auto tl = createIndex(0, 0);
	auto br = createIndex(rowCount() - 1, 0);
	emit dataChanged(tl, br, { SelectedRole });
}

void CameraModel::select(const QPoint& target) {
	auto index = pointToIndex(target);
	m_selected[index] = true;
	auto mi = createIndex(index, 0);
	emit dataChanged(mi, mi, { SelectedRole });
}

void CameraModel::multiselect(const QPoint &tl, const QPoint &br) {
    for (int y = tl.y(); y <= br.y(); ++y) {
		for (int x = tl.x(); x <= br.x(); ++x) {
			select(QPoint(x, y));
		}
	}
}

void CameraModel::highlight(const QPoint &target) {
    auto index = pointToIndex(target);
    m_highlight[index] = true;
    auto mi = createIndex(index, 0);
    emit dataChanged(mi, mi, { HighlightRole });
}

void CameraModel::unhighlight() {
    fill(begin(m_highlight), end(m_highlight), false);
    auto tl = createIndex(0, 0);
    auto br = createIndex(rowCount() - 1, 0);
    emit dataChanged(tl, br, { HighlightRole });
}

QPoint CameraModel::indexToPoint(int index) const {
	int col = index % m_row;
	int row = (index - col) / m_col;
	return QPoint(col, row);
}

int CameraModel::pointToIndex(const QPoint& point) const {
	return point.x() + point.y() * m_row;
}

std::vector<QPoint> CameraModel::autoFill() const {
    if (count(begin(m_hasImage), end(m_hasImage), true) < 2)
        return vector<QPoint>();
	// Find top-left and bottom-right filled cell
	auto first_i = find(begin(m_hasImage), end(m_hasImage), true);
	int first = distance(begin(m_hasImage), first_i);
	auto last_i = find(rbegin(m_hasImage), rend(m_hasImage), true);
	int last = distance(begin(m_hasImage), last_i.base());
    int tly = indexToPoint(first).y();
    int bry = indexToPoint(last).y();
    int tlx = -1;
    for (int x = 0; x < m_col; ++x) {
        for (int y = 0; y < m_row; ++y) {
            if (m_hasImage.at(pointToIndex(QPoint(x, y)))) {
                tlx = x;
                break;
            }
        }
        if (tlx != -1)
            break;
    }
    int brx = -1;
    for (int x = m_col-1; x >= 0; --x) {
        for (int y = 0; y < m_row; ++y) {
            if (m_hasImage.at(pointToIndex(QPoint(x, y)))) {
                brx = x;
                break;
            }
        }
        if (brx != -1)
            break;
    }
    
    QPoint tl { tlx, tly };
    QPoint br { brx, bry };
    
    std::vector<QPoint> res, temp;
    for (int y = tl.y(); y <= br.y(); ++y) {
		for (int x = tl.x(); x <= br.x(); ++x) {
			temp.push_back(QPoint(x, y));
		}
	}
    int x = 0;
    auto pred = [&](int a, int b) { 
        return (find(begin(temp), end(temp), QPoint(a, b)) != end(temp)) & 
                !m_hasImage.at(pointToIndex(QPoint(a, b)));
    };
    for (int y = 0; y < m_row; ++y) {
        for (int i = 0; i < m_col; ++i) {
            if (pred(x, y))
                res.push_back(QPoint(x, y));
            x = (y%2 == 0) ? x + 1 : x - 1;
        }
        x = (y%2 == 0) ? x - 1 : x + 1;
    } 
    return res;
}

std::vector<QPoint> CameraModel::boxFill() const {
    // Selection is already restricted to box area by UI,
    // thus we simply return selected cells with empty image
	std::vector<QPoint> res;
    int x = 0;
    auto pred = [this](int a, int b) { 
        return m_selected.at(pointToIndex(QPoint(a,b))) & 
                !m_hasImage.at(pointToIndex(QPoint(a,b))); 
    };
    for (int y = 0; y < m_row; ++y) {
        for (int i = 0; i < m_col; ++i) {
            if (pred(x, y))
                res.push_back(QPoint(x, y));
            x = (y%2 == 0) ? x + 1 : x - 1;
        }
        x = (y%2 == 0) ? x - 1 : x + 1;
    }
	return res;
}

int CameraModel::selectedCount() {
    return count(begin(m_selected), end(m_selected), true);
}
