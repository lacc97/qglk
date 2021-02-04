#ifndef QGLK_CHUNK_DRIVER_HPP
#define QGLK_CHUNK_DRIVER_HPP

#include <glk.hpp>

#include "memory_driver.hpp"

namespace qglk::stream_drivers {
    class chunk : public memory<true> {
      public:
        explicit chunk(Glk::Blorb::Chunk ch) : memory<true>{std::span(ch.data(), ch.length())}, m_chunk{std::move(ch)} {}

      private:
        Glk::Blorb::Chunk m_chunk;
    };
}    // namespace qglk::stream_drivers

#endif    //QGLK_CHUNK_DRIVER_HPP
