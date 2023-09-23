# cursed-gpt

ncurses interface to GPT chat api.

This is a fun personal project, mostly for refreshing my C++ skills. Currently only simple chat works.
Forgive my code quality, I haven't used C++ since before C++11, and just getting used to it.

## Usage:

To exit chat type `/q`. Other commands not supported yet.

## Known issues:

- backspace or any other control keys do not work
- network loop isn't properly closed before exit

## Planned features (in arbitrary order):
- readline / libedit support
- loading chat templates - a set of messages that are prepended to the conversation, to set up the chat for particular task
- saving and loading chat sessions
- impersonating system or assistant
- deleting messages from history
- editing messages in history
- changing model and temperature parameters
- regenerating chat from the beginning (with current parameters)

*this repository is a mirror of my private gitea repository. I'll try to keep it up to date, but if you want to contribute for some reason be prepared to rebase your code in case this mirror isn't up to date*
