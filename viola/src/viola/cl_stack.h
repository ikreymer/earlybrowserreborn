/*
 * class        : stack
 * superClass	: pane
 */
#include "cl_pane.h"

extern ClassInfo class_stack;
int helper_stack_get();
int helper_stack_set();

int meth_stack_anyCard();
int meth_stack_card();
int meth_stack_config();
int meth_stack_countBCards();
int meth_stack_countFCards();
int meth_stack_create();
int meth_stack_currentCard();
int meth_stack_expose();
int meth_stack_firstCard();
int meth_stack_get();
int meth_stack_getCurrentCard();
int meth_stack_initialize();
int meth_stack_lastCard();
int meth_stack_nextCard();
int meth_stack_previousCard();
int meth_stack_render();
int meth_stack_set();
int meth_stack_setCurrentCard();

VObj *findStackObj();
