/*
 * class        : cosmic
 * superClass	: NULL
 */

extern ClassInfo class_cosmic;
int helper_cosmic_get();
int helper_cosmic_set();

int meth_cosmic_clone();
int meth_cosmic_clone2();
int meth_cosmic_cloneID();
int meth_cosmic_create();
int meth_cosmic_debug();
int meth_cosmic_destroy();
int meth_cosmic_detach();
int meth_cosmic_exist();
int meth_cosmic_exit();
int meth_cosmic_freeSelf();
int meth_cosmic_info();
int meth_cosmic_interpret();
int meth_cosmic_loadObjFile();
int meth_cosmic_object();
int meth_cosmic_pop();
int meth_cosmic_push();
int meth_cosmic_quit();
int meth_cosmic_save();
int meth_cosmic_saveAs();
int meth_cosmic_saveObjFile();
int meth_cosmic_send();
int meth_cosmic_test1();
int meth_cosmic_test2();
int meth_cosmic_test3();
int meth_cosmic_test4();
int meth_cosmic_tweak();
int meth_cosmic_usual();

int meth_cosmic_modalExit();

void sendInitToChildren();
