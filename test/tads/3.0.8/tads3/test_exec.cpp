/* 
 *   Copyright 1999, 2002 Michael J. Roberts
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
 *   T3 Image File Test - Load and Execute 
 */

#include <stdlib.h>

#include "os.h"
#include "t3std.h"
#include "vmmain.h"
#include "vmconsol.h"
#include "t3test.h"
#include "vmhostsi.h"


/*
 *   Client services interface 
 */
class MyClientIfc: public CVmMainClientIfc
{
public:
    /* set plain ASCII mode */
    void set_plain_mode() { os_plain(); }

    /* create the main console */
    CVmConsoleMain *create_console(struct vm_globals *)
    {
        return new CVmConsoleMain();
    }

    /* delete the console */
    void delete_console(struct vm_globals *vmg, CVmConsoleMain *con)
    {
        VMGLOB_PTR(vmg);

        /* flush any pending buffered output */
        con->flush(vmg_ VM_NL_NONE);

        /* delete the output formatter */
        delete con;
    }

    /* initialize */
    void client_init(struct vm_globals *,
                     const char *,
                     const char *,
                     const char *,
                     const char *)
    {
        /* do nothing */
    }

    /* terminate */
    void client_terminate(struct vm_globals *) { }

    /* pre-execution initialization */
    void pre_exec(struct vm_globals *globals)
    {
        VMGLOB_PTR(globals);
        
        /*
         *   Turn off MORE mode in the display formatter, since this host
         *   environment is meant primarily for automated testing.  
         */
        G_console->set_more_state(FALSE);
    }

    /* post-execution termination/error termination */
    void post_exec(struct vm_globals *) { }
    void post_exec_err(struct vm_globals *) { }

    /* display an error */
    void display_error(struct vm_globals *, const char *msg,
                       int add_blank_line)
    {
        /* display the error on the stdio console */
        printf("%s\n", msg);

        /* add a blank line if desired */
        if (add_blank_line)
            printf("\n");
    }
};

/*
 *   Main program entrypoint 
 */
int main(int argc, char **argv)
{
    int stat;
    MyClientIfc clientifc;
    CVmHostIfc *hostifc = new CVmHostIfcStdio(argv[0]);

    /* initialize for testing */
    test_init();

    /* 
     *   Initialize the OS layer.  Since this is a command-line-only
     *   implementation, there's no need to ask the OS layer to try to get
     *   us a filename to run, so pass in null for the prompt and filename
     *   buffer.  
     */
    os_init(&argc, argv, 0, 0, 0);

    /* run the image */
    stat = vm_run_image_main(&clientifc, "test_exec", argc, argv,
                             TRUE, TRUE, hostifc);

    /* uninitialize the OS layer */
    os_uninit();

    /* done with the host interface */
    delete hostifc;

    /* show any unfreed memory */
    t3_list_memory_blocks(0);

    /* done */
    return stat;
}

