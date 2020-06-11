#include "pairwidget.hpp"

#include <QChildEvent>

Glk::PairWidget::~PairWidget() {
    clearChildren();
}

void Glk::PairWidget::childEvent(QChildEvent* event) {
    if(event->child()->isWidgetType()) {
        QWidget* child = static_cast<QWidget*>(event->child());
        if(event->added()) {
            assert(!m_Children[0] || !m_Children[1]);

            if(m_Children[0])
                m_Children[1] = child;
            else
                m_Children[0] = child;
        } else if(event->removed()) {
            assert(m_Children[0] || m_Children[1]);
            assert(child == m_Children[0] || child == m_Children[1]);

            if(m_Children[0] == child)
                m_Children[0] = nullptr;
            else
                m_Children[1] = nullptr;
        }
    }
}

void Glk::PairWidget::clearChildren() {
    if(m_Children[0])
        m_Children[0]->setParent(nullptr);
    if(m_Children[1])
        m_Children[1]->setParent(nullptr);
}
