#include "constraint.hpp"

#include <string_view>

#include <fmt/format.h>

#include <QGridLayout>

#include "pairwindow.hpp"

#define BORDER_SIZE (5)

Glk::WindowArrangement::WindowArrangement(Glk::WindowArrangement::Method method_, glui32 size_)
    : m_Method(method_),
      m_Size(size_) {}

Glk::WindowArrangement* Glk::WindowArrangement::fromMethod(glui32 met, glui32 size) {
    if(isVertical(met))
        return new VerticalWindowConstraint(static_cast<Method>(met), size);
    else
        return new HorizontalWindowConstraint(static_cast<Method>(met), size);
}

QGridLayout* Glk::WindowArrangement::setupLayout(PairWidget* parent) const {
    delete parent->layout();
    parent->clearChildren();

    auto layout = new QGridLayout(parent);
    layout->setMargin(0);

    return layout;
}

void Glk::WindowArrangement::showChildWidgets(Glk::PairWindowController* parentController) const {
    parentController->window<PairWindow>()->firstWindow()->controller()->widget()->show();
    parentController->window<PairWindow>()->secondWindow()->controller()->widget()->show();
}

Glk::HorizontalWindowConstraint::HorizontalWindowConstraint(Glk::WindowArrangement::Method method_, glui32 size_)
    : WindowArrangement(method_, size_) {}

void Glk::HorizontalWindowConstraint::setupWidgets(PairWindowController* parentController) const {
    assert(parentController);
    assert(parentController->window<PairWindow>()->firstWindow());
    assert(parentController->window<PairWindow>()->secondWindow());

    PairWidget* parent = parentController->widget<PairWidget>();
    Window* key = parentController->window<PairWindow>()->keyWindow();
    QWidget* first = parentController->window<PairWindow>()->firstWindow()->controller()->widget();
    QWidget* second = parentController->window<PairWindow>()->secondWindow()->controller()->widget();

    QGridLayout* layout = setupLayout(parent);
    if(key) {
        if(isFixed()) {
            first->setMinimumSize(key->controller()->toQtSize({static_cast<int>(size()), 0}));
            first->setSizePolicy(key->controller()->widget() == first ? QSizePolicy::Fixed : QSizePolicy::Minimum,
                                 QSizePolicy::Ignored);
        } else {
            first->setMinimumSize(0, 0);
            first->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        }

        second->setMinimumSize(0, 0);
        second->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        if(constrainsLeft()) {
            layout->addWidget(first, 0, 0);
            layout->addWidget(second, 0, 1);

            if(isProportional()) {
                layout->setColumnStretch(0, size());
                layout->setColumnStretch(1, 100 - size());
            }
        } else {
            layout->addWidget(second, 0, 0);
            layout->addWidget(first, 0, 1);

            if(isProportional()) {
                layout->setColumnStretch(0, 100 - size());
                layout->setColumnStretch(1, size());
            }
        }
    } else {
        second->setMinimumSize(0, 0);
        second->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        layout->addWidget(second);
    }

    showChildWidgets(parentController);
}

Glk::VerticalWindowConstraint::VerticalWindowConstraint(Glk::WindowArrangement::Method method_, glui32 size_)
    : WindowArrangement(method_, size_) {}

void Glk::VerticalWindowConstraint::setupWidgets(PairWindowController* parentController) const {
    assert(parentController);

    PairWidget* parent = parentController->widget<PairWidget>();
    Window* key = parentController->window<PairWindow>()->keyWindow();
    QWidget* first = parentController->window<PairWindow>()->firstWindow()->controller()->widget();
    QWidget* second = parentController->window<PairWindow>()->secondWindow()->controller()->widget();

    QGridLayout* layout = setupLayout(parent);
    if(key) {
        if(isFixed()) {
            first->setMinimumSize(key->controller()->toQtSize({0, static_cast<int>(size())}));
            first->setSizePolicy(QSizePolicy::Ignored,
                                 key->controller()->widget() == first ? QSizePolicy::Fixed : QSizePolicy::Minimum);
        } else {
            first->setMinimumSize(0, 0);
            first->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        }

        second->setMinimumSize(0, 0);
        second->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        if(constrainsAbove()) {
            layout->addWidget(first, 0, 0);
            layout->addWidget(second, 1, 0);

            if(isProportional()) {
                layout->setRowStretch(0, size());
                layout->setRowStretch(1, 100 - size());
            }
        } else {
            layout->addWidget(second, 0, 0);
            layout->addWidget(first, 1, 0);

            if(isProportional()) {
                layout->setRowStretch(0, 100 - size());
                layout->setRowStretch(1, size());
            }
        }
    } else {
        second->setMinimumSize(0, 0);
        second->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        layout->addWidget(second);
    }

    showChildWidgets(parentController);
}

