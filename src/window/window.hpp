#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <QIODevice>
#include <QSet>
#include <QWidget>

#include "glk.hpp"

#include "inputprovider.hpp"
#include "windowstream.hpp"

namespace Glk {
    class PairWindow;
    class LineEventRequest;

    class Window : public Object, public QWidget {
            friend winid_t glk_window_open(winid_t, glui32, glui32, glui32, glui32);
        public:
            enum Type : glui32 {
                Blank = wintype_Blank,
                Graphics = wintype_Graphics,
                Pair = wintype_Pair,
                TextBuffer = wintype_TextBuffer,
                TextGrid = wintype_TextGrid,
            };

            virtual ~Window();

            Object::Type objectType() const override {
                return Object::Type::Window;
            }

            Glk::KeyboardInputProvider* keyboardInputProvider() {
                return mp_KIProvider;
            }
            
            inline PairWindow* windowParent() const {
                return mp_Parent;
            }
            void setWindowParent(PairWindow* prnt);
            inline void unparent() {
                setParent(NULL);
                mp_Parent = NULL;
            }
            inline Glk::WindowStream* windowStream() const {
                return mp_Stream;
            }

            virtual QSize windowSize() const;

            inline glui32 pixelWidth(glui32 units) const {
                return unitsToPixels(QSize(units, 0)).width();
            }
            inline glui32 pixelHeight(glui32 units) const {
                return unitsToPixels(QSize(0, units)).height();
            }
            inline glui32 unitWidth(glui32 pixels) const {
                return pixelsToUnits(QSize(pixels, 0)).width();
            }
            inline glui32 unitHeight(glui32 pixels) const {
                return pixelsToUnits(QSize(0, pixels)).height();
            }

            virtual Type windowType() const = 0;

            virtual void clearWindow() = 0;

        protected:
            Window(QIODevice* device_, glui32 rock_ = 0, bool acceptsCharRequest = false, bool acceptsLineRequest = false);

            void keyPressEvent(QKeyEvent* event) override;

            virtual QSize pixelsToUnits(const QSize& pixels) const = 0;
            virtual QSize unitsToPixels(const QSize& units) const = 0;

        private:
            PairWindow* mp_Parent;
            Glk::WindowStream* mp_Stream;
            Glk::KeyboardInputProvider* mp_KIProvider;
    };
}

extern QSet<Glk::Window*> s_WindowSet;

#define TO_WINID(ptr) (reinterpret_cast<winid_t>(ptr))
#define FROM_WINID(ptr) (reinterpret_cast<Glk::Window*>(ptr))

#endif
