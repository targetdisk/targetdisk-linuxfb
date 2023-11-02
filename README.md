# Targetdisk Graphics for Linux Framebuffer
![A bouncing trident.](epic_bounce.gif)

## Building
Generate your `img_data.h` file from the raw pixmap mentioned in `config.json`:
```
$ ./configure.py
```

> <h3>⚠  Note: </h3>
>
> If you aren't an MLG pro gamer that cloned this repo with
> `--recurse-submodules`, you'll need to run
> `git submodule update --init --recursive` in order to get
> the `rgba2header` script from the `td-img` Git submodule.

Compile your dancing trident:
```
$ make trident
```

Enjoy:
```
$ ./trident
```
