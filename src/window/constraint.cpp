#include "constraint.hpp"

#include <QDebug>
#include <QGridLayout>

#include "pairwindow.hpp"

#define BORDER_SIZE (5)

Glk::WindowConstraint::WindowConstraint(Glk::WindowConstraint::Method method_, glui32 size_) : m_Method(method_), m_Size(size_) {}

void Glk::WindowConstraint::setChildWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* firstwin, Glk::Window* secondwin) const {
    parentw->mp_Key = keywin;

    if(parentw->mp_First && parentw->mp_First->windowParent() == parentw)
        parentw->mp_First->orphan();

    parentw->mp_First = firstwin;
    parentw->mp_First->setWindowParent(parentw);
    parentw->mp_First->show();


    if(parentw->mp_Second && parentw->mp_Second->windowParent() == parentw)
        parentw->mp_Second->orphan();

    parentw->mp_Second = secondwin;
    parentw->mp_Second->setWindowParent(parentw);
    parentw->mp_Second->show();
}

Glk::HorizontalWindowConstraint::HorizontalWindowConstraint(Glk::WindowConstraint::Method method_, glui32 size_) : WindowConstraint(method_, size_) {}

void Glk::HorizontalWindowConstraint::setupWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* firstwin, Glk::Window* secondwin) const {
    Q_ASSERT_X(parentw, "Glk::WindowConstraint::setupWindows", "parent must not be NULL");
    Q_ASSERT_X(firstwin && secondwin, "Glk::WindowConstraint::setChildWindows", "children must not be NULL");
    (keywin ? Q_ASSERT_X(keywin->windowType() != Glk::Window::Pair, "Glk::WindowConstraint::setChildWindows", "keywin must not be a pair window") : static_cast<void>(0));
    
    firstwin->orphan();
    secondwin->orphan();
    
    if(parentw->layout())
        delete parentw->layout();

    QGridLayout* layout = new QGridLayout(parentw);
    layout->setMargin(0);

    if(keywin) {
        if(isFixed()) {
            firstwin->setMinimumSize(keywin->pixelWidth(size()), 0);
            firstwin->setSizePolicy((keywin == firstwin ? QSizePolicy::Fixed : QSizePolicy::Minimum), QSizePolicy::Ignored);
        } else {
            firstwin->setMinimumSize(0, 0);
            firstwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        }

        secondwin->setMinimumSize(0, 0);
        secondwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        if(constrainsLeft()) {
            layout->addWidget(firstwin, 0, 0);
            layout->addWidget(secondwin, 0, 1);

            if(isProportional()) {
                layout->setColumnStretch(0, size());
                layout->setColumnStretch(1, 100 - size());
            }
        } else {
            layout->addWidget(secondwin, 0, 0);
            layout->addWidget(firstwin, 0, 1);

            if(isProportional()) {
                layout->setColumnStretch(0, 100 - size());
                layout->setColumnStretch(1, size());
            }
        }
    } else {
        secondwin->setMinimumSize(0, 0);
        secondwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        layout->addWidget(secondwin);
    }

    setChildWindows(parentw, keywin, firstwin, secondwin);
    
    QDebug debug = qDebug();
    debug << (parentw) << "horizontal layout: [";
    for(int ii = 0; ii < parentw->layout()->count(); ii++)
        debug << "\n{" << parentw->layout()->itemAt(ii)->widget() << "," << parentw->layout()->itemAt(ii)->geometry() << "}";
    debug << "\n]";
}

Glk::VerticalWindowConstraint::VerticalWindowConstraint(Glk::WindowConstraint::Method method_, glui32 size_) : WindowConstraint(method_, size_) {}

void Glk::VerticalWindowConstraint::setupWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* firstwin, Glk::Window* secondwin) const {
    Q_ASSERT_X(parentw, "Glk::WindowConstraint::setupWindows", "parent must not be NULL");
    Q_ASSERT_X(firstwin && secondwin, "Glk::WindowConstraint::setChildWindows", "children must not be NULL");
    (keywin ? Q_ASSERT_X(keywin->windowType() != Glk::Window::Pair, "Glk::WindowConstraint::setChildWindows", "keywin must not be a pair window") : static_cast<void>(0));
    
    firstwin->orphan();
    secondwin->orphan();
    
    if(parentw->layout())
        delete parentw->layout();

    QGridLayout* layout = new QGridLayout(parentw);
//     layout->setMargin(0);

    if(keywin) {
        if(isFixed()) {
            firstwin->setMinimumSize(0, keywin->pixelHeight(size()));
            firstwin->setSizePolicy(QSizePolicy::Ignored, (keywin == firstwin ? QSizePolicy::Fixed : QSizePolicy::Minimum));
        } else {
            firstwin->setMinimumSize(0, 0);
            firstwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        }

        secondwin->setMinimumSize(0, 0);
        secondwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        if(constrainsAbove()) {
            layout->addWidget(firstwin, 0, 0);
            layout->addWidget(secondwin, 1, 0);

            if(isProportional()) {
                layout->setRowStretch(0, size());
                layout->setRowStretch(1, 100 - size());
            }
        } else {
            layout->addWidget(secondwin, 0, 0);
            layout->addWidget(firstwin, 1, 0);

            if(isProportional()) {
                layout->setRowStretch(0, 100 - size());
                layout->setRowStretch(1, size());
            }
        }
    } else {
        secondwin->setMinimumSize(0, 0);
        secondwin->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        layout->addWidget(secondwin);
    }

    setChildWindows(parentw, keywin, firstwin, secondwin);
    
    QDebug debug = qDebug();
    debug << (parentw) << "vertical layout: [";
    for(int ii = 0; ii < parentw->layout()->count(); ii++)
        debug << "\n{" << parentw->layout()->itemAt(ii)->widget() << "," << parentw->layout()->itemAt(ii)->geometry() << "}";
    debug << "\n]";
}

