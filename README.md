# Knights

Knights is KDE's chess frontend. It supports playing local games against human players or against chess engines (XBoard and UIC) as well as playing online games on FICS server.

[![](https://cdn.kde.org/screenshots/knights/knights_main.png "Main Window")](https://cdn.kde.org/screenshots/knights/knights_main.png)

Furthermore, it is possible to watch two different chess engines playing against each other.
[![](https://cdn.kde.org/screenshots/knights/knights_seek_graph.png "Seek Graph")](https://cdn.kde.org/screenshots/knights/knights_seek_graph.png)


Knights is not bound to any specific chess engine and talks to every engine supporting either the XBoard or the UIC protocols. For couple of well known open-source chess engines provided by linux distributions, knights has an auto-detection of installed engines which simplifies the initial configuration.
[![](https://cdn.kde.org/screenshots/knights/knights_auto_detect_engines.png "Auto Detection of Chess Engines)](https://cdn.kde.org/screenshots/knights/knights_auto_detect_engines.png)


# Required Packages

* CMake version 2.8.12 or higher
* ECM (extra cmake modules)
* KDE Frameworks (including the devel-package)
* KDE Games libraries (including the devel-package)
* Qt version 5.6 or higher (including the devel-package, libqt5-devel or similar)


# How To Build Knights

To build and to install the project, execute

* cd <project_name_path>
* mkdir build
* cd build
* cmake -DCMAKE_INSTALL_PREFIX=<path_to_install_>
* make
* make install or sudo make install or su -c 'make install'

To uninstall the project, execute
* make uninstall or sudo make uninstall or su -c 'make uninstall'


# Bug Reports and Wishes
Use [KDE Bugtracking System](https://bugs.kde.org/) to report any kind of issues with Knights.
