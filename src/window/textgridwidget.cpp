#include "textgridwidget.hpp"

#include <memory>

#include <QFontDatabase>
#include <QPainter>
#include <QPaintEvent>

Glk::TextGridWidget::TextGridWidget()
    : WindowWidget{} {
    setFocusPolicy(Qt::StrongFocus);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    installInputFilter(this);
}

void Glk::TextGridWidget::paintEvent(QPaintEvent* event) {
    assert(!m_Grid.empty() && !m_Grid[0].empty());

    QWidget::paintEvent(event);

    QRect unitBox(0, 0, contentsRect().width() / m_Grid.size(), contentsRect().height() / m_Grid[0].size());
    int glyphHMargin = (unitBox.width() - fontMetrics().horizontalAdvance('m')) / 2;
    int glyphVMargin = (unitBox.height() - fontMetrics().height()) / 2;
    QRect glyphBox(glyphHMargin, glyphVMargin, fontMetrics().horizontalAdvance('m'), fontMetrics().height());

    std::unique_ptr<QPainter> p = std::make_unique<QPainter>(this);

    for(size_t xx = 0; xx < m_Grid.size(); ++xx) {
        for(size_t yy = 0; yy < m_Grid[xx].size(); ++yy) {
            QRect thisGlyphBox = glyphBox.translated(xx * unitBox.width(), yy * unitBox.height());

            if(event->rect().intersects(thisGlyphBox))
                p->drawText(QPoint(thisGlyphBox.x(), thisGlyphBox.y() + fontMetrics().ascent()),
                            QString::fromUcs4(&m_Grid[xx][yy], 1));
        }
    }
}

void Glk::TextGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    emit resized();
}