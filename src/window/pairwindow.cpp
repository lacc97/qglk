#include "pairwindow.hpp"

#include <cassert>

#include "qglk.hpp"
#include "pairwindowcontroller.hpp"
#include "window_stream_driver.hpp"

Glk::PairWindow::PairWindow(Window* key, Window* first, Window* second, WindowArrangement* winArrangement,
                            PairWindowController* winController, PairWindow* parent)
    : Window(Type::Pair, winController, std::make_unique<qglk::stream_drivers::window>(this), parent),
      mp_Key{key},
      mp_First{first},
      mp_Second{second},
      mp_Arrangement{winArrangement} {
    assert(mp_Key);
    assert(mp_First);
    assert(mp_Second);
    assert(mp_Arrangement);

    assert(mp_Key == mp_First);

    assert(!mp_First->parent() || !mp_Second->parent());

    if(mp_First->parent())
        mp_First->parent()->replaceChild(mp_First, this);
    mp_First->setParent(this);

    if(mp_Second->parent())
        mp_Second->parent()->replaceChild(mp_Second, this);
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

void Glk::PairWindow::removeChild(Glk::Window* child) {
    assert(child);

    replaceChild(child, nullptr);
}

void Glk::PairWindow::replaceChild(Glk::Window* oldChild, Glk::Window* newChild) {
    assert(oldChild == mp_First || oldChild == mp_Second);
    assert(newChild != oldChild);
    assert(newChild != this);
    assert(!newChild || !(newChild == mp_First || newChild == mp_Second));

    auto fn_is_key_invalid = [oldChild, newChild](Window* key) -> bool {
        if(newChild && newChild->windowType() == Pair) {
            if(oldChild->windowType() == Pair) {
                return static_cast<PairWindow*>(oldChild)->isDescendant(key) &&
                       !static_cast<PairWindow*>(newChild)->isDescendant(key);
            } else {
                return key == oldChild && !static_cast<PairWindow*>(newChild)->isDescendant(key);
            }
        } else if(oldChild->windowType() == Pair) {
            return key != newChild && static_cast<PairWindow*>(oldChild)->isDescendant(key);
        } else {
            return key != newChild && key == oldChild;
        }
    };


    oldChild->setParent(nullptr);
    if(oldChild == mp_First) {
        mp_First = newChild;
    } else {
        mp_Second = newChild;
    }

    for(PairWindow* ancestor = this; ancestor; ancestor = ancestor->parent()) {
        if(ancestor->keyWindow() && fn_is_key_invalid(ancestor->keyWindow()))
            ancestor->setArrangement(nullptr, ancestor->arrangement());
    }

    if(newChild) {
        if(newChild->parent() && newChild->parent() != this)
            newChild->parent()->removeChild(newChild);

        newChild->setParent(this);
    }

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

Glk::Window* Glk::PairWindow::sibling(Glk::Window* win) const {
    assert(win == mp_First || win == mp_Second);

    if(win == mp_First)
        return mp_Second;
    else
        return mp_First;
}
