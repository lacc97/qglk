#include "pairwindow.hpp"

#include <cassert>

#include "qglk.hpp"
#include "stream/nulldevice.hpp"

// Glk::PairLayout::PairLayout(QWidget* parent_, Glk::WindowConstraint* constraint_) : QLayout(parent_), mp_Constraint(constraint_) {}
//
// Glk::PairLayout::~PairLayout() {
//     delete mp_Constraint;
//
//     QLayoutItem* item;
//
//     while((item = takeAt(0)))
//         delete item;
// }
//
// void Glk::PairLayout::addItem(QLayoutItem* it) {
//     if(!mp_Key)
//         mp_Key = it;
//     else if(!mp_Split)
//         mp_Split = it;
// }
//
// int Glk::PairLayout::count() const {
//     int c = 0;
//
//     if(mp_Key)
//         c++;
//
//     if(mp_Split)
//         c++;
//
//     return c;
// }
//
// QLayoutItem* Glk::PairLayout::itemAt(int index) const {
//     switch(index) {
//         case 0:
//             return (mp_Key ? mp_Key : mp_Split);
//
//         case 1:
//             return (mp_Split ? mp_Split : NULL);
//
//         default:
//             return NULL;
//     }
// }
//
// QSize Glk::PairLayout::minimumSize() const {
//     return mp_Constraint->minimumSize(static_cast<Window*>(mp_Key->widget()));
// }
//
// void Glk::PairLayout::setGeometry(const QRect& rect) {
//     QLayout::setGeometry(rect);
//
//     if(!mp_Key) {
//         assert(rect.width() + rect.height() == 0);
//         mp_Split->setGeometry(rect);
//         return;
//     }
//
//     QPair<QRect, QRect> geom = mp_Constraint->geometry(rect, static_cast<Window*>(mp_Key->widget()));
//
//     mp_Key->setGeometry(geom.first);
//     mp_Split->setGeometry(geom.second);
// }
//
// QSize Glk::PairLayout::sizeHint() const {
//     return mp_Constraint->minimumSize(static_cast<Window*>(mp_Key->widget()));
// }
//
// QLayoutItem* Glk::PairLayout::takeAt(int index) {
//     QLayoutItem* it = NULL;
//
//     switch(index) {
//         case 0:
//             if(mp_Key) {
//                 it = mp_Key;
//                 mp_Key = NULL;
//             } else {
//                 it = mp_Split;
//                 mp_Split = NULL;
//             }
//
//             break;
//
//         case 1:
//             if(mp_Split) {
//                 it = mp_Split;
//                 mp_Split = NULL;
//             }
//
//             break;
//     }
//
//     return it;
// }

Glk::PairWindow::PairWindow(Glk::Window* key_, Glk::Window* split_, WindowConstraint* constraint_) : Window(new NullDevice()), mp_Key(NULL), mp_Split(NULL), mp_Constraint(constraint_) {
    mp_Constraint->setupWindows(this, key_, split_);
}

Glk::PairWindow::~PairWindow() {
    delete mp_Constraint;
}

void Glk::PairWindow::removeChildWindow(Window* ptr) { // TODO fix
    if(ptr == mp_Split) {
        Glk::PairWindow* prnt = static_cast<Glk::PairWindow*>(windowParent());

        if(prnt) {
            if(prnt->keyWindow() == this)
                prnt->mp_Key = mp_Key;
            else
                prnt->mp_Split = mp_Key;

            prnt->mp_Constraint->setupWindows(prnt, prnt->mp_Key, prnt->mp_Split);
            mp_Key->show();
        } else {
            QGlk* qglk = &QGlk::getMainWindow();
            
            qglk->setCentralWidget(mp_Key);
            mp_Key->setWindowParent(NULL);
        }

        unparent();
        mp_Split = NULL;
    } else {
        mp_Key = NULL;
        mp_Split->hide();
    }

    ptr->unparent();
}

QSize Glk::PairWindow::pixelsToUnits(const QSize& pixels) const {
    if(!mp_Key)
        return QSize();

    return QSize(mp_Key->unitWidth(pixels.width()), mp_Key->unitHeight(pixels.height()));
}

QSize Glk::PairWindow::unitsToPixels(const QSize& units) const {
    if(!mp_Key)
        return QSize();

    return QSize(mp_Key->pixelWidth(units.width()), mp_Key->pixelHeight(units.height()));
}
