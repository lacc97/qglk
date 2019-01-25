#charset "us-ascii"

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3 
 */

/*
 *   TADS basic data manipulation intrinsic function set 
 */

#ifndef TADSGEN_H
#define TADSGEN_H

/*
 *   define the TADS function set 
 */
intrinsic 'tads-gen/030006'
{
    dataType(val);
    getArg(idx);
    firstObj(cls?, flags?);
    nextObj(obj, cls?, flags?);
    randomize();
    rand(x, ...);
    toString(val, radix?);
    toInteger(str, radix?);
    getTime(timeType?);
    rexMatch(pat, str, index?);
    rexSearch(pat, str, index?);
    rexGroup(groupNum);
    rexReplace(pat, str, replacement, flags, index?);
    savepoint();
    undo();
    saveGame(filename);
    restoreGame(filename);
    restartGame();
    max(val1, ...);
    min(val1, ...);
    makeString(val, repeatCount?);
    getFuncParams(func);
}

/*
 *   flags for firstObj() and nextObj()
 */
#define ObjInstances  0x0001
#define ObjClasses    0x0002
#define ObjAll        (ObjInstances | ObjClasses)

/*
 *   rexReplace() flags 
 */
#define ReplaceOnce  0x0000
#define ReplaceAll   0x0001

/*
 *   getTime() flags 
 */
#define GetTimeDateAndTime  1
#define GetTimeTicks        2


#endif /* TADSGEN_H */

