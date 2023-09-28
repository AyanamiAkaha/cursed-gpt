# cursed-gpt

ncurses interface to GPT chat api.

This is a fun personal project, mostly for refreshing my C++ skills. Currently only simple chat works.
Forgive my code quality, I haven't used C++ since before C++11, and just getting used to it.

## Building:

```
meson setup builddir
meson compile -C builddir
```

The executable will be at builddir/gpt-chat

## Usage:

If you don't have already, generate [GPT API key](https://help.openai.com/en/articles/4936850-where-do-i-find-my-secret-api-key),
and export it as an environment variable `GPT_API_KEY`.

```
export GPT_API_KEY=<your api key>
```

Run the chat with `./builddir/gpt-chat`.

Supported commands:

- `/q` - exit the app
- `/new` - open new chat window
- `/next`, `/prev` - move between windows
- `/save <fname>` - save chat from current window to `fname.json`

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
