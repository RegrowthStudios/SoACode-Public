# Seed of Andromeda
This repository contains the source code for Seed of Andromeda.

## Getting Started
This guide will walk you through setting up as a developer for
Seed of Andromeda.  There is a basic requirement of having several
packages installed prior to being able to develop.  In addition,
we support all three major operating systems:

### Installing and building:
* [Windows](#Windows)
* [Mac](#Mac)
* [Linux](#Linux)
 
### Contributing
* [Wiki](https://github.com/RegrowthStudios/SoACode/wiki)
* [Issues](https://github.com/RegrowthStudios/SoACode/issues)


## Windows

### Required pre-setup
1.  Compiler: [Microsoft Visual Studio 2013](http://www.microsoft.com/en-us/download/details.aspx?id=43729)
*  Software Version Control:  [Git](http://git-scm.com/downloads)
*  MSVS SVC Plugin:  [MSVS Git Plugin](http://msdn.microsoft.com/en-us/library/hh850437.aspx)

### Installation
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
git clone --recurse-submodules git@github.com:RegrowthStudios/SoACode.git soa
```

### Building
1. Pull latest code
```
cd c:\repos\soa
git checkout develop
git pull --recurse-submodules
```
2. Run build script
```
build.bat
```


## Mac

### Required pre-setup
1. Compiler: [Xcode](https://developer.apple.com/xcode/)
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

### Installation
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
git clone --recurse-submodules git@github.com:RegrowthStudios/SoACode.git soa
```

### Building
1. Pull latest code
```
cd ~/repos/soa
git checkout develop
git pull --recurse-submodules
```
2. Run build script
```
./build.sh
```


## Linux

### Required pre-setup
1. Compiler: gcc or clang
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

### Installation
1. Open a terminal.
2. Create a folder to hold the repositories
```
mkdir ~/repos
```
3. Clone the Seed of Andromeda repositories
```
cd ~/repos
git clone --recurse-submodules git@github.com:RegrowthStudios/SoACode.git soa
```

### Building
1. Pull latest code
```
cd ~/repos/soa
git checkout develop
git pull --recurse-submodules
```
2. Run build script
```
./build.sh
```
