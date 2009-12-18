/*
 * class        : pane
 * superClass	: field
 */
#include "cl_field.h"

extern ClassInfo class_pane;
int helper_pane_get();
int helper_pane_set();

enum PaneConfigTypes {
	PANE_CONFIG_FREE,
	PANE_CONFIG_N2S,
	PANE_CONFIG_S2N,
	PANE_CONFIG_W2E,
	PANE_CONFIG_E2W,
	PANE_CONFIG_N2S_EDGE,
	PANE_CONFIG_S2N_EDGE,
	PANE_CONFIG_W2E_EDGE,
	PANE_CONFIG_E2W_EDGE,
	PANE_CONFIG_CENTER,
};
extern char *paneConfigStr[];

int meth_pane_config();
int meth_pane_clone();
int meth_pane_clone2();
int meth_pane_expose();
int meth_pane_get();
int meth_pane_initialize();
int meth_pane_render();
int meth_pane_set();
