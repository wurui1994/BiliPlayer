#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtCore/QList>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>

class SeekBar : public QSlider
{
    Q_OBJECT
public:
    explicit SeekBar(QWidget *parent = 0);
signals:
	void tryToSeek(double time);
public slots:
	void setValueNoSignal(int value);
    void setTracking(int _totalTime);
    void setTicks(QList<int> values);
	int positionValue(int pos);
protected:
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);
	void leaveEvent(QEvent *event);
	void enterEvent(QEvent *event);
private:
    QList<int> ticks;
    bool tickReady;
    int totalTime;
	QPoint last_pos;
	bool m_isPressed;
	QLabel* m_label;
};

#endif // SEEKBAR_H
