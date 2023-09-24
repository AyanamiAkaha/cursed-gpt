#include "app.hh"

COMMAND(q, app->quit(0))
COMMAND(new, app->newChat())
COMMAND(next, app->nextChat())
COMMAND(prev, app->prevChat())