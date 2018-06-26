#include "pairwindow.hpp"

#include <cassert>

#include "qglk.hpp"
#include "stream/nulldevice.hpp"

Glk::PairWindow::PairWindow(Glk::Window* key_, Glk::Window* first_, Glk::Window* second_, WindowConstraint* constraint_) : Window(new NullDevice()), mp_Key(NULL), mp_First(NULL), mp_Second(NULL), mp_Constraint(constraint_) {
    mp_Constraint->setupWindows(this, key_, first_, second_);
}

Glk::PairWindow::~PairWindow() {
    delete mp_Constraint;
}

void Glk::PairWindow::removeChildWindow(Window* ptr) {
    Q_ASSERT_X(ptr == mp_First || ptr == mp_Second, "Glk::PairWindow::removeChildWindow(Window* ptr)", "ptr must be either mp_First or mp_Second");

    Glk::PairWindow* prnt = windowParent();
    Glk::Window* sibling = (ptr == mp_First ? mp_Second : mp_First);

    if(prnt) {
        Glk::Window* prntf = prnt->mp_First;
        Glk::Window* prnts = prnt->mp_Second;

        if(ptr == mp_Second) { // this means that mp_Second is or could contain the key window of a parent window
            Glk::Window* kwin = mp_Second;

            while(kwin->windowType() == Glk::Window::Pair)
                kwin = static_cast<Glk::PairWindow*>(kwin)->mp_Second;

            Glk::PairWindow* ancestor = prnt;

            while(ancestor && ancestor->mp_Key != kwin)
                ancestor = ancestor->windowParent();

            if(ancestor != NULL)
                ancestor->mp_Constraint->setupWindows(ancestor, NULL, ancestor->mp_First, ancestor->mp_Second);
        }

        prntf = (prntf == this ? sibling : prntf);
        prnts = (prnts == this ? sibling : prnts);

        Q_ASSERT_X(sibling == prntf || sibling == prnts, "Glk::PairWindow::removeChildWindow(Window* ptr)", "this should be a child of prnt");

        prnt->mp_Constraint->setupWindows(prnt, prnt->mp_Key, prntf, prnts);
    } else {
        QGlk::getMainWindow().setRootWindow(sibling);
    }
}

QSize Glk::PairWindow::pixelsToUnits(const QSize& pixels) const {
    return pixels;
}

QSize Glk::PairWindow::unitsToPixels(const QSize& units) const {
    return units;
}
