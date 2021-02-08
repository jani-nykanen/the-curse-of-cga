## The Curse of CGA

...is a puzzle game for MS-DOS with CGA graphics. Or, actually, a prototype of a puzzle game. Or something I call "finished prototype": the game has all the major features, it only lacks of things like audio, options, more animations and a proper story intro (and proper ending). However, I don't want to call it "beta" or "alpha" because the will never be a truly "finished" version, just this "unfinished finished" one. Getting confusing? That's okay.

-----

## Playing

A downloadable binary can be found in the releases: https://github.com/jani-nykanen/the-curse-of-cga/releases. You need DOSBox (https://www.dosbox.com) to play it.

I wanted to have a playable browser build, but em-dosbox and emscripten both refused to work (syntax errors, conflicting files and all that kind of stuff).

-----

## Building

You see a makefile there? You cannot use it to build the game. Reason: OpenWatcom's command-line tools are weird and documentation even weirder, so instead I ran the IDE through Wine, put all source files there and then built the project. You can do the same.

-----

## License

LGPL, maybe?


-----

(c) 2021 Jani Nykänen

