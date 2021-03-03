#ifndef QGLK_CHUNK_DRIVER_HPP
#define QGLK_CHUNK_DRIVER_HPP

#include <glk.hpp>

#include "memory_driver.hpp"

namespace qglk::stream_drivers {
    class chunk : public memory<true> {
      public:
        explicit chunk(qglk::blorb::chunk ch) : memory<true>{ch.get_data()}, m_chunk{std::move(ch)} {}

      private:
        qglk::blorb::chunk m_chunk;
    };
}    // namespace qglk::stream_drivers

#endif    //QGLK_CHUNK_DRIVER_HPP
