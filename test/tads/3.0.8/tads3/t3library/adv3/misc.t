#charset "us-ascii"

/* Copyright (c) 2000, 2002 Michael J. Roberts.  All Rights Reserved. */
/*
 *   TADS 3 Library - miscellaneous definitions
 *   
 *   This module contains miscellaneous definitions that don't have a
 *   natural grouping with any larger modules, and which aren't complex
 *   enough to justify modules of their own.  
 */

/* include the library header */
#include "adv3.h"


/* ------------------------------------------------------------------------ */
/*
 *   When a call is made to a property not defined or inherited by the
 *   target object, the system will automatically invoke this method.  The
 *   method will be invoked with a property pointer as its first argument,
 *   and the original arguments as the remaining arguments.  The first
 *   argument gives the property that was invoked and not defined by the
 *   object.  A typical definition in an object would look like this:
 *   
 *   propNotDefined(prop, [args]) { ... }
 *   
 *   If this method is not defined by the object, the system simply
 *   returns nil as the value of the undefined property evaluation or
 *   method invocation.
 */
property propNotDefined;
export propNotDefined;


/* ------------------------------------------------------------------------ */
/*
 *   The library base class for the gameMain object.
 *   
 *   Each game MUST define an object called 'gameMain' to define how the
 *   game starts up.  You can use GameMainDef as the base class of your
 *   'gameMain' object, in which case the only thing you're required to
 *   specify in your object is the 'initialPlayerChar' property - you can
 *   inherit everything else from the GameMainDef class if you don't
 *   require any further customizations.  
 */
class GameMainDef: object
    /*
     *   The initial player character.  Each game's 'gameMain' object MUST
     *   define this to refer to the Actor object that serves as the
     *   initial player character. 
     */
    initialPlayerChar = nil

    /*
     *   Show the game's introduction.  This routine is called by the
     *   default newGame() just before entering the main command loop.  The
     *   command loop starts off by showing the initial room description,
     *   so there's no need to do that here.
     *   
     *   Most games will want to override this, to show a prologue message
     *   setting up the game's initial situation for the player.  We don't
     *   show anything by default.  
     */
    showIntro() { }

    /*
     *   Show the "goodbye" message.  This is called after the main command
     *   loop terminates.
     *   
     *   We don't show anything by default.  If you want to show a "thanks
     *   for playing" type of message as the game exits, override this
     *   routine with the desired text.  
     */
    showGoodbye() { }

    /*
     *   Begin a new game.  This default implementation shows the
     *   introductory message, calls the main command loop, and finally
     *   shows the goodbye message.
     *   
     *   You can override this routine if you want to customize the startup
     *   protocol.  For example, if you want to create a pre-game options
     *   menu, you could override this routine to show the list of options
     *   and process the user's input.  If you need only to customize the
     *   introduction and goodbye messages, you can simply override
     *   showIntro() and showGoodbye() instead.  
     */
    newGame()
    {
        /* 
         *   Show the statusline before we display our introductory.  This
         *   will help minimize redrawing - if we waited until after
         *   displaying some text, we might have to redraw some of the
         *   screen to rearrange things for the new screen area taken up by
         *   the status line, which could be visible to the user.  By
         *   setting up the status line first, we'll probably have less to
         *   redraw because we won't have anything on the screen yet when
         *   figuring the layout.  
         */
        statusLine.showStatusLine();

        /* show the introduction */
        showIntro();

        /* run the game, showing the initial location's full description */
        runGame(true);

        /* show the end-of-game message */
        showGoodbye();
    }

    /*
     *   Restore a game and start it running.  This is invoked when the
     *   user launches the interpreter using a saved game file; for
     *   example, on a Macintosh, this happens when the user double-clicks
     *   on a saved game file on the desktop.
     *   
     *   This default implementation bypasses any normal introduction
     *   messages: we simply restore the game file if possible, and
     *   immediately start the game's main command loop.  Most games won't
     *   need to override this, but you can if you need some special effect
     *   in the restore-at-startup case.  
     */
    restoreAndRunGame(filename)
    {
        local succ;

        /* mention that we're about to restore the saved position */
        libMessages.noteMainRestore();

        /* try restoring it */
        succ = RestoreAction.startupRestore(filename);

        /* show a blank line after the restore result message */
        "<.p>";

        /* if we were successful, run the game */
        if (succ)
        {
            /* 
             *   Run the command loop.  There's no need to show the room
             *   description, since the RESTORE action will have already
             *   done so. 
             */
            runGame(nil);

            /* show the end-of-game message */
            showGoodbye();
        }
    }

    /*
     *   Set the interpreter window title, if applicable to the local
     *   platform.  This simply displays a <TITLE> tag to set the title to
     *   the string found in the versionInfo object.  
     */
    setGameTitle()
    {
        /* write the <TITLE> tag with the game's name */
        "<title><<versionInfo.name>></title>";
    }

    /*
     *   Set up the HTML-mode about-box.  By default, this does nothing.
     *   Games can use this routine to show an <ABOUTBOX> tag, if desired,
     *   to set up the contents of an about-box for HTML TADS platforms.
     *   
     *   Note that an <ABOUTBOX> tag must be re-initialized each time the
     *   main game window is cleared, so this routine should be called
     *   again after any call to clearScreen().  
     */
    setAboutBox()
    {
        /* we don't show any about-box by default */
    }

    /*
     *   The gameMain object also specifies some settings that control
     *   optional library behavior.  If you want the standard library
     *   behavior, you can just inherit the default settings from this
     *   class.  Some games might want to select non-default variations,
     *   though.  
     */

    /* 
     *   The maximum number of points possible in the game.  If the game
     *   includes the scoring module at all, and this is non-nil, the SCORE
     *   and FULL SCORE commands will display this value to the player as a
     *   rough indication of how much farther there is to go in the game.
     *   
     *   By default, we initialize this on demand, by calculating the sum
     *   of the point values of the Achievement objects in the game.  The
     *   game can override this if needed to specify a specific maximum
     *   possible score, rather than relying on the automatic calculation.
     */
    maxScore()
    {
        local m;
        
        /* ask the score module (if any) to compute the maximum score */
        m = (libGlobal.scoreObj != nil
             ? libGlobal.scoreObj.calcMaxScore : nil);

        /* supersede this initializer with the calculated value */
        maxScore = m;

        /* return the result */
        return m;
    }

    /*
     *   The score ranking list - this provides a list of names for
     *   various score levels.  If the game provides a non-nil list here,
     *   the SCORE and FULL SCORE commands will show the rank along with
     *   the score ("This makes you a Master Adventurer").
     *   
     *   This is a list of score entries.  Each score entry is itself a
     *   list of two elements: the first element is the minimum score for
     *   the rank, and the second is a string describing the rank.  The
     *   ranks should be given in ascending order, since we simply search
     *   the list for the first item whose minimum score is greater than
     *   our score, and use the preceding item.  The first entry in the
     *   list would normally have a minimum of zero points, since it
     *   should give the initial, lowest rank.
     *   
     *   If this is set to nil, which it is by default, we'll simply skip
     *   score ranks entirely.  
     */
    scoreRankTable = nil

    /* 
     *   Verbose mode.  If this is true, the full room description is
     *   displayed each time the player enters a room, regardless of
     *   whether or not the player has seen the room before; if this is
     *   nil, the full description is only displayed on the player's first
     *   entry to a room, and only the short description on re-entry.
     *   Note that the library provides VERBOSE and TERSE commands that
     *   let the player change this setting dynamically.  
     */
    verboseMode = true

    /* 
     *   Flag: show automatic exit listings in the status line.  We enable
     *   this by default.  
     *   
     *   This is an author-configured option.  The library doesn't provide
     *   a command to let the player control this setting (although a game
     *   could certainly add one, if desired).  
     */
    showExitsInStatusline = true

    /*
     *   Option flag: allow the player to use "you" and "me"
     *   interchangeably in referring to the player character.  We set
     *   this true by defalt, so that the player can refer to the player
     *   character in either the first or second person, as long as the
     *   player character normally uses either or these (in other words,
     *   this option is meaningless in a game when the narration refers to
     *   the player character in the third person).
     *   
     *   If desired, the game can set this flag to nil to force the player
     *   to use the correct pronoun to refer to the player character.
     *   
     *   We set the default to allow using "you" and "me" interchangeably
     *   because this will create no confusion in most games, and because
     *   most experienced IF players will be accustomed to using "me" to
     *   refer to the player character (because the majority of IF refers
     *   to the player character in the second person, and expects the
     *   player to conflate the player character with the player and hence
     *   to refer to the player character in the first person).  It is
     *   relatively unconventional for a game to refer to the player
     *   character in the first person in the narration, and thus to
     *   expect the player to use the second person to refer to the PC; as
     *   a result, experience players might tend to use the first person
     *   out of habit in such games, and might find it jarring to find the
     *   usage unacceptable.  Furthermore, in games that use a
     *   first-person narration, it seems unlikely that there will also be
     *   a second-person element to the narration; as long as both aren't
     *   present, it will cause no confusion for the game to accept either
     *   "you" or "me" as equivalent in commands.  However, the library
     *   provides this option in case such as situation does arise.  
     */
    allowYouMeMixing = true

    /*
     *   Option flag: filter plural phrase matches exclude the most obvious
     *   illogicalities, such as trying to TAKE an object that's already
     *   being held, or trying to OPEN an object that's already open.
     *   
     *   This is set to true by default, which means that we exclude an
     *   object from matching a plural phrase when the object's "verify"
     *   routine for the verb has an "illogical-already" or an
     *   "illogical-self" result.
     *   
     *   If you would prefer that plural words are simply matched to
     *   everything present that matches the vocabulary, without any
     *   filtering at all, override this and set it to nil.  
     */
    filterPluralMatches = true

    /*
     *   Option flag: allow ALL to be used for every verb.  This is true by
     *   default, which means that players will be allowed to use ALL with
     *   any command - OPEN ALL, EXAMINE ALL, etc.
     *   
     *   Some authors don't like to allow players to use ALL with so many
     *   verbs, because they think it's a sort of "cheating" when players
     *   try things like OPEN ALL.  This option lets you disable ALL for
     *   most verbs; if you set this to nil, only the basic inventory
     *   management verbs (TAKE, TAKE FROM, DROP, PUT IN, PUT ON) will
     *   allow ALL, and other verbs will simply respond with an error
     *   ("'All' isn't allowed with that verb").
     *   
     *   If you're writing an especially puzzle-oriented game, you might
     *   want to set this to nil.  It's a trade-off though, as some people
     *   will think your game is less player-friendly if you disable ALL.  
     */
    allVerbsAllowAll = true
