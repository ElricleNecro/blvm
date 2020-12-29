# blvm

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
