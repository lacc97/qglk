#include "pairwindow.hpp"

#include <cassert>

#include "qglk.hpp"
#include "stream/nulldevice.hpp"
#include "pairwindowcontroller.hpp"


Glk::PairWindow::PairWindow(Window* key, Window* first, Window* second, WindowArrangement* winArrangement,
                            PairWindowController* winController, PairWindow* parent)
    : Window(Type::Pair, winController,new WindowDevice{this},parent),
      mp_Key{key},
      mp_First{first},
      mp_Second{second},
      mp_Arrangement{winArrangement} {
    assert(mp_Key);
    assert(mp_First);
    assert(mp_Second);
    assert(mp_Arrangement);

    assert(mp_Key == mp_First);

    mp_First->setParent(this);
    mp_Second->setParent(this);
}

bool Glk::PairWindow::isDescendant(Glk::Window* win) const {
    if(!win)
        return false;

    bool des = (win == mp_First || win == mp_Second);

    if(!des) {
        if(mp_First && mp_First->windowType() == Glk::Window::Pair)
            des = static_cast<Glk::PairWindow*>(mp_First)->isDescendant(win);

        if(mp_Second && mp_Second->windowType() == Glk::Window::Pair)
            des = des || static_cast<Glk::PairWindow*>(mp_Second)->isDescendant(win);
    }

    return des;
}

Glk::Window* Glk::PairWindow::removeChild(Glk::Window* deadChild) {
    assert(deadChild);
    assert(deadChild == mp_First || deadChild == mp_Second);

    Glk::Window* survivingChild = (deadChild == mp_First ? mp_Second : mp_First);

    Glk::PairWindow* ancestor = parent();

    if(deadChild->windowType() == Pair) {
        auto deadChildPairWindow = static_cast<PairWindow*>(deadChild);

        while(ancestor) {
            if(ancestor->keyWindow() && deadChildPairWindow->isDescendant(ancestor->keyWindow()))
                ancestor->setArrangement(nullptr, ancestor->arrangement());
            ancestor = ancestor->parent();
        }
    } else {
        while(ancestor) {
            if(ancestor->keyWindow() && ancestor->keyWindow() == deadChild)
                ancestor->setArrangement(nullptr, ancestor->arrangement());
            ancestor = ancestor->parent();
        }
    }

    controller()->requestSynchronization();

    survivingChild->setParent(nullptr);
    return survivingChild;
}

void Glk::PairWindow::replaceChild(Glk::Window* oldChild, Glk::Window* newChild) {
    assert(oldChild == mp_First || oldChild == mp_Second);
    assert(!newChild || !(newChild == mp_First || newChild == mp_Second));
    assert(newChild != this);
    assert(!mp_Key || oldChild != mp_First);

    oldChild->setParent(nullptr);
    if(oldChild == mp_First) {
        mp_First = newChild;
    } else {
        mp_Second = newChild;
    }

    if(newChild)
        newChild->setParent(this);

    setArrangement(mp_Key, mp_Arrangement.get());
}

void Glk::PairWindow::setArrangement(Glk::Window* key, Glk::WindowArrangement* arrange) {
    assert(!key || !mp_First || isDescendant(key));
    assert(!key || key->windowType() != Window::Pair);

    mp_Key = key;

    if(arrange != mp_Arrangement.get())
        mp_Arrangement.reset(arrange);

    // ensure mp_Key is mp_First or a descendant of mp_First
    if(mp_Key) {
        if(mp_Second && (mp_Second == mp_Key || (mp_Second->windowType() == Glk::Window::Pair &&
                                                 static_cast<PairWindow*>(mp_Second)->isDescendant(mp_Key)))) {
            auto temp = mp_First;
            mp_First = mp_Second;
            mp_Second = temp;
        }
    }

    controller()->requestSynchronization();
}
