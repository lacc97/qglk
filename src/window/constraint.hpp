#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include <QPair>
#include <QRect>

#include "glk.hpp"

#include "window/window.hpp"

namespace Glk {
    class Window;

    class WindowConstraint {
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
            WindowConstraint(Method method_, glui32 size_);
            virtual ~WindowConstraint() = default;

            inline Method method() const {
                return m_Method;
            }
            inline glui32 size() const {
                return m_Size;
            }

            inline bool isBordered() const {
                return !bool(method() & NoBorder);
            }
            inline bool isProportional() const {
                return bool(method() & Proportional);
            }
            inline bool isFixed() const {
                return bool(method() & Fixed);
            }
            inline bool isHorizontal() const {
                return !(bool(method() & Above) || bool(method() & Below));
            }
            inline bool isVertical() const {
                return bool(method() & Above) || bool(method() & Below);
            }

            virtual void setupWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* splitwin) = 0;

//             virtual QPair<QRect, QRect> geometry(const QRect& region, Window* key) = 0;
//             virtual QSize minimumSize(Window* key) = 0;

        protected:
            void setChildWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* splitwin);

        private:
            Method m_Method;
            glui32 m_Size;
    };

    class HorizontalWindowConstraint : public WindowConstraint {
        public:
            HorizontalWindowConstraint(Method method_, glui32 size_);

            inline bool constrainsLeft() const {
                return !bool(method() & WindowConstraint::Right);
            }
            inline bool constrainsRight() const {
                return bool(method() & WindowConstraint::Right);
            }

            void setupWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* splitwin) override;

//             QPair<QRect, QRect> geometry(const QRect& region, Window* key) override;
//             QSize minimumSize(Glk::Window* key) override;
    };

    class VerticalWindowConstraint : public WindowConstraint {
        public:
            VerticalWindowConstraint(Method method_, glui32 size_);

            inline bool constrainsAbove() const {
                return bool(method() & WindowConstraint::Above);
            }
            inline bool constrainsBelow() const {
                return bool(method() & WindowConstraint::Below);
            }

            void setupWindows(Glk::PairWindow* parentw, Glk::Window* keywin, Glk::Window* splitwin) override;

//             QPair<QRect, QRect> geometry(const QRect& region, Window* key) override;
//             QSize minimumSize(Glk::Window* key) override;
    };
}

#endif

