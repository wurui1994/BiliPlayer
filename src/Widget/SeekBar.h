#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <QMouseEvent>
#include <QPaintEvent>
#include <QList>
#include <QSlider>

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
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);
private:
    QList<int> ticks;
    bool tickReady;
    int totalTime;
	QPoint last_pos;
};

#endif // SEEKBAR_H
