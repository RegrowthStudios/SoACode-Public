
[![dicord](https://img.shields.io/discord/459062989094649866.svg?logo=discord "Discord")](https://discord.gg/b2bf775)
![travis](https://img.shields.io/travis/RegrowthStudios/SoACode-Public/develop.svg?style=flat-square&label=Linux "Travis CI")
![appveyor](https://img.shields.io/appveyor/ci/SamThePsychoticLeprechaun/soacode-public/develop.svg?style=flat-square&label=Windows "AppVeyor CI")

# Seed of Andromeda
This repository contains the source code for Seed of Andromeda.

## Getting Started
This guide will walk you through setting up as a contributor to the
Seed of Andromeda project. There is a basic requirement of having several
packages installed prior to being able to develop. We support all three
major operating systems: Windows, Mac and Linux.

### Contributing
Before beginning your SoA journey, please take a moment to use the following resources
to get an idea of how to contribute, what you might be able to contribute to specifically,
and to meet some of the other contributors.
* [Wiki](https://github.com/RegrowthStudios/SoACode-Public/wiki)
* [Issues](https://github.com/RegrowthStudios/SoACode-Public/issues)
* [Discord](https://discord.gg/b2bf775)

### Setting Up:
**IMPORTANT**: Before following any of the instructions linked below for the platforms we support,
please do take a second to fork the repository! If you are new to GitHub, you can 
do so by clicking the "fork" button on the top right of this page.

If you have cloned the repository before forking, no worries! We can fix it, by following [these instructions](#fixing-a-pre-fork-clone).

Now we're forked, follow the link to the section on setting up for your OS of choice:
* [Windows](#windows)
* [Mac](#mac)
* [Linux](#linux)

### Building:
Now you have a copy of the code, and perhaps have played with it a little, why not give it
a whirl?
* [Building](#building-1)


## Setting Up

### Windows

#### Prerequisites
*  Compiler: [Microsoft Visual Studio 2015 (only)](https://visualstudio.microsoft.com/)
*  Software Version Control:  [Git](http://git-scm.com/downloads)
*  (Optional) MSVS SVC Plugin:  [MSVS Git Plugin](http://msdn.microsoft.com/en-us/library/hh850437.aspx)
*  (Optional) TortoiseGit: [tortoisegit](https://tortoisegit.org/download)

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

### Linux

#### Prerequisites
* Compiler: gcc or clang
    * Install per your preferred operating system package control...
    * Portage:
    ```bash
    sudo emerge -DuNqa gcc   # for gcc
    sudo emerge -DuNqa clang  # for clang
    ```
    * PacMan:
    ```bash
    sudo pacman -S gcc
    sudo pacman -S clang
    ```
    * Apt:
    ```bash
    sudo apt-get install gcc
    sudo apt-get install clang
    ```
    * Yum:
    ```bash
    sudo yum install gcc
    sudo yum install clang
    ```
* Software Version Control: [Git](http://git-scm.com/downloads)
    * Portage:
    ```bash
    sudo emerge -DuNqa git
    ```
    * PacMan:
    ```bash
    sudo pacman -S git
    ```
    * Apt:
    ```bash
    sudo apt-get install git
    ```
    * Yum:
    ```bash
    sudo yum install git
    ```

# Setup
1. Create a folder to hold the repositories and change to directory
    * Windows
    ```cmd
    Windows + R
    cmd
        
    cd c:\
    mkdir -p repos
    cd c:\repos
    ```
    * Linux
    ```bash
    mkdir ~/repos
    cd ~/repos
    ```
    * Mac
    ```bash
    cmd + space
    Terminal
        
    mkdir ~/repos
    cd ~/repos
    ```
2. Clone the Seed of Andromeda repositories
```cmd
git clone --recurse-submodules https://github.com/YOUR_GITHUB_NAME/SoACode-Public.git soa
```
3. Change to soa direcotory
    * Windows
    ```
    cd c:\repos\soa
    ```
    * Linux & Mac
    ```
    cd ~/repos/soa
    ```
4. (optional) Do this step only if you plan to fork your own Vorb or SoAGameData repos.
    * Fork both the [Vorb](https://github.com/RegrowthStudios/Vorb) and/or [GameData](https://github.com/RegrowthStudios/SoAGameData) repos in github.
    * Set origin of submodules to your forked repositories
    ```cmd
    # Assuming we're already inside the top-level directory of your SoACode-Public repository.
    cd Vorb
    git remote set-url origin https://github.com/YOUR_GITHUB_NAME/Vorb.git
    cd game
    git remote set-url origin https://github.com/YOUR_GITHUB_NAME/SoAGameData.git
    ```

# Building

1. Pull latest code (from inside .../repos/soa)
    ```bash
    git checkout develop    # or your current branch
    git pull --recurse-submodules
    ```
2. Run the build script (--help for options)
    * Windows:
    ```cmd
    build.bat    # or compile from within your Visual Studio environment
    ```
    * Linux:
    ```bash
    ./build.sh
    ```
3. Run the built executable
    * Windows:
    ```cmd
    build/SoA/launch-soa-{Release|Debug}.cmd    # or launch from within your Visual Studio environment
    ```
    * Linux:
    ```bash
    ./build/SoA/launch-soa.sh
    ```

# Fixing a Pre-Fork Clone

So, you've accidentally cloned the repository before forking it, eh? No problem. Just run the following git commands inside of the repository and everything will be as it should be!

Firstly, if you still haven't, fork the repositories you want to contribute to!

Now you have a fork we want to set `origin` of each of your local repositories (which is the default remote repository to push changes to) to your corresponding forked repositories:
```bash
# Assuming we're already inside the top-level directory of your SoACode-Public repository.
git remote set-url origin https://github.com/YOUR_GITHUB_NAME/SoACode-Public.git
cd Vorb
git remote set-url origin https://github.com/YOUR_GITHUB_NAME/Vorb.git
cd game
git remote set-url origin https://github.com/YOUR_GITHUB_NAME/SoAGameData.git
```
If you haven't forked, either Vorb or SoAGameData as you don't intend to contribute to that repository, then you don't need to do run the commands corresponding to that repository.

That's it! It's all fixed. :)
