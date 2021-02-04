#ifndef QGLK_OBJECT_HPP
#define QGLK_OBJECT_HPP

#include <cstddef>

#include <map>
#include <memory>
#include <sstream>
#include <type_traits>

#include <QByteArray>
#include <QMetaType>
#include <QString>

#include <glk.hpp>

namespace Glk {
    class Dispatch;

    class Object {
        Q_DISABLE_COPY(Object)

        friend class Dispatch;

      public:
        enum Type : glui32 { Window = 0, Stream = 1, FileReference = 2, SoundChannel = 3 };

        virtual ~Object() = default;


        [[nodiscard]] inline gidispatch_rock_t dispatchRock() const {
            return m_DispatchRock;
        }

        [[nodiscard]] virtual Type objectType() const = 0;

        [[nodiscard]] inline glui32 rock() const {
            return m_Rock;
        }

      protected:
        explicit Object(glui32 rock_ = 0) : m_Rock{rock_} {}

      private:
        glui32 m_Rock;
        gidispatch_rock_t m_DispatchRock;
    };
}    // namespace Glk

namespace qglk {
    class object {
        friend class ::Glk::Dispatch;

      public:
        enum type : glui32 { eWindow = 0, eStream = 1, eFileReference = 2, eSoundChannel = 3 };

        [[nodiscard]] constexpr glui32 get_rock() const noexcept {
            return m_rock;
        }
        [[nodiscard]] constexpr gidispatch_rock_t get_dispatch_rock() const noexcept {
            return m_dispatch_rock;
        }

        [[nodiscard]] constexpr type get_type() const noexcept {
            return m_type;
        }

      protected:
        explicit constexpr object(glui32 rock, type type) noexcept : m_type{type}, m_rock{rock}, m_dispatch_rock{} {}

        void init_base() noexcept;
        void destroy_base() noexcept;

      private:
        type m_type;
        glui32 m_rock;
        gidispatch_rock_t m_dispatch_rock;
    };

    template <typename T>
    concept glk_object = std::is_base_of_v<object, T>;
}    // namespace qglk

#endif    //QGLK_OBJECT_HPP