;

/* ------------------------------------------------------------------------ */
/*
 *   Clear the main game window.  In most cases, you should call this
 *   rather than calling the low-level clearScreen() function directly,
 *   since this routine takes care of a couple of chores that should
 *   usually be done at the same time.
 *   
 *   First, we flush the transcript to ensure that no left-over reports
 *   that were displayed before we cleared the screen will show up on the
 *   new screen.  Second, we call the low-level clearScreen() function to
 *   actually clear the display window.  Finally, we re-display any
 *   <ABOUTBOX> tag, to ensure that the about-box will still be around;
 *   this is necessary because any existing <ABOUTBOX> tag is lost after
 *   the screen is cleared.  
 */
cls()
{
    /* flush any captured transcript output */
    if (gTranscript != nil)
        gTranscript.flushForInput();

    /* clear the screen */
    clearScreen();

    /* re-initialize any <ABOUTBOX> tag */
    gameMain.setAboutBox();
}

/* ------------------------------------------------------------------------ */
/*
 *   Run the game.  We start by showing the description of the initial
 *   location, if desired, and then we read and interpret commands until
 *   the game ends (via a "quit" command, winning, death of the player
 *   character, or any other way of terminating the game).
 *   
 *   This routine doesn't return until the game ends.
 *   
 *   Before calling this routine, the caller should already have set the
 *   global variable gPlayerChar to the player character actor.
 *   
 *   'look' is a flag indicating whether or not to look around; if this is
 *   true, we'll show a full description of the player character's initial
 *   location, as though the player were to type "look around" as the first
 *   command.  
 */
runGame(look)
{
    /* show the starting location */
    if (look)
    {
        /* run the initial "look around" in a dummy command context */
        withActionEnv(EventAction, gPlayerChar,
                      {: gPlayerChar.lookAround(true) });
    }

    /* run the scheduling loop until the game ends */
    runScheduler();
}

/* ------------------------------------------------------------------------ */
/*
 *   Main program entrypoint.  The core run-time start-up code calls this
 *   after running pre-initialization and load-time initialization.  This
 *   entrypoint is called when we're starting the game normally; when the
 *   game is launched through a saved-position file, mainRestore() will be
 *   invoked instead.  
 */
main(args)
{
    /* initialize the display */
    initDisplay();

    /* call the gameMain object to set up a new game */
    gameMain.newGame();
}

/*
 *   Initialize the display.  We call this when we first enter the
 *   interpreter to set up the main game window.  
 */
initDisplay()
{
    /* set the interpreter window title */
    gameMain.setGameTitle();

    /* set up the ABOUT box */
    gameMain.setAboutBox();
}

/*
 *   Main program entrypoint for restoring a saved-position file.  This is
 *   invoked from the core run-time start-up code when the game is launched
 *   from the operating system via a saved-position file.  For example, on
 *   Windows, double-clicking on a saved-position file on the Windows
 *   desktop launches the interpreter, which looks in the save file to find
 *   the game executable to run, then starts the game and invokes this
 *   entrypoint.  
 */
mainRestore(args, restoreFile)
{
    /* initialize the display */
    initDisplay();

    /* call the gameMain object to restore the specified saved game */
    gameMain.restoreAndRunGame(restoreFile);
}

/* ------------------------------------------------------------------------ */
/*
 *   Determine if the given object overrides the definition of the given
 *   property inherited from the given base class.  Returns true if the
 *   object derives from the given base class, and the object's definition
 *   of the property comes from a different place than the base class's
 *   definition of the property.  
 */
