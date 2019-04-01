#ifndef FILEREF_HPP
#define FILEREF_HPP

#include <QFile>
#include <QFileInfo>
#include <QSet>

#include "glk.hpp"

namespace Glk {
    class FileReference : public Object {
        Q_DISABLE_COPY(FileReference)
        
        public:
            enum Usage : glui32 {
                SavedGame = fileusage_SavedGame,
                Transcript = fileusage_Transcript,
                InputRecord = fileusage_InputRecord,
                Data = fileusage_Data,

                BinaryMode = fileusage_BinaryMode,
                TextMode = fileusage_TextMode
            };

            FileReference(const QFileInfo& fi_, glui32 usage_, glui32 rock_);
            FileReference(const FileReference& fref_, glui32 usage_, glui32 rock_);
            ~FileReference();

            Glk::Object::Type objectType() const override {
                return Object::Type::FileReference;
            }
            
            QString path();

            bool exists() const;
            void remove() const;

            QFile* file() const; // returns unowned pointer
            glui32 usage() const;

        private:
            QFileInfo m_FileInfo;
            glui32 m_Usage;
    };
}

inline const frefid_t TO_FREFID(Glk::FileReference* fref) {
    return reinterpret_cast<frefid_t>(fref);
}
inline Glk::FileReference* const FROM_FREFID(frefid_t fref) {
    return reinterpret_cast<Glk::FileReference*>(fref);
}

#endif
