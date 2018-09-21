#ifndef FRAMELESS_HELPER_H
#define FRAMELESS_HELPER_H

// ref: https://github.com/sangxiaokai/FrameLessWidget

#include <QObject>
#include <QRect>
#include <QRubberBand>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QApplication>
#include <QStyleOptionRubberBand>

class QWidget;
class FramelessHelperPrivate;

class FramelessHelper : public QObject
{
    Q_OBJECT

public:
    explicit FramelessHelper(QObject *parent = 0);
    ~FramelessHelper();
    // 
    void activateOn(QWidget *topLevelWidget);
    // 
    void removeFrom(QWidget *topLevelWidget);
    // 
    void setWidgetMovable(bool movable);
    // 
    void setWidgetResizable(bool resizable);
    // 
    void setRubberBandOnMove(bool movable);
    // 
    void setRubberBandOnResize(bool resizable);
    // 
    void setBorderWidth(uint width);
    // 
    void setTitleHeight(uint height);
    bool widgetResizable();
    bool widgetMovable();
    bool rubberBandOnMove();
    bool rubberBandOnResisze();
    uint borderWidth();
    uint titleHeight();

protected:
    // 
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    FramelessHelperPrivate *d;
};


/*****
 * CursorPosCalculator
 * 
*****/
class CursorPosCalculator
{
public:
    explicit CursorPosCalculator();
    void reset();
    void recalculate(const QPoint &globalMousePos, const QRect &frameRect);

public:
    bool m_bOnEdges              : true;
    bool m_bOnLeftEdge           : true;
    bool m_bOnRightEdge          : true;
    bool m_bOnTopEdge            : true;
    bool m_bOnBottomEdge         : true;
    bool m_bOnTopLeftEdge        : true;
    bool m_bOnBottomLeftEdge     : true;
    bool m_bOnTopRightEdge       : true;
    bool m_bOnBottomRightEdge    : true;

    static int m_nBorderWidth;
    static int m_nTitleHeight;
};


/*****
 * WidgetData
 *
*****/
class WidgetData
{
public:
    explicit WidgetData(FramelessHelperPrivate *d, QWidget *pTopLevelWidget);
    ~WidgetData();
    QWidget* widget();
    // 
    void handleWidgetEvent(QEvent *event);
    // 
    void updateRubberBandStatus();

private:
    // 
    void updateCursorShape(const QPoint &gMousePos);
    // 
    void resizeWidget(const QPoint &gMousePos);
    // 
    void moveWidget(const QPoint &gMousePos);
    // 
    void handleMousePressEvent(QMouseEvent *event);
    // 
    void handleMouseReleaseEvent(QMouseEvent *event);
    // 
    void handleMouseMoveEvent(QMouseEvent *event);
    // 
    void handleLeaveEvent(QEvent *event);
    // 
    void handleHoverMoveEvent(QHoverEvent *event);

private:
    FramelessHelperPrivate *d;
    QRubberBand *m_pRubberBand;
    QWidget *m_pWidget;
    QPoint m_ptDragPos;
    CursorPosCalculator m_pressedMousePos;
    CursorPosCalculator m_moveMousePos;
    bool m_bLeftButtonPressed;
    bool m_bCursorShapeChanged;
    bool m_bLeftButtonTitlePressed;
    Qt::WindowFlags m_windowFlags;
};

/*****
 * FramelessHelperPrivate
*****/

class FramelessHelperPrivate
{
public:
    QHash<QWidget*, WidgetData*> m_widgetDataHash;
    bool m_bWidgetMovable        : true;
    bool m_bWidgetResizable      : true;
    bool m_bRubberBandOnResize   : true;
    bool m_bRubberBandOnMove     : true;
};

#endif //FRAMELESS_HELPER_H
