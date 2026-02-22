# Insync Dolphin plugin

Insync context menus and overlay icons use the KDE6 API and will not work in KDE5 and earlier.

### Context menus
Insync context menus are implemented using the KDE API [KAbstractFileItemActionPlugin]. The `KAbstractFileItemActionPlugin:actions` is called whenever a user right-clicks on a file or directory and should return a list of `QAction`s. If this returns an empty list, no additional context menus are added. This `QAction`s listed are always appended to the default list of options in the context menu. In our implementation, we return a [KActionMenu] containing the list of `QAction`s (`Add to Insync`, `Share`, etc) so that all of our context menu options are containined in its own submenu. Right now, we only support context menu actions when a single file/directory is selected as the client doesn't support it yet. A `.desktop` KDE service file is needed for this to show up in the list of plugins a user can enable in the Dolphin settings.


### Overlay icons
Insync file overlay status icons are implemented using [KOverlayIconPlugin]. The `KOverlayIconPlugin:getOverlays` is called on every file/directory currently showing in Dolphin. Since it is called on every file/directory, we create a new `QLocalSocket` on every call as it can cause segfaults if there are too many files/directories being queried. The build library file for this is added to the `KDE_PLUGIN_DIR/kf5/overlayicon` folder and it should work without needing a KDE service file.


### Helper library
We have an additional library `InsyncDolphinPluginHelper` that's used in both the context menu and overlay icon libraries for doing common functions (connecting to `insync.sock`, sending/receiving commands to the `insync.sock`, etc).

### Dependencies
On Fedora, you need at least these:
```
sudo dnf install kf6-kcoreaddons-devel kf6-kio-devel kf6-kconfig-devel extra-cmake-modules qt6-qtbase-devel g++
```
On Kubuntu (and possibly Ubuntu and Debian), you need at least these:
```
sudo apt install kf6-kcoreaddons-dev kf6-kio-dev kf6-kconfig-dev extra-cmake-modules qt6-base-dev g++ cmake
```
### Build
```
cmake -DCMAKE_INSTALL_PREFIX=/usr -B build/
cmake --build build/
```
### Install

```
sudo cmake --install build
```

Note that everything is installed into `/usr`, not `/usr/local`(which does not work).

### Uninstall
```
sudo rm /usr/lib64/libinsyncdolphinpluginhelper.so /usr/lib64/qt6/plugins/kf6/kfileitemaction/insyncfileitemaction.so /usr/lib64/qt6/plugins/kf6/overlayicon/insyncoverlayicon.so /usr/share/icons/hicolor/64x64/emblems/emblem-insync-* /usr/share/icons/hicolor/scalable/emblems/emblem-insync-* ; sudo rmdir /usr/share/icons/hicolor/64x64/emblems /usr/share/icons/hicolor/scalable/emblems

```
In case this throws an error, note where the files you are trying to remove are placed during the installation and remove those.

### Additional references
`QJson` Docs: https://doc.qt.io/qt-6/json.html


[KAbstractFileItemActionPlugin]: https://api.kde.org/kabstractfileitemactionplugin.html
[KActionMenu]: https://api.kde.org/kactionmenu.html
[KOverlayIconPlugin]: https://api.kde.org/koverlayiconplugin.html