overrides(obj, base, prop)
{
    return (obj.ofKind(base)
            && (obj.propDefined(prop, PropDefGetClass)
                != base.propDefined(prop, PropDefGetClass)));
}

/* ------------------------------------------------------------------------ */
/*
 *   Library Pre-Initializer.  This object performs the following
 *   initialization operations immediately after compilation is completed:
 *   
 *   - adds each defined Thing to its container's contents list
 *   
 *   - adds each defined Sense to the global sense list
 *   
 *   This object is named so that other libraries and/or user code can
 *   create initialization order dependencies upon it.  
 */
adv3LibPreinit: PreinitObject
    execute()
    {
        /* set the initial player character, as specified in gameMain */
        gPlayerChar = gameMain.initialPlayerChar;
        
        /* 
         *   visit every VocabObject, and run its vocabulary initializer
         *   (this routine will be defined in the language-specific part
         *   of the library to enter each object's vocabulary words into
         *   the dictionary) 
         */
        forEachInstance(VocabObject, { obj: obj.initializeVocab() });

        /* visit every Thing, and run its general initializer */
        forEachInstance(Thing, { obj: obj.initializeThing() });

        /* initialize SpecialTopic objects */
        forEachInstance(SpecialTopic, { obj: obj.initializeSpecialTopic() });

        /* 
         *   Initialize each MultiInstance object.  Do this after
         *   initializing the Thing objects, because we'll be dynamically
         *   constructing new Thing objects for the instances.  Those new
         *   Things will be initialized by their constructors, so we don't
         *   want to initialize them redundantly with explicit
         *   initializeThing calls.  
         */
        forEachInstance(MultiInstance, { obj: obj.initializeLocation() });

        /* add every Sense to the global sense list */
        forEachInstance(Sense, { obj: libGlobal.allSenses += obj });

        /* 
         *   initialize each ActorState - do this before initializing
         *   actors, since we want each actor's initial state to plug
         *   itself into its actor before we initialize the actors 
         */
        forEachInstance(ActorState, { obj: obj.initializeActorState() });

        /* initialize each Actor */
        forEachInstance(Actor, { obj: obj.initializeActor() });

        /* 
         *   initialize the AltTopics first, to set up their parents' lists
         *   of their AltTopic children 
         */
        forEachInstance(AltTopic, { obj: obj.initializeAltTopic() });

        /* initialize the topic database entries */
        forEachInstance(TopicEntry, { obj: obj.initializeTopicEntry() });

        /* initialize the suggested topics */
        forEachInstance(SuggestedTopic,
            { obj: obj.initializeSuggestedTopic() });

        /* initialize the master direction list */
        Direction.initializeDirectionClass();

        /* initialize the noise/odor notification daemon */
        new Daemon(SensoryEmanation, &noteSenseChanges, 1);

        /* set up a daemon for the current location */
        new Daemon(BasicLocation, &dispatchRoomDaemon, 1);

        /* 
         *   Initialize the status line daemon.  Set this daemon's event
         *   order to a high value so that it runs last, after all other
         *   daemons - we want this to be the last prompt daemon so that
         *   the status line is updated after any other daemons have done
         *   their jobs already, in case any of them move the player
         *   character to a new location or affect the score, or make any
         *   other changes that should be reflected on the status line.  
         */
        local sld = new PromptDaemon(statusLine, &showStatusLineDaemon);
        sld.eventOrder = 1000;

        /* 
         *   Attach the command sequencer output filter, the
         *   language-specific message parameter substitution filter, the
         *   style tag formatter filter, and the paragraph filter to the
         *   main output stream.  Stack them so that the paragraph manager
         *   is at the bottom, since the library tag filter can produce
         *   paragraph tags and thus needs to sit atop the paragraph
         *   filter.  Put the command sequencer above those, since it
         *   might need to write style tags.  Finally, put the sense
         *   context filter on top of those.  
         */
        mainOutputStream.addOutputFilter(typographicalOutputFilter);
        mainOutputStream.addOutputFilter(mainParagraphManager);
        mainOutputStream.addOutputFilter(styleTagFilter);
        mainOutputStream.addOutputFilter(langMessageBuilder);
        mainOutputStream.addOutputFilter(commandSequencer);
        mainOutputStream.addOutputFilter(conversationManager);
        mainOutputStream.addOutputFilter(senseContext);

        /* 
         *   Attach our message parameter filter and style tag filter to
         *   the status line streams.  We don't need most of the main
         *   window's filters in the status line.  
         */
        statusTagOutputStream.addOutputFilter(styleTagFilter);
        statusTagOutputStream.addOutputFilter(langMessageBuilder);

        statusLeftOutputStream.addOutputFilter(styleTagFilter);
        statusLeftOutputStream.addOutputFilter(langMessageBuilder);

        statusRightOutputStream.addOutputFilter(styleTagFilter);
        statusRightOutputStream.addOutputFilter(langMessageBuilder);
    }

    /* 
     *   Make sure the output streams we depend on are initialized before
     *   me (so that they set up properly internally).  Also, make sure
     *   that the message builder object (langMessageBuilder) is set up
     *   first, so that we can add entries to its parameter substitution
     *   table.  
     */
    execBeforeMe = [mainOutputStream, statusTagOutputStream,
                    statusLeftOutputStream, statusRightOutputStream,
                    langMessageBuilder]
;

/* ------------------------------------------------------------------------ */
/*
 *   Library Initializer.  This object performs the following
 *   initialization operations each time the game is started:
 *   
 *   - sets up the library's default output function 
 */
adv3LibInit: InitObject
    execute()
    {
        /* 
         *   Set up our default output function.  Note that we must do
         *   this during run-time initialization each time we start the
         *   game, rather than during pre-initialization, because the
         *   default output function state is not part of the load-image
         *   configuration. 
         */
        t3SetSay(say);
    }
;


/* ------------------------------------------------------------------------ */
/*
 *   Generic script object.  This class can be used to implement a simple
 *   state machine.  
 */
class Script: object
    /* 
     *   Get the current state.  This returns a value that gives the
     *   current state of the script, which is usually simply an integer.  
     */
    getScriptState()
    {
        /* by default, return our state property */
        return curScriptState;
    }

    /*
     *   Process the next step of the script.  This routine must be
     *   overridden to perform the action of the script.  This routine's
     *   action should call getScriptState() to get our current state, and
     *   should update the internal state appropriately to take us to the
     *   next step after the current one.
     *   
     *   By default, we don't do anything at all.  
     */
    doScript()
    {
        /* override to carry out the script */
    }

    /* 
     *   Property giving our current state.  This should never be used
     *   directly; instead, getScriptState() should always be used, since
     *   getScriptState() can be overridden so that the state depends on
     *   something other than this internal state property. The meaning of
     *   the state identifier is specific to each subclass.  
     */
    curScriptState = 0
;

/* ------------------------------------------------------------------------ */
/*
 *   An "event list."  This is a general-purpose type of script that lets
 *   you define the scripted events separately from the Script object.
 *   
 *   The script is driven by a list of values; each value represents one
 *   step of the script.  Each value can be a single-quoted string, in
 *   which case the string is simply displayed; a function pointer, in
 *   which case the function is invoked without arguments; another Script
 *   object, in which case the object's doScript() method is invoked; a
 *   property pointer, in which case the property of 'self' (the EventList
 *   object) is invoked with no arguments; or nil, in which case nothing
 *   happens.
 *   
 *   This base type of event list runs through the list once, in order, and
 *   then simply stops doing anything once we pass the last event.  
 */
