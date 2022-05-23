# bddi - BSPWM dynamic desktop icons

![Example with Polybar and NerdFont icons](demo.gif)

This daemon dynamically renames BSPWMs desktops based on what programs are open within them.

(The demo GIF shows Polybar with NerdFont icons being used as the window names)

## How?
The program opens a connection to BSPWMs UNIX socket, which is also how `bspc` communicates with it.  
Via this socket, `bddi` listenes for node events like opening, closing or moving a window.  
When recieved, the nodes open on the desktop associated with the event will be requested, parsed and  
based on this a dynamic name will be generated and set.  
Using an Icon font for the names is recommended.

--- 

There's also a Python version available in the `python` branch if you feel more comfortable with running that.
It has the same featureset, only that you can configure it directly within the python file.

I wanted to exercise my C skills a little, thus the rewrite published here.

## Build & Install

### Requirements

* make      - For compilation with make
* gcc       - For compilation with make
* [json-c](https://github.com/json-c/json-c)    - For parsing the desktop info returned by BSPWM
* (optional) A [Nerd Fonts](https://www.nerdfonts.com/) capable font

### Installation

Installing can be done with the provided makefile as follows:

```sh
make
make install
```

This will create a configuration file under ~/.config/bddi and copy the executable to /usr/bin

## Configuration

The program will look in $HOME/.config/bddi for a file called `icons.list`.
This file may hold one definition per line of the format:

```txt
WM_CLASS NAME
```

WM_CLASS is a X11 property associated with each open window.
All windows of a certain program usually hold the same WM_CLASS, so they can be identified.
To find the WM_CLASS of a program you can use the `xprop` utility.

The NAME part can be any string. It's recommended to use icons from an icon font here.

## Bugs

This is a hobby project and thus comes with absolutely NO WARRANTY.  
I do not in any way claim this program to be free from catastrophic bugs or complete.  
USE AT YOUR OWN RISK.

If you do find errors, please do open an Issue.
I am interested in learning how to do things better.

## Similar Projects

* [btops](https://github.com/cmschuetz/btops) - A more fully featured program providing utility functions for BSPWM. Written in Go.
