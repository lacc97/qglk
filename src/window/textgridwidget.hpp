#ifndef TEXTGRIDWIDGET_HPP
#define TEXTGRIDWIDGET_HPP

#include <vector>

#include <QLabel>

#include "glk.hpp"

#include "windowwidget.hpp"

//#include <fmt/format.h>
//
//namespace fmt {
//    template <>
//    struct formatter<QSizePolicy::Policy> : formatter<std::string_view> {
//        auto format(QSizePolicy::Policy e, format_context& ctx) {
//            using namespace std::string_view_literals;
//
//            auto s = "<unknown>"sv;
//            switch(e) {
//                case QSizePolicy::Fixed:
//                    s = "Fixed"sv;
//                    break;
//                case QSizePolicy::Minimum:
//                    s = "Minimum"sv;
//                    break;
//                case QSizePolicy::Maximum:
//                    s = "Maximum"sv;
//                    break;
//                case QSizePolicy::Preferred:
//                    s = "Preferred"sv;
//                    break;
//                case QSizePolicy::MinimumExpanding:
//                    s = "Minimum Expanding"sv;
//                    break;
//                case QSizePolicy::Expanding:
//                    s = "Expanding"sv;
//                    break;
//                case QSizePolicy::Ignored:
//                    s = "Ignored"sv;
//                    break;
//            }
//            return formatter<std::string_view>::format(s, ctx);
//        }
//    };
//
//    template<>
//    struct formatter<QWidget> {
//        auto parse(format_parse_context &ctx) {
//            return ctx.begin();
//        }
//
//        auto format(const QWidget &w, format_context &ctx) {
//            return format_to(ctx.out(),
//                             "{{\n"
//                             "  - size:         {0:^4} × {1:^4}\n"
//                             "  - minimum size: {2:^4} × {3:^4}\n"
//                             "  - size policy:  {4} × {5}\n"
//                             "}}",
//                             w.size().width(), w.size().height(), w.minimumSize().width(), w.minimumSize().height(),
//                             w.sizePolicy().horizontalPolicy(), w.sizePolicy().verticalPolicy());
//        }
//    };
//}

namespace Glk {
    class TextGridWidget : public WindowWidget {
        Q_OBJECT
        public:
            TextGridWidget();

            void setGrid(const std::vector<std::vector<glui32>>& newGrid);

            [[nodiscard]] inline int charHeight() const {
                return mp_Label->fontMetrics().height();
            }

            [[nodiscard]] inline int charWidth() const {
                return mp_Label->fontMetrics().boundingRect('m').width();
            }

//            [[nodiscard]] inline int horizontalMargins() const {
//                return contentsMargins().left() + contentsMargins().right();
//            }
//
//            [[nodiscard]] inline int verticalMargins() const {
//                return contentsMargins().top() + contentsMargins().bottom();
//            }

        signals:
            void resized();

        protected:
            void resizeEvent(QResizeEvent* event) override;

        private:
            QLabel* mp_Label;
    };
}


#endif //TEXTGRIDWIDGET_HPP
