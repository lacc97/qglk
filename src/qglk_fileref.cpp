#include "file/fileref.hpp"
#include "thread/taskrequest.hpp"

#include <QFileDialog>
#include <QSet>
#include <QTemporaryDir>

#include "qglk.hpp"

QTemporaryDir s_TempDir;
glui32 s_TempCounter = 0;

QSet<Glk::FileReference*> s_FileReferenceSet;

QString extFromUsage(glui32 usg) {
    switch(usg & 0x03) {
        case Glk::FileReference::SavedGame:
            return QStringLiteral("glksave");

        case Glk::FileReference::Data:
            return QStringLiteral("glkdata");

        default:
            return QStringLiteral("txt");
    }
}

frefid_t glk_fileref_create_temp(glui32 usage, glui32 rock) {
    if(s_TempDir.isValid()) {
        QString fname("temp" + QString::number(s_TempCounter++) + "." + extFromUsage(usage));
        QFileInfo finfo(s_TempDir.filePath(fname));

        Glk::FileReference* fref = new Glk::FileReference(finfo, usage, rock);

        return TO_FREFID(fref);
    }

    return NULL;
}

frefid_t glk_fileref_create_by_prompt(glui32 usage, glui32 fmode, glui32 rock) {
    QString ffilter;

    switch(usage & 0x3) {
        case fileusage_SavedGame:
            ffilter = QObject::tr("Save data(*.glksave)");
            break;

        case fileusage_Data:
            ffilter = QObject::tr("Data(*.glkdata)");
            break;

        case fileusage_Transcript:
        case fileusage_InputRecord:
            ffilter = QObject::tr("Text(*.txt)");
            break;
    }

    ffilter += ";;" + QObject::tr("All files(*)");

    QString fname;

    Glk::sendTaskToEventThread([&] {
        switch(fmode) {
            case filemode_Read:
                fname = QFileDialog::getOpenFileName(&QGlk::getMainWindow(), QObject::tr("Open File"), QString(), ffilter);
                break;

            case filemode_Write:
                fname = QFileDialog::getSaveFileName(&QGlk::getMainWindow(), QObject::tr("Save File"), QString(), ffilter);
                break;

            case filemode_ReadWrite:
                fname = QFileDialog::getSaveFileName(&QGlk::getMainWindow(), QObject::tr("Edit File"), QString(), ffilter);
                break;

            case filemode_WriteAppend:
                fname = QFileDialog::getSaveFileName(&QGlk::getMainWindow(), QObject::tr("Edit File"), QString(), ffilter);
                break;
        }
    });

    if(fname.isNull())
        return NULL;

    return TO_FREFID(new Glk::FileReference(QFileInfo(fname), usage, rock));
}

frefid_t glk_fileref_create_by_name(glui32 usage, char* name, glui32 rock) {
    return TO_FREFID(new Glk::FileReference(QFileInfo(name), usage, rock));
}

frefid_t glk_fileref_create_from_fileref(glui32 usage, frefid_t fref, glui32 rock) {
    return TO_FREFID(new Glk::FileReference(*FROM_FREFID(fref), usage, rock));
}

void glk_fileref_destroy(frefid_t fref) {
    delete FROM_FREFID(fref);
}

frefid_t glk_fileref_iterate(frefid_t fref, glui32* rockptr) {
    if(fref == NULL) {
        auto iter = s_FileReferenceSet.begin();

        if(iter == s_FileReferenceSet.end())
            return NULL;

        if(rockptr)
            *rockptr = (*iter)->rock();

        return TO_FREFID(*iter);
    }

    auto iter = s_FileReferenceSet.find(FROM_FREFID(fref));
    iter++;

    if(iter == s_FileReferenceSet.end())
        return NULL;

    if(rockptr)
        *rockptr = (*iter)->rock();

    return TO_FREFID(*iter);
}

glui32 glk_fileref_get_rock(frefid_t fref) {
    return FROM_FREFID(fref)->rock();
}

void glk_fileref_delete_file(frefid_t fref) {
    FROM_FREFID(fref)->remove();
}

glui32 glk_fileref_does_file_exist(frefid_t fref) {
    return FROM_FREFID(fref)->exists();
}