class EventList: Script
    construct(lst) { eventList = lst; }

    /* the list of events */
    eventList = []

    /* advance to the next state */
    advanceState()
    {
        /* increment our state index */
        ++curScriptState;
    }

    /* by default, start at the first list element */
    curScriptState = 1

    /* process the next step of the script */
    doScript()
    {
        local idx;
        
        /* get our current event state */
        idx = getScriptState();

        /* if it's a valid index in our list, fire the event */
        if (idx >= 1 && idx <= eventList.length())
        {
            /* carry out the event */
            doScriptEvent(eventList[idx]);
        }

        /* perform any end-of-script processing */
        scriptDone();
    }

    /* carry out one script event */
    doScriptEvent(evt)
    {
        /* check what kind of event we have */
        switch (dataTypeXlat(evt))
        {
        case TypeSString:
            /* it's a string - display it */
            say(evt);
            break;
            
        case TypeObject:
            /* it must be a Script object - invoke its doScript() method */
            evt.doScript();
            break;
            
        case TypeFuncPtr:
            /* it's a function pointer - invoke it */
            (evt)();
            break;
            
        case TypeProp:
            /* it's a property of self - invoke it */
            self.(evt)();
            break;
            
        default:
            /* do nothing in other cases */
            break;
        }
    }

    /*
     *   Perform any end-of-script processing.  By default, we advance the
     *   script to the next state.
     *   
     *   Some scripts might want to override this.  For example, a script
     *   could be driven entirely by some external timing; the state of a
     *   script could vary once per turn, for example, or could change each
     *   time an actor pushes a button.  In these cases, invoking the
     *   script wouldn't affect the state of the event list, so the
     *   subclass would override scriptDone() so that it does nothing at
     *   all.  
     */
    scriptDone()
    {
        /* advance to the next state */
        advanceState();
    }
;

/*
 *   An "external" event list is one whose state is driven externally to
 *   the script.  Specifically, the state is *not* advanced by invoking the
 *   script; the state is advanced exclusively by some external process
 *   (for example, by a daemon that invokes the event list's advanceState()
 *   method).  
 */
class ExternalEventList: EventList
    scriptDone() { }
;

/*
 *   A cyclical event list - this runs through the event list in order,
 *   returning to the first element when we pass the last element.  
 */
class CyclicEventList: EventList
    advanceState()
    {
        /* go to the next state */
        ++curScriptState;

        /* if we've passed the end of the list, loop back to the start */
        if (curScriptState > eventList.length())
            curScriptState = 1;
    }
;

/*
 *   A stopping event list - this runs through the event list in order,
 *   then stops at the last item and repeats it each time the script is
 *   subsequently invoked. 
 *   
 *   This is often useful for things like ASK ABOUT topics, where we reveal
 *   more information when asked repeatedly about a topic, but eventually
 *   reach a point where we've said everything:
 *   
 *.  >ask bob about black book
 *.  "What makes you think I know anything about it?" he says, his
 *   voice shaking.
 *   
 *   >again
 *.  "No! You can't make me tell you!"
 *   
 *   >again
 *.  "All right, I'll tell you what you want to know!  But I warn you,
 *   these are things mortal men were never meant to know.  Your life, your
 *   very soul will be in danger from the moment you hear these dark secrets!"
 *   
 *   >again
 *.  [scene missing]
 *   
 *   >again
 *.  "I've already told you all I know."
 *   
 *   >again
 *.  "I've already told you all I know."
 */
class StopEventList: EventList
    advanceState()
    {
        /* if we haven't yet reached the last state, go to the next one */
        if (curScriptState < eventList.length())
            ++curScriptState;
    }
;

/*
 *   A synchronized event list.  This is an event list that takes its
 *   actions from a separate event list object.  We get our current state
 *   from the other list, and advancing our state advances the other list's
 *   state in lock step.  Set 'masterObject' to refer to the master list
 *   whose state we synchronize with.  
 *   
 *   This can be useful, for example, when we have messages that reflect
 *   two different points of view on the same events: the messages for each
 *   point of view can be kept in a separate list, but the one list can be
 *   a slave of the other to ensure that the two lists are based on a
 *   common state.  
 */
class SyncEventList: EventList
    /* my master event list object */
    masterObject = nil

    /* my state is simply the master list's state */
    getScriptState() { return masterObject.getScriptState(); }

    /* to advance my state, advance the master list's state */
    advanceState() { masterObject.advanceState(); }
;

/*
 *   Randomized event list.  This is similar to a regular event list, but
 *   chooses an event at random each time it's invoked.  In addition, you
 *   can adjust the frequency at which events fire at all, so that an event
 *   only occurs some percentage of the time that the event list is
 *   invoked.  
 */
class RandomEventList: EventList
    /* percentage of the time an event occurs */
    eventPercent = 100

    /* 
     *   Random atmospheric events can get repetitive after a while, so we
     *   provide an easy way to reduce the frequency of our events after a
     *   while.  This way, we'll generate the events more frequently at
     *   first, but once the player has seen them enough to get the idea,
     *   we'll cut back.  Sometimes, the player will spend a lot of time in
     *   one place trying to solve a puzzle, so the same set of random
     *   events can get stale.  Set eventReduceAfter to the number of times
     *   you want the events to be generated at full frequency; after we've
     *   fired events that many times, we'll change eventPercent to
     *   eventReduceTo.  If eventReduceAfter is nil, we won't ever change
     *   eventPercent.  
     */
    eventReduceAfter = nil
    eventReduceTo = nil

    /* process the next step of the script */
    doScript()
    {
        local idx;

        /* check the odds to see if we want to fire an event at all */
        if (!checkEventOdds())
            return;
        
        /* get our next random number */
        idx = getNextRandom();
        
        /* run the event, if the index is valid */
        if (idx >= 1 && idx <= eventList.length())
            doScriptEvent(eventList[idx]);
    }
    
    /*
     *   Get the next random state.  By default, we simply return a number
     *   from 1 to the number of entries in our event list.  This is a
     *   separate method to allow subclasses to customize the way the
     *   random number is selected.  
     */
    getNextRandom()
    {
        /*   
         *   Note that rand(n) returns a number from 0 to n-1 inclusive;
         *   since list indices run from 1 to list.length, add one to the
         *   result of rand(list.length) to get a value in the proper range
         *   for a list index.  
         */
        return rand(eventList.length()) + 1;
    }

    /*
     *   Check the event odds to see if we want to fire an event at all on
     *   this invocation.  
     */
    checkEventOdds()
    {
        /* 
         *   check the event odds to see if we fire an event this time; if
         *   not, we're done with the script invocation 
         */
        if (rand(100) >= eventPercent)
            return nil;

        /* 
         *   we're firing an event this time, so count this against the
         *   reduction limit, if there is one 
         */
        if (eventReduceAfter != nil)
        {
            /* decrement the limit counter */
            --eventReduceAfter;
            
            /* if it has reached zero, apply the reduced frequency */
            if (eventReduceAfter == 0)
            {
                /* apply the reduced frequency */
                eventPercent = eventReduceTo;
                
                /* we no longer have a limit to look for */
                eventReduceAfter = nil;
            }
        }

        /* indicate that we do want to fire an event */
        return true;
    }
