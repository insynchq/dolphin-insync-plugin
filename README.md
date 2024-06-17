# Insync Dolphin plugin

Insync context menus and overlay icons use the KDE5 API and may not work in KDE4.

### Context menus
Insync context menus are implemented using the KDE API [KAbstractFileItemActionPlugin]. The `KAbstractFileItemActionPlugin:actions` is called whenever a user right-clicks on a file or directory and should return a list of `QAction`s. If this returns an empty list, no additional context menus are added. This `QAction`s listed are always appended to the default list of options in the context menu. In our implementation, we return a [KActionMenu] containing the list of `QAction`s (`Add to Insync`, `Share`, etc) so that all of our context menu options are containined in its own submenu. Right now, we only support context menu actions when a single file/directory is selected as the client doesn't support it yet. A `.desktop` KDE service file is needed for this to show up in the list of plugins a user can enable in the Dolphin settings.


### Overlay icons
Insync file overlay status icons are implemented using [KOverlayIconPlugin]. The `KOverlayIconPlugin:getOverlays` is called on every file/directory currently showing in Dolphin. Since it is called on every file/directory, we create a new `QLocalSocket` on every call as it can cause segfaults if there are too many files/directories being queried. The build library file for this is added to the `KDE_PLUGIN_DIR/kf5/overlayicon` folder and it should work without needing a KDE service file.


### Helper library
We have an additional library `InsyncDolphinPluginHelper` that's used in both the context menu and overlay icon libraries for doing common functions (connecting to `insync.sock`, sending/receiving commands to the `insync.sock`, etc).


### Build
```
mkdir build && cd build
cmake ..
cmake --build .
```


### Additional references
`QJson` Docs: https://doc.qt.io/qt-5/json.html


[KAbstractFileItemActionPlugin]: https://api.kde.org/frameworks/kio/html/classKAbstractFileItemActionPlugin.html
[KActionMenu]: https://api.kde.org/frameworks/kwidgetsaddons/html/classKActionMenu.html
[KOverlayIconPlugin]: https://api.kde.org/frameworks/kio/html/classKOverlayIconPlugin.html
