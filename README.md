# Pacmanager
<img src="logo.svg" alt="drawing" width="200"/>

# ⚠️ This project has been rewritten ⚠️
A full rewrite of this project can now be found in the [pacmanager-rs](https://github.com/alcalino-git/pacmanager-rs) repository, as the name indicates, it is written in rust 🦀 and features robust memory safety 💪 that fixex any and all segmentation faults found on this version

--- 

### Description
Pacmanager is a simple GUI wrapper for the pacman package manager intended for use on Arch or Arch-based systems

### Capabilities
- Search for packages on the remote repos
- Filter packages by status
- Sort packages by their properties
- View info about packages
- Install and update packages
- Perform full system updates
- Remove local packages
- Ability to display package installation progress in the UI

### Pending
- Some package properties are not displayed
- The program creates no logs at all

### Known issues
- Repeteadly activating the search bar in quick sucession causes the program to become unstable as every query must complete before the next one can begin. Adittionally, each query requires launching a new thread
- Repeteadly activating the search bar in quick sucession might cause the loading spinner to not appear at all. This does not affect functionality but does make the process appear frozen for a few seconds
  
*If you find any more bugs, be sure to report them to the `issues` section*

## Instalation
#### AUR
```bash
yay -S  pacmanagergui-git
```
#### Manual
```bash
git clone https://github.com/alcalino-git/pacmanager.git
cd pacmanager
meson setup build
meson compile -C build
install -Dm755 ./build/pacmanager "/usr/bin/pacmanager"
```

