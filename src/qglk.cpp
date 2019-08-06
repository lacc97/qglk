#include "qglk.hpp"
#include "ui_qglk.h"

#include <QResizeEvent>
#include <QThread>
#include <QThreadPool>

#include "glk.hpp"

extern "C" {
#include "glkstart.h"
}

#include "exception.hpp"

#include "window/pairwindow.hpp"

#define ex_Void (0)
#define ex_Int (1)
#define ex_Bool (2)

static int errflag = FALSE;

// Following two functions taken from glkterm-1.0.4
//
static int string_to_bool(char* str) {
    if(!strcmp(str, "y") || !strcmp(str, "yes"))
        return TRUE;

    if(!strcmp(str, "n") || !strcmp(str, "no"))
        return FALSE;

    if(!strcmp(str, "on"))
        return TRUE;

    if(!strcmp(str, "off"))
        return FALSE;

    if(!strcmp(str, "+"))
        return TRUE;

    if(!strcmp(str, "-"))
        return FALSE;

    return -1;
}

static int extract_value(int argc, char* argv[], char* optname, int type,
                         int* argnum, int* result, int defval) {
    int optlen, val;
    char* cx, * origcx, firstch;

    optlen = strlen(optname);
    origcx = argv[*argnum];
    cx = origcx;

    firstch = *cx;
    cx++;

    if(strncmp(cx, optname, optlen))
        return FALSE;

    cx += optlen;

    switch(type) {

        case ex_Void:
            if(*cx)
                return FALSE;

            *result = TRUE;
            return TRUE;

        case ex_Int:
            if(*cx == '\0') {
                if((*argnum) + 1 >= argc) {
                    cx = "";
                } else {
                    (*argnum) += 1;
                    cx = argv[*argnum];
                }
            }

            val = atoi(cx);

            if(val == 0 && cx[0] != '0') {
                printf("%s: %s must be followed by a number\n",
                       argv[0], origcx);
                errflag = TRUE;
                return FALSE;
            }

            *result = val;
            return TRUE;

        case ex_Bool:
            if(*cx == '\0') {
                if((*argnum) + 1 >= argc) {
                    val = -1;
                } else {
                    char* cx2 = argv[(*argnum) + 1];
                    val = string_to_bool(cx2);

                    if(val != -1)
                        (*argnum) += 1;
                }
            } else {
                val = string_to_bool(cx);

                if(val == -1) {
                    printf("%s: %s must be followed by a boolean value\n",
                           argv[0], origcx);
                    errflag = TRUE;
                    return FALSE;
                }
            }

            if(val == -1)
                val = !defval;

            *result = val;
            return TRUE;

    }

    return FALSE;
}

Glk::Runnable::Runnable(int argc_, char** argv_)
    : argc(argc_),
      argv(argv_) {
}

void Glk::Runnable::run() {
    mp_Thread = QThread::currentThread();

    // Code taken from glkterm-1.0.4
    //
    int ix, jx, val;
    glkunix_startup_t startdata;

    /* Test for compile-time errors. If one of these spouts off, you
        must edit glk.h and recompile. */
    if(sizeof(glui32) != 4) {
        printf("Compile-time error: glui32 is not a 32-bit value. Please fix glk.h.\n");
        return;
    }

    if((glui32) (-1) < 0) {
        printf("Compile-time error: glui32 is not unsigned. Please fix glk.h.\n");
        return;
    }

    /* Now some argument-parsing. This is probably going to hurt. */
    startdata.argc = 0;
    startdata.argv = (char**) malloc(argc * sizeof(char*));

    /* Copy in the program name. */
    startdata.argv[startdata.argc] = argv[0];
    startdata.argc++;

    for(ix = 1; ix < argc && !errflag; ix++) {
        glkunix_argumentlist_t* argform;
        int inarglist = FALSE;
        char* cx;

        for(argform = glkunix_arguments;
            argform->argtype != glkunix_arg_End && !errflag;
            argform++) {

            if(argform->name[0] == '\0') {
                if(argv[ix][0] != '-') {
                    startdata.argv[startdata.argc] = argv[ix];
                    startdata.argc++;
                    inarglist = TRUE;
                }
            } else if((argform->argtype == glkunix_arg_NumberValue)
                      && !strncmp(argv[ix], argform->name, strlen(argform->name))
                      && (cx = argv[ix] + strlen(argform->name))
                      && (atoi(cx) != 0 || cx[0] == '0')) {
                startdata.argv[startdata.argc] = argv[ix];
                startdata.argc++;
                inarglist = TRUE;
            } else if(!strcmp(argv[ix], argform->name)) {
                int numeat = 0;

                if(argform->argtype == glkunix_arg_ValueFollows) {
                    if(ix + 1 >= argc) {
                        printf("%s: %s must be followed by a value\n",
                               argv[0], argform->name);
                        errflag = TRUE;
                        break;
                    }

                    numeat = 2;
                } else if(argform->argtype == glkunix_arg_NoValue) {
                    numeat = 1;
                } else if(argform->argtype == glkunix_arg_ValueCanFollow) {
                    if(ix + 1 < argc && argv[ix + 1][0] != '-') {
                        numeat = 2;
                    } else {
                        numeat = 1;
                    }
                } else if(argform->argtype == glkunix_arg_NumberValue) {
                    if(ix + 1 >= argc
                       || (atoi(argv[ix + 1]) == 0 && argv[ix + 1][0] != '0')) {
                        printf("%s: %s must be followed by a number\n",
                               argv[0], argform->name);
                        errflag = TRUE;
                        break;
                    }

                    numeat = 2;
                } else {
                    errflag = TRUE;
                    break;
                }

                for(jx = 0; jx < numeat; jx++) {
                    startdata.argv[startdata.argc] = argv[ix];
                    startdata.argc++;

                    if(jx + 1 < numeat)
                        ix++;
                }

                inarglist = TRUE;
                break;
            }
        }

        if(inarglist || errflag)
            continue;

        if(argv[ix][0] != '-') {
            printf("%s: unwanted argument: %s\n", argv[0], argv[ix]);
            errflag = TRUE;
            break;
        }
    }

    if(errflag) {
        printf("usage: %s [ options ... ]\n", argv[0]);

        if(glkunix_arguments[0].argtype != glkunix_arg_End) {
            glkunix_argumentlist_t* argform;
            printf("game options:\n");

            for(argform = glkunix_arguments;
                argform->argtype != glkunix_arg_End;
                argform++) {
                if(strlen(argform->name) == 0)
                    printf("  %s\n", argform->desc);
                else if(argform->argtype == glkunix_arg_ValueFollows)
                    printf("  %s val: %s\n", argform->name, argform->desc);
                else if(argform->argtype == glkunix_arg_NumberValue)
                    printf("  %s val: %s\n", argform->name, argform->desc);
                else if(argform->argtype == glkunix_arg_ValueCanFollow)
                    printf("  %s [val]: %s\n", argform->name, argform->desc);
                else
                    printf("  %s: %s\n", argform->name, argform->desc);
            }
        }

        printf("NUM values can be any number. BOOL values can be 'yes' or 'no', or no value to toggle.\n");
        return;
    }

    try {
        if(!glkunix_startup_code(&startdata)) {
            return;
        }

        glk_main();
    } catch(Glk::ExitException& ex) {
        if(ex.interrupted() && bool(QGlk::getMainWindow().interruptHandler()))
            QGlk::getMainWindow().interruptHandler()();
    }

    return;
}