;

/*
 *   Shuffled event list.  This is just like a random event list, except
 *   that we fire our events in a "shuffled" order rather than an
 *   independently random order.  "Shuffled order" means that we fire the
 *   events in random order, but we don't re-fire an event until we've run
 *   through all of the other events.  The effect is as though we were
 *   dealing from a deck of cards.
 *   
 *   For the first time through the main list, we normally shuffle the
 *   strings immediately at startup, but this is optional.  If shuffleFirst
 *   is set to nil, we will NOT shuffle the list the first time through -
 *   we'll run through it once in the given order, then shuffle for the
 *   next time through, then shuffle again for the next, and so on.  So, if
 *   you want a specific order for the first time through, just define the
 *   list in the desired order and set shuffleFirst to nil.
 *   
 *   You can optionally specify a separate list of one-time-only sequential
 *   strings in the property firstEvents.  We'll run through these strings
 *   once.  When we've exhausted them, we'll switch to the main eventList
 *   list, showing it one time through in its given order, then shuffling
 *   it and running through it again, and so on.  The firstEvents list is
 *   never shuffled - it's always shown in exactly the order given.  
 */
class ShuffledEventList: RandomEventList
    /* 
     *   a list of events to go through sequentially, in the exact order
     *   specified, before firing any events from the main list
     */
    firstEvents = []

    /*
     *   Flag: shuffle the eventList list before we show it for the first
     *   time.  By default, this is set to true, so that the behavior is
     *   random on each independent run of the game.  However, it might be
     *   desirable in some cases to always use the original ordering of the
     *   eventList list the first time through the list.  If this is set to
     *   nil, we won't shuffle the list the first time through.  
     */
    shuffleFirst = true

    /* process the next step of the script */
    doScript()
    {
        local evt;
        local firstLen = firstEvents.length();
        local eventLen = eventList.length();

        /* check the odds to see if we want to fire an event at all */
        if (!checkEventOdds())
            return;

        /* 
         *   States 1..N, where N is the number of elements in the
         *   firstEvents list, simply show the firstEvents elements in
         *   order.
         *   
         *   If we're set to shuffle the main eventList list initially, all
         *   states above N simply show elements from the eventList list in
         *   shuffled order.
         *   
         *   If we're NOT set to shuffle the main eventList list initially,
         *   the following apply:
         *   
         *   States N+1..N+M, where M is the number of elements in the
         *   eventList list, show the eventList elements in order.
         *   
         *   States above N+M show elements from the eventList list in
         *   shuffled order.  
         */
        if (curScriptState <= firstLen)
        {
            /* simply fetch the next string from firstEvents */
            evt = firstEvents[curScriptState++];
        }
        else if (!shuffleFirst && curScriptState <= firstLen + eventLen)
        {
            /* fetch the next string from eventList */
            evt = eventList[curScriptState++ - firstLen];
        }
        else
        {
            /* we're showing shuffled strings from the eventList list */
            evt = eventList[getNextRandom()];
        }

        /* execute the event */
        doScriptEvent(evt);
    }


    /*
     *   Get the next random event.  We'll pick an event from our list of
     *   events using a ShuffledIntegerList to ensure we pick each value
     *   once before re-using any values.  
     */
    getNextRandom()
    {
        /* if we haven't created our shuffled list yet, do so now */
        if (shuffledList_ == nil)
        {
            /* 
             *   create a shuffled integer list - we'll use these shuffled
             *   integers as indices into our event list 
             */
            shuffledList_ = new ShuffledIntegerList(1, eventList.length());

            /* 
             *   tell the list to avoid re-using the same element twice in
             *   a row on shuffling boundaries 
             */
            shuffledList_.suppressRepeats = true;
        }

        /* ask the shuffled list to pick an element */
        return shuffledList_.getNextValue();
    }

    /* our ShuffledList - we'll initialize this on demand */
    shuffledList_ = nil
;

/* ------------------------------------------------------------------------ */
/*
 *   Shuffled List - this class keeps a list of values that can be returned
 *   in random order, but with the constraint that we never repeat a value
 *   until we've handed out every value.  Think of a shuffled deck of
 *   cards: the order of the cards handed out is random, but once a card is
 *   dealt, it can't be dealt again until we put everything back into the
 *   deck and reshuffle.  
 */
class ShuffledList: object
    /* 
     *   the list of values we want to shuffle - initialize this in each
     *   instance to the set of values we want to return in random order 
     */
    valueList = []

    /*
     *   Flag: suppress repeated values.  We mostly suppress repeats by
     *   our very design, since we run through the entire list before
     *   repeating anything in the list.  However, there's one situation
     *   (in a list with more than one element) where a repeat can occur:
     *   immediately after a shuffle, we could select the last element
     *   from the previous shuffle as the first element of the new
     *   shuffle.  If this flag is set, we'll suppress this type of repeat
     *   by choosing again any time we're about to choose a repeat.  
     */
    suppressRepeats = nil

    /* create from a given list */
    construct(lst)
    {
        /* remember our list of values */
        valueList = lst;
    }

    /* 
     *   Get a random value.  This will return a randomly-selected element
     *   from 'valueList', but we'll return every element of 'valueList'
     *   once before repeating any element.
     *   
     *   If we've returned every value on the current round, we'll
     *   automatically shuffle the values and start a new round.  
     */
    getNextValue()
    {
        local i;
        local ret;
        local justReshuffled = nil;

        /* if we haven't initialized our vector, do so now */
        if (valuesVec == nil)
        {
            /* create the vector */
            valuesVec = new Vector(valueList.length(), valueList);

            /* all values are initially available */
            valuesAvail = valuesVec.length();
        }

        /* if we've exhausted our values on this round, start over */
        if (valuesAvail == 0)
        {
            /* shuffle the elements */
            reshuffle();

            /* note that we just did a shuffle */
            justReshuffled = true;
        }

        /* pick a random element from the 'available' partition */
        i = rand(valuesAvail) + 1;

        /*
         *   If we just reshuffled, and we're configured to suppress a
         *   repeat immediately after a reshuffle, and we chose the first
         *   element of the vector, and we have at least two elements,
         *   choose a different element.  The first element in the vector
         *   is always the last element we return from each run-through,
         *   since the 'available' partition is at the start of the list
         *   and thus shrinks down until it contains only the first
         *   element. 
         */
        if (justReshuffled && suppressRepeats && valuesAvail > 1)
        {
            /* 
             *   we don't want repeats, so choose anything besides the
             *   first element; keep choosing until we get another element 
             */
            while (i == 1)
                i = rand(valuesAvail) + 1;
        }

        /* remember the element we're returning */
        ret = valuesVec[i];

        /*
         *   Move the value at the top of the 'available' partition down
         *   into the hole we're creating at 'i', since we're about to
         *   reduce the size of the 'available' partition to reflect the
         *   use of one more value; that would leave the element at the top
         *   of the partition homeless, so we need somewhere to put it.
         *   Luckily, we also need to delete element 'i', since we're using
         *   this element.  Solve both problems at once by moving element
         *   we're rendering homeless into the hole we're creating.  
         */
        valuesVec[i] = valuesVec[valuesAvail];

        /* move the value we're returning into the top slot */
        valuesVec[valuesAvail] = ret;

        /* reduce the 'available' partition by one */
        --valuesAvail;

        /* return the result */
        return ret;
    }

    /*
     *   Shuffle the values.  This puts all of the values back into the
     *   deck (as it were) for a new round.  It's never required to call
     *   this, because getNextValue() automatically shuffles the deck and
     *   starts over each time it runs through the entire deck.  This is
     *   provided in case the caller has a reason to want to put all the
     *   values back into play immediately, before every value has been
     *   dealt on the current round.  
     */
    reshuffle()
    {
        /* 
         *   Simply reset the counter of available values.  Go with the
         *   original source list's length, in case we haven't initialized
         *   our internal vector yet. 
         */
        valuesAvail = valueList.length();
    }

    /*
     *   Internal vector of available/used values.  Elements from 1 to
     *   'valuesAvail', inclusive, are still available for use on this
     *   round.  Elements above 'valuesAvail' have already been used.  
     */
    valuesVec = nil
    
    /* number of values still available on this round */ 
    valuesAvail = 0
