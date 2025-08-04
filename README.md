[![bombkurdistan-bin](https://img.shields.io/aur/version/bombkurdistan-bin?color=1793d1&label=bombkurdistan-bin&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/bombkurdistan-bin/)
[![bombkurdistan-git](https://img.shields.io/aur/version/bombkurdistan-git?color=1793d1&label=bombkurdistan-git&logo=arch-linux&style=for-the-badge)](https://aur.archlinux.org/packages/bombkurdistan-git/)

# Kurdistan bombalayici

<img src="assets/icon.png" width="300" height="300" alt="bombkurdistan"/>

Undertale tarzinda kurdistani bombaladigimiz bir oyun, tamamiyla zevk ve eglence amacli yapilmistir. Lutfen ciddiye
almayiniz basimi belaya sokmayin!

Raylib kutuphanesini denemek istiyordum cok, raylibe baslamak isteyen varsa temel bir proje gibi dusunebilirsiniz.

Ve evet kodlarimin igrenc oldugunu biliyorum benim sucum degil...

## Download

Eger ki build almak istemiyorsaniz direkt [releases](https://github.com/XielQs/BombKurdistan/releases) sayfasindan
indirebilirsiniz.

Platformunuza gore (Windows/Linux) `bombkurdistan_linux.zip` veya `bombkurdistan_windows.zip` dosyasini indirin ve
zipten cikartin. Ardindan `./bombkurdistan` veya `./bombkurdistan.exe` dosyasini calistirabilirsiniz.

## Build

```bash
git clone https://github.com/XielQs/BombKurdistan --recurse-submodules
```

```bash
chmod +x ./build.sh
./build.sh
```

### Build for Windows

Windows'dan windowsa build alamiyorsunuz uzgunum o yuzden linux!!!

```bash
chmod +x ./build.sh
./build.sh -w
```

## Run

Build aldiktan sonra oyunun bulundugu dizine platformunuza gore `./bombkurdistan` veya `./bombkurdistan.exe` olusacak,
onu calistirabilirsiniz.

## Gameplay

https://github.com/user-attachments/assets/95879509-924b-4f56-b1af-2e562864e58d

https://github.com/user-attachments/assets/4d711217-3279-4683-b387-9cbfc0646ffb

### Controls

#### Keyboard

- `W` - yukari
- `A` - sola
- `S` - asagi
- `D` - saga
- `ESC` - menu/cikis
- `M` - sesi ac/kapat
- `R` - yeniden basla
- `C` - yapimcilar menusu
- `P` - oyunu durdur

#### Gamepad

- `dpad yukari` - yukari
- `dpad sol` - sola
- `dpad asagi` - asagi
- `dpad sag` - saga
- `ps daire` - menu/cikis
- `ps kare` - sesi ac/kapat
- `ps x` - yeniden basla
- `ps ucgen` - yapimcilar menusu
- `ps L1` - oyunu durdur

## Special Thanks

Bu oyunun yapilmasinda yardimci olan ve bana ilham veren kisilere tesekkur ederim:

- [larei](https://github.com/lareii)
- [kosero](https://github.com/kosero)
- yesil asya

## License

[MIT](LICENSE)
