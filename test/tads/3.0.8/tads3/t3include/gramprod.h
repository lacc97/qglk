#charset "us-ascii"

/* 
 *   Copyright (c) 2000, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3.  
 */

#ifndef _GRAMPROD_H_
#define _GRAMPROD_H_

/* include our base class definition */
#include "systype.h"

/* 
 *   Properties for the first and last token indices, and the complete
 *   original token list.
 *   
 *   Each match tree object will have the firstTokenIndex and lastTokenIndex
 *   properties set to the bounding token indices for its subtree.  Each
 *   match tree object will also have tokenList set to the original token
 *   list passed to the parseTokens() that created the match tree, and
 *   tokenMatchList set to a list of the Dictionary's comparator's
 *   matchValues() result for each token match.  
 */
property firstTokenIndex, lastTokenIndex, tokenList, tokenMatchList;
export firstTokenIndex 'GrammarProd.firstTokenIndex';
export lastTokenIndex  'GrammarProd.lastTokenIndex';
export tokenList       'GrammarProd.tokenList';
export tokenMatchList  'GrammarProd.tokenMatchList';


/*
 *   'grammar production' metaclass 
 */
intrinsic class GrammarProd 'grammar-production/030000': Object
{
    parseTokens(tokenList, dict);
}

#endif /* _GRAMPROD_H_ */