;

/*
 *   A Shuffled Integer List is a special kind of Shuffled List that
 *   returns integers in a given range.  Like an ordinary Shuffled List,
 *   we'll return integers in the given range in random order, but we'll
 *   only return each integer once during a given round; when we exhaust
 *   the supply, we'll reshuffle the set of integers and start over.  
 */
class ShuffledIntegerList: ShuffledList
    /* 
     *   The minimum and maximum values for our range.  Instances should
     *   define these to the range desired. 
     */
    rangeMin = 1
    rangeMax = 10

    /* initialize the value list on demand */
    valueList = nil

    /* construct with the given range */
    construct(rmin, rmax)
    {
        rangeMin = rmin;
        rangeMax = rmax;
    }

    /* get the next value */
    getNextValue()
    {
        /* if we haven't set up our value list yet, do so now */
        if (valueList == nil)
        {
            local cnt;
            local i;
            
            /* 
             *   Set up a vector with the required number of elements.  We
             *   have to add one to the difference between our max and min
             *   values, because we want the set to be inclusive of the
             *   endpoints. 
             */
            cnt = rangeMax - rangeMin + 1;
            valueList = new Vector(cnt);

            /* put a nil value in each slot */
            valueList.fillValue(nil, 1, cnt);

            /* now populate the slots with the integers from our range */
            i = rangeMin;
            valueList.applyAll({x: i++});
        }

        /* use the inherited handling to select from our value list */
        return inherited();
    }
;


/* ------------------------------------------------------------------------ */
/*
 *   Library global variables 
 */
libGlobal: object
    /* 
     *   Sense cache - we keep SenseInfo lists here, keyed by [pov,sense];
     *   we normally discard the cached information at the start of each
     *   turn, and disable caching entirely at the start of the "action"
     *   phase of each turn.  We leave caching disabled during each turn's
     *   action phase because this is the phase where simulation state
     *   changes are typically made, and hence it would be difficult to
     *   keep the cache coherent during this phase.
     *   
     *   When this is nil, it indicates that caching is disabled.  We only
     *   allow caching during certain phases of execution, when game state
     *   is not conventionally altered, so that we don't have to do a lot
     *   of work to keep the cache up to date.  
     */
    senseCache = nil

    /*
     *   Can-Touch cache - we keep CanTouchInfo entires here, keyed by
     *   [from,to].  This cache is the touch-path equivalent of the sense
     *   cache, and is enabled and disabled 
     */
    canTouchCache = nil

    /* 
     *   Connection list cache - this is a cache of all of the objects
     *   connected by containment to a given object. 
     */
    connectionCache = nil

    /* 
     *   Actor visual ambient cache - this keeps track of the ambient light
     *   level at the given actor. 
     */
    actorVisualAmbientCache = nil

    /* enable the cache, clearing any old cached information */
    enableSenseCache()
    {
        /* create a new, empty lookup table for the sense cache */
        senseCache = new LookupTable(32, 64);

        /* create the can-touch cache */
        canTouchCache = new LookupTable(32, 64);

        /* create the actor visual ambient cache */
        actorVisualAmbientCache = new LookupTable(32, 64);

        /* create a connection list cache */
        connectionCache = new LookupTable(32, 64);
    }

    /* disable the cache */
    disableSenseCache()
    {
        /* forget the cache tables */
        senseCache = nil;
        canTouchCache = nil;
        actorVisualAmbientCache = nil;
        connectionCache = nil;
    }

    /* 
     *   Invalidate the sense cache.  This can be called if something
     *   happens during noun resolution or verification that causes any
     *   cached sense information to become out of date.  For example, if
     *   you have to create a new game-world object during noun-phrase
     *   resolution, this should be called to ensure that the new object's
     *   visibility is properly calculated and incorporated into the cached
     *   information.  
     */
    invalSenseCache()
    {
        /* remember whether or not caching is currently enabled */
        local wasEnabled = (senseCache != nil);

        /* clear the cache by disabling it */
        disableSenseCache();

        /* if the cache was previously enabled, re-enable it */
        if (wasEnabled)
            enableSenseCache();
    }

    /*
     *   List of all of the senses.  The library pre-initializer will load
     *   this list with a reference to each instance of class Sense. 
     */
    allSenses = []

    /*
     *   The current player character 
     */
    playerChar = nil

    /*
     *   The current perspective object.  This is usually the actor
     *   performing the current command. 
     */
    pointOfView = nil

    /*
     *   The stack of point of view objects.  The last element of the
     *   vector is the most recent point of view after the current point
     *   of view.  
     */
    povStack = static new Vector(32)

    /* 
     *   The global score object.  We use a global for this, rather than
     *   referencing libScore directly, to allow the score module to be
     *   left out entirely if the game doesn't make use of scoring.  The
     *   score module should set this during pre-initialization.  
     */
    scoreObj = nil

    /* 
     *   The global Footnote class object.  We use a global for this,
     *   rather than referencing Footnote directly, to allow the footnote
     *   module to be left out entirely if the game doesn't make use of
     *   footnotes.  The footnote class should set this during
     *   pre-initialization.  
     */
    footnoteClass = nil

    /* the total number of turns so far */
    totalTurns = 0

    /* 
     *   flag: the parser is in 'debug' mode, in which it displays the
     *   parse tree for each command entered 
     */
    parserDebugMode = nil

    /*
     *   Most recent command, for 'undo' purposes.  This is the last
     *   command the player character performed, or the last initial
     *   command a player directed to an NPC.
     *   
     *   Note that if the player directed a series of commands to an NPC
     *   with a single command line, only the first command on such a
     *   command line is retained here, because it is only the first such
     *   command that counts as a player's turn in terms of the game
     *   clock.  Subsequent commands are executed by the NPC's on the
     *   NPC's own time, and do not count against the PC's game clock
     *   time.  The first command counts against the PC's clock because of
     *   the time it takes the PC to give the command to the NPC.  
     */
    lastCommandForUndo = ''

    /* 
     *   Most recent target actor phrase; this goes with
     *   lastCommandForUndo.  This is nil if the last command did not
     *   specify an actor (i.e., was implicitly for the player character),
     *   otherwise is the string the player typed specifying a target
     *   actor.  
     */
    lastActorForUndo = ''

    /*
     *   Current command information.  We keep track of the current
     *   command's actor and action here, as well as the verification
     *   result list and the command report list.  
     */
    curActor = nil
    curIssuingActor = nil
    curAction = nil
    curVerifyResults = nil

    /* the exitLister object, if included in the build */
    exitListerObj = nil

    /* the hint manager, if included in the build */
    hintManagerObj = nil
