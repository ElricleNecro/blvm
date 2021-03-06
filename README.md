# blvm [![pipeline status](https://gitlab.com/ElricleNecro/blvm/badges/master/pipeline.svg)](https://gitlab.com/ElricleNecro/blvm/-/commits/master)

## Licensing:
[`GPL-v3`](LICENSE) for programs

[`LGPL-v3`](lib.LICENSE) for `blvm` library.

## How to compile:

```console
$ premake gmake2
$ make
```

## How to use:

### Compile an example:

```console
$ ./build/Debug/bin/blasm examples/fibonacci.blasm /tmp/fibonacci.bl
```

### Run the compiled example:

```console
$ ./build/Debug/bin/blvi /tmp/fibonacci.bl
```
