# Pacmanager
<img src="logo.svg" alt="drawing" width="200"/>

### Description
Pacmanager is a simple GUI wrapper for the pacman package manager intended for use on Arch or Arch-based systems

### Capabilities
- Search for packages on the remote repos
- Filter packages by status
- View info about packages
- Install and update packages
- Perform full system updates
- Remove local packages
- Ability to display package installation progress in the UI


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