;

/* ------------------------------------------------------------------------ */
/*
 *   FinishType objects are used in finishGameMsg() to indicate what kind
 *   of game-over message to display.  We provide a couple of standard
 *   objects for the most common cases. 
 */
class FinishType: object
    /* the finishing message, as a string or library message property */
    finishMsg = nil
;

/* 'death' - the game has ended due to the player character's demise */
ftDeath: FinishType finishMsg = &finishDeathMsg;

/* 'victory' - the player has won the game */
ftVictory: FinishType finishMsg = &finishVictoryMsg;

/* 'failure' - the game has ended in failure (but not necessarily death) */
ftFailure: FinishType finishMsg = &finishFailureMsg;

/* 'game over' - the game has simply ended */
ftGameOver: FinishType finishMsg = &finishGameOverMsg;

/*
 *   Finish the game, showing a message explaining why the game has ended.
 *   This can be called when an event occurs that ends the game, such as
 *   the player character's death, winning, or any other endpoint in the
 *   story.
 *   
 *   We'll show a message defined by 'msg', using a standard format.  The
 *   format depends on the language, but in English, it's usually the
 *   message surrounded by asterisks: "*** You have won! ***".  'msg' can
 *   be:
 *   
 *.    - nil, in which case we display nothing
 *.    - a string, which we'll display as the message
 *.    - a FinishType object, from which we'll get the message
 *   
 *   After showing the message (if any), we'll prompt the user with
 *   options for how to proceed.  We'll always show the QUIT, RESTART, and
 *   RESTORE options; other options can be offered by listing one or more
 *   FinishOption objects in the 'extra' parameter, which is given as a
 *   list of FinishOption objects.  The library defines a few non-default
 *   finish options, such as finishOptionUndo and finishOptionCredits; in
 *   addition, the game can subclass FinishOption to create its own custom
 *   options, as desired.  
 */
finishGameMsg(msg, extra)
{
    local lst;

    /* translate the message, if specified */
    if (dataType(msg) == TypeObject)
    {
        /* it's a FinishType object - get its message property or string */
        msg = msg.finishMsg;

        /* if it's a library message property, look it up */
        if (dataType(msg) == TypeProp)
            msg = libMessages.(msg);
    }

    /* if we have a message, display it */
    if (msg != nil)
        libMessages.showFinishMsg(msg);

    /* if the extra options include a scoring option, show the score */
    if (extra != nil && extra.indexWhich({x: x.showScoreInFinish}) != nil)
    {
        "<.p>";
        libGlobal.scoreObj.showScore();
        "<.p>";
    }

    /*
     *   Since we need to interact directly with the player, any sense
     *   context currently in effect is now irrelevant.  Reset the sense
     *   context by setting the 'source' object to nil to indicate that we
     *   don't need any sense blocking at all.  We can just set the context
     *   directly, since this routine will never return into the
     *   surrounding command processing - we always either terminate the
     *   program or proceed to a different game context (via undo, restore,
     *   restart, etc).  By the same token, the actor we're talking to now
     *   is the player character.  
     */
    senseContext.setSenseContext(nil, sight);
    gActor = gPlayerChar;

    /* start with the standard options */
    lst = [finishOptionRestore, finishOptionRestart];

    /* add any additional options in the 'extra' parameter */
    if (extra != nil)
        lst += extra;

    /* always add 'quit' as the last option */
    lst += finishOptionQuit;

    /* process the options */
    processOptions(lst);
}

/* finish the game, offering the given extra options but no message */
finishGame(extra)
{
    finishGameMsg(nil, extra);
}

/*
 *   Show failed startup restore options.  If a restore operation fails at
 *   startup, we won't just proceed with the game, but ask the user what
 *   they want to do; we'll offer the options of restoring another game,
 *   quitting, or starting the game from the beginning.  
 */
failedRestoreOptions()
{
    /* process our set of options */
    processOptions([restoreOptionRestoreAnother, restoreOptionStartOver,
                    finishOptionQuit]);
}

/*
 *   Process a list of finishing options.  We'll loop, showing prompts and
 *   reading responses, until we get a response that terminates the loop.  
 */
processOptions(lst)
{
    /* keep going until we get a valid response */
promptLoop:
    for (;;)
    {
        local resp;
        
        /* show the options */
        finishOptionsLister.showListAll(lst, 0, 0);

        /* switch to before-command mode for reading the interactive input */
        "<.commandbefore>";

        /* 
         *   update the status line, in case the score or turn counter has
         *   changed (this is especially likely when we first enter this
         *   loop, since we might have just finished the game with our
         *   previous action, and that action might well have awarded us
         *   some points) 
         */
        statusLine.showStatusLine();

        /* read a response */
        resp = inputManager.getInputLine(nil, nil);

        /* switch to command-after mode */
        "<.commandafter>";

        /* check for a match to each of the options in our list */
        foreach (local cur in lst)
        {
            /* if this one matches, process the option */
            if (cur.responseMatches(resp))
            {
                /* it matches - carry out the option */
                if (cur.doOption())
                {
                    /* 
                     *   they returned true - they want to continue asking
                     *   for more options
                     */
                    continue promptLoop;
                }
                else
                {
                    /* 
                     *   they returned nil - they want us to stop asking
                     *   for options and return to our caller 
                     */
                    return;
                }
            }
        }

        /*
         *   If we got this far, it means that we didn't get a valid
         *   option.  Display our "invalid option" message, and continue
         *   looping so that we show the prompt again and read a new
         *   option.  
         */
        libMessages.invalidFinishOption(resp);
    }
}

/*
 *   Finish Option class.  This is the base class for the abstract objects
 *   representing options offered by finishGame.  
 */
