#ifndef QGLK_POINTERWRAP_HPP
#define QGLK_POINTERWRAP_HPP

namespace wrap {
    template <class T>
    struct ptr_wrap {
        T* ptr;
    };

    template <class T>
    inline auto ptr(T* ptr) {
        return ptr_wrap<T>{ptr};
    }
}

#endif //QGLK_POINTERWRAP_HPP
