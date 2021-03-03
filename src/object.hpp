#ifndef QGLK_OBJECT_HPP
#define QGLK_OBJECT_HPP

#include <cstddef>

#include <any>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <type_traits>

#include <cppcoro/generator.hpp>

#include <spdlog/spdlog.h>

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
    class object;

    template <typename T>
    concept glk_object = std::is_base_of_v<object, T> && !std::is_same_v<object, T>;

    class object {
        friend class object_list;

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
        explicit object(glui32 rock, type type) noexcept : m_type{type}, m_rock{rock}, m_dispatch_rock{} {}
        virtual ~object() = default;

        void register_object() noexcept;
        void unregister_object() noexcept;

      private:
        type m_type;
        glui32 m_rock;
        gidispatch_rock_t m_dispatch_rock;

        std::list<object*>::iterator m_iterator;
    };

    class object_list {
      public:
        explicit object_list(object::type type) : m_type{type} {}

        void add(object* p_t);
        void remove(object* p_t);

        template <glk_object T>
        T* next(T* p_cur) const noexcept {
            auto* p_next = next_impl(p_cur);

            assert(!p_next || dynamic_cast<T*>(p_next));
            return static_cast<T*>(p_next);
        }

        template <typename T = object> requires std::is_base_of_v<object, T>
        cppcoro::generator<T*> as_range() const noexcept {
            auto it = m_list.begin();
            while(it != m_list.end()) {
                auto* p = *it;
                ++it;

                assert(dynamic_cast<T*>(p));
                co_yield static_cast<T*>(p);
            }
        }

      private:
        object* next_impl(object* p_cur) const noexcept;

        object::type m_type;
        std::list<object*> m_list;
    };
}    // namespace qglk

#endif    //QGLK_OBJECT_HPP