class FinishOption: object
    /* 
     *   The description, as displayed in the list of options.  For the
     *   default English messages, this is expected to be a verb phrase in
     *   infinitive form, and should show the keyword accepted as a
     *   response in all capitals: "RESTART", "see some AMUSING things to
     *   do", "show CREDITS". 
     */
    desc = ""

    /* 
     *   By default, the item is listed.  If you want to create an
     *   invisible option that's accepted but which isn't listed in the
     *   prompt, just set this to nil.  Invisible options are sometimes
     *   useful when the output of one option mentions another option; for
     *   example, the CREDITS message might mention a LICENSE command for
     *   displaying the license, so you want to make that command available
     *   without cluttering the prompt with it.  
     */
    isListed = true

    /* our response keyword */
    responseKeyword = ''

    /* 
     *   a single character we accept as an alternative to our full
     *   response keyword, or nil if we don't accept a single-character
     *   response 
     */
    responseChar = nil
    
    /* 
     *   Match a response string to this option.  Returns true if the
     *   string matches our response, nil otherwise.  By default, we'll
     *   return true if the string exactly matches responseKeyword or
     *   exactly matches our responseChar (if that's non-nil), but this
     *   can be overridden to match other strings if desired.  By default,
     *   we'll match the response without regard to case.
     */
    responseMatches(response)
    {
        /* do all of our work in lower-case */
        response = response.toLower();

        /* 
         *   check for a match the full response keyword or to the single
         *   response character 
         */
        return (response == responseKeyword.toLower()
                || (responseChar != nil
                    && response == responseChar.toLower()));
    }

    /*
     *   Carry out the option.  This is called when the player enters a
     *   response that matches this option.  This routine must perform the
     *   action of the option, then return true to indicate that we should
     *   ask for another option, or nil to indicate that the finishGame()
     *   routine should simply return.  
     */
    doOption()
    {
        /* tell finishGame() to ask for another option */
        return true;
    }

    /* 
     *   Flag: show the score with the end-of-game announcement.  If any
     *   option in the list of finishing options has this flag set, we'll
     *   show the score using the same message that the SCORE command
     *   uses. 
     */
    showScoreInFinish = nil
;

/*
 *   QUIT option for finishGame.  The language-specific code should modify
 *   this to specify the description and response keywords.  
 */
finishOptionQuit: FinishOption
    doOption()
    {
        /* 
         *   carry out the Quit action - this will signal a
         *   QuittingException, so this call will never return 
         */
        QuitAction.terminateGame();
    }
;

/*
 *   RESTORE option for finishGame. 
 */
finishOptionRestore: FinishOption
    doOption()
    {
        /* 
         *   Try restoring.  If this succeeds (i.e., it returns true), tell
         *   the caller to stop looping and to proceed with the game by
         *   returning nil.  If this fails, tell the caller to keep looping
         *   by returning true.
         */
        if (RestoreAction.askAndRestore())
        {
            /* 
             *   we succeeded, so we're now restored to some prior game
             *   state - terminate any remaining processing in the command
             *   that triggered the end-of-game options
             */
            throw new TerminateCommandException();
        }
        else
        {
            /* it failed - tell the caller to keep looping */
            return true;
        }
    }
;

/*
 *   RESTART option for finishGame 
 */
finishOptionRestart: FinishOption
    doOption()
    {
        /* 
         *   carry out the restart - this will not return, since we'll
         *   reset the game state and re-enter the game at the restart
         *   entrypoint 
         */
        RestartAction.doRestartGame();
    }
;

/*
 *   START FROM BEGINNING option for failed startup restore.  This is just
 *   like finishOptionRestart, but shows a different option name.  
 */
restoreOptionStartOver: finishOptionRestart
;

/* 
 *   RESTORE ANOTHER GAME option for failed startup restore.  This is just
 *   like finishOptionRestore, but shows a different option name. 
 */
restoreOptionRestoreAnother: finishOptionRestore
;

/*
 *   UNDO option for finishGame 
 */
finishOptionUndo: FinishOption
    doOption()
    {
        /* try performing the undo */
        if (UndoAction.performUndo(nil))
        {
            /* act as though UNDO were the last actual command, for AGAIN */
            AgainAction.saveForAgain(gPlayerChar, gPlayerChar,
                                     nil, UndoAction);
            
            /* 
             *   Success - terminate the current command with no further
             *   processing.
             */
            throw new TerminateCommandException();
        }
        else
        {
            /* 
             *   failure - show a blank line and tell the caller to ask
             *   for another option, since we couldn't carry out this
             *   option 
             */
            "<.p>";
            return true;
        }
    }
;

/*
 *   FULL SCORE option for finishGame
 */
finishOptionFullScore: FinishOption
    doOption()
    {
        /* show a blank line before the score display */
        "\b";

        /* run the Full Score action */
        FullScoreAction.showFullScore();

        /* show a paragraph break after the score display */
        "<.p>";

        /* 
         *   this option has now had its full effect, so tell the caller
         *   to go back and ask for a new option 
         */
        return true;
    }

    /* 
     *   by default, show the score with the end-of-game announcement when
     *   this option is included 
     */
    showScoreInFinish = true
;

/*
 *   Option to show the score in finishGame.  This doesn't create a listed
 *   option in the set of offered options, but rather is simply a flag to
 *   finishGame() that the score should be announced along with the
 *   end-of-game announcement message. 
 */
finishOptionScore: FinishOption
    /* show the score in the end-of-game announcement */
    showScoreInFinish = true

    /* this is not a listed option */
    isListed = nil

    /* this option isn't selectable, so it has no effect */
    doOption() { }
;

/*
 *   CREDITS option for finishGame 
 */
finishOptionCredits: FinishOption
    doOption()
    {
        /* show a blank line before the credits */
        "\b";

        /* run the Credits action */
        CreditsAction.execSystemAction();

        /* show a paragraph break after the credits */
        "<.p>";

        /* 
         *   this option has now had its full effect, so tell the caller
         *   to go back and ask for a new option 
         */
        return true;
    }
;

/*
 *   AMUSING option for finishGame 
 */
finishOptionAmusing: FinishOption
    /*
     *   The game must modify this object to define a doOption method.  We
     *   have no built-in way to show a list of amusing things to try, so
     *   if a game wants to offer this option, it must provide a suitable
     *   definition here.  (We never offer this option by default, so a
     *   game need not provide a definition if the game doesn't explicitly
     *   offer this option via the 'extra' argument to finishGame()).  
     */
;

/* ------------------------------------------------------------------------ */
/*
 *   Utility functions 
 */

/*
 *   nilToList - convert a 'nil' value to an empty list.  This can be
 *   useful for mix-in classes that will be used in different inheritance
 *   contexts, since the classes might or might not inherit a base class
 *   definition for list-valued methods such as preconditions.  This
 *   provides a usable default for list-valued methods that return nothing
 *   from superclasses. 
 */
nilToList(val)
{
    return (val != nil ? val : []);
}

/* 
 *   partitionList - partition a list into a pair of two lists, the first
 *   containing items that match the predicate 'fn', the second containing
 *   items that don't match 'fn'.  'fn' is a function pointer (in most
 *   cases, probably an anonymous function) that takes a single argument, a
 *   list element, and returns true or nil.
 *   
 *   The return value is a list with two elements.  The first element is a
 *   list giving the elements of the original list for which 'fn' returns
 *   true, the second element is a list giving the elements for which 'fn'
 *   returns nil.
 *   
 *   (Contributed by Tommy Nordgren.)  
 */
partitionList(lst, fn)
{
    local lst1 = lst.subset(fn);
    local lst2 = lst.subset({x : !fn(x)});
    
    return [lst1, lst2];
}
