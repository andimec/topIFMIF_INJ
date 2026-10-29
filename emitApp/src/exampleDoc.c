/*
 * C.E.A. IRFU/SIS/LDII
 *
 * @(#) $Id: exampleDoc.c,v 1.5 2012/08/08 13:07:45 cmsmgr Exp $
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * ylussign  20/01/10  created
 */

/*
 * The purpose of this document is to show how to generate documentation
 * from a program source file with doxygen. Two examples are given.
 */
 
/************************************************************************
 *
 *   1. How to generate a single man page for a program or a function.
 *
 ************************************************************************/

/** 
 * @page exampleMan
 * Brief description of the program/function
 *
 * @section SYNOPSIS
 * Explain here how this program/function is called
 * 
 * @section DESCRIPTION
 * This is an example of how to generate a man page with doxygen.
 * From this piece of documentation, doxygen will generate a file
 * exampleMan.3 in emitApp/man/man3 and a file exampleMan.html
 * in emitApp/html.
 * 
 * Some doxygen command examples are given below. 
 *
 * Use a blank line to start a new paragraph.
 *
 * This command @n forces to start a new line.
 * 
 * This command displays the next word using a @b bold font.
 * This command displays the next word @e underlined or in @e italics.
 * 
 * A '-' at start of a line generates a bullet list. If the line starts
 * with '-#', it generates a numbered list. Nesting of lists is based
 * on indentation of the items. Example:
 * -# first level
 *   - second level
 *   - second level
 * -# first level
 * 
 * @note
 * This command starts a paragraph where a note can be entered.
 *
 * The following command starts parameters description:
 * @param[out] param1 parameter description
 * @param[in] param2 parameter description
 *
 * The following command starts return values description:
 * @retval OK return value description
 * @retval ERROR return value description
 *
 * @section COPYRIGHT
 * Other sections may be added.
 *
 * @section see-also SEE ALSO
 * Use this form of the command if the section name has more than one word.
 */



/************************************************************************
 *
 *   2. How to generate a javadoc style document for this source file.
 *
 ************************************************************************/
 
/** 
 * @file
 * @brief Brief description of the file contents.
 *
 * Detailed description of the file.
 * From this piece of documentation, doxygen will generate a file
 * exampleDoc.c.3 in emitApp/man/man3 and a file exampleDoc_8c.html
 * in emitApp/html.
 *
 * @note
 * This command starts a paragraph where a note can be entered.
 */
 
/**
 * From these pieces of documentation, doxygen will generate a function
 * documentation section in the file document.
 *
 * @retval OK 
 * @retval ERROR driver not initialized
 */
int exampleFunction
    (
    /** [in] param1 description */
    int param1,
    /** [out] param2 description */
    int *param2,
    );

/**
 * From this piece of documentation, doxygen will generate a variable
 * documentation section in the file document.
 */
int exampleVariable;

