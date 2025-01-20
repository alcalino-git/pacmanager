

# Maintainer: alcalino alanhumber333@gmail.com
pkgname='pacmanager' # '-bzr', '-git', '-hg' or '-svn'
pkgver=1.0
pkgrel=1
pkgdesc="Simple GUI wrapper for the pacman package manager"
arch=('x86_64')
url="https://github.com/alcalino-git/pacmanager"
license=('MIT-0')
depends=('gtk4' 'pacman' 'boost' 'glibc' 'glib2' 'meson')
makedepends=('git') # 'bzr', 'git', 'mercurial' or 'subversion'
install=
source=('pacmanager::git://github.com/alcalino-git/pacmanager.git')
sha256sums=('SKIP')

# Please refer to the 'USING VCS SOURCES' section of the PKGBUILD man page for
# a description of each element in the source array.

pkgver() {
	cd "$pkgname"
	printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}


build() {
	cd "$pkgname"
	meson compile -C build
}

package() {
	cd "$pkgname"
	install -Dm755 ./build/pacmanager "$pkgdir/usr/bin/pacmanager"
}