QGlk::QGlk(int argc, char** argv)
    : QMainWindow(),
      mp_UI{new Ui::QGlk},
      mp_Runnable{new Glk::Runnable(argc, argv)},
      mp_RootWindow{nullptr},
      m_DeleteQueue{},
      m_EventQueue{},
      m_WindowList{},
      m_StreamList{},
      m_FileReferenceList{},
      m_SoundChannelList{},
      m_InterruptHandler{},
      m_DefaultStyles{},
      m_TextBufferStyles{} {
    setMinimumSize(800, 600);
    mp_UI->setupUi(this);

    mp_Runnable->setAutoDelete(true);

    QObject::connect(&m_EventQueue, &Glk::EventQueue::canSynchronize,
                     this, &QGlk::synchronize,
                     Qt::BlockingQueuedConnection);
}

QGlk::~QGlk() {
    // windows contain their own window streams so we first delete windows, then streams
    while(!m_WindowList.empty())
        delete m_WindowList.first();

    while(!m_StreamList.empty())
        delete m_StreamList.first();

    while(!m_FileReferenceList.empty())
        delete m_FileReferenceList.first();

    while(!m_SoundChannelList.empty())
        delete m_SoundChannelList.first();

    delete mp_UI;
}

void QGlk::addToDeleteQueue(Glk::WindowController* winController) {
    m_DeleteQueue.push_back(winController);
}

void QGlk::run() {
    QThreadPool::globalInstance()->start(mp_Runnable);
}

bool QGlk::event(QEvent* event) {
    if(event->type() == Glk::TaskEvent::Type)
        return handleGlkTask(static_cast<Glk::TaskEvent*>(event));
    else
        return QMainWindow::event(event);
}

void QGlk::synchronize() {


    while(!m_DeleteQueue.empty()) {
        delete m_DeleteQueue.front();
        m_DeleteQueue.pop_front();
    }

    if(!mp_RootWindow) {
        if(centralWidget() != nullptr)
            setCentralWidget(nullptr);
    } else if(centralWidget() != mp_RootWindow->controller()->widget()) {
        setCentralWidget(mp_RootWindow->controller()->widget());
        mp_RootWindow->controller()->widget()->show();

        mp_RootWindow->controller()->synchronize();
    } else {
        auto fn_recursive_synchronize = [](auto&& this_fn, Glk::WindowController* win) mutable -> void {
            if(win->window()->windowType() == Glk::Window::Pair && !win->requiresSynchronization()) {
                this_fn(this_fn, win->window<Glk::PairWindow>()->firstWindow()->controller());
                this_fn(this_fn, win->window<Glk::PairWindow>()->secondWindow()->controller());
            } else if(win->requiresSynchronization()) {
                win->synchronize();
            }
        };

        fn_recursive_synchronize(fn_recursive_synchronize, mp_RootWindow->controller());
    }
}

void QGlk::closeEvent(QCloseEvent* event) {
    m_EventQueue.interrupt();
    event->accept();
}

void QGlk::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    eventQueue().push(event_t{evtype_Arrange, NULL, 0, 0});
}

bool QGlk::handleGlkTask(Glk::TaskEvent* event) {
    event->execute();
    return event->handled();
}

