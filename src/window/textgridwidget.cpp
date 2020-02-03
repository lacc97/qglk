#include "textgridwidget.hpp"

#include <memory>

#include <QDebug>
#include <QFontDatabase>
#include <QGridLayout>
#include <QPainter>
#include <QPaintEvent>

Glk::TextGridWidget::TextGridWidget()
    : WindowWidget{}, mp_Label{} {
    QGridLayout* lay = new QGridLayout(this);
    lay->setMargin(0);

    mp_Label = new QLabel(this);
    mp_Label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mp_Label->setTextFormat(Qt::PlainText);
    mp_Label->setAlignment(Qt::AlignCenter);

    lay->addWidget(mp_Label);


    setFocusPolicy(Qt::StrongFocus);

    installInputFilter(this);
}

void Glk::TextGridWidget::setGrid(const std::vector<std::vector<glui32>> &newGrid) {
    QString str;
    {
        size_t ii = 0;
        for(const auto& line : newGrid) {
            str.append(QString::fromUcs4(line.data(), line.size()));
            if(ii != newGrid.size()-1)
                str.append('\n');
            ++ii;
        }
    }

    mp_Label->setText(str);
}

void Glk::TextGridWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    emit resized();
}