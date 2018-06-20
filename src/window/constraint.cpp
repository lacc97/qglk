#include "constraint.hpp"

#include <QGridLayout>

#include "pairwindow.hpp"

#define BORDER_SIZE (5)

Glk::WindowConstraint::WindowConstraint(Glk::WindowConstraint::Method method_, glui32 size_) : m_Method(method_), m_Size(size_) {}

void Glk::WindowConstraint::setChildWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* splitwin) {
    if(parentw->mp_Key)
        parentw->mp_Key->unparent();

    parentw->mp_Key = keywin;
    parentw->mp_Key->setWindowParent(parentw);
    parentw->mp_Key->show();

    
    if(parentw->mp_Split)
        parentw->mp_Split->unparent();

    parentw->mp_Split = splitwin;
    parentw->mp_Split->setWindowParent(parentw);
    parentw->mp_Split->show();
}

Glk::HorizontalWindowConstraint::HorizontalWindowConstraint(Glk::WindowConstraint::Method method_, glui32 size_) : WindowConstraint(method_, size_) {}

void Glk::HorizontalWindowConstraint::setupWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* splitwin) {
    if(parentw->layout())
        delete parentw->layout();

    QGridLayout* layout = new QGridLayout(parentw);

    if(isFixed()) {
        keywin->setMinimumSize(keywin->pixelWidth(size()), 0);
        keywin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

        splitwin->setMinimumSize(0, 0);
        splitwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    } else {
        keywin->setMinimumSize(0, 0);
        keywin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        splitwin->setMinimumSize(0, 0);
        splitwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    if(constrainsLeft()) {
        layout->addWidget(keywin, 0, 0);
        layout->addWidget(splitwin, 0, 1);

        if(isProportional()) {
            layout->setColumnStretch(0, size());
            layout->setColumnStretch(1, 100 - size());
        }
    } else {
        layout->addWidget(splitwin, 0, 0);
        layout->addWidget(keywin, 0, 1);

        if(isProportional()) {
            layout->setColumnStretch(0, 100 - size());
            layout->setColumnStretch(1, size());
        }
    }

    setChildWindows(parentw, keywin, splitwin);
}

// QPair<QRect, QRect> Glk::HorizontalWindowConstraint::geometry(const QRect& region, Glk::Window* key) {
//     QPair<QRect, QRect> pair;
//     int keyx, keyw, splitx;
//
//     if(isProportional())
//         keyw = region.width() * size() / 100;
//     else
//         keyw = key->pixelWidth(size());
//
//     if(constrainsLeft()) {
//         keyx = region.x();
//         splitx = region.x() + keyw;
//     } else {
//         keyx = region.x() + region.width() - keyw;
//         splitx = region.x();
//     }
//
//     pair.first = QRect(keyx, region.y(), keyw, region.height());
//     pair.second = QRect(splitx, region.y(), region.width() - keyw, region.height());
//
//     if(isBordered() && (keyw > 0) && (region.width() - keyw > 0)) {
//         if(constrainsLeft()) {
//             pair.first.adjust(0, 0, -BORDER_SIZE / 2, 0);
//             pair.second.adjust(BORDER_SIZE / 2, 0, 0, 0);
//         } else {
//             pair.first.adjust(BORDER_SIZE / 2, 0, 0, 0);
//             pair.second.adjust(0, 0, -BORDER_SIZE / 2, 0);
//         }
//     }
//
//     return pair;
// }
//
// QSize Glk::HorizontalWindowConstraint::minimumSize(Glk::Window* key) {
//     return isFixed() ? QSize(key->pixelWidth(size()), 0) : QSize();
// }


Glk::VerticalWindowConstraint::VerticalWindowConstraint(Glk::WindowConstraint::Method method_, glui32 size_) : WindowConstraint(method_, size_) {}

void Glk::VerticalWindowConstraint::setupWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* splitwin) {
    if(parentw->layout())
        delete parentw->layout();

    QGridLayout* layout = new QGridLayout(parentw);

    if(isFixed()) {
        keywin->setMinimumSize(0, keywin->pixelHeight(size()));
        keywin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

        splitwin->setMinimumSize(0, 0);
        splitwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    } else {
        keywin->setMinimumSize(0, 0);
        keywin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        splitwin->setMinimumSize(0, 0);
        splitwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    if(constrainsAbove()) {
        layout->addWidget(keywin, 0, 0);
        layout->addWidget(splitwin, 1, 0);

        if(isProportional()) {
            layout->setRowStretch(0, size());
            layout->setRowStretch(1, 100 - size());
        }
    } else {
        layout->addWidget(splitwin, 0, 0);
        layout->addWidget(keywin, 1, 0);

        if(isProportional()) {
            layout->setRowStretch(0, 100 - size());
            layout->setRowStretch(1, size());
        }
    }

    setChildWindows(parentw, keywin, splitwin);
}


// QPair<QRect, QRect> Glk::VerticalWindowConstraint::geometry(const QRect& region, Glk::Window* key) {
//     QPair<QRect, QRect> pair;
//     int keyy, keyh, splity;
//
//     if(isProportional())
//         keyh = region.height() * size() / 100;
//     else
//         keyh = key->pixelHeight(size());
//
//     if(constrainsAbove()) {
//         keyy = region.y();
//         splity = region.y() + keyh;
//     } else {
//         keyy = region.y() + region.height() - keyh;
//         splity = region.y();
//     }
//
//     pair.first = QRect(region.x(), keyy, region.width(), keyh);
//     pair.second = QRect(region.x(), splity, region.width(), region.height() - keyh);
//
//     if(isBordered() && (keyh > 0) && (region.height() - keyh > 0)) {
//         if(constrainsAbove()) {
//             pair.first.adjust(0, 0, 0, -BORDER_SIZE / 2);
//             pair.second.adjust(0, BORDER_SIZE / 2, 0, 0);
//         } else {
//             pair.first.adjust(0, BORDER_SIZE / 2, 0, 0);
//             pair.second.adjust(0, 0, 0, -BORDER_SIZE / 2);
//         }
//     }
//
//     return pair;
// }
//
// QSize Glk::VerticalWindowConstraint::minimumSize(Glk::Window* key) {
//     return isFixed() ? QSize(0, key->pixelHeight(size())) : QSize();
// }
