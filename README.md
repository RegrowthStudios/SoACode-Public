# Seed of Andromeda
This repository contains the source code for Seed of Andromeda.

## Getting Started
This guide will walk you through setting up as a contributor to the
Seed of Andromeda project. There is a basic requirement of having several
packages installed prior to being able to develop. We support all three
major operating systems: Windows, Mac and Linux.

Firstly, before continuing if you haven't already, be sure to fork the repository -
clicking the fork button on the top right of this page. Once you have a personal
version of the repository follow the instructions below.

### Setting Up:
* [Windows](#windows)
* [Mac](#mac)
* [Linux](#linux)

### Building:
* [Building](#building-1)
 
### Contributing
* [Wiki](https://github.com/RegrowthStudios/SoACode-Public/wiki)
* [Issues](https://github.com/RegrowthStudios/SoACode-Public/issues)
* [Discord](https://discord.gg/b2bf775)


## Setting Up

### Windows

#### Prerequisites
*  Compiler: [Microsoft Visual Studio 2015+](https://visualstudio.microsoft.com/)
*  Software Version Control:  [Git](http://git-scm.com/downloads)
*  (Optional) MSVS SVC Plugin:  [MSVS Git Plugin](http://msdn.microsoft.com/en-us/library/hh850437.aspx)
*  (Optional) TortoiseGit: [tortoisegit](https://tortoisegit.org/download)

#### Setup
1. Open a dos window.
```
Windows + R
cmd
```
2. Create a folder to hold the repositories
```
cd c:\
mkdir -p repos
```
3. Clone the Seed of Andromeda repositories
```
c:\
cd c:\repos
git clone --recurse-submodules https://github.com/YOUR_GITHUB_NAME/SoACode-Public.git soa
cd c:\repos\soa
```

### Mac

#### Prerequisites
* Compiler: [Xcode](https://developer.apple.com/xcode/)
* Software Version Control: [Git](http://git-scm.com/downloads)
    Optionally, with [Homebrew](http://brew.sh/):
    ```brew install git```
* Preferred editor: [Sublime Text](http://www.sublimetext.com/) and optional packages
    * [PackageControl](https://sublime.wbond.net/installation)
    * [CMake](https://sublime.wbond.net/packages/CMake) - CMake.tmLanguage
    * [GitGutter](https://sublime.wbond.net/packages/GitGutter) - A Sublime Text 2/3 plugin to see git diff in gutter
    * [SublimeCodeIntel](https://sublime.wbond.net/packages/SublimeCodeIntel) - Full-featured code intelligence and smart autocomplete engine
    * [SublimeLinter](https://sublime.wbond.net/packages/SublimeLinter) -- Interactive code linting framework for Sublime Text 3
    * [SublimeLinter-cpplint](https://sublime.wbond.net/packages/SublimeLinter-cpplint) -- This linter plugin for SublimeLinter provides an interface to cpplint.
    * [SublimeLinter-pep8](https://sublime.wbond.net/packages/SublimeLinter-pep8) - SublimeLinter plugin for python, using pep8.
    * [SublimeLinter-contrib-clang](https://sublime.wbond.net/packages/SublimeLinter-contrib-clang) - https://sublime.wbond.net/packages/SublimeLinter-contrib-clang

#### Setup
1. Open a terminal.
```
cmd + space
Terminal
```
2. Create a folder to hold the repositories
```
mkdir ~/repos
```
3. Clone the Seed of Andromeda repositories
```
cd ~/repos
git clone --recurse-submodules https://github.com/YOUR_GITHUB_NAME/SoACode-Public.git soa
cd ~/repos/soa
```

### Linux

#### Prerequisites
* Compiler: gcc or clang
    * Install per your preferred operating system package control...
    * Portage:
    ```
    sudo emerge -DuNqa gcc   # for gcc
    sudo emerge -DuNqa clang  # for clang
    ```
    * PacMan:
    ```
    sudo pacman -S gcc
    sudo pacman -S clang
    ```
    * Apt:
    ```
    sudo apt-get install gcc
    sudo apt-get install clang
    ```
    * Yum:
    ```
    sudo yum install gcc
    sudo yum install clang
    ```
* Software Version Control: [Git](http://git-scm.com/downloads)
    * Portage:
    ```
    sudo emerge -DuNqa git
    ```
    * PacMan:
    ```
    sudo pacman -S git
    ```
    * Apt:
    ```
    sudo apt-get install git
    ```
    * Yum:
    ```
    sudo yum install git
    ```

#### Setup
1. Open a terminal.
2. Create a folder to hold the repositories
```
mkdir ~/repos
```
3. Clone the Seed of Andromeda repositories
```
cd ~/repos
git clone --recurse-submodules https://github.com/YOUR_GITHUB_NAME/SoACode-Public.git soa
cd ~/repos/soa
```

## Building
1. Pull latest code (from inside .../repos/soa)
```
git checkout develop    # or your current branch
git pull --recurse-submodules
```
2. Run the build script (--help for options)
    * Windows:
    ```
    build.bat    # or compile from within your Visual Studio environment
    ```
    * Linux:
    ```
    ./build.sh
    ```
3. Run the built executable
    * Windows:
    ```
    build/SoA/launch-soa-{Release|Debug}.cmd    # or launch from within your Visual Studio environment
    ```
    * Linux:
    ```
    ./build/SoA/launch-soa.sh
    ```
