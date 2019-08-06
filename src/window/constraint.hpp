#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include <QPair>
#include <QRect>

#include "glk.hpp"

#include "window/windowcontroller.hpp"
#include "pairwindowcontroller.hpp"

namespace Glk {
    class Window;

    class WindowArrangement {
            Q_DISABLE_COPY(WindowArrangement)

        public:
            enum Method : glui32 {
                Left = winmethod_Left,
                Right = winmethod_Right,
                Above = winmethod_Above,
                Below = winmethod_Below,
                Proportional = winmethod_Proportional,
                Fixed = winmethod_Fixed,
                Border = winmethod_Border,
                NoBorder = winmethod_NoBorder
            };

            static inline bool isBordered(glui32 met) {
                return (met & NoBorder) == 0;
            }

            static inline bool isProportional(glui32 met) {
                return (met & Proportional) != 0;
            }

            static inline bool isFixed(glui32 met) {
                return (met & Fixed) != 0;
            }

            static inline bool isHorizontal(glui32 met) {
                return (met & Above) == 0;
            }

            static inline bool isVertical(glui32 met) {
                return (met & Above) != 0;
            }

            [[nodiscard]] static WindowArrangement* fromMethod(glui32 met, glui32 size);

            [[nodiscard]] static QString methodString(glui32 met);

            WindowArrangement(Method method_, glui32 size_);

            virtual ~WindowArrangement() = default;

            inline Method method() const {
                return m_Method;
            }

            inline glui32 size() const {
                return m_Size;
            }

            inline bool isBordered() const {
                return isBordered(method());
            }

            inline bool isProportional() const {
                return isProportional(method());
            }

            inline bool isFixed() const {
                return isFixed(method());
            }

            inline bool isHorizontal() const {
                return isHorizontal(method());
            }

            inline bool isVertical() const {
                return isVertical(method());
            }

            virtual void setupWidgets(PairWindowController* parentController) const = 0;

        protected:
            void selectChildWindows(PairWindowController* parent, QWidget*& first, QWidget*& second) const;

            void showChildWindows(PairWindowController* parent) const;

        private:
            Method m_Method;
            glui32 m_Size;
    };

    class HorizontalWindowConstraint : public WindowArrangement {
            Q_DISABLE_COPY(HorizontalWindowConstraint)

        public:
            HorizontalWindowConstraint(Method method_, glui32 size_);

            inline bool constrainsLeft() const {
                return (method() & 1) == 0;
            }

            inline bool constrainsRight() const {
                return (method() & 1) != 0;
            }

            void setupWidgets(PairWindowController* parentController) const override;
    };

    class VerticalWindowConstraint : public WindowArrangement {
            Q_DISABLE_COPY(VerticalWindowConstraint)

        public:
            VerticalWindowConstraint(Method method_, glui32 size_);

            inline bool constrainsAbove() const {
                return WindowArrangement::isVertical(method()) && (method() & 1) == 0;
            }

            inline bool constrainsBelow() const {
                return WindowArrangement::isVertical(method()) && (method() & 1) != 0;
            }

            void setupWidgets(PairWindowController* parentController) const override;
    };
}

#endif

