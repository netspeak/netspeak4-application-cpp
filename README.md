# Netspeak 4 application C++

[![Actions Status](https://github.com/netspeak/netspeak4-application-cpp/workflows/Build/badge.svg)](https://github.com/netspeak/netspeak4-application-cpp/actions)

This is the core library of Netspeak 4.

It contains methods to create indexes and make queries on Netspeak indexes. The Netspeak 4 Protobuf messages and the Antlr4 grammar of the Netspeak 4 query language are also part of this project. Whatever resource is shared between Netspeak's projects is probably in here.


## Getting started

### Docker

If you just want to run Netspeak without having to setup anything, then our Docker image will help you.

```bash
docker run webis/netspeak:4.1.2 netspeak4 --help
```

### Installing dependencies

Before you can compile and run anything, you have to install the dependencies. To do this, simply run:

```bash
sudo bash build/install-dependencies.sh
```

This will install, a C++ compiler, build tools, gRPC, Protobuf tools, and other general dependencies.

Some of the dependencies will by compiled from source and might be compiled/installed in parallel causing noticeable stuttering or even freezes for a few seconds. The heaviest dependencies will be installed globally, so they only have to be installed once.

#### CI

The install script will also install some dependencies that are used by other scripts (e.g. the ones that generate Protobuf and ANTLR4 files). If you do not use these scripts (e.g. in CI), then running `sudo bash build/install-dependencies.sh ci` instead will only install the dependencies that are necessary to compile the project.

#### For Windows users

Netspeak 4 is a Linux project. To compile and run it on your system, install WSL with any Linux distribution and run all commands there.

### Building

This project has a `debug` build and a `release` build. To make/update either one run:

```bash
# debug build
bash build/make-debug.sh
# release build
bash build/make-release.sh
```

The Netspeak executable can then be found at `build/{debug,release}/netspeak4`.

### Running tests

Run the following command:

```bash
bash build/run-tests.sh
```

This will use the `debug` build to run tests.

(It's recommended to do it like this because otherwise the test application might have problems locating its test resources.)

### Writing code

To write code, open the project in your IDE of choice (e.g. VSCode with the C++ plugin) and start typing!

Keep in mind that new files have to registered in `CMakeLists.txt`.

We use `clang-format` to format our code. If your IDE doesn't support this, you can use the `./build/format-all.sh` script to reformat all source files.


## Using the Netspeak4 CLI

After compiling, you can run the Netspeak executable which will start a small command line interface for Netspeak.

The Netspeak CLI aims to be self-documenting, so this section will be brief. To view the full CLI documentation run `netspeak4 --help`. This will list all commands and how to use them.

#### `build`

The `build` command can be used to build a new Netspeak4 index from a given data set. The basic usage works as follows:

```bash
./netspeak4 build -i "/data-set" -o "/my-index"
```

`/data-set` is the input directory that contains all phrases. Netspeak will assume all files in the input directory are simple text files containing phrase-frequency pairs.

Netspeak will parse all files in the input directory and create a index in `/my-index`, the output directory.

Depending on the number of phrase, the build process may take several hours. The whole process is usually bound by whatever storage medium is used to store the phrase and the index, so you can use your computer while the index is being created.

__NOTE:__ When building large indexes using `./netspeak4 build -i <input> -o <output>`, be sure that the limit on the maximum number of opened files by one process is high enough.
__If it's too low, the build process will fail.__
For small indexes a limit for 1024 is sufficient but for larger data sets (>10GB input), be sure it's at least 2048. You can set the limit using the `ulimit` command. <br>
WSL users: This limit will be reset with every restart of your Linux subsystem.


## Logging

The `serve` and `proxy` sub-command both support logging. This means that they will log every request they receive and every error produced.

### Tracking ID

Tracking is implemented via optional client metadata. Clients can provide a number to track their session to help improve the service.

The tracking number has to be provided as client metadata using the key `netspeak-tracking-id`. The value of that key has to be a 128bit hexadecimal number matching the following regex: `^[0-9A-Fa-f]{32}$`.

There are no requirements for the actual number but it is recommended for them to be randomly generated.


## Protobuf

This project also contain the Netspeak4 protobuf message definitions: `conf/NetspeakMessages.proto`.

After changing the definition, you have to run `build/generate-protobuf-files.sh` which, as the name suggests, will (re-)generate all protobuf C++ files. The generated files are located under `src/netspeak/generated/`.

You also have to update the generated protobuf files `netspeak4-application-java` by running `src/scripts/generate-protobuf-files.sh` (this requires Java >= 1.6 to be installed).

_Note:_ In case you OS doesn't support the specific protobuf version required, you the dev console from `netspeak4-deployment`.


## Antlr4

The Antlr4 grammar of the Netspeak4 query language can be found under `conf/grammar/`. Changes to these files require you to regenerate the Antlr4 files by running `build/generate-antlr4-files.sh` (this requires Java >= 1.6 to be installed).


## Netspeak query language

The Netspeak query syntax as described here should be used as reference. There
might be other syntax information out there, e.g. at netspeak.org, which
provides some syntactical simplifications in form of easier to use wildcards or
operators. However, these modified syntaxes are just front-ends and do not work
with the original Netspeak interface. Here is the truth:

```
?   is a placeholder for exactly one word and can be sequenced to search for
    exaclty two, three, four ... words.

    Example:    how to ? this
                -> how to use this
                -> how to do this
                -> how to cite this

*   is a placeholder for zero or many words.

    Example:    see * works
                -> see how it works
                -> see if it works
                -> see what works

[]  compares options, i.e. it checks each word or phrase between these
    brackets plus the so called empty word at that position in the query.

    Example:    it's [ great well "so good" ]
                -> it's
                -> it's great
                -> it's well
                -> it's so good

{}  checks the order, i.e. it tries to find each permutation of the given
    sequence of words or phrases at that position in the query.

    Example:    for { "very important people" only }
                -> for very important people only
                -> for only very important people

#   searches for alternatives of the word following. This operator requests
    the optional Netspeak hash-dictionary component and uses [] to compare
    each retrieved alternative (except that the empty word is not checked).
    The mapping from word to alternatives is completely up to the user when
    building Netspeak, for netspeak.org we use this operator for a synonym
    search providing the Wordnet dictionary.

    Example:    waiting for #response
                -> waiting for response
                -> waiting for answer
                -> waiting for reply
```

You can combine the introduced wildcards and operators as you want, but with the
exception that you may not place any wildcard within bracket operators. Also
nested brackets are not allowed. As you can see in the examples above you can
quote phrases to be handled as one entity is `[]` and `{}`.


## Index configuration files

Netspeak loads indexes using configuration files. These are small `.properties` files that contain information about the location and directory structure of the index, metadata (like name and language), and runtime parameters like the cache size.

The following section will explain all supported keys.

### Metadata

The following keys hold metadata about the corpus used to create the current index.

- `corpus.key = string` _(required)_

  A unique key to identify the corpus. Two corpora with the same key are always assumed to be equal.

- `corpus.name = string` _(required)_

  The human-readable name of the corpus.

- `corpus.language = string` _(required)_

  The [ISO 639-1 code](https://en.wikipedia.org/wiki/ISO_639-1) of the language of the corpus.

### Performance

The following keys are parameters used to fine-tune performance.

- `cache.capacity = size_t` _(optional)_

  This sets the capacity of Netspeak's main cache: the norm query cache. This LFU cache stores the outputs of the query processor. It has a static capacity that limits the maximum number of items.

  The default cache capacity of the current implementation is 1 million. At this capacity, an empty cache will use about 100MB and a full cache will use about 3GB of memory (depends on the cached queries). Other implementation may use different defaults.

- `search.regex.max-matches = uint32` _(optional)_

  The maximum number of regex matches. The current implementation replaces regex queries with a set of matching words (e.g. `route?` may be replaced with `[ router routed ]`). This parameter sets the maximum amount of words each regex query can be replaced with.

  Choosing a large maximum can cause queries containing regexes to become very slow. This is only a maximum and the implementation may choose to use fewer words to (hopefully) stay fast.

  You can set this to 0 to disable regex queries.

  The default is implementation defined. The current implementation has a default of 100.

- `search.regex.max-time = uint32` _(optional)_

  The maximum amount of time in milliseconds Netspeak is allowed to search for regex matches. Choosing a generous duration will lead to more accurate search results but may degrade performance.

  The default is implementation-defined but will generally around be a few milliseconds.

### Paths

The following keys are paths to locate the index.

- `path.to.phrase-index = path` _(optional)_ <br>
  `path.to.phrase-corpus = path` _(optional)_ <br>
  `path.to.phrase-dictionary = path` _(optional)_ <br>
  `path.to.postlist-index = path` _(optional)_ <br>
  `path.to.hash-dictionary = path` _(optional)_ <br>
  `path.to.regex-vocabulary = path` _(optional)_ <br>

  Paths to the individual components of a Netspeak index.

- `path.to.home = path` _(optional)_

  This will set all unset `path.to.xxxx` values to `<path.to.home>/xxxx`.

  If a Netspeak index follows the default index format (all indexes created by Netspeak do), then `path.to.home` can be used to set all index paths at once.

  Note: "unset" means not present in this configuration file. Keys from the `extends` file are not considered.

#### Note

All paths (if not absolute) are relative to the directory the configuration file lives in.

### Other

- `extends = path` _(optional)_

  The path of another configuration files.

  All keys that are not present in the current configuration file will be retrieved from this one instead. This can be used to implement inheritance.


## gRPC-web proxy

A [gRPC-web proxy](https://github.com/netspeak/grpcwebproxy) is necessary to use Netspeak in browsers. This is how you use the proxy.

### Linux

You need to have Docker installed.

Serve the index(es):

```cmd
docker run -p 9000:9000 -v /path/to/index:/index:ro webis/netspeak:4.1.2 netspeak4 serve -c /index/index.properties -p 9000
```

Run the proxy:

```cmd
docker run --network="host" webis/grpcwebproxy:0.14.0 grpcwebproxy --allow_all_origins --backend_addr=localhost:9000 --backend_tls=false --run_tls_server=false
```

### Windows

To run the gRPC-web proxy on Windows, you need to have WSL2 and Docker Desktop (with WSL2 backend) installed.

Serve the index(es):

```cmd
docker run -p 9000:9000 -v C:\path\to\index:/index:ro webis/netspeak:4.1.2 netspeak4 serve -c /index/index.properties -p 9000
```

Run the proxy:

```cmd
docker run -p 8080:8080 webis/grpcwebproxy:0.14.0 grpcwebproxy --allow_all_origins --backend_addr=host.docker.internal:9000 --backend_tls=false --run_tls_server=false
```

---

## Contributors

Michael Schmidt (2018 - 2020)

Martin Trenkmann (2008 - 2013)

Martin Potthast (2008 - 2020)

Benno Stein (2008 - 2020)
