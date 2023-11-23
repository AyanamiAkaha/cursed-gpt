#include "app.hh"
#include "chat.hh"

COMMAND(q, app->quit(0))
COMMAND(new, app->cmdNew(args))
COMMAND(next, app->nextChat())
COMMAND(prev, app->prevChat())
COMMAND(save, app->saveCurrentChat(args))
COMMAND(load, app->loadChat(args))
COMMAND(export, app->exportCurrentChat(args))
COMMAND(temperature, app->setTemperature(args))
COMMAND(model, app->setModel(args))
COMMAND(list, app->cmdList(args))
COMMAND(system, app->currentChat().systemPrompt(args))
COMMAND(gpt, app->currentChat().impersonateAssistant(args))
COMMAND(resend, app->currentChat().send())