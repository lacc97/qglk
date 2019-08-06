#include "windowwidget.hpp"


#include <QKeyEvent>
#include <QMouseEvent>

#include "log/log.hpp"
#include "thread/taskrequest.hpp"

Glk::WindowWidget::WindowWidget()
    : QWidget{nullptr},
      m_ReceivingCharInput{false},
      m_ReceivingHyperlinkInput{false},
      m_ReceivingLineInput{false},
      m_LineTerminators{},
      m_ReceivingMouseInput{false} {}

bool Glk::WindowWidget::eventFilter(QObject* obj, QEvent* ev) {
    if(obj == mp_InputWidget) {
        if(ev->type() == QEvent::KeyPress && handleKeyPressEvent(static_cast<QKeyEvent*>(ev)))
            return true;
        else if(ev->type() == QEvent::MouseButtonPress && handleMousePressEvent(static_cast<QMouseEvent*>(ev)))
            return true;
    }

    return QWidget::eventFilter(obj, ev);
}

void Glk::WindowWidget::cancelCharInput() {
    assert(onEventThread());

    if(charInputPending())
        m_ReceivingCharInput = false;
}

void Glk::WindowWidget::cancelHyperlinkInput() {
    assert(onEventThread());

    assert(false);
    // TODO implement
}

void Glk::WindowWidget::cancelLineInput() {
    assert(onEventThread());

    if(lineInputPending()) {
        emit lineInput(Qt::Key_Enter, lineInputBuffer());
        m_ReceivingLineInput = false;
        onLineInputFinished();
    }
}

void Glk::WindowWidget::cancelMouseInput() {
    assert(onEventThread());

    if(charInputPending())
        m_ReceivingMouseInput = false;
}

void Glk::WindowWidget::requestCharInput() {
    assert(onEventThread());
    assert(mp_InputWidget);
    assert(!lineInputPending());
    assert(!charInputPending());

    m_ReceivingCharInput = true;
    mp_InputWidget->setFocus();

    onCharInputRequested();
}

void Glk::WindowWidget::requestHyperlinkInput() {
    assert(false);
    // TODO request hyperlink input
}

void Glk::WindowWidget::requestLineInput(glui32 maxInputLength, const std::set<Qt::Key>& terminators) {
    assert(onEventThread());
    assert(mp_InputWidget);
    assert(!charInputPending());
    assert(!lineInputPending());

    m_ReceivingLineInput = true;
    m_LineTerminators = terminators;
    mp_InputWidget->setFocus();

    onLineInputRequested();
}

void Glk::WindowWidget::requestMouseInput() {
    assert(onEventThread());
    assert(mp_InputWidget);
    assert(!mouseInputPending());

    m_ReceivingMouseInput = true;
//    mp_InputWidget->setFocus();

    onMouseInputRequested();
}

bool Glk::WindowWidget::handleKeyPressEvent(QKeyEvent* ev) {
    if(charInputPending()) {
        emit characterInput(static_cast<Qt::Key>(ev->key()), ev->text());
        m_ReceivingCharInput = false;
        onCharInputFinished();
        return true;
    } else if(lineInputPending() && isLineInputTerminatorKey(static_cast<Qt::Key>(ev->key()))) {
        emit lineInput(static_cast<Qt::Key>(ev->key()), lineInputBuffer());
        m_ReceivingLineInput = false;
        onLineInputFinished();
        return true;
    }

    return false;
}

bool Glk::WindowWidget::handleMousePressEvent(QMouseEvent* ev) {
    if(mouseInputPending()) {
        emit mouseInput(ev->pos());
        m_ReceivingMouseInput = false;
        onMouseInputFinished();
        return true;
    }

    return false;
}

void Glk::WindowWidget::installInputFilter(QWidget* widget) {
    assert(onEventThread());
    assert(!charInputPending());
    assert(!lineInputPending());
    assert(!mouseInputPending());
    assert(!hyperlinkInputPending());

    mp_InputWidget = widget;
    mp_InputWidget->installEventFilter(this);
}

QString Glk::WindowWidget::lineInputBuffer() {
    log_warn() << "Default Glk::WindowWidget::lineInputBuffer() implementation. Returning QString().";

    return QString();
}

void Glk::WindowWidget::onCharInputRequested() {

}

void Glk::WindowWidget::onCharInputFinished() {

}

void Glk::WindowWidget::onHyperlinkInputRequested() {

}

void Glk::WindowWidget::onLineInputRequested() {

}

void Glk::WindowWidget::onLineInputFinished() {

}

void Glk::WindowWidget::onMouseInputRequested() {

}

void Glk::WindowWidget::onMouseInputFinished() {

}
